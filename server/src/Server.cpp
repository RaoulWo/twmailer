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
        try
        {
            TryListenForClients(mailSpoolDir);
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

    void Server::CommunicateWithClient(int* socket, const std::string& mailSpoolDir)
    {
        try
        {
            TryCommunicateWithClient(socket, mailSpoolDir);
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

    void Server::TryListenForClients(const std::string& mailSpoolDir)
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
            std::cout << "Client connected from " << inet_ntoa(cliaddress.sin_addr) << ":" << ntohs(cliaddress.sin_port) << '\n';

            // Add new socket to sockets vector
            sockets.push_back(new_socket);
            // Add new thread to threads vector
            threads.push_back(std::thread([=] {CommunicateWithClient(&sockets[sockets.size() - 1], mailSpoolDir); }));

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

    void Server::TryCommunicateWithClient(int* socket, const std::string& mailSpoolDir)
    {
        std::cout << "Socket: " << *socket << "is here!" << '\n';

        char buffer[BUF];
        int size;

        do
        {
            // Receive message
            size = recv(*socket, buffer, BUF - 1, 0);
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

            // TODO Handle received message

            // Quit if QUIT\n received
            if (strcmp(buffer, "QUIT\n") == 0)
            {
                std::cout << "Client logged out!" << '\n';
                break;
            }

            // TODO Handle sending response

        } while (!abortRequested);

        // Free the descriptor
        if (*socket != -1)
        {
            if (shutdown(*socket, SHUT_RDWR) == -1)
            {
                std::cerr << "Shutdown new_socket!" << '\n';
            }
            if (close(*socket) == -1)
            {
                std::cerr << "Close new_socket!" << '\n';
            }

            *socket = -1;
        }
    }

}