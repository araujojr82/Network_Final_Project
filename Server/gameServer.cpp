/*gameServer.cpp
===============================================================================
|                                                                             |
|   Network Programming Final Assignment                                      |
|   Professor Lukas Gustafson                                                 |
|                                                                             |
|   Developed by Euclides Araujo                                              |
|   Based on previous work done by himself, Benjamin Taylor and Jorge Amengol |
|                                                                             |
|   This Server handles all user requests such as create and authenticate     |
|   users, join and leave rooms as well as sending messages. It uses simple   |
|   layer protocols to communicate with clients and Google Protocol Buffer    |
|   to communicate with the Authentication Server on top of the simple layer. |
|   The initial code for the TCP connections of this Server was inspired by   |
|   the code here:                                                            |
|    www.winsocketdotnetworkprogramming.com/winsock2programming/              |
|    winsock2advancediomethod5a.html                                          |
===============================================================================
*/

#define UNICODE
#define WIN32_LEAN_AND_MEAN

#include <Windows.h>
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <iostream>
#include <vector>
#include <cBuffer.h>
#include <MessageIDs.h>
#include <authentication.pb.h>
#include "lobby.h"

#pragma comment(lib, "Ws2_32.lib")

#define DEFAULT_PORT "5000"
#define DEFAULT_BUFFER_LENGTH 512

using namespace std;

/**
	This struct holds essential information about the server comunication,
	including the SOCKET itself. Each socket will have one of this struct so
	the server can iterate through each one of them and send messages to a
	certain socket if it is in the same room of the sender.
*/
typedef struct _SOCKET_INFORMATION
{
	_SOCKET_INFORMATION()
		: m_isAuthServer( false )
		, m_isAuthenticated( false )
		, m_hasNewData( false )
		, m_requestedID( false )
		, m_userID( false )
		{

		}

	char m_buffer[DEFAULT_BUFFER_LENGTH];
	WSABUF m_dataBuffer;               // The Data Buffer itself
	SOCKET m_socket;                   // The socket
	DWORD m_bytesSent;                 // Bytes sent in one iteration
	DWORD m_bytesReceived;             // Bytes received in one iteration
	vector<string> m_rooms;            // List of rooms for a socket
	string m_userName;                 // The user name for a socket
	bool m_isAuthServer;               // Used to control whether a message comes from the authentication server or not
	bool m_isAuthenticated;            // Says if a socket was previously authenticated
	bool m_hasNewData;                 // Used to control if a socket has new data to be sent
	long long m_requestedID;           // Stores the requested ID used by Google Protocol Buffers
	long long m_userID;                // User ID received from the Auth. Server

} SOCKET_INFORMATION, *LPSOCKET_INFORMATION;

// Prototypes
BOOL createSocketInformation( SOCKET );
void freeSocketInformation( DWORD Index );
void readSocket( LPSOCKET_INFORMATION );
void sendMsg( LPSOCKET_INFORMATION, string msg, string userName );
void sendMsg( LPSOCKET_INFORMATION, string msg );
void treatMessage( LPSOCKET_INFORMATION, string msg );
void removePlayersFromRoom( string room );
void msgToRoom( LPSOCKET_INFORMATION, string room, string msg, string sender );

// Global variables
LPSOCKET_INFORMATION g_socketArray[FD_SETSIZE]; // An array of sockets. 
								// It is limited by the max number of sockets FD_SETSIZE
DWORD g_totalSockets = 0;       // Total of registered sockets
int g_result;                   // Catches errors
FD_SET g_writeSet;              // A set of sockets to be written
FD_SET g_readSet;               // A set of sockets to be read
long long g_requestedID = 0;    // Controls unique request IDs

vector< Lobby* > g_vecServerLobbies;	// The List of existing Lobbies

