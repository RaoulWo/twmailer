#ifndef SERVER_H
#define SERVER_H

#include <netinet/in.h>
#include <string>

namespace TwMailer
{

    constexpr int BUF = 1024;
    constexpr int PORT = 6543;
    constexpr int MSGSIZE = 11;

    class Server
    {
    public:
        void Start(std::string port, std::string mailSpoolDir);
        void ListenForClients();
        void Abort();

    private:
        void TryStart(std::string port, std::string mailSpoolDir);
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