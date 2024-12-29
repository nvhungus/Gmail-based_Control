#include "client.h"
#include <iostream>
using namespace std;
Client::Client() : ConnectSocket(INVALID_SOCKET) {}

Client::~Client() {
    if (ConnectSocket != INVALID_SOCKET) {
        closesocket(ConnectSocket);
        WSACleanup();
    }
}

void Client::setServerIP(const  string& ip) {
    serverIP = ip;
}   

bool Client::connectToServer() {
    WSADATA wsaData;
    struct addrinfo* result = NULL, hints;

    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) return false;

    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    if (getaddrinfo(serverIP.c_str(), port.c_str(), &hints, &result) != 0) {
        WSACleanup();
        return false;
    }

    ConnectSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
    if (ConnectSocket == INVALID_SOCKET || connect(ConnectSocket, result->ai_addr, (int)result->ai_addrlen) == SOCKET_ERROR) {
        freeaddrinfo(result);
        WSACleanup();
        return false;
    }

    freeaddrinfo(result);
    return true;
}

string Client::sendMessage(const string& message) {
    send(ConnectSocket, message.c_str(), static_cast<int>(message.length()), 0);

    std::vector<char> recvbuf(2048);
    string response;
    int recvResult;

    // Nhận phản hồi theo từng phần
    do {
        recvResult = recv(ConnectSocket, recvbuf.data(), recvbuf.size(), 0);
        if (recvResult > 0) {
            response.append(recvbuf.data(), recvResult);
        }
        else if (recvResult < 0) {
            return "Error: Failed to receive response.";
        }
    } while (recvResult > 0 && recvResult == static_cast<int>(recvbuf.size()));

    return response.empty() ? "No response or connection lost." : response;
}

std::vector<std::string> takeMailConfirm(const std::string& filename) {
    std::vector<std::string> emails;
    std::ifstream file(filename); // Mở file đọc

    if (file.is_open()) {
        std::string line;
        while (std::getline(file, line)) {
            if (!line.empty()) { // Bỏ qua dòng trống
                emails.push_back(line);
            }
        }
        file.close();
    }
    else {
        std::cerr << "Failed to open file: " << filename << std::endl;
    }

    return emails;
}