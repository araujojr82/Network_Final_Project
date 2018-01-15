#ifndef _InitInfo_HG_
#define _InitInfo_HG_

// This struct holds information about the active user
struct InitInfo 
{
    char* serverAddr;       // The server address
    std::string room;       // The room to be connected
    std::string username;
    //std::string email;
    std::string password;
};

#endif // !_cInitParameters_HG