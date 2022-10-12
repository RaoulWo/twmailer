#include "Parser.h"

#include <iostream>
#include <stdexcept>

namespace TwMailer
{

    void Parser::Parse(int argc, char** argv)
    {
        try
        {
            TryParse(argc, argv);
        }
        catch(const std::runtime_error& e)
        {
            std::cerr << e.what() << '\n';
            LogUsage();
            exit(EXIT_FAILURE);
        }
        catch(...)
        {
            std::cerr << "An unexpected error occured when trying to parse the arguments!" << '\n';
            LogUsage();
            exit(EXIT_FAILURE);
        }
        
    }

    const std::string& Parser::GetIp() const 
    {
        return ip;
    }

    const std::string& Parser::GetPort() const
    {
        return port;
    }

    void Parser::TryParse(int argc, char** argv)
    {
        if (argc != 3)
        {
            throw std::runtime_error("Incorrect number of arguments!");
        }

        ip = argv[1];
        port = argv[2];        
    }

    void Parser::LogUsage() const
    {
        std::cerr << "Usage Client: ./twmailer <ip> <port>" << '\n';
    }

}