#include "cConnection.h"
#include "cBuffer.h"
#include "MessageIDs.h"

cConnection::cConnection()
{
	m_connectSocket = INVALID_SOCKET;
	m_result = NULL;
	m_ptr = NULL;
	m_recvBuffer = new char[DEFAULT_BUFFER_LENGTH];
	m_recvBufferLength = DEFAULT_BUFFER_LENGTH;
}

cConnection::~cConnection()
{
	delete m_recvBuffer;
}

void cConnection::connectToServer( InitInfo info )
{
	// Initialize Winsock
	m_iResult = WSAStartup( MAKEWORD( 2, 2 ), &m_wsaData );
	if( m_iResult != 0 )
	{
		printf( "WSAStartup failed: %d\n", m_iResult );
		return;
	}

	// What we want to support
	ZeroMemory( &m_hints, sizeof( m_hints ) );
	m_hints.ai_family = AF_UNSPEC;
	m_hints.ai_socktype = SOCK_STREAM;
	m_hints.ai_protocol = IPPROTO_TCP;

	// Resolve the server address and port
	m_iResult = getaddrinfo( info.serverAddr, DEFAULT_PORT, &m_hints, &m_result );
	if( m_iResult != 0 )
	{
		printf( "getaddrinfo failed with error: %d\n", m_iResult );
		WSACleanup();
		return;
	}

	// Attempt to connect to an address until one succeeds
	for( m_ptr = m_result; m_ptr != NULL; m_ptr = m_ptr->ai_next )
	{
		// Create a SOCKET for connecting to server
		m_connectSocket = socket( m_ptr->ai_family, m_ptr->ai_socktype, m_ptr->ai_protocol );

		if( m_connectSocket == INVALID_SOCKET )
		{
			printf( "socket() failed with error: %d\n", m_iResult );
			WSACleanup();
			return;
		}

		// Connect to server
		m_iResult = connect( m_connectSocket, m_ptr->ai_addr, ( int )m_ptr->ai_addrlen );

		if( m_iResult == SOCKET_ERROR )
		{
			closesocket( m_connectSocket );
			m_connectSocket = INVALID_SOCKET;
			continue;
		}
		break;
	}//for(m_ptr = m_result...

	freeaddrinfo( m_result );

	if( m_connectSocket == INVALID_SOCKET )
	{
		printf( "Unable to connect to server" );
		WSACleanup();
		return;
	}

	// We got a connection!
	this->m_isAlive = 1;

}// mConnect(InitInfo info)

