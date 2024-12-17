#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <stdio.h>
#include <thread>
#include <mutex>
#include "UDPServer.h"

#ifdef _WIN32
#include <winsock2.h>
#pragma comment(lib, "ws2_32.lib") // Winsock Library
#else
using SOCKET = int;
#define WSAGetLastError() 1
#endif

#define BUFLEN 1024  // Max length of buffer
#define PORT 8888    // Default port

std::mutex print_mutex; // For thread-safe printing

void receiveData(UDPServer& server, struct sockaddr_in& si_other, int slen) {
    char buf[BUFLEN];
    while (true) {
        memset(buf, 0, BUFLEN); // Clear buffer
        server.RecvDatagram(buf, BUFLEN, (struct sockaddr*)&si_other, &slen);

        std::lock_guard<std::mutex> lock(print_mutex);
        printf("Received packet from %s:%d\n", inet_ntoa(si_other.sin_addr), ntohs(si_other.sin_port));
        printf("Data: %s\n", buf);
    }
}

void sendData(UDPServer& server, struct sockaddr_in& si_other, int slen) {
    char msg[BUFLEN];
    while (true) {
        printf("\nEnter message to send: ");
        fgets(msg, BUFLEN, stdin);

        std::lock_guard<std::mutex> lock(print_mutex);
        server.SendDatagram(msg, (int)strlen(msg), (struct sockaddr*)&si_other, slen);
    }
}

int main(int argc, char* argv[]) {
    struct sockaddr_in si_other;
    unsigned short srvport = (argc == 1) ? PORT : atoi(argv[1]);
    int slen = sizeof(si_other);

    UDPServer server(srvport);

    printf("UDP Server is running on port %d...\n", srvport);

    // Start threads for sending and receiving data
    std::thread recvThread(receiveData, std::ref(server), std::ref(si_other), slen);
    std::thread sendThread(sendData, std::ref(server), std::ref(si_other), slen);

    // Wait for threads to complete (they won't in this infinite loop)
    recvThread.join();
    sendThread.join();

    return 0;
}
