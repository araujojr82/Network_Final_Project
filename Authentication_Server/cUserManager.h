#ifndef _cUserManager_HG_
#define _cUserManager_HG_

#include <string>

class cUserManager 
{
public:

    // Returns the UserID or '-1' if it already exists
    // Return other negative numbers for other erros like:
    // -2: Email has more than 64 characters
    // -3: Size of hash is not 64 characters
    // (...)
    long long createUserAccount(std::string email, std::string password);

    // Returns the UserID or:
    // -1 if the user was not found
    // -4 Password error
    // Writes the creationDate back if user and password are ok
    // Return other negative numbers for other erros like:
    // -2: Email has more than 64 characters
    // -3: Size of hash is not 64 characters
    // (...)
    long long authenticateAccount(std::string email, std::string password, std::string &creationDate);

};

#endif // !_cUserManager_HG_