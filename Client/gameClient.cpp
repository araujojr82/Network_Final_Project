/*gameClient.cpp
================================================================================
|                                                                              |
|   Network Programming Final Assignment                                       |
|   Professor Lukas Gustafson                                                  |
|                                                                              |
|   Developed by Euclides Araujo                                               |
|   Based on previous work done by himself, Benjamin Taylor and Jorge Amengol  |
|                                                                              |
|   This Client is the user interfaces and send and receive messages from      |
|   the Server. Is uses the Basic Protocol Structure described in the main     |
|   project folder, however, for incoming messages, it only reads the          |
|   messages as is in the receiving buffer.                                    |
|                                                                              |
|   The connection part of the Client was based on the code given in class by  |
|   Professor Lukas Gustafson                                                  |
================================================================================
*/

#include <iostream>
#include <sstream>
#include <string>
#include <iomanip>
#include <vector>
#include "cConnection.h"
#include "InitInfo.h"
#include <MessageIDs.h>
#include "lobby.h"

using namespace std;

#define PROMPT "->"

// Function signatures
void checkRoom( InitInfo&, string, vector<string>& );
void checkLeftRoom( InitInfo&, string, vector<string>& );
void updateLobbyList( InitInfo&, string, cConnection& );
int readCase( string&, InitInfo&, vector<string>&, cConnection& );

// Global variables
vector<Lobby> g_vecServerLobbies;	// The List of existing Lobbies

int main()
{
	InitInfo user;  // user struct data

	cout << "+===============================================================================+" << endl;
	cout << "|                 _          _     _               _                            |" << endl;
	cout << "|                | |    ___ | |__ | |__  _   _ ___| |_ ___ _ __                 |" << endl;
	cout << "|                | |   / _ \\| '_ \\| '_ \\| | | / __| __/ _ \\ '__|                |" << endl;
	cout << "|                | |__| (_) | |_) | |_) | |_| \\__ \\ ||  __/ |                   |" << endl;
	cout << "|                |_____\\___/|_.__/|_.__/ \\__, |___/\\__\\___|_|                   |" << endl;
	cout << "|                                        |___/                                  |" << endl;
	cout << "+===============================================================================+" << endl;
	cout << "Please type in the Game Server's ip adress:" << endl
		<< PROMPT;

	// Get the server address
	char serverAddr[MAX_ARRAY_SIZE];
	cin >> serverAddr;
	user.serverAddr = serverAddr;

	// Read the user's first name
	cout << "Please type in the Game Server's port (default 5000):" << endl 
	<< PROMPT;
	char serverPort[MAX_PORT_SIZE];
	cin >> serverPort;
	user.serverPort = serverPort;

	// Read the user's first name
	cout << "Please type in your username: " << endl
	<< PROMPT;
	string firstName;
	cin >> firstName;
	user.username = firstName;

	// Try to make a connection
	cConnection myConn;
	myConn.connectToServer( user );

	if( myConn.m_isAlive )
	{
		system( "cls" );
		cout << "===============================================================================" << endl;
		cout << "|                 _          _     _               _                            |" << endl;
		cout << "|                | |    ___ | |__ | |__  _   _ ___| |_ ___ _ __                 |" << endl;
		cout << "|                | |   / _ \\| '_ \\| '_ \\| | | / __| __/ _ \\ '__|                |" << endl;
		cout << "|                | |__| (_) | |_) | |_) | |_| \\__ \\ ||  __/ |                   |" << endl;
		cout << "|                |_____\\___/|_.__/|_.__/ \\__, |___/\\__\\___|_|                   |" << endl;
		cout << "|                                        |___/                                  |" << endl;
		cout << "================================================================================" << endl;
		cout << user.username << ", you are now connected to the server." << endl
			<< "You can type -help for a list of commands." << endl << endl;
		system( "pause" );
	}
	else
	{
		cout << "Something went wrong. We didn't connected to the server!" << endl;
		Sleep( 6000 );
	}

	// Loop variables
	string myMessage;               // The message that the this client typed
	string srvMessage;              // The message that comes from the server
	string chatBuffer;              // Stores all the received messages
	vector<string> connectedRooms;  // Stores the connected rooms locally

	// Main chat loop
	while( myConn.m_isAlive )
	{
		// Get new messages
		srvMessage = myConn.getMessages();

		// Is there a new message from Server?
		if( srvMessage != "" )
		{
			checkRoom( user, srvMessage, connectedRooms );
			checkLeftRoom( user, srvMessage, connectedRooms );
			updateLobbyList( user, srvMessage, myConn );
			chatBuffer += srvMessage;
		}

		// Format the chat screen - The best way we could so far! :/
		system( "cls" );
		for( int i = 0; i < 30; i++ )
		{
			cout << endl;
		}

		cout << chatBuffer;
		cout << "________________________________________________________________________________" << endl;
		cout << "Press Enter to update messages. Type -help for help." << endl;		
		cout << user.username << " you are currently connected to: " << endl;
		for( int i = 0; i < connectedRooms.size(); i++ )
		{
			if( i > 0 ) cout << ", " << connectedRooms[i];
			else cout << connectedRooms[i];			
		}
		cout << endl;
		cout << "================================================================================" << endl;

		// Get input
		cout << PROMPT;
		getline( cin, myMessage );

		// Treat the input
		if( readCase( myMessage, user, connectedRooms, myConn ) )
			break;  // Received an exit signal

	}//!while (myConn.isAlive)

	// There is no connection anymore. Close it!
	myConn.closeConnection();
	system( "cls" );
	cout << "================================================================================" << endl;
	cout << "                        Thanks for using Lobbyster!" << endl;
	cout << "================================================================================" << endl;
	Sleep( 4000 );


	return 0;
}