int main()
{
	// Socket variables
	SOCKET listenSocket;    // The listen socket
	SOCKET acceptedSocket;  // The accepted socket
	WSADATA wsaData;        // Windows Socket information
	DWORD socketsIndex;     // Sockets Index
	DWORD totalOfSockets;   // Total of sockets in a read/write set
	ULONG nonBlockMode;     // Non bloking mode
	DWORD Flags;            // Control flags for WSARecv
	DWORD sentBytes;        // Number of bytes sent
	DWORD receivedBytes;    // Number of bytes received

	struct addrinfo* addrInfoResult;    // Response information about the host
	struct addrinfo hints;              // Hints of type of supported sockets

	addrInfoResult = nullptr;

	// The type of sockets and protocold we want to support
	ZeroMemory( &hints, sizeof( hints ) );
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
	hints.ai_flags = AI_PASSIVE;

	// WSA Startup
	g_result = WSAStartup( MAKEWORD( 2, 2 ), &wsaData );
	if( g_result != 0 )
	{
		printf( "WSAStartup failed: %d\n", g_result );
		return 1;
	}

	// Socket()
	listenSocket = socket( AF_INET, SOCK_STREAM, 0 );
	if( listenSocket == INVALID_SOCKET )
	{
		printf( "socket() failed with error %d\n", WSAGetLastError() );
		WSACleanup();
		return 1;
	}

	// Bind()
	getaddrinfo( NULL, DEFAULT_PORT, &hints, &addrInfoResult );
	g_result = bind( listenSocket, addrInfoResult->ai_addr, ( int )addrInfoResult->ai_addrlen );

	if( g_result == SOCKET_ERROR )
	{
		printf( "bind() failed with error: %d\n", WSAGetLastError() );
		freeaddrinfo( addrInfoResult );
		closesocket( listenSocket );
		WSACleanup();
		return 1;
	}

	// Listen()
	if( listen( listenSocket, SOMAXCONN ) )
	{
		printf( "listen() failed with error: %d\n", WSAGetLastError() );
		closesocket( listenSocket );
		WSACleanup();
		return 1;
	}

	// Change the socket mode on the listening socket from blocking to
	// non-block so the application will not block waiting for requests
	nonBlockMode = 1;
	if( ioctlsocket( listenSocket, FIONBIO, &nonBlockMode ) == SOCKET_ERROR )
	{
		printf( "ioctlsocket() failed with error %d\n", WSAGetLastError() );
		return 1;
	}
	else
		printf( "ioctlsocket() is OK!\n" );

	//-------------------------------------------------------------------------
	// Main loop block
	while( true )
	{

		// Prepare the Read and Write socket sets for network I/O notification
		FD_ZERO( &g_readSet );
		FD_ZERO( &g_writeSet );

		// Always look for connection attempts
		FD_SET( listenSocket, &g_readSet );

		// Favor the sockets with new data to send
		for( socketsIndex = 0; socketsIndex < g_totalSockets; socketsIndex++ )
		{
			if( g_socketArray[socketsIndex]->m_hasNewData )
				FD_SET( g_socketArray[socketsIndex]->m_socket, &g_writeSet );
			else
				FD_SET( g_socketArray[socketsIndex]->m_socket, &g_readSet );
		}

		// Select()
		if( ( totalOfSockets = select( 0, &g_readSet, &g_writeSet, 0, 0 ) ) == SOCKET_ERROR )
		{
			printf( "select() returned with error %d\n", WSAGetLastError() );
			return 1;
		}
		else
			printf( "select() is OK!\n" );

		// Check for arriving connections on the listening socket.
		if( FD_ISSET( listenSocket, &g_readSet ) )
		{
			totalOfSockets--;

			if( ( acceptedSocket = accept( listenSocket, 0, 0 ) ) != INVALID_SOCKET ) {

				// Set the accepted socket to non-blocking mode so the server
				// will not get caught in a blocked condition on WSASends
				nonBlockMode = 1;
				if( ioctlsocket( acceptedSocket, FIONBIO, &nonBlockMode ) == SOCKET_ERROR )
				{
					printf( "ioctlsocket(FIONBIO) failed with error %d\n", WSAGetLastError() );
					return 1;
				}
				else
					printf( "ioctlsocket(FIONBIO) is OK!\n" );

				if( createSocketInformation( acceptedSocket ) == FALSE )
				{
					printf( "createSocketInformation(acceptedSocket) failed!\n" );
					return 1;
				}
				else
					printf( "createSocketInformation() is OK!\n" );

			}
			else
			{
				if( WSAGetLastError() != WSAEWOULDBLOCK )
				{
					printf( "accept() failed with error %d\n", WSAGetLastError() );
					return 1;
				}
				else
					printf( "accept() is fine!\n" );
			}

		}//if (FD_ISSET(listenSocket, &g_readSet))

		// Check each socket for Read and Write notification until the number
		// of sockets in totalOfSockets is satisfied
		for( socketsIndex = 0; totalOfSockets > 0 && socketsIndex < g_totalSockets; socketsIndex++ )
		{
			LPSOCKET_INFORMATION SocketInfo = g_socketArray[socketsIndex];

			// If the g_readSet is marked for this socket then this means data
			// is available to be read on the socket
			if( FD_ISSET( SocketInfo->m_socket, &g_readSet ) )
			{
				totalOfSockets--;

				SocketInfo->m_dataBuffer.buf = SocketInfo->m_buffer;
				SocketInfo->m_dataBuffer.len = DEFAULT_BUFFER_LENGTH;

				Flags = 0;
				if( WSARecv( SocketInfo->m_socket, &( SocketInfo->m_dataBuffer ), 1, &receivedBytes, &Flags, NULL, NULL ) == SOCKET_ERROR )
				{
					if( WSAGetLastError() != WSAEWOULDBLOCK )
					{
						printf( "WSARecv() failed with error %d\n", WSAGetLastError() );
						freeSocketInformation( socketsIndex );
					}
					else
						printf( "WSARecv() is OK!\n" );

					continue;
				}
				else 
				{
					SocketInfo->m_bytesReceived = receivedBytes;

					// If zero bytes were received, this indicates the peer closed the connection.
					if( receivedBytes == 0 )
					{
						freeSocketInformation( socketsIndex );
						continue;
					}
					else
						readSocket( g_socketArray[socketsIndex] );

				} // else of if (WSARecv...
			} // if (WSARecv...


			// If the g_writeSet is marked on this socket then 
			// this means that there are sockets to send data
			if( FD_ISSET( SocketInfo->m_socket, &g_writeSet ) )
			{
				totalOfSockets--;

				if( WSASend( SocketInfo->m_socket, &( SocketInfo->m_dataBuffer ), 1, &sentBytes, 0, NULL, NULL ) == SOCKET_ERROR )
				{
					if( WSAGetLastError() != WSAEWOULDBLOCK ) 
					{
						printf( "WSASend() failed with error %d\n", WSAGetLastError() );
						freeSocketInformation( socketsIndex );
					}
					else
						printf( "WSASend() is OK!\n" );

					continue;
				}
				else 
				{
					SocketInfo->m_bytesSent += sentBytes;

					if( SocketInfo->m_bytesSent == SocketInfo->m_bytesReceived )
					{
						SocketInfo->m_bytesSent = false;
						SocketInfo->m_bytesReceived = false;
					}

				} // else of if (WSASend(SocketInfo->Socket...

				// No more data for this Socket
				SocketInfo->m_hasNewData = false;

			}//if (FD_ISSET(SocketInfo->m_socket...
		}//for (socketsIndex = 0;...
	}//while (true)...
}

