#ifndef CLIENT_H
#define CLIENT_H

#include <netinet/in.h> // For sockaddr_in
#include <string>

namespace TwMailer 
{
    
    constexpr int BUF = 1024;
    constexpr int PORT = 6543;

    class Client
    {
    public:
        Client(std::string ip, std::string port);

        void Start();
        void ConnectToServer();

    private:
        void TryStart();
        void TryConnectToServer();

        std::string ip;
        std::string port;

        int create_socket;
        char buffer[BUF];
        struct sockaddr_in address;
        int size;
        bool isQuit = false;
    };

}

#endif