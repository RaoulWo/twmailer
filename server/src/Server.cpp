#include "Server.h"

#include <arpa/inet.h>
#include <fstream>
#include <filesystem>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>



namespace TwMailer
{
    void Server::Start(int port)
    {
        try
        {
            TryStart(port);
        }
        catch(const std::runtime_error& e)
        {
            std::cerr << e.what() << '\n';
            exit(EXIT_FAILURE);
        }
        catch(...)
        {
            std::cerr << "An unexpected error occured when trying to start the server!" << '\n';
            exit(EXIT_FAILURE);
        }
    }

    void Server::ListenForClients(const std::string& mailSpoolDir)
    {
        this->mailSpoolDir = mailSpoolDir;

        try
        {
            TryListenForClients();
        }
        catch(const std::runtime_error& e)
        {
            std::cerr << e.what() << '\n';
            exit(EXIT_FAILURE);
        }
        catch(...)
        {
            std::cerr << "An unexpected error occured when trying to listen for clients!" << '\n';
            exit(EXIT_FAILURE);
        }
    }

    void Server::CommunicateWithClient(int* socket)
    {
        try
        {
            TryCommunicateWithClient(socket);
        }
        catch(const std::runtime_error& e)
        {
            std::cerr << e.what() << '\n';
            exit(EXIT_FAILURE);
        }
        catch(...)
        {
            std::cerr << "An unexpected error occured when trying to communicate with a client!" << '\n';
            exit(EXIT_FAILURE);
        }
    }

    void Server::Abort()
    {
        abortRequested = true;

        if (new_socket != -1)
        {
            if (shutdown(new_socket, SHUT_RDWR) == -1)
            {
                std::cerr << "Shutdown new_socket!" << '\n';
            }
            if (close(new_socket) == -1)
            {
                std::cerr << "Close new_socket!" << '\n';
            }
            new_socket = -1;
        }

        if (create_socket != -1)
        {
            if (shutdown(create_socket, SHUT_RDWR) == -1)
            {
                std::cerr << "Shutdown create_socket!" << '\n';
            }
            if (close(create_socket) == -1)
            {
                std::cerr << "Close create_socket!" << '\n';
            }
            create_socket = -1;
        }
    }

    void Server::TryStart(int port)
    {
        int reuseValue = 1;

        if ((create_socket = socket(AF_INET, SOCK_STREAM, 0)) == -1)
        {
            throw std::runtime_error("Socket could not be opened!");
        }
        if (setsockopt(create_socket, SOL_SOCKET, SO_REUSEADDR, &reuseValue, sizeof(reuseValue)) == -1)
        {
            throw std::runtime_error("Set socket options: SO_REUSEADDR!");
        }
        if (setsockopt(create_socket, SOL_SOCKET, SO_REUSEPORT, &reuseValue, sizeof(reuseValue)) == -1)
        {
            throw std::runtime_error("Set socket options: SO_REUSEPORT!");
        }

        memset(&address, 0, sizeof(address));
        address.sin_family = AF_INET;
        address.sin_addr.s_addr = INADDR_ANY;
        address.sin_port = htons(port);

        if (bind(create_socket, (struct sockaddr*)&address, sizeof(address)) == -1)
        {
            throw std::runtime_error("Could not bind address to socket!");
        }
    }

    void Server::TryListenForClients()
    {
        
            int queuedRequests = 5;

        while (!abortRequested)
        {
            if (listen(create_socket, queuedRequests) == -1)
            {
                throw std::runtime_error("Could not listen for clients!");
            }

            std::cout << "Waiting for connections ..." << '\n';

            // Accept connection setup
            addrlen = sizeof(struct sockaddr_in);
            if ((new_socket = accept(create_socket, (struct sockaddr*)&cliaddress, &addrlen)) == -1)
            {
                if (abortRequested)
                {
                    std::cerr << "Accept error after abort was requested!" << '\n';
                }
                else
                {
                    std::cerr << "Accept error!" << '\n';
                }
                break;
            }

            // Start client
            std::cout << "Client connected from " << inet_ntoa(cliaddress.sin_addr) << ":" << ntohs(cliaddress.sin_port) << '\n';

            // Add new socket to sockets 
            
            //sockets.push_back(new_socket);

            // Add new thread to threads vector
            threads.push_back(std::thread([=] {CommunicateWithClient(&new_socket); }));

            //CommunicateWithClient(&new_socket);

        }

        // Free the descriptor
        if (create_socket != -1)
        {
            if (shutdown(create_socket, SHUT_RDWR) == -1)
            {
                std::cerr << "Shutdown create_socket!" << '\n';
            }
            if (close(create_socket) == -1)
            {
                std::cerr << "Close create_socket!" << '\n';
            }
            create_socket = -1;
        }

        // Iterate over the thread vector
        for (std::thread & th : threads)
        {
            // If thread Object is Joinable then Join that thread.
            if (th.joinable())
                th.join();
        }
        
    }

