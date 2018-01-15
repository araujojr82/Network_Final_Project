#ifndef _cUserManager_HG_
#define _cUserManager_HG_

#include <string>

class cUserManager 
{
public:

    // Returns the AccountID or '-1' if it already exists    
    long long createUserAccount(std::string username, std::string password);

    // Returns the AccountID or:
    // -1 if the user was not found
    // -4 Password error    
    long long authenticateAccount(std::string username, std::string password );

};

#endif // !_cUserManager_HG_