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

enum AssistType
{
	ASSIST_TYPE_NONE,
	ASSIST_TYPE_LITE,
	ASSIST_TYPE_STANDARD,
	ASSIST_TYPE_ROBBED
};

struct Assist
{
	Player *player;
	AssistType assistLevel;
};

class DamageTracker
{
	public:
		Player *player;
		Player *attacker;
		int damage;
};

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
		bool shipLocked;
		bool lagged;
		int timer;
		int specTimer;
		int repTimer;
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
		bool locked;
		int countdown;
		int timer;
		int elapsed;
		Team teams[2];
		std::vector<DamageTracker> damageTracker;
};

class Logger
{
	public:
		static void log(const char *msg);
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
	char *botVersion;
	char *botName;
	char *botDLL;

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
	char *getReadableElapsed(bool useSeconds);

	void findPlayersInFreqs();
	void parseCommand(Player *p, char* command);
	void setSquads(Player *p, const char* command);
	void setFreqs(Player *p, const char* command);
	void getStatus(Player *p);
	void startMatch();
	void endMatch();
	void prepareMatch();
	void aboutBot(Player *p);
	void shipChange(Player *p, const char* shipStr);
	void setLives(Player *p, const char* livesStr);
	void announce();

	Team* playerTeam(Player *p);
	MatchPlayer* findPlayer(char* playerName);
	void announceScore();
	void checkRemainingPlayers(Team* team);
	void gameEnd();
	void printScoreBoxTop(char* squadName);
	void printPlayerData(MatchPlayer p);
	void printTeamData(Team t);

	void playerKilled(Player* p, Player* k);
	void playerSpecced(Player* p);
	void setSpecTimer(Player* p);
	void setRepelTimer(Player* p);
	void trackDamage(Player* p, Player* k, int damage);
	void resetDamage(Player* p);
	void devalueDamage();
	Assist* findAssist(Player* p, Player* k);
	void checkForcedRepel(Player* p);
};


// The botlist contains every bot to ever be spawned of this type,
// some entries may no longer exist.

extern _linkedlist <botInfo> botlist;

botInfo *findBot(CALL_HANDLE handle);


#endif	// SPAWN_H
