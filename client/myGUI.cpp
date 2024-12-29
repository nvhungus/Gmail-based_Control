#include <wx/wx.h>
#include "client.h"
#include <thread>
#include <atomic>
#include <wx/timer.h>
using namespace std;

class MyApp : public wxApp {
public:
    virtual bool OnInit();
};

static std::string request_mail;

class MyFrame : public wxFrame {
public:
    MyFrame(const wxString& title);

private:
    wxPanel* mainPanel; // Main panel for dynamic switching
    wxPanel* mailPanel; // Panel for Mail process
    wxPanel* offPanel;  // Panel for Off process
    wxPanel* chatbotPanel;

    wxTextCtrl* mailRequestCtrl;   // Biến thành viên để hiển thị yêu cầu mail
    wxTextCtrl* processingResultCtrl;

    wxTextCtrl* ChatbotrequestCtrl;   // Biến thành viên để hiển thị yêu cầu mail
    wxTextCtrl* ChatbotresponseCtrl;

    wxTextCtrl* emailTextCtrl;
    wxTextCtrl* displayTextCtrl;
    Client client;

    void InitStartPanel();
    void InitMailPanel();
    void InitOffPanel();
    void InitChatbotPanel();

    void OnMailProcess(wxCommandEvent& event);
    void OnOffProcess(wxCommandEvent& event);
    void OnChatbot(wxCommandEvent& event);
    void OnConnect(wxCommandEvent& event);
    void OnDisconnect(wxCommandEvent& event);
    void OnConnect2(wxCommandEvent& event);
    void OnDisconnect2(wxCommandEvent& event);
    void OnListApp(wxCommandEvent& event);
    void OnListServices(wxCommandEvent& event);
    void OnWebcam(wxCommandEvent& event);
    void ChangeMode(wxCommandEvent& event);
    void Screencapture(wxCommandEvent& event);
    void OnShutdown(wxCommandEvent& event);
    void OnStartApp(wxCommandEvent& event);
    void OnStopApp(wxCommandEvent& event);
    void OnStopService(wxCommandEvent& event);
    void OnKeylogger(wxCommandEvent& event);
    void OnTreeFolder(wxCommandEvent& event);
    void sentToChatbot(wxCommandEvent& event);
    void OnBack(wxCommandEvent& event);
    void OnStartService(wxCommandEvent& event);

    wxDECLARE_EVENT_TABLE();
};

// Enum for button IDs
enum {
    ID_MAIL_PROCESS = 1,
    ID_OFF_PROCESS,
    ID_CHATBOT_PROCESS,
    ID_CONNECT,
    ID_DISCONNECT,
    ID_CONNECT2,
    ID_DISCONNECT2,
    ID_LIST_APP,
    ID_LIST_SERVICES,
    ID_START_SERVICES,
    ID_SCREENSHOT,
    ID_WEBCAM,
    ID_SHUTDOWN,
    ID_START_APP,
    ID_STOP_APP,
    ID_STOP_SERVICE,
    ID_CHANGE_MODE,
    ID_KEYLOGGER,
    ID_TREE_FOLDER,
    ID_SENT_CHATBOT,
    ID_BACK
};

wxBEGIN_EVENT_TABLE(MyFrame, wxFrame)
EVT_BUTTON(ID_MAIL_PROCESS, MyFrame::OnMailProcess)
EVT_BUTTON(ID_OFF_PROCESS, MyFrame::OnOffProcess)
EVT_BUTTON(ID_CHATBOT_PROCESS, MyFrame::OnChatbot)
EVT_BUTTON(ID_CONNECT, MyFrame::OnConnect)
EVT_BUTTON(ID_CONNECT2, MyFrame::OnConnect2)
EVT_BUTTON(ID_DISCONNECT, MyFrame::OnDisconnect)
EVT_BUTTON(ID_DISCONNECT2, MyFrame::OnDisconnect2)
EVT_BUTTON(ID_LIST_APP, MyFrame::OnListApp)
EVT_BUTTON(ID_LIST_SERVICES, MyFrame::OnListServices)
EVT_BUTTON(ID_START_SERVICES, MyFrame::OnStartService)
EVT_BUTTON(ID_SCREENSHOT, MyFrame::Screencapture)
EVT_BUTTON(ID_WEBCAM, MyFrame::OnWebcam)
EVT_BUTTON(ID_SHUTDOWN, MyFrame::OnShutdown)
EVT_BUTTON(ID_START_APP, MyFrame::OnStartApp)
EVT_BUTTON(ID_STOP_APP, MyFrame::OnStopApp)
EVT_BUTTON(ID_STOP_SERVICE, MyFrame::OnStopService)
EVT_BUTTON(ID_KEYLOGGER, MyFrame::OnKeylogger)
EVT_BUTTON(ID_CHANGE_MODE, MyFrame::ChangeMode)
EVT_BUTTON(ID_TREE_FOLDER, MyFrame::OnTreeFolder)
EVT_BUTTON(ID_SENT_CHATBOT, MyFrame::sentToChatbot)
EVT_BUTTON(ID_BACK, MyFrame::OnBack)
wxEND_EVENT_TABLE()