    void Server::TryCommunicateWithClient(int* socket)
    {
        std::cout << "Socket: " << *socket << " is here!" << '\n';
        int clientSocket = *socket;
        *socket = -1;
        char buffer[BUF];
        int size;

        do
        {
            // Receive message
            size = recv(clientSocket, buffer, BUF - 1, 0);
            // Error handling
            if (size == -1)
            {
                if (abortRequested)
                {
                    std::cerr << "recv error after aborted request!" << '\n';
                }
                else
                {
                    std::cerr << "recv error!" << '\n';
                }
                break;
            }
            else if (size == 0)
            {
                std::cout << "Client closed remote socket!" << '\n';
                break;
            }

            std::cout << "Message received:" << '\n' << buffer << '\n';

            // Handle request
            std::string response = HandleRequest(buffer);

            // Quit without response if QUIT request received
            if (strcmp(response.c_str(), "QUIT\n") == 0)
            {
                std::cout << "Client logged out!" << '\n';
                break;
            }

            // Send response
            SendResponse(&clientSocket, response);
        } while (!abortRequested);

        // Free the descriptor
        if (clientSocket != -1)
        {
            if (shutdown(clientSocket, SHUT_RDWR) == -1)
            {
                std::cerr << "Shutdown new_socket!" << '\n';
            }
            if (close(clientSocket) == -1)
            {
                std::cerr << "Close new_socket!" << '\n';
            }

            clientSocket= -1;
        }
    }

    std::string Server::HandleRequest(const std::string& request)
    {
        // Split the request at the \n into tokens
        std::vector<std::string> tokens = ParseText(request);

        std::string result;

        if (tokens[0] == "LOGIN")
        {
            result = HandleLoginRequest(tokens);
        }
        else if (tokens[0] == "SEND")
        {
            result = HandleSendRequest(tokens);
        }
        else if (tokens[0] == "LIST")
        {
            result = HandleListRequest(tokens);
        }
        else if (tokens[0] == "READ")
        {
            result = HandleReadRequest(tokens);
        }
        else if (tokens[0] == "DEL")
        {
            result = HandleDeleteRequest(tokens);
        }
        else if (tokens[0] == "QUIT")
        {
            result = HandleQuitRequest(tokens);
        }
        else 
        {
            result = HandleBadRequest(tokens);
        }

        return result;
    }

    void Server::SendResponse(int* socket, const std::string& response) const
    {
        std::cout << "Sending response:" << '\n' << response << '\n';

        if (send(*socket, response.c_str(), response.size(), 0) == -1)
        {
            std::cerr << "Send the response failed!" << '\n';
        }
    }

    std::string Server::HandleLoginRequest(const std::vector<std::string>& tokens)
    {
        std::string username = tokens[1];
        std::string password = tokens[2];

        // Check if the user exists
        bool userExists = CheckIfUserExists(username, password);

        return userExists ? "OK\n" : "ERR\n";
    }

    std::string Server::HandleSendRequest(const std::vector<std::string>& tokens)
    {
        std::string sender = tokens[1];
        std::string receiver = tokens[2];
        std::string subject = tokens[3];
        std::string message = tokens[4];

        // Check if entries for sender and receiver exist
        bool senderEntryExists = EntryExistsInPath(sender, mailSpoolDir); 
        bool receiverEntryExists = EntryExistsInPath(receiver, mailSpoolDir); 

        std::string senderPath = mailSpoolDir + "/" + sender;
        std::string receiverPath = mailSpoolDir + "/" + receiver;

        // If not create the directory entries
        if (!senderEntryExists)
        {
            _mutex.lock();
            bool result = std::filesystem::create_directory(senderPath);
            _mutex.unlock();
            if (!result)
            {
                return "ERR\n";
            }
        }
        if (!receiverEntryExists)
        {
            _mutex.lock();
            bool result = std::filesystem::create_directory(receiverPath);
            _mutex.unlock();
            if (!result)
            {
                return "ERR\n";
            }
        }

        // Construct path for message file
        int numberOfEntries = GetNumberOfEntriesInPath(receiverPath);
        std::string filePath = receiverPath + "/" + std::to_string(numberOfEntries);

        // Create the message file consisting of sender, subject and message
        std::ofstream ofs(filePath);
        ofs << sender + '\n' + subject + '\n' + message + '\n';
        ofs.close();

        return "OK\n";
    }

