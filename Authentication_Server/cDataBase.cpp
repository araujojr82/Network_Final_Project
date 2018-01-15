#include "cDataBase.h"
#include <vector>

struct UserInfo
{
	long long id = -1;
	std::string username;
	std::string salt;
	std::string password;
	std::string last_login;
};

std::vector<UserInfo> user;
long long gId = 0;

bool findUsername( std::string username )
{
	string resultUsername, queryAsString;

	try {
		sql::Driver *driver;
		sql::Connection *con;
		sql::Statement *stmt;
		sql::ResultSet  *res;

		/* Create a connection */
		driver = get_driver_instance();
		con = driver->connect( "tcp://localhost:3306", "root", "fanshawe" );

		/* Connect to the MySQL database */
		con->setSchema( "lobbyster" );

		stmt = con->createStatement();

		queryAsString = "SELECT username FROM accounts WHERE username = '" + username + "'";
		res = stmt->executeQuery( queryAsString.c_str() );

		while( res->next() )
		{
			resultUsername = res->getString( "username" );
		}

		delete res;
		delete stmt;
		delete con;
	}

	catch( sql::SQLException &e ) {
		cout << "# ERR: SQLException in " << __FILE__;
		cout << "(" << __FUNCTION__ << ") on line " << __LINE__ << endl;
		cout << "# ERR: " << e.what();
		cout << " (MySQL error code: " << e.getErrorCode();
		cout << ", SQLState: " << e.getSQLState() << " )" << endl;

		return false;
	}

	if( resultUsername == username )
	{
		return true;
	}

	return false;
}

long long cDataBase::insertUser( std::string username, std::string password, std::string salt )
{

	if( findUsername( username ) )
	{
		// The user already exists
		return -1;
	}

	try
	{
		sql::Driver *driver;
		sql::Connection *con;
		sql::PreparedStatement  *prep_stmt;

		/* Create a connection */
		driver = get_driver_instance();
		con = driver->connect( "tcp://localhost:3306", "root", "fanshawe" );

		/* Connect to the MySQL database */
		con->setSchema( "lobbyster" );

		prep_stmt = con->prepareStatement( "INSERT INTO accounts( username, salt, password, last_login ) VALUES( ? , ? , ? , CURRENT_TIMESTAMP )" );
		prep_stmt->setString( 1, username );
		prep_stmt->setString( 2, salt );
		prep_stmt->setString( 3, password );
		prep_stmt->execute();

		//prep_stmt = con->prepareStatement( "INSERT INTO user( last_login, creation_date )  VALUES( CURRENT_TIMESTAMP , CURRENT_TIMESTAMP )" );
		//prep_stmt->execute();

		delete prep_stmt;
		delete con;
	}

	catch( sql::SQLException &e )
	{
		cout << "# ERR: SQLException in " << __FILE__;
		cout << "(" << __FUNCTION__ << ") on line " << __LINE__ << endl;
		cout << "# ERR: " << e.what();
		cout << " (MySQL error code: " << e.getErrorCode();
		cout << ", SQLState: " << e.getSQLState() << " )" << endl;
	}

	UserInfo thisUser;
	thisUser.id = ++gId;
	thisUser.password = password;
	thisUser.salt = salt;
	thisUser.last_login = "Some date";
	user.push_back( thisUser );
	return thisUser.id;

}

long long cDataBase::selectUser( std::string username, std::string &password, std::string &salt )
{
	try
	{
		sql::Driver *driver;
		sql::Connection *con;
		sql::Statement *stmt;
		sql::ResultSet  *res;

		string queryAsString, userIdAsString;
		UserInfo thisUser;

		/* Create a connection */
		driver = get_driver_instance();
		con = driver->connect( "tcp://localhost:3306", "root", "fanshawe" );

		/* Connect to the MySQL database */
		con->setSchema( "lobbyster" );

		stmt = con->createStatement();

		queryAsString = "SELECT id, username, salt, password, last_login FROM accounts WHERE username = '" + username + "'";
		res = stmt->executeQuery( queryAsString.c_str() );
		while( res->next() )
		{
			thisUser.id = res->getInt( "id" );
			thisUser.username = res->getInt( "username" );
			thisUser.salt = res->getString( "salt" );
			thisUser.password = res->getString( "password" );
			thisUser.last_login = res->getString( "last_login" );
			
		}

		if( thisUser.username == username )
		{
			userIdAsString = std::to_string( thisUser.id );
			//queryAsString = "SELECT last_login, creation_date FROM user WHERE id = '" + userIdAsString + "'";
			//res = stmt->executeQuery( queryAsString.c_str() );
			//while( res->next() )
			//{
			//	thisUser.creationDate = res->getString( "creation_date" );
			//	thisUser.lastLogin = res->getString( "last_login" );
			//}

			// User exists update the last_login information
			sql::PreparedStatement  *prep_stmt;
			prep_stmt = con->prepareStatement( "UPDATE accounts SET last_login = CURRENT_TIMESTAMP WHERE id = '" + userIdAsString + "'" );
			prep_stmt->execute();

			delete prep_stmt;

			salt = thisUser.salt;
			//creationDate = thisUser.creationDate;
			password = thisUser.password;

			delete stmt;
			delete res;
			delete con;

			return thisUser.id;
		}

		// Authentication failed
		delete stmt;
		delete res;
		delete con;
	}

	catch( sql::SQLException &e )
	{
		cout << "# ERR: SQLException in " << __FILE__;
		cout << "(" << __FUNCTION__ << ") on line " << __LINE__ << endl;
		cout << "# ERR: " << e.what();
		cout << " (MySQL error code: " << e.getErrorCode();
		cout << ", SQLState: " << e.getSQLState() << " )" << endl;

		return -1;
	}

	return -1;
}