/**
	Creates the Socket Information and initialize some of its parameters
*/
BOOL createSocketInformation( SOCKET s )
{
	LPSOCKET_INFORMATION SI;

	printf( "Accepted socket number %d\n", s );

	if( ( SI = ( LPSOCKET_INFORMATION )GlobalAlloc( GPTR, sizeof( SOCKET_INFORMATION ) ) ) == NULL ) 
	{
		printf( "GlobalAlloc() failed with error %d\n", GetLastError() );
		return FALSE;
	}
	else
		printf( "GlobalAlloc() for SOCKET_INFORMATION is OK!\n" );

	// Prepare SocketInfo structure for use
	SI->m_socket = s;
	SI->m_bytesSent = 0;
	SI->m_bytesReceived = 0;

	g_socketArray[g_totalSockets] = SI;
	g_totalSockets++;
	return( TRUE );
}

/**
	Frees the socket information
*/
void freeSocketInformation( DWORD Index )
{
	LPSOCKET_INFORMATION SI = g_socketArray[Index];
	DWORD i;

	closesocket( SI->m_socket );
	printf( "Closing socket number %d\n", SI->m_socket );
	GlobalFree( SI );

	// Squash the socket array
	for( i = Index; i < g_totalSockets; i++ )
	{
		g_socketArray[i] = g_socketArray[i + 1];
	}

	g_totalSockets--;
}

