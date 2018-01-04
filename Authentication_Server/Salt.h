#ifndef _SALT_HG_
#define _SALT_HG_

#include <stdio.h>
#include <tchar.h>
#include <windows.h>
#include <Wincrypt.h>
#include <string>

/**
    Generates a salt and passes it through a string
*/
void generateSalt(std::string &salted);

#endif // !_SALT_HG_