// Send a message to the server according to the msgID
void cConnection::sendMessage( InitInfo info, char msgID, string message )
{
	// Packet length
	int packetLength = 0;

	// Buffer
	cBuffer* connBuff = NULL;

	switch( msgID )
	{

	case LIST_LOBBY:
	{
		// User name length:
		char userNameLength = info.username.size();

		// Room length:
		char roomNameLength = info.room.size();

		//                Packet        MSG_ID         ROOM SIZE         ROOM           NAME SIZE          NAME
		packetLength = sizeof( int ) + sizeof( char ) + sizeof( char ) + roomNameLength + sizeof( char ) + userNameLength;

		// Check if the packet is too big
		if( packetLength > 519 )
		{ // max lenth shouldn't be more than 519 bytes
			cout << "Buffer overflow sendind the JOIN message!\n";
			Sleep( 3000 );
			return;
		}

		// Now serialize it
		connBuff = new cBuffer( packetLength );
		connBuff->serializeIntLE( packetLength );
		connBuff->serializeChar( LIST_LOBBY );

		connBuff->serializeChar( roomNameLength );

		for( int i = 0; i < roomNameLength; i++ )
		{
			connBuff->serializeChar( info.room[i] );
		}

		connBuff->serializeChar( userNameLength );

		for( int i = 0; i < userNameLength; i++ )
		{
			connBuff->serializeChar( info.username[i] );
		}

	}//!Block for local variables
	break;	// case LIST_LOBBY	

	case GET_LOBBY_NUM:
	{
		// Message length
		short msgLength = message.size();

		//                Packet        MSG_ID        MSG LENGHT     MESSAGE
		packetLength = sizeof( int ) + sizeof( char ) + sizeof( short ) + msgLength;

		// Check if the packet is too big
		if( packetLength > 65542 )
		{ // max lenth shouldn't be more than 65,542 bytes        
			cout << "Buffer overflow sendind the TEXT message!\n";
			Sleep( 3000 );
			return;
		}

		// Now serialize it
		connBuff = new cBuffer( packetLength );
		connBuff->serializeIntLE( packetLength );
		connBuff->serializeChar( GET_LOBBY_NUM );

		connBuff->serializeShortLE( msgLength );
		for( int i = 0; i < msgLength; i++ )
		{
			connBuff->serializeChar( message.at( i ) );
		}

	}//!Block for local variables
	break;	// case GET_LOBBY_NUM	

	case CREATE_ROOM:
	{
		// User name length:
		char userNameLength = info.username.size();

		//// Room length:
		//char roomNameLength = info.room.size();

		// Message length
		short msgLength = message.size();

		//                Packet        MSG_ID          NAME SIZE        NAME             MSG LENGHT        MESSAGE
		packetLength = sizeof( int ) + sizeof( char ) + sizeof( char ) + userNameLength + sizeof( short ) + msgLength;
		
		// Check if the packet is too big
		if( packetLength > 65542 )
		{ // max lenth shouldn't be more than 65,542 bytes        
			cout << "Buffer overflow sendind the CREATE message!\n";
			Sleep( 3000 );
			return;
		}

		// Now serialize it
		connBuff = new cBuffer( packetLength );
		connBuff->serializeIntLE( packetLength );
		connBuff->serializeChar( CREATE_ROOM );

		connBuff->serializeChar( userNameLength );

		for( int i = 0; i < userNameLength; i++ )
		{
			connBuff->serializeChar( info.username[i] );
		}
		
		connBuff->serializeShortLE( msgLength );
		
		for( int i = 0; i < msgLength; i++ )
		{
			connBuff->serializeChar( message.at( i ) );
		}		

	}//!Block for local variables
		break;	// case CREATE_ROOM	

	case JOIN_ROOM:
	{
		// User name length:
		char userNameLength = info.username.size();

		// Room length:
		char roomNameLength = info.room.size();

		//                Packet        MSG_ID         ROOM SIZE         ROOM           NAME SIZE          NAME
		packetLength = sizeof( int ) + sizeof( char ) + sizeof( char ) + roomNameLength + sizeof( char ) + userNameLength;

		// Check if the packet is too big
		if( packetLength > 519 )
		{ // max lenth shouldn't be more than 519 bytes
			cout << "Buffer overflow sendind the JOIN message!\n";
			Sleep( 3000 );
			return;
		}

		// Now serialize it
		connBuff = new cBuffer( packetLength );
		connBuff->serializeIntLE( packetLength );
		connBuff->serializeChar( JOIN_ROOM );

		connBuff->serializeChar( roomNameLength );

		for( int i = 0; i < roomNameLength; i++ )
		{
			connBuff->serializeChar( info.room[i] );
		}

		connBuff->serializeChar( userNameLength );

		for( int i = 0; i < userNameLength; i++ )
		{
			connBuff->serializeChar( info.username[i] );
		}

	}//!Block for local variables
	break;	// case JOIN_ROOM	

	case LEAVE_ROOM:
	{
		// Room length:
		char roomNameLength = info.room.size();

		//                Packet        MSG_ID         ROOM SIZE         ROOM 
		packetLength = sizeof( int ) + sizeof( char ) + sizeof( char ) + roomNameLength;

		// Check if the packet is too big
		if( packetLength > 262 ) 
		{ // max lenth shouldn't be more than 262 bytes        
			cout << "Buffer overflow sendind the LEAVE message!\n";
			Sleep( 3000 );
			return;
		}

		// Now serialize it
		connBuff = new cBuffer( packetLength );
		connBuff->serializeIntLE( packetLength );
		connBuff->serializeChar( LEAVE_ROOM );

		connBuff->serializeChar( roomNameLength );
		for( int i = 0; i < roomNameLength; i++ )
		{
			connBuff->serializeChar( info.room[i] );
		}

	}//!Block for local variables
		break;	// case LEAVE_ROOM

	case SEND_TEXT:
	{
		// Message length
		short msgLength = message.size();

		//                Packet        MSG_ID        MSG LENGHT     MESSAGE
		packetLength = sizeof( int ) + sizeof( char ) + sizeof( short ) + msgLength;

		// Check if the packet is too big
		if( packetLength > 65542 ) 
		{ // max lenth shouldn't be more than 65,542 bytes        
			cout << "Buffer overflow sendind the TEXT message!\n";
			Sleep( 3000 );
			return;
		}

		// Now serialize it
		connBuff = new cBuffer( packetLength );
		connBuff->serializeIntLE( packetLength );
		connBuff->serializeChar( SEND_TEXT );

		connBuff->serializeShortLE( msgLength );
		for( int i = 0; i < msgLength; i++ )
		{
			connBuff->serializeChar( message.at( i ) );
		}

	}//!Block for local variables
		break;	// case SEND_TEXT

	case CREATE_ACCOUNT:
	{
		// Email length:
		char emailLength = info.email.size();

		// Password length:
		char passwordLength = info.password.size();

		//                Packet        MSG_ID         EMAIL SIZE      EMAIL        PWD SIZE        PASSWORD
		packetLength = sizeof( int ) + sizeof( char ) + sizeof( char ) + emailLength + sizeof( char ) + passwordLength;

		// Check if the packet is too big
		if( packetLength > 519 )
		{ // max lenth shouldn't be more than 519 bytes        
			cout << "Buffer overflow sendind the ADD message!\n";
			Sleep( 3000 );
			return;
		}

		// Now serialize it
		connBuff = new cBuffer( packetLength );
		connBuff->serializeIntLE( packetLength );
		connBuff->serializeChar( CREATE_ACCOUNT );

		connBuff->serializeChar( emailLength );
		for( int i = 0; i < emailLength; i++ )
		{
			connBuff->serializeChar( info.email[i] );
		}

		connBuff->serializeChar( passwordLength );
		for( int i = 0; i < passwordLength; i++ )
		{
			connBuff->serializeChar( info.password[i] );
		}

	}//!Block for local variables
		break; // case CREATE_ACCOUNT

	case AUTHENTICATE:
	{
		// Email length:
		char emailLength = info.email.size();

		// Password length:
		char passwordLength = info.password.size();

		//                Packet        MSG_ID         EMAIL SIZE      EMAIL        PWD SIZE        PASSWORD
		packetLength = sizeof( int ) + sizeof( char ) + sizeof( char ) + emailLength + sizeof( char ) + passwordLength;

		// Check if the packet is too big
		if( packetLength > 519 ) 
		{ // max lenth shouldn't be more than 519 bytes        
			cout << "Buffer overflow sendind the ADD message!\n";
			Sleep( 3000 );
			return;
		}

		// Now serialize it
		connBuff = new cBuffer( packetLength );
		connBuff->serializeIntLE( packetLength );
		connBuff->serializeChar( AUTHENTICATE );

		connBuff->serializeChar( emailLength );
		for( int i = 0; i < emailLength; i++ )
		{
			connBuff->serializeChar( info.email[i] );
		}

		connBuff->serializeChar( passwordLength );
		for( int i = 0; i < passwordLength; i++ )
		{
			connBuff->serializeChar( info.password[i] );
		}

	}//!Block for local variables
		break; // case AUTHENTICATE

	case VALIDATE_SERVER:
	{
		// Server Name Length
		char serverNameLength = info.username.size();

		// Hash length
		char hashLength = message.size();

		// Next two lines not splitted for better understanding of the protocol
		//                Packet        MSG_ID       NAME LENGTH        NAME          HASH LENGTH     HASH
		packetLength = sizeof( int ) + sizeof( char ) + sizeof( char ) + serverNameLength + sizeof( char ) + hashLength;

		// Check if the packet is too big
		if( packetLength > 519 ) 
		{ // max lenth shouldn't be more than 519 bytes        
			cout << "Buffer overflow sendind the VALIDATE_SERVER message!\n";
			Sleep( 3000 );
			return;
		}

		// Now serialize it
		connBuff = new cBuffer( packetLength );
		connBuff->serializeIntLE( packetLength );
		connBuff->serializeChar( VALIDATE_SERVER );

		connBuff->serializeChar( serverNameLength );
		for( int i = 0; i < serverNameLength; i++ )
		{
			connBuff->serializeChar( info.username[i] );
		}

		connBuff->serializeChar( hashLength );
		for( int i = 0; i < hashLength; i++ )
		{
			connBuff->serializeChar( message.at( i ) );
		}

	}//!Block for local variables
		break;	// case VALIDATE_SERVER

	case CREATE_ACCOUNT_WEB_SUCCESS:
	{
		short msgLength = message.size();

		//                Packet        MSG_ID      PROTOC LENGTH   PROTOC MSG 
		packetLength = sizeof( int ) + sizeof( char ) + sizeof( short ) + msgLength;

		// Check if the packet is too big
		if( packetLength > 65543 ) 
		{ // max lenth shouldn't be more than 65,543 bytes        
			cout << "Buffer overflow sending the CREATE_ACCOUNT_WEB_SUCCESS message!\n";
			Sleep( 3000 );
			return;
		}

		// Now serialize it
		connBuff = new cBuffer( packetLength );
		connBuff->serializeIntLE( packetLength );
		connBuff->serializeChar( CREATE_ACCOUNT_WEB_SUCCESS );

		connBuff->serializeShortLE( msgLength );
		for( int i = 0; i < msgLength; i++ )
		{
			connBuff->serializeChar( message.at( i ) );
		}

	}//!Block for local variables
		break;	// case CREATE_ACCOUNT_WEB_SUCCESS

	case CREATE_ACCOUNT_WEB_FAILURE:
	{
		short msgLength = message.size();

		//                Packet        MSG_ID      PROTOC LENGTH   PROTOC MSG 
		packetLength = sizeof( int ) + sizeof( char ) + sizeof( short ) + msgLength;

		// Check if the packet is too big
		if( packetLength > 65543 ) 
		{ // max lenth shouldn't be more than 65,543 bytes        
			cout << "Buffer overflow sending the CREATE_ACCOUNT_WEB_FAILURE message!\n";
			Sleep( 3000 );
			return;
		}

		// Now serialize it
		connBuff = new cBuffer( packetLength );
		connBuff->serializeIntLE( packetLength );
		connBuff->serializeChar( CREATE_ACCOUNT_WEB_FAILURE );

		connBuff->serializeShortLE( msgLength );
		for( int i = 0; i < msgLength; i++ )
		{
			connBuff->serializeChar( message.at( i ) );
		}

	}//!Block for local variables
		break;	// case CREATE_ACCOUNT_WEB_FAILURE

	case AUTHENTICATE_WEB_SUCCESS:
	{
		short msgLength = message.size();
		//                Packet        MSG_ID      PROTOC LENGTH   PROTOC MSG 
		packetLength = sizeof( int ) + sizeof( char ) + sizeof( short ) + msgLength;

		// Check if the packet is too big
		if( packetLength > 65543 ) 
		{ // max lenth shouldn't be more than 65,543 bytes        
			cout << "Buffer overflow sending the CREATE_ACCOUNT_WEB_SUCCESS message!\n";
			Sleep( 3000 );
			return;
		}

		// Now serialize it
		connBuff = new cBuffer( packetLength );
		connBuff->serializeIntLE( packetLength );
		connBuff->serializeChar( AUTHENTICATE_WEB_SUCCESS );

		connBuff->serializeShortLE( msgLength );
		for( int i = 0; i < msgLength; i++ )
		{
			connBuff->serializeChar( message.at( i ) );
		}

	}//!Block for local variables
		break;	// case AUTHENTICATE_WEB_SUCCESS

	case AUTHENTICATE_WEB_FAILURE:
	{
		short msgLength = message.size();
		//                Packet        MSG_ID      PROTOC LENGTH   PROTOC MSG 
		packetLength = sizeof( int ) + sizeof( char ) + sizeof( short ) + msgLength;

		// Check if the packet is too big
		if( packetLength > 65543 ) { // max lenth shouldn't be more than 65,543 bytes        
			cout << "Buffer overflow sending the CREATE_ACCOUNT_WEB_FAILURE message!\n";
			Sleep( 3000 );
			return;
		}

		// Now serialize it
		connBuff = new cBuffer( packetLength );
		connBuff->serializeIntLE( packetLength );
		connBuff->serializeChar( AUTHENTICATE_WEB_FAILURE );

		connBuff->serializeShortLE( msgLength );
		for( int i = 0; i < msgLength; i++ )
		{
			connBuff->serializeChar( message.at( i ) );
		}

	}//!Block for local variables
		break;	// case AUTHENTICATE_WEB_FAILURE

	default:
		break;

	}// !switch (msgID)

	// Finelly, send the message to the server
	m_iResult = send( m_connectSocket, connBuff->getBuffer(), packetLength, 0 );
	if( m_iResult == SOCKET_ERROR ) 
	{
		printf( "socket() failed with error: %d\n", m_iResult );
		closesocket( m_connectSocket );
		WSACleanup();

		// Delete the buffer
		delete connBuff;

		return;
	}
	else 
	{ // Delete the buffer
		delete connBuff;
	}

}// !mSendMessage()

