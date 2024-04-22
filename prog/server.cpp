#include <iostream>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <string>
#include <fstream>

#pragma comment(lib, "Ws2_32.lib")

using namespace std;

const int MAX_CLIENTS = 2;  // Maximum number of clients
SOCKET clients[MAX_CLIENTS] = { INVALID_SOCKET, INVALID_SOCKET };
int clientCount = 0;

string caesarEncrypt(const string& text, int shift) {
    string result = "";
    for (char c : text) {
        if (isalpha(c)) {
            char base = isupper(c) ? 'A' : 'a';
            c = static_cast<char>(((c - base + shift) % 26) + base);
        }
        result += c;
    }
    return result;
}

void main() {
    WSADATA wsaData;
    int result = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (result != 0) {
        cout << "WSAStartup failed: " << result << endl;
        return;
    }

    SOCKET serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == INVALID_SOCKET) {
        cout << "Socket creation failed: " << WSAGetLastError() << endl;
        WSACleanup();
        return;
    }

    sockaddr_in serverAddress;
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(8080);
    serverAddress.sin_addr.s_addr = INADDR_ANY;

    if (bind(serverSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) == SOCKET_ERROR) {
        cout << "Bind failed: " << WSAGetLastError() << endl;
        closesocket(serverSocket);
        WSACleanup();
        return;
    }

    cout << "Server is listening..." << endl;
    if (listen(serverSocket, 5) == SOCKET_ERROR) {
        cout << "Listen failed: " << WSAGetLastError() << endl;
        closesocket(serverSocket);
        WSACleanup();
        return;
    }

    ofstream chatLog("chatlog.txt", ios::app);  // Log file for storing chat history

    while (clientCount < MAX_CLIENTS) {
        SOCKET clientSocket = accept(serverSocket, nullptr, nullptr);
        if (clientSocket == INVALID_SOCKET) {
            cout << "Accept failed: " << WSAGetLastError() << endl;
            continue;
        }

        clients[clientCount++] = clientSocket;
        cout << "Client connected, total clients: " << clientCount << endl;
    }

    char buffer[1024];
    while (true) {
        for (int i = 0; i < MAX_CLIENTS; ++i) {
            memset(buffer, 0, sizeof(buffer));
            int bytesReceived = recv(clients[i], buffer, sizeof(buffer), 0);
            if (bytesReceived == SOCKET_ERROR) {
                cout << "Receive failed: " << WSAGetLastError() << endl;
                continue;
            } else if (bytesReceived == 0) {
                cout << "Client disconnected" << endl;
                closesocket(clients[i]);
                clients[i] = INVALID_SOCKET;
                --clientCount;
                continue;
            }

            string receivedData(buffer, bytesReceived);
            chatLog << "Client " << i << ": " << receivedData << endl;  // Log message
            cout << "Client " << i << ": " << receivedData << endl;  // Output message

            // Forward message to the other client
            for (int j = 0; j < MAX_CLIENTS; ++j) {
                if (clients[j] != INVALID_SOCKET && j != i) {
                    send(clients[j], receivedData.c_str(), receivedData.length(), 0);
                }
            }
        }
    }

    // Cleanup on exit
    for (int i = 0; i < MAX_CLIENTS; ++i) {
        if (clients[i] != INVALID_SOCKET) {
            closesocket(clients[i]);
        }
    }
    closesocket(serverSocket);
    WSACleanup();
    chatLog.close();
}