//-----------------------------------------------------------------------------
// Helper Functions
//
// Check if we were connected to a room by the Game Server. If yes, we update our local rooms vector.
void checkRoom( InitInfo& user, string message, vector<string>& connectedRooms )
{
	// The beggining og the message should be:
	string msgHead = "Game Server->" + user.username + " has connected to ";

	// Is it big enough?
	if( message.size() > msgHead.size() )
	{
		// Check if the begginings match
		for( int i = 0; i < msgHead.size(); i++ )
		{
			if( message.at( i ) != msgHead.at( i ) )
				return;
		}

		// Format the string to contain only the room name
		string theRoom = message.substr( msgHead.size(), message.size() - msgHead.size() - 1 );
		connectedRooms.push_back( theRoom );
	}
}

// Check if we were removed from a lobby by the Game Server. If yes, we update our local lobby vector.
void checkLeftRoom( InitInfo& user, string message, vector<string>& connectedRooms )
{
	// The beggining og the message should be:
	string msgHead = "Game Server->" + user.username + " has left ";

	// Is it big enough?
	if( message.size() > msgHead.size() )
	{
		// Check if the begginings match
		for( int i = 0; i < msgHead.size(); i++ )
		{
			if( message.at( i ) != msgHead.at( i ) )
				return;
		}
		
		// Format the string to contain only the room name
		int breakPosition = message.find_first_of( '\n' );
		string theRoom = message.substr( msgHead.size(), breakPosition - msgHead.size() );

		vector<string> theNewRooms;
		theNewRooms.clear();
		for( int i = 0; i < connectedRooms.size(); i++ )
		{
			if( connectedRooms[i] != theRoom )
				theNewRooms.push_back( connectedRooms[i] );
		}
		connectedRooms.clear();
		connectedRooms = theNewRooms;
	}
}

