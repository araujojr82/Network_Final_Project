#ifndef _lobby_HG_
#define _lobby_HG_

#include <string>

using namespace std;

// The message IDs to be used throughout the solution
enum gameModes
{
	FREE_FOR_ALL = 0,
	DUEL = 1,
	TEAM = 2
};

struct Lobby
{
	string mapName;				//1. The name of the map
	string lobbyName;			//2. The name of the Lobby
	gameModes gameMode;			//3. The mode of the game
	int openSpots;				//4. The number of open spots
	int totalSpots;				//5. The total number of spots
	string hostName;			//6. The name of the host
};

string getGameModeText( gameModes mode)
{
	switch( mode )
	{
	case FREE_FOR_ALL:
		return "Free for All";
		break;
	case DUEL:
		return "Duel";
		break;
	case TEAM:
		return "Team";
		break;
	default:
		return "UNKNOWN";
		break;
	}

	return "ERROR";
}

#endif // _lobby_HG_