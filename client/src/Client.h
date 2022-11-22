#ifndef CLIENT_H
#define CLIENT_H

#include <netinet/in.h> // For sockaddr_in
#include <string>
#include <vector>

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
        void StorePassword();
        void PrintMenu() const;
        char GetUserMenuInput();
        bool IsValidMenuChar(char c) const;
        std::string ConstructRequest(char c) const;
        std::string ConstructLoginRequest() const;
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
        void HandleResponse(const std::string& response);
        std::vector<std::string> ParseResponse(const std::string& response) const;

        std::string ip;
        int port;

        int create_socket;
        char buffer[BUF];
        struct sockaddr_in address;
        int size;
        bool isQuit = false;

        std::string username = "";
        std::string password = "";

        bool isLoggedIn = false;
        int failedLoginAttempts = 0;
    };

    int getch();
    std::string getPassword(bool showAsterisk);
}

#endif