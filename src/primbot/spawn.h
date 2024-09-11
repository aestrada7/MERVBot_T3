/*
	MERVBot Spawn-callback by Catid@pacbell.net
*/

#ifndef SPAWN_H
#define SPAWN_H

#define STRING_CAST_CHAR
#include "..\dllcore.h"

#include "..\clientprot.h"

struct PlayerTag
{
	Player *p;
	int index;
	int data;
};

class BoxData
{
	public:
		int x;
		int y;
		int p1_safe_x;
		int p1_safe_y;
		int p2_safe_x;
		int p2_safe_y;
		Player *player_1;
		Player *player_2;
		int player_1_score;
		int player_2_score;
		int limit;
		int timer;
		bool locked;
};

#define MAX_OBJECTS 20

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
	BoxData boxes[3];

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
		boxes[0].x = 500;
		boxes[0].y = 819;
		boxes[0].p1_safe_x = 399;
		boxes[0].p1_safe_y = 819;
		boxes[0].p2_safe_x = 586;
		boxes[0].p2_safe_y = 819;
		boxes[0].player_1 = NULL;
		boxes[0].player_2 = NULL;
		boxes[0].player_1_score = 0;
		boxes[0].player_2_score = 0;
		boxes[0].limit = 10;
		boxes[0].timer = -1;
		boxes[0].locked = false;
		boxes[1].x = 180;
		boxes[1].y = 1;
		boxes[1].p1_safe_x = 92;
		boxes[1].p1_safe_y = 1;
		boxes[1].p2_safe_x = 279;
		boxes[1].p2_safe_y = 1;
		boxes[1].player_1 = NULL;
		boxes[1].player_2 = NULL;
		boxes[1].player_1_score = 0;
		boxes[1].player_2_score = 0;
		boxes[1].limit = 10;
		boxes[1].timer = -1;
		boxes[1].locked = false;
		boxes[2].x = 180;
		boxes[2].y = 480;
		boxes[2].p1_safe_x = 92;
		boxes[2].p1_safe_y = 482;
		boxes[2].p2_safe_x = 279;
		boxes[2].p2_safe_y = 482;
		boxes[2].player_1 = NULL;
		boxes[2].player_2 = NULL;
		boxes[2].player_1_score = 0;
		boxes[2].player_2_score = 0;
		boxes[2].limit = 10;
		boxes[2].timer = -1;
		boxes[2].locked = false;
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
	void sendArena(char *msg);

	void sendPublicMacro(char *msg);
	void sendPublicMacro(BYTE snd, char *msg);

	void sendChannel(char *msg);			// #;Message
	void sendRemotePrivate(char *msg);		// :Name:Messsage
	void sendRemotePrivate(char *name, char *msg);

	void gotHelp(Player *p, Command *c);
	void gotCommand(Player *p, Command *c);
	void gotRemoteHelp(char *p, Command *c, Operator_Level l);
	void gotRemote(char *p, Command *c, Operator_Level l);

	char *getShipName(int ship);
	void warpTo(Player *p, int x, int y);
	void announceScore(int idx);
	void announceWinner(int idx);
	bool enterBox(int idx, Player *p);
	bool leaveBox(int idx, Player *p);
	int playerInBox(Player *p);

	void assignToBox(Player *p, int selectedBox);
	void duel(Player *p);
	void aboutBot(Player *p);
	void listDuels();
	void setLimit(Player *p, int newLimit);
	void cleanBoxes();
	void resign(Player *p);

	void playerKilled(Player *p, Player *k);
	void shipChanged(Player *p, int oldship, int oldteam);
	void playerLeftArena(Player *p);

	void setTimer(int secs, int idx);
	void timerExpired(int idx);

	void debug();
};


// The botlist contains every bot to ever be spawned of this type,
// some entries may no longer exist.

extern _linkedlist <botInfo> botlist;

botInfo *findBot(CALL_HANDLE handle);


#endif	// SPAWN_H
