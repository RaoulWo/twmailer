#ifndef PARSER_H
#define PARSER_H

#include <string>

namespace TwMailer
{

    class Parser
    {
    public:
        // Parse method
        void Parse(int argc, char** argv);

        // Getters
        const std::string& GetPort() const;
        const std::string& GetMailSpoolDir() const;
    private:
        void TryParse(int argc, char** argv);
        void LogUsage() const;

        std::string port;
        std::string mailSpoolDir;
    };

}

#endif