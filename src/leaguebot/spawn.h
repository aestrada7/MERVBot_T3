/*
	MERVBot Spawn-callback by Catid@pacbell.net
*/

#ifndef SPAWN_H
#define SPAWN_H

#define STRING_CAST_CHAR
#include "..\dllcore.h"

#include "..\clientprot.h"
#include <vector>

struct PlayerTag
{
	Player *p;
	int index;
	int data;
};

#define MAX_OBJECTS 20

class MatchPlayer
{
	public:
		char *name;
		int lives;
		int kills;
		int assists;
		int deaths;
		int lagouts;
		int forcedReps;
		int teamkills;
		float mvpPoints;
		Player *player;
};

class Team
{
	public:
		int freq;
		char *squad;
		int score;
		std::vector<MatchPlayer> players;
};

class Match
{
	public:
		int sideAX;
		int sideAY;
		int sideBX;
		int sideBY;
		int numPlayers;
		int lives;
		int duration;
		bool lagEnforcing;
		char *gameType;
		Team teams[2];
};

class botInfo
{
	bool CONNECTION_DENIED;

	_linkedlist <PlayerTag> taglist;
	int get_tag(Player *p, int index);
	void set_tag(Player *p, int index, int data);
	void killTags(Player *p);
	void killTags();

	CALL_HANDLE handle;
	CALL_COMMAND callback;
	CALL_PLIST playerlist;
	CALL_FLIST flaglist;
	CALL_MAP map;
	CALL_BLIST bricklist;
	char *arena;
	arenaSettings *settings;
	Player *me;
	bool biller_online;

	objectInfo object_array[MAX_OBJECTS];
	int num_objects;
	Player *object_dest;

	int countdown[4];

	// Put bot data here
	Match match;

public:
	botInfo(CALL_HANDLE given)
	{
		handle = given;
		callback = 0;
		playerlist = 0;
		flaglist = 0;
		map = 0;
		countdown[0] = 0;
		countdown[1] = 0;
		countdown[2] = 0;
		countdown[3] = 0;
		CONNECTION_DENIED = false;
		me = 0;
		biller_online = true;
		num_objects = 0;
		object_dest = NULL;

		// Put initial values here
		Match match;
	}

	void clear_objects();
	void object_target(Player *p);
	void toggle_objects();
	void queue_enable(int id);
	void queue_disable(int id);

	void gotEvent(BotEvent &event);

	void tell(BotEvent event);

	bool validate(CALL_HANDLE given) { return given == handle; }

	void sendPrivate(Player *player, char *msg);
	void sendPrivate(Player *player, BYTE snd, char *msg);

	void sendTeam(char *msg);
	void sendTeam(BYTE snd, char *msg);

	void sendTeamPrivate(Uint16 team, char *msg);
	void sendTeamPrivate(Uint16 team, BYTE snd, char *msg);

	void sendPublic(char *msg);
	void sendPublic(BYTE snd, char *msg);

	void sendPublicMacro(char *msg);
	void sendPublicMacro(BYTE snd, char *msg);

	void sendChannel(char *msg);			// #;Message
	void sendRemotePrivate(char *msg);		// :Name:Messsage
	void sendRemotePrivate(char *name, char *msg);

	void gotHelp(Player *p, Command *c);
	void gotCommand(Player *p, Command *c);
	void gotRemoteHelp(char *p, Command *c, Operator_Level l);
	void gotRemote(char *p, Command *c, Operator_Level l);

	char *getShipName(int id);
	void warpTo(Player *p, int x, int y);

	void findPlayersInFreqs();
	void parseCommand(char* command);
	void setSquads(char* command);
	void setFreqs(const char* command);
	void getStatus();
	void startMatch();
	void endMatch();
	void prepareMatch();

	Team* playerTeam(MatchPlayer& player);
	MatchPlayer* findMatchPlayer(char* playerName);
	void announceScore();
	void checkRemainingMatchPlayers(Team* team);
	void gameEnd();
	void printScoreBoxTop(char* squadName);
	void printMatchPlayerData(MatchPlayer p);
	void printTeamData(Team t);
};


// The botlist contains every bot to ever be spawned of this type,
// some entries may no longer exist.

extern _linkedlist <botInfo> botlist;

botInfo *findBot(CALL_HANDLE handle);


#endif	// SPAWN_H