wxIMPLEMENT_APP(MyApp);

bool MyApp::OnInit() {
    MyFrame* frame = new MyFrame("Client-Server Application");
    frame->Show(true);
    return true;
}

MyFrame::MyFrame(const wxString& title)
    : wxFrame(NULL, wxID_ANY, title, wxDefaultPosition, wxSize(800, 600)) {

    mainPanel = new wxPanel(this, wxID_ANY);

    // Initialize Start Panel
    InitStartPanel();
}

void MyFrame::InitStartPanel() {
    wxBoxSizer* startSizer = new wxBoxSizer(wxVERTICAL);
    wxSize buttonSize(120, 35);
    // Các nút Mail process và Off process
    wxButton* mailProcessButton = new wxButton(mainPanel, ID_MAIL_PROCESS, "Mail remote", wxDefaultPosition , buttonSize);
    wxButton* offProcessButton = new wxButton(mainPanel, ID_OFF_PROCESS, "Custom mode", wxDefaultPosition, buttonSize);
    wxButton* chatbotButton = new wxButton(mainPanel, ID_CHATBOT_PROCESS, "CHAT WITH AI", wxDefaultPosition, buttonSize);
    // Thêm các nút vào sizer với căn giữa ngang và dọc
    startSizer->Add(mailProcessButton, 0, wxALL | wxALIGN_CENTER_HORIZONTAL | wxALIGN_CENTER_VERTICAL, 10);
    startSizer->Add(offProcessButton, 0, wxALL | wxALIGN_CENTER_HORIZONTAL | wxALIGN_CENTER_VERTICAL, 10);
    startSizer->Add(chatbotButton, 0, wxALL | wxALIGN_CENTER_HORIZONTAL | wxALIGN_CENTER_VERTICAL, 10);
    wxInitAllImageHandlers();
    wxBitmap bitmap(wxT("ava_1.png"), wxBITMAP_TYPE_PNG); // Đảm bảo tệp tồn tại
    if (bitmap.IsOk()) {
        wxStaticBitmap* image = new wxStaticBitmap(mainPanel, wxID_ANY, bitmap);
        startSizer->Add(image, 0, wxALL | wxALIGN_CENTER, 10);
    }
    else {
        wxStaticText* errorText = new wxStaticText(mainPanel, wxID_ANY, "Image not found!", wxDefaultPosition, wxDefaultSize, wxALIGN_CENTER);
        startSizer->Add(errorText, 0, wxALL | wxALIGN_CENTER, 10);
    }
    mainPanel->SetSizerAndFit(startSizer);
}