    std::string Server::HandleListRequest(const std::vector<std::string>& tokens)
    {
        std::string result;

        std::string username = tokens[1];

        // Check if entry exists 
        bool entryExists = EntryExistsInPath(username, mailSpoolDir);
        if (!entryExists)
        {
            return "0\n";
        }

        // Construct path of user
        std::string path = mailSpoolDir + "/" + username;

        // Get number of entries in user directory
        int numOfEntries = GetNumberOfEntriesInPath(path);
        if (numOfEntries == 0)
        {
            return "0\n";
        }
        result += std::to_string(numOfEntries) + '\n';

        // Iterate through file entries and extract the subjects
        std::vector<std::string> subjects;

        for (const auto& e : std::filesystem::directory_iterator(path))
        {
            // Read the file content and store it in a variable
            std::ifstream ifs(path + "/" + e.path().filename().string());
            std::stringstream buffer;
            buffer << ifs.rdbuf();
            std::string content = buffer.str();

            // Split the content of the file
            std::vector<std::string> tokens = ParseText(content);

            // Add the subject to subjects
            subjects.push_back(e.path().filename().string() + " " + tokens[1]);
        }

        // Add the subjects to the resulting message
        for (const auto& s : subjects)
        {
            result += (s + '\n');
        }

        return result;
    }

    std::string Server::HandleReadRequest(const std::vector<std::string>& tokens)
    {
        std::string username = tokens[1];
        std::string messageNumber = tokens[2];

        // Check if user directory exists
        bool userExists = EntryExistsInPath(username, mailSpoolDir);
        if (!userExists)
        {
            return "ERR\n";
        }

        // Construct path to user
        std::string path = mailSpoolDir + "/" + username;
    
        // Check if file with message number exists
        bool messageExists = EntryExistsInPath(messageNumber, path);
        if (!messageExists)
        {
            return "ERR\n";
        }

        // Read the file content and store it in a variable
        std::ifstream ifs(path + "/" + messageNumber);
        std::stringstream buffer;
        buffer << ifs.rdbuf();
        std::string content = buffer.str();

        return "OK\n" + content + "\n";
    }

    std::string Server::HandleDeleteRequest(const std::vector<std::string>& tokens)
    {
        std::string username = tokens[1];
        std::string messageNumber = tokens[2];

        // Check if user directory exists
        bool userExists = EntryExistsInPath(username, mailSpoolDir);
        if (!userExists)
        {
            return "ERR\n";
        }

        // Construct path to user
        std::string path = mailSpoolDir + "/" + username;
    
        // Check if file with message number exists
        bool messageExists = EntryExistsInPath(messageNumber, path);
        if (!messageExists)
        {
            return "ERR\n";
        }
        _mutex.lock();
        bool result = std::filesystem::remove(path + "/" + messageNumber);
        _mutex.unlock();
        if (!result)
        {
            return "ERR\n";
        }

        return "OK\n";
    }

    std::string Server::HandleQuitRequest(const std::vector<std::string>& tokens)
    {
        return "QUIT\n";
    }

    std::string Server::HandleBadRequest(const std::vector<std::string>& tokens)
    {
        return "ERR\n";
    }

    std::vector<std::string> Server::ParseText(const std::string& text) const
    {
        std::string tmp = text;
        std::vector<std::string> tokens;
        std::string delimiter = "\n";

        size_t last = 0; 
        size_t next = 0; 

        while ((next = tmp.find(delimiter, last)) != std::string::npos) 
        {   
            tokens.push_back(tmp.substr(last, next-last));

            last = next + 1; 
        } 
        tokens.push_back(tmp.substr(last));

        return tokens;
    }

    bool Server::EntryExistsInPath(const std::string& entry, const std::string& path) const
    {
        for (const auto& e : std::filesystem::directory_iterator(path))
        {
            std::string entryName = e.path().filename().string();

            if (entryName == entry)
            {
                return true;
            }
        }

        return false;
    }

    int Server::GetNumberOfEntriesInPath(const std::string& path) const
    {
        int entries = 0;

        for (const auto& e : std::filesystem::directory_iterator(path))
        {
            entries++;
        }

        return entries;
    }

    bool Server::CheckIfUserExists(const std::string& username, const std::string& password) const
    {
        // TODO Implement LDAP Lookup

        return username == "raoul" && password == "raoul";
    }

}