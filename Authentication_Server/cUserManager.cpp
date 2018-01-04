#include "cUserManager.h"
#include "cDataBase.h"
#include "sha256.h"
#include "Salt.h"

cDataBase dataBase;

long long cUserManager::createUserAccount( std::string email, std::string password )
{
	std::string salt;
	generateSalt( salt );
	std::string hashedPassord = sha256( password + salt );

	return dataBase.insertUser( email, hashedPassord, salt );
}

long long cUserManager::authenticateAccount( std::string email, std::string password, std::string &creationDate )
{
	std::string dbHashedPassord;
	std::string dbSalt;
	long long result = dataBase.selectUser( email, dbHashedPassord, dbSalt, creationDate );

	if( result > 0 )
	{
		// We get a user
		std::string userHashedPassword = sha256( password + dbSalt );
		if( userHashedPassword == dbHashedPassord ) return result;
		else return -4;
	}

	return result;
}