void MyFrame::InitMailPanel() {
    if (mailPanel) return; // Avoid reinitialization

    // Tạo panel và sizer chính
    mailPanel = new wxPanel(this, wxID_ANY);
    wxBoxSizer* mailSizer = new wxBoxSizer(wxVERTICAL);

    // Add Back button at the top
    wxButton* backButton = new wxButton(mailPanel, ID_BACK, "Back");
    mailSizer->Add(backButton, 0, wxALL | wxALIGN_LEFT, 10);

    // Nút "Change mode"
    wxButton* changeModeButton = new wxButton(mailPanel, ID_CHANGE_MODE, "Choose mail remote");
    mailSizer->Add(changeModeButton, 0, wxALL | wxALIGN_CENTER_HORIZONTAL, 10);

    // Dòng trên cùng chứa IP Label, TextCtrl, và các nút kết nối
    wxBoxSizer* topSizer = new wxBoxSizer(wxHORIZONTAL);
    wxStaticText* ipLabel = new wxStaticText(mailPanel, wxID_ANY, "IP Address (Gmail):");
    emailTextCtrl = new wxTextCtrl(mailPanel, wxID_ANY, "", wxDefaultPosition, wxSize(300, -1));
    wxButton* connectButton2 = new wxButton(mailPanel, ID_CONNECT2, "Connect");
    wxButton* disconnectButton2 = new wxButton(mailPanel, ID_DISCONNECT2, "Disconnect");

    // Bố trí các thành phần của dòng trên cùng
    topSizer->Add(ipLabel, 0, wxALL | wxALIGN_CENTER_VERTICAL, 5);
    topSizer->Add(emailTextCtrl, 1, wxALL | wxEXPAND, 5); // TextCtrl mở rộng
    topSizer->Add(connectButton2, 0, wxALL | wxALIGN_CENTER_VERTICAL, 5);
    topSizer->Add(disconnectButton2, 0, wxALL | wxALIGN_CENTER_VERTICAL, 5);

    // Thêm topSizer vào sizer chính
    mailSizer->Add(topSizer, 0, wxEXPAND | wxALL, 5);

    // Label và TextCtrl cho Mail Requests
    wxStaticText* requestLabel = new wxStaticText(mailPanel, wxID_ANY, "Mail Requests:");
    mailRequestCtrl = new wxTextCtrl(mailPanel, wxID_ANY, "", wxDefaultPosition, wxSize(-1, 150), wxTE_MULTILINE | wxTE_READONLY);

    mailSizer->Add(requestLabel, 0, wxALL, 5);
    mailSizer->Add(mailRequestCtrl, 1, wxEXPAND | wxALL, 5); // Chiều cao cố định và mở rộng ngang

    // Label và TextCtrl cho Processing Results
    wxStaticText* resultLabel = new wxStaticText(mailPanel, wxID_ANY, "Processing Results:");
    processingResultCtrl = new wxTextCtrl(mailPanel, wxID_ANY, "", wxDefaultPosition, wxSize(-1, 150), wxTE_MULTILINE | wxTE_READONLY);

    mailSizer->Add(resultLabel, 0, wxALL, 5);
    mailSizer->Add(processingResultCtrl, 1, wxEXPAND | wxALL, 5); // Chiều cao cố định và mở rộng ngang

    // Gán sizer chính cho panel và sắp xếp lại giao diện
    mailPanel->SetSizer(mailSizer);
    mailPanel->Fit();    // Điều chỉnh kích thước panel
    mailPanel->Layout(); // Sắp xếp lại các thành phần
}
void MyFrame::InitChatbotPanel() {
    if (chatbotPanel) return; // Tránh tái khởi tạo

    // Tạo panel và sizer chính
    chatbotPanel = new wxPanel(this, wxID_ANY);
    wxBoxSizer* chatbotSizer = new wxBoxSizer(wxVERTICAL);

    // Add Back button at the top
    wxButton* backButton = new wxButton(chatbotPanel, ID_BACK, "Back");
    chatbotSizer->Add(backButton, 0, wxALL | wxALIGN_LEFT, 10);

    wxButton* sentToChatbot = new wxButton(chatbotPanel, ID_SENT_CHATBOT, "Send");
    chatbotSizer->Add(sentToChatbot, 0, wxALL | wxALIGN_CENTER_HORIZONTAL, 10);

    // Label và TextCtrl cho Mail Requests
    wxStaticText* requestLabel = new wxStaticText(chatbotPanel, wxID_ANY, "Request:");
    ChatbotrequestCtrl = new wxTextCtrl(chatbotPanel, wxID_ANY, "", wxDefaultPosition, wxSize(-1, 150), wxTE_MULTILINE);

    // Label và TextCtrl cho Processing Results
    wxStaticText* response = new wxStaticText(chatbotPanel, wxID_ANY, "Response:");
    ChatbotresponseCtrl = new wxTextCtrl(chatbotPanel, wxID_ANY, "", wxDefaultPosition, wxSize(-1, 150), wxTE_MULTILINE | wxTE_READONLY);

    // Thêm các điều khiển vào sizer
    chatbotSizer->Add(requestLabel, 0, wxALL, 5);
    chatbotSizer->Add(ChatbotrequestCtrl, 1, wxEXPAND | wxALL, 5);
    chatbotSizer->Add(response, 0, wxALL, 5);
    chatbotSizer->Add(ChatbotresponseCtrl, 1, wxEXPAND | wxALL, 5);

    // Gán sizer chính cho panel và sắp xếp lại giao diện
    chatbotPanel->SetSizer(chatbotSizer);
    chatbotPanel->Fit();
    chatbotPanel->Layout();
}