/*
	Reads all messages in Buffer for the Socket and pass them to treatment
*/
void readSocket( LPSOCKET_INFORMATION sa )
{
	bool isMsgIncomplete = true;
	int currMsgLength = 0;
	string currBuffer;

	// For all received bytes
	for( int i = 0; i <= sa->m_bytesReceived; i++ ) 
	{
		// Is the current message complete?
		if( isMsgIncomplete ) 
		{
			// Do we have the message length?
			if( !currMsgLength ) 
			{
				// Read the buffer until we have a prefix message length
				currBuffer.push_back( sa->m_dataBuffer.buf[i] );

				// Do  we have 4 bytes?
				if( currBuffer.size() == 4 ) 
				{
					// Deserialize the message length
					cBuffer buff( 4 );
					for( int j = 0; j < 4; j++ ) 
					{
						buff.serializeChar( currBuffer.at( j ) );
					}
					currMsgLength = buff.deserializeIntBE();

				} // if (currBuffer.size() == 4)

			}
			else 
			{ // Since we have the message length...    

			 // Continue reading until the buffer is complete
				if( currMsgLength != currBuffer.size() ) 
				{
					currBuffer.push_back( sa->m_dataBuffer.buf[i] );
				}
				else 
				{
					// We got our full message
					treatMessage( sa, currBuffer );
					currBuffer = "";
					!isMsgIncomplete;
					!currMsgLength;
				}
			} // elseif (!currMsgLength)...
		}
		else 
		{
			// The message is complete, but there's data in the buffer yet
			currMsgLength = 0;
			!isMsgIncomplete;
			currBuffer.push_back( sa->m_dataBuffer.buf[i] );
		} // elseif (isMsgIncomplete)
	}
}

Lobby* FindLobbyByName( string roomName )
{
	// Look in the Lobby Vector for a lobby with this name
	for( int i = 0; i != g_vecServerLobbies.size(); i++ )
	{
		if( g_vecServerLobbies[i]->lobbyName == roomName )
		{
			return g_vecServerLobbies[i];
		}
	}
	return NULL;
}

bool DeleteLobbyByName( string roomName )
{
	// Look in the Lobby Vector for a lobby with this name
	for( int i = 0; i != g_vecServerLobbies.size(); i++ )
	{
		if( g_vecServerLobbies[i]->lobbyName == roomName )
		{
			g_vecServerLobbies.erase( g_vecServerLobbies.begin() + i );
			return true;
		}
	}
	return false;
}

