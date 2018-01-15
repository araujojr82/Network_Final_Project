/*Authentication_Server.cpp
================================================================================
|                                                                              |
|   Network Programming Final Assignment                                       |
|   Professor Lukas Gustafson                                                  |
|                                                                              |
|   Developed by Euclides Araujo                                               |
|   Based on previous work done by himself, Benjamin Taylor and Jorge Amengol  |
|                                                                              |
|   This Authentication Server uses a Database in MySQL to authenticate and    |
|   verify the data from the Server. It uses salt to create secure passwords.  |
|                                                                              |
================================================================================
*/

#include <cConnection.h>
#include <InitInfo.h>
#include <string>
#include <MessageIDs.h>
#include <authentication.pb.h>
#include "cUserManager.h"

using namespace std;

// Global variables
cUserManager g_userManager; // The global user manager

int main()
{
	// Init connection info
	InitInfo authServer;
	cout << "Authentication Server initialization process\n"
		<< "Please enter the IP adress of the Chat Server:\n";
	char address[256];
	cin >> address;
	authServer.serverAddr = address;

	// Name of the Authentication Server
	authServer.username = "AuthServer";

	// Create a cCoonection object
	cConnection myConn;

	// Connect to the Chat Server
	myConn.connectToServer( authServer );

	// Validation Process...
	//-------------------------------------------------------------------------

	// Send a validation message with a hash to the Chat Server
	myConn.sendMessage( authServer, VALIDATE_SERVER, "TEMP_HASH" );

	string chatServerMsg;
	chatServerMsg = myConn.getMessages();

	// We didn't hear from the server yet
	while( chatServerMsg == "" && myConn.m_isAlive )
	{
		cout << "The Chat Server did not validate your server yet.\n"
			<< "Try again? (y) or (n):";
		char answer;
		cin >> answer;

		while( answer != 'y' && answer != 'n' )
		{
			cout << "You must type 'y' or 'n':";
			cin >> answer;
		}

		if( answer == 'y' )
		{
			myConn.sendMessage( authServer, VALIDATE_SERVER, "TEMP_HASH" );
		}
		else
		{
			myConn.m_isAlive = false;
			break;
		}

		chatServerMsg = myConn.getMessages();
	}

	// End of Validation Process
	//-------------------------------------------------------------------------

	// Print the Chat Server Message (if there is one)
	cout << chatServerMsg;

	// Main Loop
	while( myConn.m_isAlive )
	{
		chatServerMsg = myConn.getMessages();

		if( chatServerMsg != "" )
		{
			// Remove the MSGID and the trailing '\n'
			char msgID = chatServerMsg.at( 0 );
			string pbStr = chatServerMsg.substr( 1, chatServerMsg.size() - 2 );

			switch( msgID )
			{
			case CREATE_ACCOUNT_WEB:
			{
				// Parse the prococbuff
				authentication::CreateAccountWeb caw;
				caw.ParseFromString( pbStr );

				long long result = g_userManager.createUserAccount( caw.email(), caw.plaintextpassword() );

				if( result != -1 )
				{
					// Return the CREATE_ACCOUNT_WEB_SUCCESS
					authentication::CreateAccountWebSuccess caws;
					caws.set_requestid( caw.requestid() );
					caws.set_userid( result );
					string sendMsg = caws.SerializeAsString();
					myConn.sendMessage( authServer, CREATE_ACCOUNT_WEB_SUCCESS, sendMsg );
				}
				else
				{
					// User account already exists
					// Return the CREATE_ACCOUNT_WEB_FAILURE
					authentication::CreateAccountWebFailure cawf;
					cawf.set_requestid( caw.requestid() );
					cawf.set_thereaseon( authentication::CreateAccountWebFailure_reason_ACCOUNT_ALREADY_EXISTS );
					string sendMsg = cawf.SerializeAsString();
					myConn.sendMessage( authServer, CREATE_ACCOUNT_WEB_FAILURE, sendMsg );
				}
			}
				break; // case CREATE_ACCOUNT_WEB

			case AUTHENTICATE_WEB:
			{
				// Parse the prococbuff
				authentication::AuthenticateWeb aw;
				aw.ParseFromString( pbStr );

				string mCreationDate;
				long long result = g_userManager.authenticateAccount( aw.email(), aw.plaintextpassword(), mCreationDate );
				if( result > 0 ) 
				{
					// The user was authenticated
					authentication::AuthenticateWebSuccess aws;
					aws.set_requestid( aw.requestid() );
					aws.set_userid( result );
					aws.set_creationdate( mCreationDate );
					string sendMsg = aws.SerializeAsString();
					myConn.sendMessage( authServer, AUTHENTICATE_WEB_SUCCESS, sendMsg );
				}
				else {
					// The user was not found or wrong password
					authentication::AuthenticateWebFailure awf;
					awf.set_requestid( aw.requestid() );
					awf.set_thereaseon( authentication::AuthenticateWebFailure_reason_INVALID_CREDENTIALS );
					string sendMsg = awf.SerializeAsString();
					myConn.sendMessage( authServer, AUTHENTICATE_WEB_FAILURE, sendMsg );
				}
			}
				break; // case AUTHENTICATE_WEB

			default:
				break;

			}// switch (msgID)
		}// if (chatServerMsg != "")
	}// while (myConn.isAlive)
	system( "pause" );
}