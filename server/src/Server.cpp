#include "Server.h"

#include <arpa/inet.h>
#include <iostream>
#include <stdexcept>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

namespace TwMailer
{
    void Server::Start(std::string port, std::string mailSpoolDir)
    {
        try
        {
            TryStart(port, mailSpoolDir);
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

    void Server::ListenForClients()
    {
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

    void Server::TryStart(std::string port, std::string mailSpoolDir)
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
        address.sin_port = htons(PORT);

        if (bind(create_socket, (struct sockaddr*)&address, sizeof(address)) == -1)
        {
            throw std::runtime_error("Could not address to socket!");
        }
    }

    void Server::TryListenForClients()
    {
        int queuedRequests = 5;
        if (listen(create_socket, queuedRequests) == -1)
        {
            throw std::runtime_error("Could not listen for clients!");
        }

        while (!abortRequested)
        {
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
            std::cout << "Client connected from " << inet_ntoa(cliaddress.sin_addr) << ":" << ntohs(cliaddress.sin_port);

            // TODO

            new_socket = -1;
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
    }

}