//	Treats a message and set the socket for reading
void treatMessage( LPSOCKET_INFORMATION sa, string msg )
{
	cBuffer buff( msg.size() );

	// Put the message in the Buffer class
	for( int i = 0; i < msg.size(); i++ ) buff.serializeChar( msg.at( i ) );

	int packetLength = buff.deserializeIntBE();

	// Sanity check for a too long message
	if( packetLength > 65542 )
		return;

	char msgID = buff.deserializeChar();

	// Treat each case according to the message ID
	switch( msgID ) 
	{

	case LIST_LOBBY:
	{
		// Send the List of Lobbys on this Server to the requester
		string listOfLobbies;
		listOfLobbies = "List of available lobbies:" + to_string( g_vecServerLobbies.size() );
		sendMsg( sa, listOfLobbies, "Game Server" );

	}
	break;	//case LIST_LOBBY:

	case GET_LOBBY_NUM:
	{
		// Deserialize the rest of the message to get the Lobby Number the client is asking		
		string lobbyNumber;
		short msgLength = buff.deserializeShortBE();
		for( short i = 0; i < msgLength; i++ )
		{
			lobbyNumber.push_back( buff.deserializeChar() );
		}	
		int lobbyNum = stoi( lobbyNumber );

		// Return the Lobby information
		string lobbyInfo = "Lobby" + to_string( lobbyNum ) + ":" +
							to_string( g_vecServerLobbies[lobbyNum]->gameMap ) + "," +
							g_vecServerLobbies[lobbyNum]->lobbyName + "," +
							to_string( g_vecServerLobbies[lobbyNum]->gameMode ) + "," +
							to_string( g_vecServerLobbies[lobbyNum]->openSpots ) + "," +
							to_string( g_vecServerLobbies[lobbyNum]->totalSpots ) + "," +
							g_vecServerLobbies[lobbyNum]->hostName;

		sendMsg( sa, lobbyInfo, "Game Server" );
	}
	break;	// case GET_LOBBY_NUM:

	case CREATE_ROOM:
	{
		bool bIsDuplicated = false;

		// The user is not authenticated
		if( !sa->m_isAuthenticated ) {
			sendMsg( sa, "You have to authenticate before creating a lobby!", "Game Server" );
		}
		else
		{// Deserialize the rest of the message and assign to the Socket
			string userName;
			char userNameLength = buff.deserializeChar();

			for( short i = 0; i < userNameLength; i++ )
			{
				userName.push_back( buff.deserializeChar() );
			}

			sa->m_userName = userName;
			sa->m_userName = userName;

			string msg;
			short msgLength = buff.deserializeShortBE();
			for( short i = 0; i < msgLength; i++ )
			{
				msg.push_back( buff.deserializeChar() );
			}

			// Split the message into the Lobby Info
			Lobby* newLobby = new Lobby();

			newLobby->gameMap = ( gameMaps )stoi( msg.substr( 0, msg.find( ",", 0 ) ) );
			msg = msg.substr( msg.find( ",", 0 ) + 1, msg.length() ); // new string

			newLobby->lobbyName = msg.substr( 0, msg.find( ",", 0 ) );
			msg = msg.substr( msg.find( ",", 0 ) + 1, msg.length() ); // new string

			newLobby->gameMode = ( gameModes )stoi( msg.substr( 0, msg.find( ",", 0 ) ) );
			msg = msg.substr( msg.find( ",", 0 ) + 1, msg.length() ); // new string					    
			
			newLobby->totalSpots = stoi( msg.substr( 0, msg.find( "\n", 0 ) ) );
			msg = msg.substr( msg.find( ",", 0 ) + 1, msg.length() ); // new string

			newLobby->hostName = userName;
			newLobby->openSpots = newLobby->totalSpots - 1;

			// Look in the Lobby Vector for a lobby with this name
			Lobby* theLobby = FindLobbyByName( newLobby->lobbyName );

			if( theLobby != NULL )
			{	// Lobby already exists, send msg to the client
				sendMsg( sa, "The lobby name already exists on this server!", "Chat Server" );
			}
			else
			{	// Add the lobby to the list of Lobbies
				g_vecServerLobbies.push_back( newLobby );
				sa->m_rooms.push_back( newLobby->lobbyName );

				// Send the Create Message to all known Sockets with this roomName
				msgToRoom( sa, newLobby->lobbyName, " has connected to ", "Game Server" );
			}

		} // elseif (!sa->m_isAuthenticated)
	} // CREATE_ROOM local scope
	break; // case CREATE_ROOM

	case JOIN_ROOM: 
	{
		bool bLobbyFound = false;

		// The user is not authenticated
		if( !sa->m_isAuthenticated ) {
			sendMsg( sa, "You have to authenticate before join a room!" , "Game Server" );
		}
		else 
		{// Deserialize the rest of the message and assign to the Socket
			string roomName;
			char roomNameLength = buff.deserializeChar();
			for( short i = 0; i < roomNameLength; i++ )
			{
				roomName.push_back( buff.deserializeChar() );
			}

			Lobby* theLobby = FindLobbyByName( roomName );

			if( theLobby != NULL )
			{
				// Does the lobby have any available spots
				if( theLobby->openSpots >= 1 )
				{	// Yes

					// Use one spot
					theLobby->openSpots--;

					sa->m_rooms.push_back( roomName );

					string userName;
					char userNameLength = buff.deserializeChar();

					for( short i = 0; i < userNameLength; i++ )
					{
						userName.push_back( buff.deserializeChar() );
					}

					// BUG: Assigning the name only once doesn't work properly!
					// Strange characters are sent instead of the user name.
					sa->m_userName = userName;
					sa->m_userName = userName;

					// Send the Join Message to all known Sockets with this roomName
					msgToRoom( sa, roomName, " has connected to ", "Game Server" );
				}
				else
				{ // No spots left, inform the user
					sendMsg( sa, "This lobby doesn't have any open spots!", "Game Server" );
				}
			}
			else
			{	// Lobby doesnt exist, inform client
				sendMsg( sa, "This lobby name doesn't exists on this server!", "Game Server" );
			}

		} // elseif (!sa->m_isAuthenticated)
	} // JOIN_ROOM local scope
				break; // case JOIN_ROOM

	case LEAVE_ROOM: 
	{
		bool bLobbyFound = false;
		string roomName;
		short roomNameLength = buff.deserializeChar();
		for( short i = 0; i < roomNameLength; i++ )
			roomName.push_back( buff.deserializeChar() );

		// Look in the Lobby Vector for a lobby with this name
		Lobby *theLobby = FindLobbyByName( roomName );

		if( theLobby != NULL )
		{	// Found the lobby
			if( theLobby->hostName == sa->m_userName )
			{				
				DeleteLobbyByName( theLobby->lobbyName );
				removePlayersFromRoom( roomName );
				//sendMsg( g_socketArray[i], "The host left the lobby so you will be disconnect from lobby" );
				//msgToRoom( sa, roomName, " has left room ", "Game Server" );
			}
			else
			{
				theLobby->openSpots++;
				msgToRoom( sa, roomName, " has left room ", "Game Server" );
			}

		}
		else
		{ // Lobby doesnt exist, inform client
			sendMsg( sa, "This lobby name doesn't exists on this server!", "Game Server" );
		}

	} // LEAVE_ROOM local scope
					 break; // case LEAVE_ROOM

	case SEND_TEXT: 
	{
		string msg;
		short msgLength = buff.deserializeShortBE();
		for( short i = 0; i < msgLength; i++ )
			msg.push_back( buff.deserializeChar() );

		for( int i = 0; i < sa->m_rooms.size(); i++ )
			for( int j = 0; j < g_totalSockets; j++ )
				for( int k = 0; k < g_socketArray[j]->m_rooms.size(); k++ )
					if( sa->m_rooms.at( i ) == g_socketArray[j]->m_rooms.at( k )
						&& sa->m_rooms.at( i ) != "" )
						sendMsg( g_socketArray[j], msg, sa->m_userName );

	} // SEND_TEXT local scope
					break; // case SEND_TEXT

	case CREATE_ACCOUNT: 
	{
		string username;
		char usernameLength = buff.deserializeChar();
		for( short i = 0; i < usernameLength; i++ )
			username.push_back( buff.deserializeChar() );

		string password;
		char passwordLength = buff.deserializeChar();
		for( short i = 0; i < passwordLength; i++ )
			password.push_back( buff.deserializeChar() );

		// Google Protocol Buffers
		authentication::CreateAccountWeb caw;
		g_requestedID++;
		sa->m_requestedID = g_requestedID;
		caw.set_requestid( g_requestedID );
		caw.set_email( username );
		caw.set_plaintextpassword( password );

		string pbStr;
		pbStr = caw.SerializeAsString();

		// Format the message to send
		int messageLength = 0;

		//                MSGID        PROTOCBUF
		messageLength = sizeof( char ) + pbStr.size();
		cBuffer authBuff( messageLength );
		authBuff.serializeChar( CREATE_ACCOUNT_WEB );
		for( int i = 0; i < pbStr.size(); i++ )
			authBuff.serializeChar( pbStr.at( i ) );

		// Send the message to the Authentication Server
		for( int i = 0; i < g_totalSockets; i++ )
			if( g_socketArray[i]->m_isAuthServer )
				sendMsg( g_socketArray[i], authBuff.getBuffer() );

	} // CREATE_ACCOUNT local scope
		break; // case CREATE_ACCOUNT

	case AUTHENTICATE: 
	{
		string username;
		char usernameLength = buff.deserializeChar();
		for( short i = 0; i < usernameLength; i++ )
			username.push_back( buff.deserializeChar() );

		string password;
		char passwordLength = buff.deserializeChar();
		for( short i = 0; i < passwordLength; i++ )
			password.push_back( buff.deserializeChar() );

		// Google Protocol Buffers
		authentication::AuthenticateWeb aw;
		g_requestedID++;
		sa->m_requestedID = g_requestedID;
		aw.set_requestid( g_requestedID );
		aw.set_email( username );
		aw.set_plaintextpassword( password );

		string pbStr;
		pbStr = aw.SerializeAsString();

		// Format the message to send
		int messageLength = 0;
		//                 MSGID        PROTOCBUF
		messageLength = sizeof( char ) + pbStr.size();
		cBuffer authBuff( messageLength );
		authBuff.serializeChar( AUTHENTICATE_WEB );
		for( int i = 0; i < pbStr.size(); i++ )
			authBuff.serializeChar( pbStr.at( i ) );

		// Send the message to the Authentication Server
		for( int i = 0; i < g_totalSockets; i++ )
			if( g_socketArray[i]->m_isAuthServer )
				sendMsg( g_socketArray[i], authBuff.getBuffer() );

	}// AUTHENTICATE local scope
		break; // case AUTHENTICATE

	case VALIDATE_SERVER: 
	{
		string serverName;
		char serverNameLength = buff.deserializeChar();
		for( int i = 0; i < serverNameLength; i++ )
			serverName.push_back( buff.deserializeChar() );

		string hash;
		char hashLength = buff.deserializeChar();
		for( int i = 0; i < hashLength; i++ )
			hash.push_back( buff.deserializeChar() );

		// Hard coded hash to validate. Auth Server has the same hash
		if( hash != "TEMP_HASH" ) 
		{
			cout << "Authentication Error!" << endl;
			return;
		}

		// Server validated
		sa->m_isAuthServer = true;

		// Twice to work!! Don't know why yet!
		sa->m_userName = serverName;
		sa->m_userName = serverName;

		cout << "Authentication Server validated!" << endl;
		sendMsg( sa, "Authentication Server validated!\n", "Game Server" );

	} // VALIDATE_SERVER local scope
		break; // case VALIDATE_SERVER

	case CREATE_ACCOUNT_WEB_SUCCESS: 
	{
		// Is it the real Authenticator?
		if( sa->m_isAuthServer ) 
		{
			string receivedStr;
			short msgLength = buff.deserializeShortBE();
			for( int i = 0; i < msgLength; i++ )
				receivedStr.push_back( buff.deserializeChar() );

			// Google Protocol Buffers
			authentication::CreateAccountWebSuccess caws;
			caws.ParseFromString( receivedStr );

			// Find correct requestedID, assign the userid to the socket that
			// requested it and send the message to it
			for( int i = 0; i < g_totalSockets; i++ ) 
			{
				if( g_socketArray[i]->m_requestedID == caws.requestid() ) 
				{
					g_socketArray[i]->m_userID = caws.userid();
					sendMsg( g_socketArray[i], "The user was created successfully!", "Authentication Server" );
				}
			}
		}//if (sa->m_isAuthServer)...    
	} // CREATE_ACCOUNT_WEB_SUCCESS local scope
		break; // case CREATE_ACCOUNT_WEB_SUCCESS

	case CREATE_ACCOUNT_WEB_FAILURE: 
	{
		// Is it the real Authenticator?
		if( sa->m_isAuthServer ) 
		{
			string receivedStr;
			short msgLength = buff.deserializeShortBE();
			for( int i = 0; i < msgLength; i++ )
				receivedStr.push_back( buff.deserializeChar() );

			// Google Protocol Buffers
			authentication::CreateAccountWebFailure cawf;
			cawf.ParseFromString( receivedStr );

			// Find correct requestedID, and send the message 
			// according to the type of failure that has occurred
			for( int i = 0; i < g_totalSockets; i++ ) 
			{
				if( g_socketArray[i]->m_requestedID == cawf.requestid() ) 
				{
					char reason = cawf.thereaseon();

					switch( reason ) 
					{
						case authentication
							::CreateAccountWebFailure_reason_ACCOUNT_ALREADY_EXISTS:
								sendMsg( g_socketArray[i], "The account already exists!", "Authentication Server" );
							break;

						case authentication
							::CreateAccountWebFailure_reason_INVALID_PASSWORD:
								sendMsg( g_socketArray[i], "The Authentication Server did not accept your password!", "Authentication Server" );
							break;

						case authentication
							::CreateAccountWebFailure_reason_INTERNAL_SERVER_ERROR:
								sendMsg( g_socketArray[i], "There was an Internal Authentication Server error!", "Authentication Server" );
							break;

								// Unknown Error
						default:
							sendMsg( g_socketArray[i], "There was an unknown error!", "Authentication Server" );

					} // switch (reason)
				}//if (g_socketArray[i]->m_requestedID...
			}//for (int i = 0; i < g_totalSockets...
		}//if (sa->m_isAuthServer)...
	} // CREATE_ACCOUNT_WEB_FAILURE local scope
		break; // case CREATE_ACCOUNT_WEB_FAILURE

	case AUTHENTICATE_WEB_SUCCESS: 
	{
		// Is it the real Authenticator?
		if( sa->m_isAuthServer ) 
		{
			string receivedStr;
			short msgLength = buff.deserializeShortBE();
			for( int i = 0; i < msgLength; i++ )
				receivedStr.push_back( buff.deserializeChar() );

			// Google Protocol Buffers
			authentication::AuthenticateWebSuccess aws;
			aws.ParseFromString( receivedStr );

			// Find correct requestedID, and send the message 
			for( int i = 0; i < g_totalSockets; i++ ) 
			{
				if( g_socketArray[i]->m_requestedID == aws.requestid() ) 
				{
					g_socketArray[i]->m_isAuthenticated = true;
					sendMsg( g_socketArray[i], "Authentication successful, last login : " + aws.creationdate(), "Authentication Server" );
				}
			}
		}
	} // AUTHENTICATE_WEB_SUCCESS local scope
		break; // case AUTHENTICATE_WEB_SUCCESS:

	case AUTHENTICATE_WEB_FAILURE: 
	{
		// Is it the real Authenticator?
		if( sa->m_isAuthServer ) 
		{
			string receivedStr;
			short msgLength = buff.deserializeShortBE();
			for( int i = 0; i < msgLength; i++ )
				receivedStr.push_back( buff.deserializeChar() );

			// Google Protocol Buffers
			authentication::AuthenticateWebFailure awf;
			awf.ParseFromString( receivedStr );

			// Find correct requestedID, and send the message 
			// according to the type of failure that has occurred
			for( int i = 0; i < g_totalSockets; i++ ) 
			{
				if( g_socketArray[i]->m_requestedID == awf.requestid() ) 
				{
					char reason = awf.thereaseon();

					switch( reason ) 
					{
						case authentication
							::AuthenticateWebFailure_reason_INVALID_CREDENTIALS:
								sendMsg( g_socketArray[i], "Invalid credentials!", "Authentication Server" );
							break;

						case authentication
							::AuthenticateWebFailure_reason_INTERNAL_SERVER_ERROR:
								sendMsg( g_socketArray[i], "There was an internal error",  "Authentication Server" );
							break;

						default:
							sendMsg( g_socketArray[i], "There was an unknown error!", "Authentication Server" );

					} // switch (reason)
				}//if (g_socketArray[i]->m_requestedID...
			}//for (int i = 0; i < g_totalSockets...
		}//if (sa->m_isAuthServer)...
	}// AUTHENTICATE_WEB_FAILURE local scope
		break; // case AUTHENTICATE_WEB_FAILURE

	default:
		break;

	} // switch (msgID)
}

