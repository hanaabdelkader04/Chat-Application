#include <iostream>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <string>
#include <cctype>
#include <random>

#pragma comment(lib, "Ws2_32.lib")

using namespace std;

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

int main() {
    WSADATA wsaData;
    int result = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (result != 0) {
        cout << "WSAStartup failed: " << result << endl;
        return 1;
    }

    SOCKET clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (clientSocket == INVALID_SOCKET) {
        cout << "Socket creation failed with error: " << WSAGetLastError() << endl;
        WSACleanup();
        return 1;
    }

    sockaddr_in serverAddress;
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(8080);
    serverAddress.sin_addr.s_addr = inet_addr("127.0.0.1");

    if (connect(clientSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) == SOCKET_ERROR) {
        cout << "Connection failed with error: " << WSAGetLastError() << endl;
        closesocket(clientSocket);
        WSACleanup();
        return 1;
    }

    cout << "Connected successfully. Enter your messages. Type 'exit' to quit:" << endl;

    random_device rd;
    mt19937 gen(rd());
    uniform_int_distribution<> distrib(1, 25);

    string input;
    while (true) {
        cout << "Enter message: ";
        getline(cin, input);
        if (input == "exit") {
            break;
        }

        int key = distrib(gen);
        string encryptedMessage = caesarEncrypt(input, key);
        string messageToSend = to_string(key) + ":" + encryptedMessage;

        if (send(clientSocket, messageToSend.c_str(), messageToSend.length(), 0) == SOCKET_ERROR) {
            cout << "Send failed with error: " << WSAGetLastError() << endl;
            break;
        }

        char buffer[1024] = {0};
        int bytesReceived = recv(clientSocket, buffer, sizeof(buffer), 0);
        if (bytesReceived == SOCKET_ERROR) {
            cout << "Receive failed with error: " << WSAGetLastError() << endl;
            break;
        } else if (bytesReceived == 0) {
            cout << "Server disconnected" << endl;
            break;
        }

        // Assuming server response includes key
        string response(buffer, bytesReceived);
        size_t colonPos = response.find(':');
        int receivedKey = stoi(response.substr(0, colonPos));
        string decryptedResponse = caesarEncrypt(response.substr(colonPos + 1), 26 - receivedKey);
        cout << "Server: " << decryptedResponse << endl;
    }

    closesocket(clientSocket);
    WSACleanup();
    return 0;
}
