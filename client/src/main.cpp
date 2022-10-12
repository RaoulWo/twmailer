#include "Client.h"
#include "Parser.h"

int main(int argc, char** argv)
{
    TwMailer::Parser* parser = new TwMailer::Parser();

    // Parse the command line arguments
    parser->Parse(argc, argv);
    std::string ip = parser->GetIp();
    std::string port = parser->GetPort();

    delete parser;

    TwMailer::Client* client = new TwMailer::Client(ip, port);
    
    // Start the client
    client->Start();

    // Connect to server
    client->ConnectToServer();

    delete client;

    return 0;
}