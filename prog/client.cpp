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

string caesarDecrypt(const string& text, int shift) {
    return caesarEncrypt(text, 26 - shift);  
}


int main() {
    WSADATA wsaData;

    // initialize Winsock
    int result = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (result != 0) {
        cout << "WSAStartup failed: " << result << endl;
        return 1;
    }

    // creating socket
    SOCKET clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (clientSocket == INVALID_SOCKET) {
        cout << "Socket creation failed with error: " << WSAGetLastError() << endl;
        WSACleanup();
        return 1;
    }

    // specifying address
    sockaddr_in serverAddress;
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(8080);
    serverAddress.sin_addr.s_addr = inet_addr("127.0.0.1"); 

    // sending connection request
    if (connect(clientSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) == SOCKET_ERROR) {
        cout << "Connection failed with error: " << WSAGetLastError() << endl;
        closesocket(clientSocket);
        WSACleanup();
        return 1;
    }

    cout << "Connected successfully. Enter your messages. Type 'exit' to quit:" << endl;

    string input;
    while (true) {
        cout << "Enter message: ";
        getline(cin, input);
        if (input == "exit") {
            break;  
        }
        
        //message encryption

        string encryptedMessage = caesarEncrypt(input, 3);
        if (send(clientSocket, encryptedMessage.c_str(), encryptedMessage.length(), 0) == SOCKET_ERROR) {
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
        string decryptedMessage = caesarDecrypt(string(buffer, bytesReceived), 3);
        cout << "Server: " << decryptedMessage << endl;
    }


    // closing socket
    closesocket(clientSocket);
    WSACleanup();

    return 0;
}
