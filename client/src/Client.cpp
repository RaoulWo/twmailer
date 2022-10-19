#include "Client.h"

#include <iostream>
#include <stdexcept>

#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

namespace TwMailer
{
    Client::Client(std::string ip, std::string port)
    { 
        this->ip = ip;
        this->port = port;
    }

    void Client::Start()
    {
        try
        {
            TryStart();
        }
        catch(const std::runtime_error& e)
        {
            std::cerr << e.what() << '\n';
            exit(EXIT_FAILURE);
        }
        catch(...)
        {
            std::cerr << "An unexpected error occured when trying to start the client!" << '\n';
            exit(EXIT_FAILURE);
        }
    }

    void Client::ConnectToServer()
    {
        try
        {
            TryConnectToServer();
        }
        catch(const std::runtime_error& e)
        {
            std::cerr << e.what() << '\n';
            exit(EXIT_FAILURE);
        }
        catch(...)
        {
            std::cerr << "An unexpected error occured when trying to connect to the server!" << '\n';
            exit(EXIT_FAILURE);
        }
    }

    void Client::TryStart()
    {
        if ((create_socket = socket(AF_INET, SOCK_STREAM, 0)) == -1)
        {
            throw std::runtime_error("Socket could not be opened!");
        }

        memset(&address, 0, sizeof(address));
        address.sin_family = AF_INET;
        address.sin_port = htons(std::stoi(port));

        inet_aton(ip.c_str(), &address.sin_addr);
    }

    void Client::TryConnectToServer()
    {
        if (connect(create_socket, (struct sockaddr*)&address, sizeof(address)) == -1)
        {
            throw std::runtime_error("Connection error, no server is available!");
        }

        std::cout << "Connection with server " << inet_ntoa(address.sin_addr) << " established!" << '\n';

        do 
        {
            size = recv(create_socket, buffer, BUF - 1, 0);
            if (size == -1)
            {
                std::cerr << "recv error!" << '\n';
                break;
            }
            else if (size == 0)
            {
                std::cout << "The server closed the remote socket!" << '\n';
                break;
            }
            else 
            {
                // TODO
            }
        }
        while (!isQuit);

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