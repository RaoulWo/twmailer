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

    int Parser::GetPort() const 
    {
        return port;
    }

    const std::string& Parser::GetMailSpoolDir() const
    {
        return mailSpoolDir;
    }

    void Parser::TryParse(int argc, char** argv)
    {
        if (argc != 3)
        {
            throw std::runtime_error("Incorrect number of arguments!");
        }

        try
        {
            port = std::stoi(argv[1]);
        }
        catch(const std::exception& e)
        {
            throw std::runtime_error("Could not convert port to number!");
        }
        
        mailSpoolDir = argv[2];        
    }

    void Parser::LogUsage() const
    {
        std::cerr << "Usage Client: ./twmailer-server <port> <mail-spool-directory-name>" << '\n';
    }

}