void MyFrame::InitOffPanel() {
    if (offPanel) return; // Avoid reinitialization

    offPanel = new wxPanel(this, wxID_ANY);
    wxBoxSizer* offSizer = new wxBoxSizer(wxVERTICAL);

    // Add Back button at the top
    wxButton* backButton = new wxButton(offPanel, ID_BACK, "Back");
    offSizer->Add(backButton, 0, wxALL | wxALIGN_LEFT, 10);

    wxBoxSizer* topSizer = new wxBoxSizer(wxHORIZONTAL);
    wxStaticText* ipLabel = new wxStaticText(offPanel, wxID_ANY, "IP Address (Gmail):");
    emailTextCtrl = new wxTextCtrl(offPanel, wxID_ANY, "", wxDefaultPosition, wxSize(300, -1));
    wxButton* connectButton = new wxButton(offPanel, ID_CONNECT, "Connect");
    wxButton* disconnectButton = new wxButton(offPanel, ID_DISCONNECT, "Disconnect");

    topSizer->Add(ipLabel, 0, wxALL | wxALIGN_CENTER_VERTICAL, 5);
    topSizer->Add(emailTextCtrl, 1, wxALL | wxEXPAND, 5);
    topSizer->Add(connectButton, 0, wxALL, 5);
    topSizer->Add(disconnectButton, 0, wxALL, 5);

    wxGridSizer* buttonSizer = new wxGridSizer(3, 4, 5, 5);
    buttonSizer->Add(new wxButton(offPanel, ID_LIST_APP, "List App"), 0, wxEXPAND);
    buttonSizer->Add(new wxButton(offPanel, ID_START_APP, "Start App"), 0, wxEXPAND);
    buttonSizer->Add(new wxButton(offPanel, ID_STOP_APP, "Stop App"), 0, wxEXPAND);
    buttonSizer->Add(new wxButton(offPanel, ID_LIST_SERVICES, "List Services"), 0, wxEXPAND);
    buttonSizer->Add(new wxButton(offPanel, ID_START_SERVICES, "Start Service"), 0, wxEXPAND);
    buttonSizer->Add(new wxButton(offPanel, ID_STOP_SERVICE, "Stop Service"), 0, wxEXPAND);
    buttonSizer->Add(new wxButton(offPanel, ID_SCREENSHOT, "Capture image"), 0, wxEXPAND);
    buttonSizer->Add(new wxButton(offPanel, ID_WEBCAM, "Webcam"), 0, wxEXPAND);
    buttonSizer->Add(new wxButton(offPanel, ID_SHUTDOWN, "Shutdown"), 0, wxEXPAND);
    buttonSizer->Add(new wxButton(offPanel, ID_KEYLOGGER, "Keylogger"), 0, wxEXPAND);
    buttonSizer->Add(new wxButton(offPanel, ID_TREE_FOLDER, "Tree Folder"), 0, wxEXPAND);

    displayTextCtrl = new wxTextCtrl(offPanel, wxID_ANY, "", wxDefaultPosition, wxDefaultSize,
        wxTE_MULTILINE | wxTE_READONLY | wxTE_RICH | wxHSCROLL);

    offSizer->Add(topSizer, 0, wxEXPAND | wxALL, 5);
    offSizer->Add(buttonSizer, 0, wxEXPAND | wxALL, 5);
    offSizer->Add(displayTextCtrl, 1, wxEXPAND | wxALL, 5);

    offPanel->SetSizer(offSizer);
}

void MyFrame::OnBack(wxCommandEvent& event) {
    // Delete current panel
    if (mailPanel) {
        mailPanel->Destroy();
        mailPanel = nullptr;
    }
    if (offPanel) {
        offPanel->Destroy();
        offPanel = nullptr;
    }
    if (chatbotPanel) {
        chatbotPanel->Destroy();
        chatbotPanel = nullptr;
    }

    // Create new main panel
    mainPanel = new wxPanel(this, wxID_ANY);
    InitStartPanel();
    SetSize(wxSize(800, 600));
    mainPanel->SetSize(wxSize(800, 600));
    SetClientSize(mainPanel->GetSize());
    mainPanel->Show();
}

void MyFrame::OnMailProcess(wxCommandEvent& event) {
    mainPanel->Destroy();
    InitMailPanel();
    SetSize(wxSize(800, 600));
    mailPanel->SetSize(wxSize(800, 600));
    SetClientSize(mailPanel->GetSize());
    mailPanel->Show();
    
}

