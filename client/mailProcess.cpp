﻿#include<curl/curl.h>
#include <curl/curl.h>
#include "json.hpp"
#include <iostream>
#include <string>
#include <sstream>
#include <windows.h>
#include<fstream>
#include"client.h"
size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* s) {
    size_t totalSize = size * nmemb;
    s->append((char*)contents, totalSize);
    return totalSize;
}

std::string getAccessToken(const std::string& code, const std::string& clientId, const std::string& clientSecret, std::string& refreshtoken) {
    CURL* curl;
    CURLcode res;
    std::string responseString;

    curl = curl_easy_init();
    if (curl) {
        std::string postData = "code=" + code +
            "&client_id=" + clientId +
            "&client_secret=" + clientSecret +
            "&redirect_uri=http://localhost" +
            "&grant_type=authorization_code";

        curl_easy_setopt(curl, CURLOPT_URL, "https://oauth2.googleapis.com/token");
        curl_easy_setopt(curl, CURLOPT_POST, 1L);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, postData.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &responseString);
        curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
        res = curl_easy_perform(curl);

        if (res != CURLE_OK) {
            std::cerr << "curl_easy_perform() failed: " << curl_easy_strerror(res) << std::endl;
        }
        else {
            auto jsonResponse = nlohmann::json::parse(responseString);
            if (jsonResponse.contains("access_token")) {
                std::string accessToken = jsonResponse["access_token"];
                std::cout << "Access Token: " << accessToken << std::endl;
                refreshtoken = jsonResponse["refresh_token"];
                std::cout << "refresh Token: " << refreshtoken << std::endl;
                return accessToken;
            }
            else {
                std::cerr << "Error: 'access_token' not found in response." << std::endl;
                std::cerr << "Full response: " << responseString << std::endl;
            }
        }

        curl_easy_cleanup(curl);
    }

    return "";
}
std::string refreshAccessToken(const std::string& refreshToken, const std::string& clientId, const std::string& clientSecret) {
    CURL* curl;
    CURLcode res;
    curl = curl_easy_init();
    std::string responseString;
    if (curl) {
        std::string postData = "client_id=" + clientId +
            "&client_secret=" + clientSecret +
            "&refresh_token=" + refreshToken +
            "&grant_type=refresh_token";
        curl_easy_setopt(curl, CURLOPT_URL, "https://oauth2.googleapis.com/token");
        curl_easy_setopt(curl, CURLOPT_POST, 1L);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, postData.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &responseString);
        curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
        res = curl_easy_perform(curl);

        if (res != CURLE_OK) {
            std::cerr << "curl_easy_perform() failed: " << curl_easy_strerror(res) << std::endl;
        }
        else {
            auto jsonResponse = nlohmann::json::parse(responseString);
            if (jsonResponse.contains("access_token")) {
                std::string accessToken = jsonResponse["access_token"];
                std::cout << "Access Token: " << accessToken << std::endl;
                return accessToken;
            }
            else {
                std::cerr << "Error: 'access_token' not found in response." << std::endl;
                std::cerr << "Full response: " << responseString << std::endl;
            }
        }

        curl_easy_cleanup(curl);
    }

    return "";
}

std::string readEmailContent(const std::string& accessToken, const std::string& messageId,  std::string&request_mail) {
    CURL* curl;
    CURLcode res;
    curl = curl_easy_init();
    std::string responseString;
    std::string decodedBody;
    if (curl) {
        struct curl_slist* headers = NULL;
        headers = curl_slist_append(headers, ("Authorization: Bearer " + accessToken).c_str());

        // Đặt URL để lấy nội dung chi tiết email
        std::string url = "https://gmail.googleapis.com/gmail/v1/users/me/messages/" + messageId;
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &responseString);

        // Thực hiện yêu cầu HTTP
        res = curl_easy_perform(curl);

        if (res != CURLE_OK) {
            std::cerr << "curl_easy_perform() failed: " << curl_easy_strerror(res) << std::endl;
        }
        else {


            try {
                // Phân tích phản hồi JSON
                auto jsonResponse = nlohmann::json::parse(responseString);
                if (jsonResponse.contains("snippet")) {
                    std::string encodedBody = jsonResponse["snippet"];
                    decodedBody = encodedBody;
                    /*  std::cout << "Decoded Email Content: " << decodedBody << std::endl;*/
                    auto headers = jsonResponse["payload"]["headers"];
                    for (const auto& header : headers) {
                        if (header["name"] == "From") {
                            std:: string senderEmail = header["value"];
                            request_mail = senderEmail;
                            break;  
                        }
                    }
                }
            }
            catch (const std::exception& e) {
                std::cerr << "Failed to parse JSON: " << e.what() << std::endl;
            }
        }

        curl_easy_cleanup(curl);
        curl_slist_free_all(headers);
        return decodedBody;
    }
    return 0;
}

