#include "Parser.h"
#include "Server.h"

#include <iostream>
#include <signal.h>

// Function declaration
void SignalHandler(int signal);

// Global server
TwMailer::Server* server = new TwMailer::Server();

int main(int argc, char** argv)
{
    TwMailer::Parser* parser = new TwMailer::Parser();

    // Parse the command line arguments
    parser->Parse(argc, argv);
    int port = parser->GetPort();
    std::string mailSpoolDir = parser->GetMailSpoolDir();

    delete parser;


    if (signal(SIGINT, SignalHandler))
    {
        std::cerr << "The signal cannot be registered!" << '\n';
        exit(EXIT_FAILURE);
    }

    // Start the server
    server->Start(port);

    // List for clients
    server->ListenForClients(mailSpoolDir);

    delete server;

    return 0;
}

void SignalHandler(int signal)
{
    if (signal == SIGINT)
    {
        std::cout << "Abort requested ..." << '\n';
        server->Abort();
    }
    else
    {
        exit(signal);
    }
}