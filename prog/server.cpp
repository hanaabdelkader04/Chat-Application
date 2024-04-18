#include <iostream>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <string> 
#include <cctype>  

#pragma comment (lib, "Ws2_32.lib") 

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
    SOCKET serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == INVALID_SOCKET) {
        cout << "Socket creation failed with error: " << WSAGetLastError() << endl;
        WSACleanup();
        return 1;
    }

    // specifying the address
    sockaddr_in serverAddress;
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(8080);
    serverAddress.sin_addr.s_addr = INADDR_ANY;

    // binding socket
    if (bind(serverSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) == SOCKET_ERROR) {
        cout << "Bind failed with error: " << WSAGetLastError() << endl;
        closesocket(serverSocket);
        WSACleanup();
        return 1;
    }
    
    cout << "Server is listening..." << endl;

    // listening to the assigned socket
    if (listen(serverSocket, 5) == SOCKET_ERROR) {
        cout << "Listen failed with error: " << WSAGetLastError() << endl;
        closesocket(serverSocket);
        WSACleanup();
        return 1;
    }

    // accepting connection request
    SOCKET clientSocket = accept(serverSocket, nullptr, nullptr);
    if (clientSocket == INVALID_SOCKET) {
        cout << "Accept failed with error: " << WSAGetLastError() << endl;
        closesocket(serverSocket);
        WSACleanup();
        return 1;
    }

    // loop to receive and respond to data continuously
    cout << "Client connected, ready to receive messages." << endl;
    string serverResponse;
    while (true) {
    char buffer[1024] = {0};
    int bytesReceived = recv(clientSocket, buffer, sizeof(buffer), 0);
    if (bytesReceived == SOCKET_ERROR) {
        cout << "Receive failed with error: " << WSAGetLastError() << endl;
        break;
    } else if (bytesReceived == 0) {
        cout << "Client disconnected" << endl;
        break;
    }

    string decryptedMessage = caesarDecrypt(string(buffer, bytesReceived), 3);
    cout << "Message from client: " << decryptedMessage << endl;

    cout << "Enter response to client: ";
    getline(cin, serverResponse);
    string encryptedResponse = caesarEncrypt(serverResponse, 3);
    if (send(clientSocket, encryptedResponse.c_str(), encryptedResponse.length(), 0) == SOCKET_ERROR) {
        cout << "Send failed with error: " << WSAGetLastError() << endl;
        break;
    }
}


    // closing the socket
    closesocket(clientSocket);
    closesocket(serverSocket);
    WSACleanup();

    return 0;
}