/**
	Sends a message to the destination 'sa' socket. The user name that is
	sending the message is passed through 'userName'
*/
void sendMsg( LPSOCKET_INFORMATION sa, string msg, string userName )
{
	// Message already goes with user name
	string formatedMsg = userName + "->" + msg;

	// Drop a too big message
	if( formatedMsg.size() > 65535 )
		return;

	// Calculate the length of the packet
	//                  packetLength        Chars
	int packetLength = sizeof( INT32 ) + formatedMsg.size();

	// Serialize it and put it in the Socket Buffer
	cBuffer buff( packetLength );
	buff.serializeIntLE( packetLength );

	for( int i = 0; i < formatedMsg.size(); i++ )
		buff.serializeChar( formatedMsg.at( i ) );

	buff.serializeChar( '\0' ); // C style string.

	sa->m_dataBuffer.buf = buff.getBuffer();
	sa->m_dataBuffer.len = packetLength;
	sa->m_hasNewData = true;
}

/**
	Sends a message to the destination 'sa' socket.
*/
void sendMsg( LPSOCKET_INFORMATION sa, string msg )
{
	// Drop a too big message
	if( msg.size() > 65535 )
		return;

	// Calculate the length of the packet
	//                  packetLength      Chars
	int packetLength = sizeof( INT32 ) + msg.size();

	// Serialize it and put it in the Socket Buffer
	cBuffer buff( packetLength );
	buff.serializeIntLE( packetLength );

	for( int i = 0; i < msg.size(); i++ )
		buff.serializeChar( msg.at( i ) );

	//buff.serializeChar('\0'); // C style string.

	sa->m_dataBuffer.buf = buff.getBuffer();
	sa->m_dataBuffer.len = packetLength;
	sa->m_hasNewData = true;
}

