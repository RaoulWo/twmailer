#ifndef CLIENT_H
#define CLIENT_H

#include <netinet/in.h> // For sockaddr_in
#include <string>

namespace TwMailer 
{
    
    // The size of the buffer
    constexpr int BUF = 1024;

    class Client
    {
    public:
        Client(std::string ip, int port);

        void Start();
        void ConnectToServer();

    private:
        void TryStart();
        void TryConnectToServer();

        std::string ip;
        int port;

        int create_socket;
        char buffer[BUF];
        struct sockaddr_in address;
        int size;
        bool isQuit = false;
    };

}

#endif