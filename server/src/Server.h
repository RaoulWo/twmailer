#ifndef SERVER_H
#define SERVER_H

#include <netinet/in.h>
#include <string>
#include <thread>
#include <vector>

namespace TwMailer
{

    // The size of the buffer
    constexpr int BUF = 1024;

    class Server
    {
    public:
        void Start(int port);
        void ListenForClients(const std::string& mailSpoolDir);
        void CommunicateWithClient(int* socket, const std::string& mailSpoolDir);
        void Abort();

    private:
        void TryStart(int port);
        void TryCommunicateWithClient(int* socket, const std::string& mailSpoolDir);
        void TryListenForClients(const std::string& mailSpoolDir);

        int create_socket = -1;
        int new_socket = -1;
        struct sockaddr_in address;
        struct sockaddr_in cliaddress;
        socklen_t addrlen;
        bool abortRequested = false;
        std::vector<int> sockets;
        std::vector<std::thread> threads;
    };

}

#endif