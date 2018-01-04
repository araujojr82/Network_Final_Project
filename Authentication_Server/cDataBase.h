#ifndef _cDataBase_HG_
#define _cDataBase_HG_
#include <string>

/* Standard C++ includes */
#include <stdlib.h>
#include <iostream>

/*
Include directly the different
headers from cppconn/ and mysql_driver.h + mysql_util.h
(and mysql_connection.h). This will reduce your build time!
*/
#include "mysql_connection.h"

#include <cppconn/driver.h>
#include <cppconn/exception.h>
#include <cppconn/resultset.h>
#include <cppconn/statement.h>
#include <cppconn/prepared_statement.h>

using namespace std;

class cDataBase
{
public:
		
	// Returns the UserID or '-1' if it already exists
	// Return other negative numbers for other erros like:
	// -2: Email has more than 64 characters
	// -3: Size of hashedPassword is not 64 characters
	// -4: Size of salt is not 64 characters
	// (...)
	long long insertUser(std::string email, std::string hashedPassword, std::string salt);

	// Returns the UserID or '-1' if the user was not found
	// Writes the hashedPassword, Salt and creationDate back when finding a user
	// Return other negative numbers for other erros like:
	// -2: Email has more than 64 characters
	// -3: Size of hashedPassword is not 64 characters
	// (...)
	long long selectUser(std::string email, std::string &hashedPassword, std::string &salt, std::string &creationDate);

};

#endif // !_cDataBase_HG_