void MyFrame::OnChatbot(wxCommandEvent& event) {
    mainPanel->Destroy();
    InitChatbotPanel();
    SetSize(wxSize(800, 600));
    chatbotPanel->SetSize(wxSize(800, 600));
    SetClientSize(chatbotPanel->GetSize());
    chatbotPanel->Show();
}   
void MyFrame::OnOffProcess(wxCommandEvent& event) {
    mainPanel->Destroy();
    InitOffPanel();
    SetSize(wxSize(800, 600)); // Đặt kích thước cố định cho cửa sổ chính
    offPanel->SetSize(wxSize(800, 600)); // Đặt kích thước cố định cho panel Off process
    SetClientSize(offPanel->GetSize());
    offPanel->Show();
}

void MyFrame::OnConnect(wxCommandEvent& event) {
    std::string serverIP = emailTextCtrl->GetValue().ToStdString();
    client.setServerIP(serverIP);
    if (client.connectToServer()) {
        displayTextCtrl->AppendText("Connected to server\n");
    }
    else {
        displayTextCtrl->AppendText("Failed to connect to server\n");
    }
}
void MyFrame::OnConnect2(wxCommandEvent& event) {
    std::string serverIP = emailTextCtrl->GetValue().ToStdString();
    client.setServerIP(serverIP);
    if (client.connectToServer()) {
        processingResultCtrl->AppendText("Connected to server\n");
    }
    else {
        processingResultCtrl->AppendText("Failed to connect to server\n");
    }
}
void MyFrame::OnListApp(wxCommandEvent& event) {
    displayTextCtrl->Clear();
    std::string response = client.sendMessage("List apps");
    displayTextCtrl->AppendText("List App:\n" + response + "\n");
}

void MyFrame::OnListServices(wxCommandEvent& event) {
    displayTextCtrl->Clear();
    std::string response = client.sendMessage("List services");
    displayTextCtrl->AppendText("List Services:\n" + response + "\n");
}

void MyFrame::OnWebcam(wxCommandEvent& event) {
    displayTextCtrl->Clear();
    std::string response = client.sendMessage("Webcam");
    displayTextCtrl->AppendText("Screenshot webcam:\n" + response + "\n");
}

