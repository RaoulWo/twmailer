#ifndef CLIENT_H
#define CLIENT_H

namespace TwMailer 
{

    class Client
    {
    public:
        void Start(int argc, char** argv);

    private:
        void TryStart(int argc, char** argv);
    };

}

#endif