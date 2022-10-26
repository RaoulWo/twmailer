#ifndef SERVER_H
#define SERVER_H

#include <netinet/in.h>
#include <string>

namespace TwMailer
{

    // The size of the buffer
    constexpr int BUF = 1024;

    class Server
    {
    public:
        void Start(int port, std::string mailSpoolDir);
        void ListenForClients();
        void Abort();

    private:
        void TryStart(int port, std::string mailSpoolDir);
        void TryListenForClients();

        int create_socket = -1;
        int new_socket = -1;
        struct sockaddr_in address;
        struct sockaddr_in cliaddress;
        socklen_t addrlen;
        bool abortRequested = false;
        int socketA;
        int socketB;
    };

}

#endif