void MyFrame::OnShutdown(wxCommandEvent& event) {
    displayTextCtrl->Clear();
    std::string response = client.sendMessage("Shutdown");
    displayTextCtrl->AppendText("Shutdown:\n" + response + "\n");
}
std::string extractEmail(const std::string& senderInfo) {
    size_t start = senderInfo.find('<');
    size_t end = senderInfo.find('>');

    // Nếu định dạng có dạng "Tên Người Gửi <email@example.com>"
    if (start != std::string::npos && end != std::string::npos) {
        return senderInfo.substr(start + 1, end - start - 1); // Lấy chuỗi giữa < và >
    }

    // Nếu chỉ chứa địa chỉ email
    return senderInfo;
}
void MyFrame::ChangeMode(wxCommandEvent& event) {
    wxArrayString appChoices;
    vector<string> listMailConfirm = takeMailConfirm("Mail_confirm.txt");
    for (auto v : listMailConfirm) {
        appChoices.Add(v);
    }
    appChoices.Add("Add Gmail");
    wxSingleChoiceDialog choiceDialog(this,
        "Select mail for sending request",
        "Mail authentication",
        appChoices);
    choiceDialog.SetSize(400, 300);
    std::string mail_comfirm = "";
    bool mail_start = false;
    if (choiceDialog.ShowModal() == wxID_OK) {
        wxString choice = choiceDialog.GetStringSelection();
        if (choice == "Add Gmail") { 
            // Yêu cầu người dùng nhập đường dẫn ứng dụng
            wxTextEntryDialog textDialog(this,
                "Paste the full path to the application you want to start:",
                "Custom Application");

            if (textDialog.ShowModal() == wxID_OK) {
                mail_comfirm = textDialog.GetValue().ToStdString();
                mail_start = true;
                std::ofstream file("Mail_confirm.txt", std::ios::app); // Mở file ở chế độ append
                if (file.is_open()) {
                    file << mail_comfirm << std::endl;
                    file.close();
                }
            }
        }
        else {
            mail_comfirm = choice.ToStdString();
            mail_start = true;
        }
    }
    if (mail_start == true) {
        mailRequestCtrl->AppendText(mail_comfirm + '\n');
        std::string redirectUri = "http://localhost";
        std::string tempFile = "oauth_code.txt";
        std::string url = "https://accounts.google.com/o/oauth2/v2/auth?"
            "scope=https://mail.google.com/&"
            "access_type=offline&"
            "include_granted_scopes=true&"
            "response_type=code&"
            "redirect_uri=" + redirectUri + "&"
            "client_id=" + clientId;
        std::string refreshtoken;
        openURL(url);
        startLocalServer(tempFile);
        std::string code = listenForOAuthCode(tempFile);
        std::string accessToken = getAccessToken(code, clientId, clientSecret, refreshtoken);
        while (true) {
            mailRequestCtrl->AppendText("Waiting for message from Mail....\n");
            int mailcount, mailcheck;
            accessToken = refreshAccessToken(refreshtoken, clientId, clientSecret);
            std::string tmp1 = readGmail(accessToken, mailcheck, request_mail);
            mailcount = mailcheck;
            std::string message;
            while (mailcount == mailcheck) {
                accessToken = refreshAccessToken(refreshtoken, clientId, clientSecret);
                message = readGmail(accessToken, mailcheck, request_mail);
                std::string cleanRequestMail = extractEmail(request_mail);
                if (mail_comfirm != cleanRequestMail) {
                    mailcount = mailcheck;
                }
                wxYield();
            }

            processingResultCtrl->AppendText("Received message: " + message + " From " + request_mail + '\n');
            if (message == "List apps") {
                std::string response = client.sendMessage("List apps+" + request_mail + '+' + refreshtoken);
                mailRequestCtrl->AppendText(response + '\n');
            }
            else if (message == "List services") {
                std::string response = client.sendMessage("List services+" + request_mail + '+' + refreshtoken);
                mailRequestCtrl->AppendText(response + '\n');
            }
            else if (message == "Webcam") {
                std::string response = client.sendMessage("Webcam+" + request_mail + '+' + refreshtoken);
                mailRequestCtrl->AppendText(response + '\n');
            }
            else if (message == "Shutdown") {
                std::string response = client.sendMessage("Shutdown");
                mailRequestCtrl->AppendText(response + '\n');
            }
            else if (message == "Screen shot") {
                std::string response = client.sendMessage("Screen shot+" + request_mail + '+' + refreshtoken);
                mailRequestCtrl->AppendText(response + '\n');
            }
            else if (message.substr(0, 10) == "Take file ") {
                std::string filePath = message.substr(10);
                std::string response = client.sendMessage("Take file+" + filePath + "+" + request_mail + '+' + refreshtoken);
                mailRequestCtrl->AppendText(response + '\n');
            }
            else if (message.substr(0, 12) == "Delete file ") {
                std::string filePath = message.substr(12);
                std::string response = client.sendMessage("Delete file+" + filePath + "+" + request_mail + '+' + refreshtoken);
                mailRequestCtrl->AppendText(response + '\n');
            }
            else if (message == "Folder tree") {
                std::string response = client.sendMessage("Tree folder+"  + request_mail + '+' + refreshtoken);
                mailRequestCtrl->AppendText(response + '\n');
            }
            else break;
        }
    }

}

void MyFrame::Screencapture(wxCommandEvent& event) {
    displayTextCtrl->Clear();
    std::string response = client.sendMessage("Screen shot");
    displayTextCtrl->AppendText("Screenshot:\n" + response + "\n");
}

void MyFrame::OnStartApp(wxCommandEvent& event) {
    displayTextCtrl->Clear();
    wxArrayString appChoices;
    appChoices.Add("Notepad");
    appChoices.Add("Calculator");
    appChoices.Add("Google Chrome");
    appChoices.Add("Custom Application");
    appChoices.Add("Viber");
    appChoices.Add("Word");
    appChoices.Add("Unikey");

    wxSingleChoiceDialog choiceDialog(this,
        "Select an application to start",
        "Start Application",
        appChoices);

    if (choiceDialog.ShowModal() == wxID_OK) {
        wxString choice = choiceDialog.GetStringSelection();
        std::wstring appPath;

        if (choice == "Notepad") {
            appPath = L"notepad.exe";
        }
        else if (choice == "Calculator") {
            appPath = L"calc.exe";
        }
        else if (choice == "Google Chrome") {
            appPath = L"C:\\Users\\Admin\\AppData\\Local\\Google\\Chrome\\Application\\chrome.exe";
        }
        else if (choice == "Viber") {
            appPath = L"C:\\Users\\Admin\\AppData\\Local\\Viber\\Viber.exe";
        }
        else if (choice == "Word") {
            appPath = L"C:\\Program Files\\Microsoft Office\\root\\Office16\\WINWORD.EXE";
        }
        else if (choice == "Unikey") {
            appPath = L"C:\\Program Files\\UniKey\\UniKeyNT.exe";
        }
        else if (choice == "Custom Application") {
            wxTextEntryDialog textDialog(this,
                "Paste the full path to the application you want to start:",
                "Custom Application");

            if (textDialog.ShowModal() == wxID_OK) {
                appPath = textDialog.GetValue().ToStdWstring();
            }
        }
        std::string message = "on_app " + std::string(appPath.begin(), appPath.end());
        std::string response = client.sendMessage(message);
        displayTextCtrl->AppendText("Start App:\n" + response + "\n");
    }
}