// Check if there's any lobby created on the server.
void updateLobbyList( InitInfo& user, string message, cConnection& myConn )
{
	// The beggining og the message should be:
	string msgHead = "Game Server->List of available lobbies:";

	string lobbyNum;

	// Is it big enough?
	if( message.size() > msgHead.size() )
	{
		// Check if the begginings match
		for( int i = 0; i < msgHead.size(); i++ )
		{
			if( message.at( i ) != msgHead.at( i ) )
				return;
		}

		string numberOfLobbies = message.substr( msgHead.size(), message.size() - msgHead.size() - 1 );

		int size = stoi( numberOfLobbies );
		//serverLobbies.resize( size );
		g_vecServerLobbies.clear();

		if( size != 0 )
		{
			string lobbyNum = to_string( 0 );

			myConn.sendMessage( user, GET_LOBBY_NUM, lobbyNum ); //Ask for the 1st lobby info

			string newSrvMessage;

			for( int i = 0; i != size; i++ )
			{
				// Get new messages
				newSrvMessage = myConn.getMessages();
				
				// Is there a new message from Server?
				if( newSrvMessage != "" )
				{
					msgHead = "Game Server->Lobby";

					// Is it big enough?
					if( newSrvMessage.size() > msgHead.size() )
					{
						// Check if the begginings match
						for( int i = 0; i < msgHead.size(); i++ )
						{
							if( newSrvMessage.at( i ) != msgHead.at( i ) )
								return;
						}

						string lInfo = newSrvMessage.substr( msgHead.size()+2, newSrvMessage.size() - msgHead.size() - 1 );

						Lobby newLobby;
						
						newLobby.gameMap = ( gameMaps )stoi( lInfo.substr( 0, lInfo.find( ",", 0 ) ) );
						lInfo = lInfo.substr( lInfo.find( ",", 0 ) + 1, lInfo.length() ); // new string

						newLobby.lobbyName = lInfo.substr( 0, lInfo.find( ",", 0 ) );
						lInfo = lInfo.substr( lInfo.find( ",", 0 ) + 1, lInfo.length() ); // new string

						newLobby.gameMode = (gameModes) stoi( lInfo.substr( 0, lInfo.find( ",", 0 ) ) );
						lInfo = lInfo.substr( lInfo.find( ",", 0 ) + 1, lInfo.length() ); // new string					    

						newLobby.openSpots = stoi( lInfo.substr( 0, lInfo.find( ",", 0 ) ) );
						lInfo = lInfo.substr( lInfo.find( ",", 0 ) + 1, lInfo.length() ); // new string

						newLobby.totalSpots = stoi( lInfo.substr( 0, lInfo.find( ",", 0 ) ) );
						lInfo = lInfo.substr( lInfo.find( ",", 0 ) + 1, lInfo.length() ); // new string

						newLobby.hostName = lInfo.substr( 0, lInfo.find( "\n", 0 ) );

						g_vecServerLobbies.push_back( newLobby );
					}
				}

				lobbyNum = to_string( i + 1 );

				if( i + 1 < size )	// Ask for the next lobby info
				{
					myConn.sendMessage( user, GET_LOBBY_NUM, lobbyNum );
				}

			}

			// Print the lobby list
			cout << "| Map Name           | Lobby Name         | Game Mode          | Spots (O/T) | Host Name          |" << endl;			
			
			for( int j = 0; j != g_vecServerLobbies.size(); j++ )
			{
				cout << "| " 
					<< setw( 18 ) << left << getServerMapText(g_vecServerLobbies[j].gameMap ) << setw( 2 ) << " | "
					<< setw( 18 ) << g_vecServerLobbies[j].lobbyName << setw( 2 ) << " | "
					<< setw( 18 ) << getGameModeText( g_vecServerLobbies[j].gameMode ) << setw( 2 ) << " | "
					<< "( " << setw( 2 ) << to_string( g_vecServerLobbies[j].openSpots )
					<< " / " << setw( 2 ) << to_string( g_vecServerLobbies[j].totalSpots )
					<< " )" << setw( 2 ) << " | "
					<< setw( 18 ) << g_vecServerLobbies[j].hostName << " | " << endl;
			}
			system( "pause" );
		}
	}
}

