#include "Client.h"

int main(int argc, char** argv)
{
    TwMailer::Client* client = new TwMailer::Client();
    client->Start(argc, argv);

    return 0;
}