void MyFrame::OnStopApp(wxCommandEvent& event) {
    displayTextCtrl->Clear();

    // Gửi lệnh tới server để lấy danh sách các ứng dụng đang chạy
    std::string response = client.sendMessage("List apps");

    if (response.empty()) {
        displayTextCtrl->AppendText("Failed to retrieve running applications.\n");
        return;
    }

    // Tách danh sách ứng dụng từ phản hồi của server
    wxArrayString appChoices;
    std::istringstream responseStream(response);
    std::string line;
    while (std::getline(responseStream, line)) {
        if (!line.empty()) {
            appChoices.Add(wxString::FromUTF8(line));
        }
    }

    // Kiểm tra nếu không có ứng dụng nào đang chạy
    if (appChoices.IsEmpty()) {
        displayTextCtrl->AppendText("No running applications found to stop.\n");
        return;
    }

    // Hiển thị danh sách cho người dùng chọn
    wxSingleChoiceDialog choiceDialog(this,
        "Select an application to stop",
        "Stop Application",
        appChoices);

    if (choiceDialog.ShowModal() == wxID_OK) {
        wxString choice = choiceDialog.GetStringSelection();

        // Tách PID từ chuỗi lựa chọn (giả định rằng PID ở cuối dòng trong ngoặc)
        std::wstring selectedApp = choice.ToStdWstring();
        size_t pidStart = selectedApp.find(L"(PID: ");
        size_t pidEnd = selectedApp.find(L")", pidStart);

        if (pidStart != std::wstring::npos && pidEnd != std::wstring::npos) {
            std::wstring pidStr = selectedApp.substr(pidStart + 6, pidEnd - pidStart - 6);
            DWORD pid = std::stoul(pidStr);

            // Gửi PID đến server để dừng ứng dụng
            std::string message = "off_app " + std::to_string(pid);
            std::string response = client.sendMessage(message);
            displayTextCtrl->AppendText("Stop App:\n" + response + "\n");
        }
        else {
            displayTextCtrl->AppendText("Invalid application selection.\n");
        }
    }
    else {
        displayTextCtrl->AppendText("No application selected to stop.\n");
    }
}

void MyFrame::OnStartService(wxCommandEvent& event) {
    displayTextCtrl->Clear();

    // Gửi lệnh tới server để lấy danh sách các dịch vụ đang chạy
    std::string response = client.sendMessage("List off services");

    if (response.empty()) {
        displayTextCtrl->AppendText("Failed to retrieve off services.\n");
        return;
    }

    // Tách danh sách dịch vụ từ phản hồi của server
    wxArrayString serviceChoices;
    std::istringstream responseStream(response);
    std::string line;
    while (std::getline(responseStream, line)) {
        if (!line.empty()) {
            serviceChoices.Add(wxString::FromUTF8(line));
        }
    }

    // Kiểm tra nếu không có dịch vụ nào đang chạy
    if (serviceChoices.IsEmpty()) {
        displayTextCtrl->AppendText("No off services found to start.\n");
        return;
    }

    // Hiển thị danh sách cho người dùng chọn
    wxSingleChoiceDialog choiceDialog(this,
        "Select a service to stop",
        "Stop Service",
        serviceChoices);

    if (choiceDialog.ShowModal() == wxID_OK) {
        wxString choice = choiceDialog.GetStringSelection();

        std::string selectedService = std::string(choice.ToUTF8());

        // Tìm vị trí của dấu "." và dấu "-"
        size_t dotPosition = selectedService.find('.');
        size_t dashPosition = selectedService.find('-');

        // Kiểm tra nếu cả dấu "." và "-" đều tồn tại
        if (dotPosition != std::string::npos && dashPosition != std::string::npos && dotPosition < dashPosition) {
            // Trích xuất phần giữa dấu "." và dấu "-"
            selectedService = selectedService.substr(dotPosition + 1, dashPosition - dotPosition - 1);

            // Loại bỏ khoảng trắng thừa (nếu có) ở đầu và cuối chuỗi
            selectedService.erase(0, selectedService.find_first_not_of(" "));
            selectedService.erase(selectedService.find_last_not_of(" ") + 1);
        }

        // Gửi thông báo dừng dịch vụ tới server
        std::string message = "start_service " + selectedService;
        std::string response = client.sendMessage(message);

        // Hiển thị phản hồi từ server
        if (!response.empty()) {
            displayTextCtrl->AppendText("Start Service:\n" + wxString::FromUTF8(response) + "\n");
        }
        else {
            displayTextCtrl->AppendText("Failed to start the selected service.\n");
        }
    }
    else {
        displayTextCtrl->AppendText("No service selected to start.\n");
    }
}

