#include "Client.h"

#include <arpa/inet.h>
#include <iostream>
#include <regex>
#include <stdexcept>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <termios.h>
#include <unistd.h>

namespace TwMailer
{
    Client::Client(std::string ip, int port)
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
        address.sin_port = htons(port);

        inet_aton(ip.c_str(), &address.sin_addr);
    }

    void Client::TryConnectToServer()
    {
        if (connect(create_socket, (struct sockaddr*)&address, sizeof(address)) == -1)
        {
            throw std::runtime_error("Connection error, no server is available!");
        }

        std::cout << "Connection with server " << inet_ntoa(address.sin_addr) << " established!" << '\n';

        // Store the username and password
        StoreUsername();
        StorePassword();

        // Construct login request
        std::string request = ConstructLoginRequest();

        // Send login request
        SendRequest(request);

        do 
        {
            // Reset the buffer
            memset(buffer, 0, sizeof(buffer));

            // Receive response
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
                // Handle response
                try
                {
                    HandleResponse(buffer);
                }
                catch(const std::exception& e)
                {
                    std::cerr << e.what() << '\n';
                    system("read REPLY");
                }

                if (isLoggedIn)
                {
                    // Handle user input
                    char c = GetUserMenuInput();
                    
                    // Construct request
                    std::string request = ConstructRequest(c);
                    
                    // Send request
                    SendRequest(request);
                }
                else if (failedLoginAttempts < 3)
                {
                    // Store the username and password
                    StoreUsername();
                    StorePassword();

                    // Construct login request
                    std::string request = ConstructLoginRequest();

                    // Send login request
                    SendRequest(request);
                }
                else 
                {
                    // TODO Timeout
                    std::cout << "3rd login failure, quitting application!" << '\n';

                    exit(EXIT_FAILURE);
                }
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

    void Client::StoreUsername()
    {
        std::string name;
        
        std::regex pattern("[a-z0-9]+");

        do 
        {
            system("clear");

            std::cout << "Enter your username (a-z, 0-9) 4-8 characters:" << '\n';
            std::cin >> name;

            system("clear");

            if (name.size() < 4)
            {
                std::cerr << "The username cannot be shorter than 4 characters!" << '\n';
            }
            else if (name.size() > 8)
            {
                std::cerr << "The username cannot be longer than 8 characters!" << '\n';
            }
            else if (!std::regex_match(name, pattern))
            {
                std::cerr << "The username must only contain the letters a-z and the digits 0-9!" << '\n';
            }
            else 
            {
                username = name;
                break;
            }

            std::cout << "Press any key to continue!" << '\n';
            system("read REPLY");

        } while (true);
    }

    void Client::StorePassword()
    {
        // Get the return character of the 'system("read REPLY")' call above
        getchar();

        password = getPassword(true);
    }

    int getch() 
    {
        int ch;
        struct termios t_old, t_new;

        tcgetattr(STDIN_FILENO, &t_old);
        t_new = t_old;
        t_new.c_lflag &= ~(ICANON | ECHO);
        tcsetattr(STDIN_FILENO, TCSANOW, &t_new);

        ch = getchar();

        tcsetattr(STDIN_FILENO, TCSANOW, &t_old);
        return ch;
    }

    std::string getPassword(bool showAsterisk)
    {
        const char BACKSPACE = 127;
        const char RETURN = 10;

        std::string password;
        unsigned char ch = 0;

        std::cout << "Enter your password:" << '\n';

        while ((ch = getch()) != RETURN)
        {
            if (ch == BACKSPACE)
            {
                if (password.length() != 0)
                {
                    if (showAsterisk)
                    {
                        std::cout <<"\b \b";

                    }
                    password.resize(password.length()-1);
                }
            }
            else
            {
                password += ch;
                if (showAsterisk)
                {
                    std::cout << '*';
                }
            }
        }
        std::cout << '\n';

        return password;
    }


    void Client::PrintMenu() const
    {
        system("clear");

        std::cout << "Welcome, " << username << "!" << '\n' << '\n';
        std::cout << "(S)end (L)ist (R)ead (D)elete (Q)uit" << '\n';
    }

    char Client::GetUserMenuInput()
    {
        char c;
        do
        {
            // Render the menu screen
            PrintMenu();

            // Get the client input
            c = getchar();

        } while (!IsValidMenuChar(c));

        return c;
    }

    bool Client::IsValidMenuChar(char c) const
    {
        return c == 'S' || c == 's' || c == 'L' || c == 'l' || c == 'R' || 
            c == 'r' || c == 'D' || c == 'd' || c == 'Q' || c == 'q';
    }

    std::string Client::ConstructRequest(char c) const
    {
        std::string request = "";

        switch (c)
        {
            case 'S':
            case 's':
                request = ConstructSendRequest();
                break;
            case 'L':
            case 'l':
                request = ConstructListRequest();
                break;
            case 'R':
            case 'r':
                request = ConstructReadRequest();
                break;
            case 'D':
            case 'd':
                request = ConstructDeleteRequest();
                break;
            case 'Q':
            case 'q':
                request = ConstructQuitRequest();
                break;
            default:
                exit(EXIT_FAILURE);
        }

        return request;
    }

    std::string Client::ConstructLoginRequest() const
    {
        return "LOGIN\n" + username + '\n' + password + '\n';
    }

    std::string Client::ConstructSendRequest() const
    {
        std::string receiver = GetReceiver();
        std::string subject = GetSubject();
        std::string message = GetMessage();

        return "SEND\n" + username + '\n' + receiver + '\n' + subject + '\n' + message + '\n' + ".\n";
    }

    std::string Client::ConstructListRequest() const
    {
        return "LIST\n" + username + '\n';
    }

    std::string Client::ConstructReadRequest() const
    {
        return "READ\n" + username + '\n' + std::to_string(GetMsgNum()) + '\n';
    }

    std::string Client::ConstructDeleteRequest() const
    {
        return "DEL\n" + username + '\n' + std::to_string(GetMsgNum()) + '\n';
    }

    std::string Client::ConstructQuitRequest() const
    {
        return "QUIT\n";
    }

    void Client::SendRequest(const std::string& request) const
    {
        std::cout << "Sending request:" << '\n' << request << '\n';

        if (send(create_socket, request.c_str(), request.size(), 0) == -1)
        {
            std::cerr << "Send the request failed!" << '\n';
        }
    }

    std::string Client::GetReceiver() const
    {
        std::string receiver = "";
        std::string input;

        std::regex pattern("[a-z0-9]+");

        do 
        {
            system("clear");

            std::cout << "Enter the name of the receiver:" << '\n';
            std::cin >> input;

            system("clear");

            if (input.size() < 4 || input.size() > 8 || !std::regex_match(input, pattern))
            {
                std::cerr << "Invalid username!" << '\n';
            }
            else 
            {
                receiver = input;
                break;
            }
        } while (true);

        return receiver;
    }

    std::string Client::GetSubject() const
    {
        std::string subject = "";
        std::string input;

        do 
        {
            system("clear");

            std::cout << "Enter the message subject (max. 80 characters):" << '\n';
            std::getline(std::cin, input);

            system("clear");

            if (input.size() == 0)
            {
                std::cerr << "Please enter the message subject!" << '\n';
            }
            else if (input.size() > 80)
            {
                std::cerr << "Only a maximum of 80 characters allowed!" << '\n';
            }
            else
            {
                subject = input;
                break;
            }
        }
        while (true);

        return subject;
    }

    std::string Client::GetMessage() const
    {
        std::string message = "";
        std::string input;

        do 
        {
            system("clear");

            std::cout << "Enter the message:" << '\n';
            std::getline(std::cin, input);

            system("clear");

            if (input.size() == 0)
            {
                std::cerr << "Please enter the message!" << '\n';
            }
            else
            {
                message = input;
                break;
            }
        }
        while (true);

        return message;
    }

    int Client::GetMsgNum() const
    {
        std::string input = "";
        int msgNum;

        do
        {
            system("clear");

            std::cout << "Enter the message number:" << '\n';
            std::cin >> input;

            system("clear");

            if (input.size() == 0)
            {
                std::cerr << "Please enter a message number!" << '\n';
                std::cout << "Press any key to continue!" << '\n';
                system("read REPLY");
            }
            else
            {
                try
                {
                    msgNum = std::stoi(input);
                    break;
                }
                catch(const std::exception& e)
                {
                    std::cerr << "Could not convert user input to message number!" << '\n';
                    std::cout << "Press any key to continue!" << '\n';
                    system("read REPLY");
                }
            }
        } while (true);

        return msgNum;
    }

    void Client::HandleResponse(const std::string& response)
    {
        if (response.size() == 0)
        {
            throw std::runtime_error("Invalid server response!");
        }

        // Split the response into tokens with '\n' as delimiter
        std::vector<std::string> tokens = ParseResponse(response);

        if (tokens[0] == "OK")
        {
            if (!isLoggedIn) isLoggedIn = true;

            for (const auto& token : tokens)
            {
                std::cout << token << '\n';
            }
        }
        else if (tokens[0] == "ERR")
        {
            if (!isLoggedIn) failedLoginAttempts++;
            
            std::cout << "ERROR" << '\n';
        }
        else
        {
            int messageCount;
            try
            {
                messageCount = std::stoi(tokens[0]);
            }
            catch(const std::exception& e)
            {
                std::cerr << e.what() << '\n';
            }

            if (messageCount <= 0)
            {
                std::cout << "You've received 0 messages!" << '\n';
            }
            else
            {
                std::cout << "You've received " << messageCount << (messageCount == 1 ? " message" : " messages!") << '\n';
                for (long unsigned i = 1; i < tokens.size(); i++)
                {
                    std::cout << tokens[i] << '\n';
                }
            }
        }

        std::cout << "Press enter button to continue!" << '\n';
        system("read REPLY");
    }

    std::vector<std::string> Client::ParseResponse(const std::string& response) const
    {
        std::vector<std::string> tokens;
        std::string delimiter = "\n";

        size_t last = 0; 
        size_t next = 0; 

        while ((next = response.find(delimiter, last)) != std::string::npos) 
        {   
            tokens.push_back(response.substr(last, next-last));

            last = next + 1; 
        } 
        tokens.push_back(response.substr(last));

        return tokens;
    }

}