/**
	Iterates through all sockets to send a message to a specific room. If the
	socket has the same room, it will receive the message.
*/
void msgToRoom( LPSOCKET_INFORMATION sa, string room, string msg, string sender )
{
	for( int i = 0; i < g_totalSockets; i++ )
	{
		for( int j = 0; j < g_socketArray[i]->m_rooms.size(); j++ )
		{
			if( g_socketArray[i]->m_rooms.at( j ) == room )
			{
				sendMsg( g_socketArray[i], sa->m_userName + msg + room, sender );
				break;
			}
		}
	}
}

void removePlayersFromRoom( string room )
{
	vector<string> theNewRooms;

	for( int i = 0; i < g_totalSockets; i++ )
	{
		theNewRooms.clear();
		for( int j = 0; j < g_socketArray[i]->m_rooms.size(); j++ )
		{
			if( g_socketArray[i]->m_rooms.at( j ) != room )
			{
				theNewRooms.push_back( g_socketArray[i]->m_rooms[j] );
			}
			else
			{
				sendMsg( g_socketArray[i], "Game Server->" + g_socketArray[i]->m_userName + " has left " + room +
					"\nGame Server->The host has left. You have been disconnect from " + room );
			}
		}
		g_socketArray[i]->m_rooms.clear();
		g_socketArray[i]->m_rooms = theNewRooms;
	}	
}