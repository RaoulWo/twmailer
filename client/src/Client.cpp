#include "Client.h"

#include <arpa/inet.h>
#include <cstdio>
#include <cstring>
#include <iostream>
#include <netinet/in.h>
#include <stdexcept>
#include <string>
#include <sys/socket.h>
#include <unistd.h>

constexpr int BUF = 1024;
constexpr int PORT = 6543;

namespace TwMailer
{

    void Client::Start(int argc, char** argv)
    {
        try
        {
            TryStart(argc, argv);
        }
        catch(const std::runtime_error& e)
        {
            std::cerr << e.what() << '\n';
            exit(EXIT_FAILURE);
        }
    }

    void Client::TryStart(int argc, char** argv)
    {
        int tcp_socket;
        char buffer[BUF];
        struct sockaddr_in address;
        int size;
        bool isQuit;

        // Create a socket
        // https://man7.org/linux/man-pages/man2/socket.2.html
        // https://man7.org/linux/man-pages/man7/ip.7.html
        // https://man7.org/linux/man-pages/man7/tcp.7.html
        // IPv4, TCP, IP
        if ((tcp_socket = socket(AF_INET, SOCK_STREAM, 0)) == -1)
        {
            throw std::runtime_error("SOCKET ERROR - Socket not created");
        }

        // Initialize address
        std::memset(&address, 0, sizeof(address)); // Initialize storage with 0
        address.sin_family = AF_INET; // IPv4
        // https://man7.org/linux/man-pages/man3/htons.3.html
        address.sin_port = htons(PORT); // Converts byte order from host to network
        // https://man7.org/linux/man-pages/man3/inet_aton.3.html
        if (argc < 2)
        {
            // Converts default IPv4 address (localhost) to binary form
            inet_aton("127.0.0.1", &address.sin_addr);
        }
        else 
        {
            // Converts IPv4 address to binary form
            inet_aton(argv[1], &address.sin_addr);
        }

        // Create a connection
        // https://man7.org/linux/man-pages/man2/connect.2.html
        if (connect(tcp_socket,
                    (struct sockaddr*)&address,
                    sizeof(address)) == -1)
        {
            throw std::runtime_error("CONNECTION ERROR - No server available");
        }

        std::cout << "Connection with server " << inet_ntoa(address.sin_addr) << " established" << '\n';

        // Receive Data
        // https://man7.org/linux/man-pages/man2/recv.2.html
        size = recv(tcp_socket, buffer, BUF - 1, 0);
        if (size == -1)
        {
            std::cerr << "RECEIVE ERROR - Message not received" << '\n';
        }
        else if (size == 0)
        {
            std::cout << "Server closed remote socket" << '\n';
        }
        else
        {
            buffer[size] = '\0';
            std::cout << buffer;
        }

        do
        {
            std::cout << ">> ";
            if (fgets(buffer, BUF, stdin) != nullptr)
            {
                int size = strlen(buffer);
                // Strip \r\n from the end
                if (buffer[size - 2] == '\r' && buffer[size -1] == '\n')
                {
                    size -= 2;
                    buffer[size] = 0;
                }
                // Strip \n from the end
                else if (buffer[size - 1] == '\n')
                {
                    size -= 1;
                    buffer[size] = 0;
                }
                isQuit = strcmp(buffer, "quit") == 0;

                // Send data
                // https://man7.org/linux/man-pages/man2/send.2.html
                if ((send(tcp_socket, buffer, size, 0)) == -1)
                {
                    std::cerr << "SENDING ERROR - Message not sent" << '\n';
                    break;
                }

                // Receive feedback
                size = recv(tcp_socket, buffer, BUF - 1, 0);
                if (size == -1)
                {
                    std::cerr << "RECEIVE ERROR - Feedback not received" << '\n';
                    break;
                }
                else if (size == 0)
                {
                    std::cout << "Server closed remote socket" << '\n';
                    break;
                }
                else 
                {
                    buffer[size] = '\0';
                    std::cout << "<< " << buffer << '\n';
                    if (strcmp("OK", buffer) != 0)
                    {
                        std::cerr << "<< SERVER ERROR - Server error occured, abort" << '\n';
                        break;
                    }
                }
            }

        } while (!isQuit);
        
        // Close the descriptor
        if (tcp_socket != -1)
        {
            if (shutdown(tcp_socket, SHUT_RDWR) == -1)
            {
                // Invalid in case the server is gone already
                std::cerr << "Shutdown tcp_socket" << '\n';
            }
            if (close(tcp_socket) == -1)
            {
                std::cerr << "Close tcp_socket" << '\n';
            }
            tcp_socket = -1;
        }
    }

}