std::string readGmail(const std::string& accessToken, int& num, std::string& request_mail) {
    CURL* curl;
    CURLcode res;
    curl = curl_easy_init();
    std::string newest;
    std::string responseString;
    if (curl) {
        struct curl_slist* headers = NULL;
        headers = curl_slist_append(headers, ("Authorization: Bearer " + accessToken).c_str());

        curl_easy_setopt(curl, CURLOPT_URL, "https://gmail.googleapis.com/gmail/v1/users/me/messages");
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &responseString);

        // Bật verbose để xem log chi tiết
        curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);

        // Thực hiện yêu cầu HTTP
        res = curl_easy_perform(curl);

        if (res != CURLE_OK) {
            std::cerr << "curl_easy_perform() failed: " << curl_easy_strerror(res) << std::endl;
        }
        else {
            // Kiểm tra kết quả trả về sau khi hoàn thành yêu cầu
            std::cout << "CURL Response Complete\n";
            //std::cout << "Gmail Response: " << responseString << std::endl;

            try {
                // Phân tích phản hồi JSON
                auto jsonResponse = nlohmann::json::parse(responseString);

                // Kiểm tra các message có trong phản hồi
                if (jsonResponse.contains("messages")) {
                    auto messages = jsonResponse["messages"];
                    std::cout << "You have " << messages.size() << " messages." << std::endl;
                    num = messages.size();
                    // Lặp qua từng tin nhắn và in ra thông tin
                    for (const auto& message : messages) {
                        std::cout << "Message ID: " << message["id"] << std::endl;
                        // Đọc nội dung của từng email
                        newest = readEmailContent(accessToken, message["id"], request_mail);
                        break;// Chỉ lấy một mail mới nhất rồi break
                    }
                }
                else {
                    std::cout << "No messages found in Gmail response." << std::endl;
                }
            }
            catch (const std::exception& e) {
                std::cerr << "Failed to parse JSON: " << e.what() << std::endl;
            }
        }

        curl_easy_cleanup(curl);
        curl_slist_free_all(headers);  // Giải phóng bộ nhớ của headers
    }
    return newest;
}


std::string base64Encode(const std::string& input) {
    // Bảng mã hóa Base64
    static const std::string base64_chars =
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz"
        "0123456789+/";

    std::string encoded;
    int val = 0;
    int valb = -6;

    // Mã hóa từng nhóm 3 byte thành 4 ký tự Base64
    for (unsigned char c : input) {
        val = (val << 8) + c;
        valb += 8;
        while (valb >= 0) {
            encoded.push_back(base64_chars[(val >> valb) & 0x3F]);
            valb -= 6;
        }
    }

    // Xử lý phần còn dư nếu chuỗi không đủ 3 byte
    if (valb > -6) {
        encoded.push_back(base64_chars[((val << 8) >> (valb + 8)) & 0x3F]);
    }

    // Thêm ký tự '=' để làm padding cho đủ bội của 4
    while (encoded.size() % 4) {
        encoded.push_back('=');
    }

    return encoded;
}

