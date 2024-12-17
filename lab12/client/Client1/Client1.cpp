#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <stdio.h>
#include <thread>
#include <mutex>
#include "UDPSocket.h"

#ifdef _WIN32
#include <winsock2.h>
#pragma comment(lib, "ws2_32.lib") // Winsock Library
#else
using SOCKET = int;
#endif

#define BUFLEN 1024   // Max length of buffer
#define PORT 8888     // Default port
#define SERVER "127.0.0.1" // Default server IP

std::mutex print_mutex; // For thread-safe printing

void receiveData(UDPSocket& client_sock, struct sockaddr_in& si_other, int slen) {
    char buf[BUFLEN];
    while (true) {
        memset(buf, 0, BUFLEN); // Clear buffer
        client_sock.RecvDatagram(buf, BUFLEN, (struct sockaddr*)&si_other, &slen);

        std::lock_guard<std::mutex> lock(print_mutex);
        printf("\nServer says: %s\n", buf);
    }
}

void sendData(UDPSocket& client_sock, struct sockaddr_in& si_other, int slen) {
    char message[BUFLEN];
    while (true) {
        printf("\nEnter message to send: ");
        fgets(message, BUFLEN, stdin);

        std::lock_guard<std::mutex> lock(print_mutex);
        client_sock.SendDatagram(message, (int)strlen(message), (struct sockaddr*)&si_other, slen);
    }
}

int main(int argc, char* argv[]) {
    struct sockaddr_in si_other;
    int slen = sizeof(si_other);

    memset((char*)&si_other, 0, sizeof(si_other));
    si_other.sin_family = AF_INET;

    // Configure server address and port
    if (argc == 1) {
        si_other.sin_port = htons(PORT);
        si_other.sin_addr.S_un.S_addr = inet_addr(SERVER);
        printf("Connecting to server %s on port %d\n", SERVER, PORT);
    }
    else if (argc == 2) {
        si_other.sin_port = htons(atoi(argv[1]));
        si_other.sin_addr.S_un.S_addr = inet_addr(SERVER);
    }
    else {
        si_other.sin_port = htons(atoi(argv[2]));
        si_other.sin_addr.S_un.S_addr = inet_addr(argv[1]);
    }

    UDPSocket client_sock;

    // Start threads for sending and receiving data
    std::thread recvThread(receiveData, std::ref(client_sock), std::ref(si_other), slen);
    std::thread sendThread(sendData, std::ref(client_sock), std::ref(si_other), slen);

    // Wait for threads to complete (they won't in this infinite loop)
    recvThread.join();
    sendThread.join();

    return 0;
}
