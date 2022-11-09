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
        void CommunicateWithClient(int* socket);
        void Abort();

    private:
        void TryStart(int port);
        void TryCommunicateWithClient(int* socket);
        void TryListenForClients();
        std::string HandleRequest(const std::string& request);
        void SendResponse(int* socket, const std::string& response) const;
        std::string HandleSendRequest(const std::vector<std::string>& tokens);
        std::string HandleListRequest(const std::vector<std::string>& tokens);
        std::string HandleReadRequest(const std::vector<std::string>& tokens);
        std::string HandleDeleteRequest(const std::vector<std::string>& tokens);
        std::string HandleQuitRequest(const std::vector<std::string>& tokens);
        std::string HandleBadRequest(const std::vector<std::string>& tokens);
        std::vector<std::string> ParseText(const std::string& text) const;

        bool EntryExistsInPath(const std::string& entry, const std::string& path) const;
        int GetNumberOfEntriesInPath(const std::string& path) const;

        int create_socket = -1;
        int new_socket = -1;
        struct sockaddr_in address;
        struct sockaddr_in cliaddress;
        socklen_t addrlen;
        bool abortRequested = false;
        std::vector<int> sockets;
        std::vector<std::thread> threads;
        std::string mailSpoolDir;
    };

}

#endif