// Hàm tạo payload email và mã hóa thành Base64
std::string createEmailPayload(const std::string& to, const std::string& from, const std::string& subject, const std::string& body) {
    std::stringstream email;
    email << "To: " << to << "\r\n"
        << "From: " << from << "\r\n"
        << "Subject: " << subject << "\r\n"
        << "Content-Type: text/plain; charset=\"UTF-8\"\r\n\r\n"
        << body;

    // Mã hóa email thành Base64
    return base64Encode(email.str());
}
std::string createEmailPayload2(const std::string& to, const std::string& from, const std::string& subject, const std::string& body, const std::string& attachmentPath) {
    // Đọc nội dung tệp đính kèm
    std::ifstream file(attachmentPath, std::ios::binary);
    std::ostringstream oss;
    oss << file.rdbuf();
    std::string fileContent = oss.str();

    // Mã hóa tệp đính kèm thành base64
    std::string encodedAttachment = base64Encode(fileContent);

    // Tạo MIME email với tệp đính kèm
    std::stringstream email;
    email << "To: " << to << "\r\n"
        << "From: " << from << "\r\n"
        << "Subject: " << subject << "\r\n"
        << "Content-Type: multipart/mixed; boundary=\"boundary_string\"\r\n\r\n"
        << "--boundary_string\r\n"
        << "Content-Type: text/plain; charset=\"UTF-8\"\r\n\r\n"
        << body << "\r\n\r\n"
        << "--boundary_string\r\n"
        << "Content-Type: image/jpeg\r\n"
        << "Content-Transfer-Encoding: base64\r\n"
        << "Content-Disposition: attachment; filename=\"image.jpg\"\r\n\r\n"
        << encodedAttachment << "\r\n"
        << "--boundary_string--";

    // Mã hóa toàn bộ email thành base64 để gửi qua Gmail API
    return base64Encode(email.str());
}
bool sendGmail(const std::string& accessToken, const std::string& to, const std::string& from, const std::string& subject, const std::string& body) {
    CURL* curl;
    CURLcode res;
    curl = curl_easy_init();

    if (curl) {
        std::string emailPayload = createEmailPayload(to, from, subject, body);
        nlohmann::json jsonPayload = {
            {"raw", emailPayload}
        };

        struct curl_slist* headers = NULL;
        headers = curl_slist_append(headers, ("Authorization: Bearer " + accessToken).c_str());
        headers = curl_slist_append(headers, "Content-Type: application/json");

        std::string responseString;
        std::string jsonData = jsonPayload.dump();

        curl_easy_setopt(curl, CURLOPT_URL, "https://gmail.googleapis.com/gmail/v1/users/me/messages/send");
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, jsonData.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &responseString);
        curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);  // Để xem log chi tiết

        res = curl_easy_perform(curl);
        std::cout << responseString;
        if (res != CURLE_OK) {
            std::cerr << "curl_easy_perform() failed: " << curl_easy_strerror(res) << std::endl;
        }
        else {
            std::cout << "Email sent successfully!" << std::endl;
        }

        curl_easy_cleanup(curl);
        curl_slist_free_all(headers);  // Giải phóng bộ nhớ headers
        return (res == CURLE_OK);
    }

    return false;
}
bool sendGmailAttachments(const std::string& accessToken, const std::string& to, const std::string& from, const std::string& subject, const std::string& body, const std::string& attachmentPath) {
    CURL* curl;
    CURLcode res;
    curl = curl_easy_init();

    if (curl) {
        std::string emailPayload = createEmailPayload2(to, from, subject, body, attachmentPath);
        nlohmann::json jsonPayload = { {"raw", emailPayload} };

        struct curl_slist* headers = NULL;
        headers = curl_slist_append(headers, ("Authorization: Bearer " + accessToken).c_str());
        headers = curl_slist_append(headers, "Content-Type: application/json");

        std::string responseString;
        std::string jsonData = jsonPayload.dump();

        curl_easy_setopt(curl, CURLOPT_URL, "https://gmail.googleapis.com/gmail/v1/users/me/messages/send");
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, jsonData.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &responseString);
        curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);

        res = curl_easy_perform(curl);
        if (res != CURLE_OK) {
            std::cerr << "curl_easy_perform() failed: " << curl_easy_strerror(res) << std::endl;
        }
        else {
            std::cout << "Email sent successfully!" << std::endl;
        }

        curl_easy_cleanup(curl);
        curl_slist_free_all(headers);
        return (res == CURLE_OK);
    }
    return false;
}

std::wstring stringToWString(const std::string& str) {
    return std::wstring(str.begin(), str.end());
}

// Hàm mở URL trong trình duyệt mặc định
void openURL(const std::string& url) {
    std::wstring wideUrl = stringToWString(url);
    ShellExecute(0, 0, wideUrl.c_str(), 0, 0, SW_SHOW);
}

std::string listenForOAuthCode(const std::string& tempFile) {
    std::ifstream file;
    std::string code;

    std::cout << "Waiting for OAuth code...\n";
    while (code.empty()) {
        file.open(tempFile);
        if (file.is_open()) {
            getline(file, code);
            file.close();
        }
        Sleep(1000); // Chờ 1 giây
    }
    return code;
}

#pragma comment(lib, "ws2_32.lib")

void startLocalServer(const std::string& tempFile) {
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);

    SOCKET serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    sockaddr_in serverAddr = {};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(80); // Port 80
    serverAddr.sin_addr.s_addr = INADDR_ANY;

    bind(serverSocket, (sockaddr*)&serverAddr, sizeof(serverAddr));
    listen(serverSocket, 1);

    std::cout << "Listening for redirect...\n";
    SOCKET clientSocket = accept(serverSocket, NULL, NULL);

    char buffer[1024] = {};
    recv(clientSocket, buffer, sizeof(buffer), 0);
    std::string request(buffer);

    // Tìm `code` trong request
    size_t codePos = request.find("code=");
    if (codePos != std::string::npos) {
        size_t endPos = request.find('&', codePos);
        std::string code = request.substr(codePos + 5, endPos - codePos - 5);

        // Ghi mã code vào file
        std::ofstream file(tempFile);
        file << code;
        file.close();

        // Trả về phản hồi
        std::string response = "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\n\r\nAuthorization complete. You can close this tab.";
        send(clientSocket, response.c_str(), response.length(), 0);
    }

    closesocket(clientSocket);
    closesocket(serverSocket);
    WSACleanup();
}