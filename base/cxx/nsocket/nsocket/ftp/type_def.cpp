#include "type_def.h"

#include <algorithm>
#include <sstream>

namespace nsocket
{
namespace ftp
{
Command parseCommand(const std::string& cmd)
{
    std::string upperCmd = cmd;
    std::transform(upperCmd.begin(), upperCmd.end(), upperCmd.begin(), ::toupper);
    if ("USER" == upperCmd)
    {
        return Command::USER;
    }
    else if ("PASS" == upperCmd)
    {
        return Command::PASS;
    }
    else if ("QUIT" == upperCmd)
    {
        return Command::QUIT;
    }
    else if ("PORT" == upperCmd)
    {
        return Command::PORT;
    }
    else if ("PASV" == upperCmd)
    {
        return Command::PASV;
    }
    else if ("TYPE" == upperCmd)
    {
        return Command::TYPE;
    }
    else if ("RETR" == upperCmd)
    {
        return Command::RETR;
    }
    else if ("STOR" == upperCmd)
    {
        return Command::STOR;
    }
    else if ("LIST" == upperCmd)
    {
        return Command::LIST;
    }
    else if ("NLST" == upperCmd)
    {
        return Command::NLST;
    }
    else if ("CWD" == upperCmd)
    {
        return Command::CWD;
    }
    else if ("CDUP" == upperCmd)
    {
        return Command::CDUP;
    }
    else if ("PWD" == upperCmd)
    {
        return Command::PWD;
    }
    else if ("MKD" == upperCmd)
    {
        return Command::MKD;
    }
    else if ("RMD" == upperCmd)
    {
        return Command::RMD;
    }
    else if ("DELE" == upperCmd)
    {
        return Command::DELE;
    }
    else if ("RNFR" == upperCmd)
    {
        return Command::RNFR;
    }
    else if ("RNTO" == upperCmd)
    {
        return Command::RNTO;
    }
    else if ("SYST" == upperCmd)
    {
        return Command::SYST;
    }
    else if ("NOOP" == upperCmd)
    {
        return Command::NOOP;
    }
    return Command::UNKNOWN;
}
} // namespace ftp
} // namespace nsocket
