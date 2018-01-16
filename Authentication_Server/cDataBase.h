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
		
	// Returns the accountID or '-1' if it already exists
	// Return other negative numbers for other erros like:	
	long long insertUser(std::string username, std::string password, std::string salt);

	// Returns the accountID or '-1' if the user was not found
	// Writes the password and salt back when finding a user	
	long long selectUser(std::string username, std::string &password, std::string &salt, std::string &lastLogin );

};

#endif // !_cDataBase_HG_