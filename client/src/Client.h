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
        void StoreUsername();
        void PrintMenu() const;
        bool IsValidMenuChar(char c) const;

        std::string ConstructRequest(char c) const;
        std::string ConstructSendRequest() const;
        std::string ConstructListRequest() const;
        std::string ConstructReadRequest() const;
        std::string ConstructDeleteRequest() const;
        std::string ConstructQuitRequest() const;

        void SendRequest(const std::string& request) const;

        std::string GetReceiver() const;
        std::string GetSubject() const;
        std::string GetMessage() const;
        int GetMsgNum() const;


        std::string ip;
        int port;

        int create_socket;
        char buffer[BUF];
        struct sockaddr_in address;
        int size;
        bool isQuit = false;

        std::string username = "";
    };

}

#endif