//Close the connection
void cConnection::closeConnection() 
{
	m_iResult = shutdown( m_connectSocket, SD_SEND );
	if( m_iResult == SOCKET_ERROR ) 
	{
		printf( "shutdown() failed with error: %d\n", m_iResult );
		closesocket( m_connectSocket );
		WSACleanup();
		return;
	}
	closesocket( m_connectSocket );
	WSACleanup();
}

// Gets new messages from the server
string cConnection::getMessages()
{
	// Set the time interval to 1 second, so we will probably 
	// see the answer from the server for our own message.
	timeval* mTime = new timeval[1];
	mTime->tv_sec = 1;
	mTime->tv_usec = 0;

	// Add current socket to fs_set
	fd_set* listOfSockets = new fd_set[1];
	listOfSockets->fd_count = 1;
	listOfSockets->fd_array[0] = m_connectSocket;

	// Select()
	if( select( 0, listOfSockets, NULL, NULL, mTime ) > 0 )
	{
		// Clear the buffer before receiving
		ZeroMemory( m_recvBuffer, m_recvBufferLength );

		// Recv()
		m_iResult = recv( m_connectSocket, m_recvBuffer, m_recvBufferLength, 0 );
		if( m_iResult == SOCKET_ERROR ) 
		{
			closesocket( m_connectSocket );
			WSACleanup();
			return "recv failed with error : " + WSAGetLastError() + '\n';
		}

		cBuffer buff( m_recvBufferLength );   // The received buffer to be
		buff.setBuffer( m_recvBuffer, m_recvBufferLength );
		int packetLength = 0;               // The packet length
		int bytesInBuffer;                  // How many bytes in buffer
		bytesInBuffer = m_recvBufferLength;
		static string controlStr;           // A string to control 
											// incompleted messages
		string retStr;                      // The returned string

		// Is there data from the last recv()?
		if( controlStr != "" )
			retStr = controlStr;

		while( bytesInBuffer != 0 ) 
		{
			if( bytesInBuffer < 4 && !packetLength ) 
			{
				// Do something with the incomplete buffer
				controlStr.push_back( buff.deserializeChar() );
				bytesInBuffer--;
			}
			else 
			{
				// We have data for the prefix length
				packetLength = buff.deserializeIntBE();

				// Read the data
				if( packetLength <= bytesInBuffer && packetLength != 0 ) 
				{
					for( int i = 0; i < packetLength - 4; i++ ) 
					{
						// Complete the message buffer
						retStr.push_back( buff.deserializeChar() );
					}

					retStr.push_back( '\n' );
					bytesInBuffer -= packetLength;

				}
				else if( packetLength == 0 )
					break;

				bytesInBuffer--;
			}
		}

		return retStr;


	}// !Select()			

	// There is nothing here. Return an empty string.
	return "";

}// !getMessages()