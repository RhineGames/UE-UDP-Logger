#include <winsock2.h>
#include <ws2tcpip.h>

#include <iostream>
#include <fstream>
#include <string>
#include <ctime>

#pragma comment(lib, "ws2_32.lib")

std::string GetTimestamp()
{
    std::time_t now = std::time(nullptr);

    char buf[64];
    std::tm tm;
    localtime_s(&tm, &now);

    strftime(buf, sizeof(buf), "%Y-%m-%d_%H-%M-%S", &tm);
    return std::string(buf);
}

int main()
{
    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
    {
        std::cout << "WSAStartup failed\n";
        return 1;
    }

    SOCKET sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (sock == INVALID_SOCKET)
    {
        std::cout << "Socket creation failed\n";
        return 1;
    }

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(7777);
    addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(sock, (sockaddr*)&addr, sizeof(addr)) == SOCKET_ERROR)
    {
        std::cout << "Bind failed\n";
        return 1;
    }

    std::string filename = "log_" + GetTimestamp() + ".txt";
    std::ofstream logfile(filename, std::ios::out);

    if (!logfile.is_open())
    {
        std::cout << "Failed to open log file\n";
        return 1;
    }

    std::cout << "Listening on UDP 7777...\n";
    std::cout << "Logging to: " << filename << "\n";
    std::cout << " ======== READY TO USE - START THE GAME ========\n\n";

    char buffer[2048];

    while (true)
    {
        sockaddr_in sender{};
        int senderSize = sizeof(sender);

        int len = recvfrom(sock, buffer, sizeof(buffer) - 1, 0,
            (sockaddr*)&sender, &senderSize);

        if (len == SOCKET_ERROR)
        {
            continue;
        }

        buffer[len] = '\0';

        std::string msg(buffer);

        // optional zusätzlicher timestamp (receiver-seitig)
        std::string ts = GetTimestamp();

        std::string finalLine = msg;

        std::cout << finalLine << std::endl;
        logfile << finalLine << std::endl;
    }

    closesocket(sock);
    WSACleanup();

    return 0;
}