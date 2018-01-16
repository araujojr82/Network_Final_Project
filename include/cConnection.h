#ifndef _cConnection_HG
#define _cConnection_HG

#include <WinSock2.h>
#include <ws2tcpip.h>
#include <iostream>
#include <algorithm> 
#include <string>
#include <Windows.h>

#include "InitInfo.h"

#define UNICODE
#define WIN32_LEAN_AND_MEAN

#pragma comment (lib, "Ws2_32.lib")

#define DEFAULT_PORT "5000"
#define DEFAULT_BUFFER_LENGTH 512
#define MAX_ARRAY_SIZE 256
#define MAX_PORT_SIZE 5

using namespace std;

// All connection states, information and functions
class cConnection 
{
public:
    cConnection();
    ~cConnection();

    // Connection variables
    WSADATA m_wsaData;            // Information about the windows socket
    SOCKET m_connectSocket;       // The socket of the client
    struct addrinfo* m_result;    // Information about the connection
    struct addrinfo* m_ptr;       // A pointer to a connection
    struct addrinfo m_hints;      // Hints about the supported configuration
    int m_iResult;                // The result for a connection
    char* m_recvBuffer;           // Received buffer
    int m_recvBufferLength;       // Received buffer length
    bool m_isAlive = 0;           // Keep track of the connection

    // Connection to the Server
    void connectToServer(InitInfo info);

    // Send a message to the server
    void sendMessage(InitInfo info, char msgID, string message);

    // Close the connection
    void closeConnection();

    // Look for new messages from the Server
    string getMessages();

private:

};

#endif // !_cConnection_HG