void MyFrame::OnStopService(wxCommandEvent& event) {
    displayTextCtrl->Clear();

    // Gửi lệnh tới server để lấy danh sách các dịch vụ đang chạy
    std::string response = client.sendMessage("List services");

    if (response.empty()) {
        displayTextCtrl->AppendText("Failed to retrieve running services.\n");
        return;
    }

    // Tách danh sách dịch vụ từ phản hồi của server
    wxArrayString serviceChoices;
    std::istringstream responseStream(response);
    std::string line;
    while (std::getline(responseStream, line)) {
        if (!line.empty()) {
            serviceChoices.Add(wxString::FromUTF8(line));
        }
    }

    // Kiểm tra nếu không có dịch vụ nào đang chạy
    if (serviceChoices.IsEmpty()) {
        displayTextCtrl->AppendText("No running services found to stop.\n");
        return;
    }

    // Hiển thị danh sách cho người dùng chọn
    wxSingleChoiceDialog choiceDialog(this,
        "Select a service to stop",
        "Stop Service",
        serviceChoices);

    if (choiceDialog.ShowModal() == wxID_OK) {
        wxString choice = choiceDialog.GetStringSelection();

        std::string selectedService = std::string(choice.ToUTF8());

        // Tìm vị trí của dấu "." và dấu "-"
        size_t dotPosition = selectedService.find('.');
        size_t dashPosition = selectedService.find('-');

        // Kiểm tra nếu cả dấu "." và "-" đều tồn tại
        if (dotPosition != std::string::npos && dashPosition != std::string::npos && dotPosition < dashPosition) {
            // Trích xuất phần giữa dấu "." và dấu "-"
            selectedService = selectedService.substr(dotPosition + 1, dashPosition - dotPosition - 1);

            // Loại bỏ khoảng trắng thừa (nếu có) ở đầu và cuối chuỗi
            selectedService.erase(0, selectedService.find_first_not_of(" "));
            selectedService.erase(selectedService.find_last_not_of(" ") + 1);
        }

        // Gửi thông báo dừng dịch vụ tới server
        std::string message = "stop_service " + selectedService;
        std::string response = client.sendMessage(message);

        // Hiển thị phản hồi từ server
        if (!response.empty()) {
            displayTextCtrl->AppendText("Stop Service:\n" + wxString::FromUTF8(response) + "\n");
        }
        else {
            displayTextCtrl->AppendText("Failed to stop the selected service.\n");
        }
    }
    else {
        displayTextCtrl->AppendText("No service selected to stop.\n");
    }
}

void MyFrame::OnKeylogger(wxCommandEvent& event) {
    displayTextCtrl->Clear();
    std::string response = client.sendMessage("keylogger");
    displayTextCtrl->AppendText("Keylogger Output:\n" + response + "\n");
}

void MyFrame::OnTreeFolder(wxCommandEvent& event) {
    displayTextCtrl->Clear();
    std::string folderTree = client.sendMessage("Tree folder");
    displayTextCtrl->AppendText("Tree Folder:\n" + folderTree + "\n");
}

void MyFrame::OnDisconnect(wxCommandEvent& event) {
    std::string response = client.sendMessage("quit");
    displayTextCtrl->AppendText("Disconnect to server\n");
}
void MyFrame::OnDisconnect2(wxCommandEvent& event) {
    std::string response = client.sendMessage("quit");
    processingResultCtrl->AppendText("Disconnect to server\n");
}
void MyFrame::sentToChatbot(wxCommandEvent& event) {
    wxString userRequest = "You said: " + ChatbotrequestCtrl->GetValue();
    GeminiAPIClient Gemini("AIzaSyCoh9dwUiTRfITMTcZBhfTmZaK6mt93cbU");
    string rawResponse = Gemini.sendRequest(userRequest.ToStdString());
    string processedResponse = Gemini.parseResponse(rawResponse);
    ChatbotresponseCtrl->AppendText(userRequest + '\n');
    ChatbotresponseCtrl->AppendText("AI said: " + processedResponse + '\n');
    ChatbotrequestCtrl->Clear();
}