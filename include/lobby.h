#ifndef _lobby_HG_
#define _lobby_HG_

#include <string>

using namespace std;

#define MAX_PLAYERS 10

enum gameModes
{
	FREE_FOR_ALL = 1,
	DUEL = 2,
	TEAM = 3
};

enum gameMaps
{
	GREEN_HILL = 1,
	MARBLE = 2 ,
	SPRING_YARD = 3,
	LABYRINTH = 4,
	STAR_LIGHT = 5,
	SCRAP_BRAIN = 6
};

vector<gameModes> vecGameModes = { FREE_FOR_ALL, DUEL, TEAM };
vector<gameMaps> vecGameMaps = { GREEN_HILL, MARBLE, SPRING_YARD, LABYRINTH, STAR_LIGHT, SCRAP_BRAIN };

struct Lobby
{
	gameMaps gameMap;				//1. The name of the map
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

string getServerMapText( gameMaps map )
{
	switch( map )
	{
	case GREEN_HILL:
		return "Green Hill Zone";
	case MARBLE:
		return "Marble Zone";
	case SPRING_YARD:
		return "Spring Yard Zone";
	case LABYRINTH:
		return "Labyrinth Zone";
	case STAR_LIGHT:
		return "Star Light Zone";	
	case SCRAP_BRAIN:
		return "Scrap Brain Zone";
	default:
		return "UNKNOWN";
	}
	return "ERROR";
}

#endif // _lobby_HG_