// Reads and treats all messages received from the server
int readCase( string& myMessage, InitInfo& user, vector<string>& connectedRooms, cConnection& myConn )
{
	// HELP
	if( myMessage == "-help" )
	{
		system( "cls" );

		for( int i = 0; i < 30; i++ )
		{
			cout << endl;
		}

		cout << "Those are the available commands:" << endl
			<< "-new    : Creates a new user" << endl
			<< "-auth   : Starts the authentication process for a user;" << endl
			<< "-list   : List the lobbies availables on the Server;" << endl
			<< "-create : Creates a lobby in the Game Server ;" << endl
			<< "-join   : Joins a lobby in the Game Server;" << endl
			<< "-leave  : Leaves a specific lobby from the Game Server;" << endl
			<< "-exit   : Exit the client application." << endl
			<< "================================================================================" << endl
			<< PROMPT;

		// Get the new command
		getline( cin, myMessage );
	}

	// LIST Lobbys
	if( myMessage == "-list" )
	{
		myMessage = "";
		myConn.sendMessage( user, LIST_LOBBY, "" );
	}

	// NEW user
	if( myMessage == "-new" )
	{
		string answer;
		myMessage = "";
		cout << "Please type in your password" << endl;
		cout << PROMPT;
		cin >> answer;
		while( answer.size() < 6 )
		{
			cout << "Password should be at least 6 characters. Try again!" << endl;
			cout << PROMPT;
			cin >> answer;
		}
		user.password = answer;
		myConn.sendMessage( user, CREATE_ACCOUNT, "" );
	}

	// AUTHENTICATION
	if( myMessage == "-auth" )
	{
		string answer;
		myMessage = "";
		cout << "Please type in your password" << endl;
		cout << PROMPT;
		cin >> answer;
		while( answer.size() < 6 )
		{
			cout << "Password should be at least 6 characters. Try again!" << endl;
			cout << PROMPT;
			cin >> answer;
		}
		user.password = answer;
		myConn.sendMessage( user, AUTHENTICATE, "" );
	}

	// JOIN a room
	if( myMessage == "-create" )
	{
		string mapName, lobbyName, gameMode, totalSpots;
		myMessage = "";
		cout << "Please type in the name of the lobby" << endl;
		cout << PROMPT;
		cin >> lobbyName;
		cout << "Select Map ( Type in the number )" << endl;
		cout << "Maps available:" << endl;
		for( int i = 0; i < vecGameMaps.size(); i++ )
		{
			cout << i+1 << ": " << getServerMapText( vecGameMaps[i] ) << endl;
		}	
		cout << PROMPT;
		cin >> mapName;
		cout << "Select Game Mode ( Type in the number )" << endl;
		for( int i = 0; i < vecGameModes.size(); i++ )
		{
			cout << i+1 << ": " << getGameModeText( vecGameModes[i] ) << endl;
		}
		cout << PROMPT;
		cin >> gameMode;
		cout << "Choose Max number of players ( MAX: " << MAX_PLAYERS << " )" << endl;
		cout << PROMPT;
		cin >> totalSpots;

		string lobbyInfo = mapName + "," + lobbyName + "," + gameMode + "," + totalSpots;
		
		myConn.sendMessage( user, CREATE_ROOM, lobbyInfo );
	}

	// JOIN a room
	if( myMessage == "-join" )
	{
		myMessage = "";
		cout << "Please type in the name of the lobby" << endl;
		cout << PROMPT;
		string answer;
		cin >> answer;
		user.room = answer;
		myConn.sendMessage( user, JOIN_ROOM, "" );
	}

	// LEAVE a room
	if( myMessage == "-leave" )
	{
		myMessage = "";
		cout << "Please type in the name of the lobby" << endl;
		cout << PROMPT;
		string answer;
		cin >> answer;
		user.room = answer;

		vector<string> theNewRooms;
		theNewRooms.clear();
		for( int i = 0; i < connectedRooms.size(); i++ )
		{
			if( connectedRooms[i] != user.room )
				theNewRooms.push_back( connectedRooms[i] );
		}
		connectedRooms.clear();
		connectedRooms = theNewRooms;
		myConn.sendMessage( user, LEAVE_ROOM, "" );

	}

	// EXIT
	if( myMessage == "-exit" )
	{
		// Signal to exit
		return 1;
	}

	// There is a message. Send it!
	if( myMessage != "" )
	{
		myConn.sendMessage( user, SEND_TEXT, myMessage );
	}

	return 0;
}