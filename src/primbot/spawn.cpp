#include "spawn.h"

#include "..\algorithms.h"

#include "..\dllcore.cpp"
#include "..\datatypes.cpp"
#include "..\algorithms.cpp"
#include "..\player.cpp"
#include "..\commtypes.cpp"

#include <string.h>
#include <sstream>
#include <windows.h>
#include <iostream>
#include <fstream>
#include <ctime>

#define UNASSIGNED 0xffff


//////// Bot list ////////

_linkedlist <botInfo> botlist;

botInfo *findBot(CALL_HANDLE handle)
{
	_listnode <botInfo> *parse = botlist.head;

	while (parse)
	{
		botInfo *item = parse->item;

		if (item->validate(handle))
			return item;

		parse = parse->next;
	}

	return 0;
}


//////// DLL "import" ////////

void botInfo::tell(BotEvent event)
{
	if (callback && handle)
	{
		event.handle = handle;
		callback(event);
	}
}

void botInfo::gotEvent(BotEvent &event)
{
	if (CONNECTION_DENIED) return;

	switch (event.code)
	{
//////// Periodic ////////
	case EVENT_Tick:
		{
			for(int i = 0; i < 4; i++)
			{
				if(boxes[i].timer == 0)
				{
					char logmsg[256];
					sprintf(logmsg, "Box %i timer expired. Attempting to warp players back to spawn.", (i + 1));
					Logger::log(logmsg);

					timerExpired(i);
				}

				if(boxes[i].timer >= 0)
				{
					boxes[i].timer--;
				}
			}
		}
		break;
//////// Arena ////////
	case EVENT_ArenaEnter:
		{
			arena = (char*)event.p[0];
			me = (Player*)event.p[1];	// if(me) {/*we are in the arena*/}
			bool biller_online = *(bool*)&event.p[2];

			botVersion = "0.4.9 (2024/11/07)";
			botName = "Primacy Bot";
			botDLL = "primbot.dll";

			char logVersion[256];
			sprintf(logVersion, "%s by VanHelsing. Version: %s\n[name: %s] [vanhelsing44@gmail.com]", botName, botVersion, botDLL);
			Logger::log("Bot connected to arena.");
			Logger::log(logVersion);

			sendPublic("?grplogin sysop <PWD>"); // this should come from an .ini file, but for now we'll do it here
		}
		break;
	case EVENT_ArenaSettings:
		{
			settings = (arenaSettings*)event.p[0];
		}
		break;
	case EVENT_ArenaLeave:
		{
			me = 0;
			killTags();
		}
		break;
	case EVENT_ArenaListEntry:
		{
			char *name = (char*)event.p[0];
			bool current = *(bool*)&event.p[1];
			int population = *(int*)&event.p[2];
		}
		break;
	case EVENT_ArenaListEnd:
		{
			char *name = (char*)event.p[0];
			bool current = *(bool*)&event.p[1];
			int population = *(int*)&event.p[2];
		}
		break;
//////// Flag ////////
	case EVENT_FlagGrab:
		{
			Player *p = (Player*)event.p[0];
			Flag *f = (Flag*)event.p[1];
		}
		break;
	case EVENT_FlagDrop:
		{
			Player *p = (Player*)event.p[0];
		}
		break;
	case EVENT_FlagMove:
		{
			Flag *f = (Flag*)event.p[0];
		}
		break;
	case EVENT_FlagVictory:
		{
			int team = *(int*)&event.p[0];
			int reward = *(int*)&event.p[1];
		}
		break;
	case EVENT_FlagGameReset:
		{
		}
		break;
	case EVENT_FlagReward:
		{
			int team = *(int*)&event.p[0];
			int reward = *(int*)&event.p[1];
		}
		break;
//////// Timed ////////
	case EVENT_TimedGameOver:
		{
			Player *p[5];
			for (int i = 0; i < 5; i++)
				p[i] = (Player*)event.p[i];
		}
		break;
//////// Soccer ////////
	case EVENT_SoccerGoal:
		{
			int team = *(int*)&event.p[0];
			int reward = *(int*)&event.p[1];
		}
		break;
	case EVENT_BallMove:
		{
			PBall *ball = (PBall*)event.p[0];
		}
		break;
//////// File ////////
	case EVENT_File:
		{
			char *name = (char*)event.p[0];
		}
		break;
//////// Player ////////
	case EVENT_PlayerEntering:
		{
			Player *p = (Player*)event.p[0];
		}
		break;
	case EVENT_PlayerMove:
		{
			Player *p = (Player*)event.p[0];
		}
		break;
	case EVENT_PlayerWeapon:
		{
			Player *p = (Player*)event.p[0];
			weaponInfo wi;
			wi.n = *(Uint16*)&event.p[1];
		}
		break;
	case EVENT_WatchDamage:
		{
			Player *p = (Player*)event.p[0];	// player
			Player *k = (Player*)event.p[1];	// attacker
			weaponInfo wi;
			wi.n = *(Uint16*)&event.p[2];
			Uint16 energy = *(Uint16*)&event.p[3];
			Uint16 damage = *(Uint16*)&event.p[4];
		}
		break;
	case EVENT_PlayerDeath:
		{
			Player *p = (Player*)event.p[0],
				   *k = (Player*)event.p[1];
			Uint16 bounty = (Uint16)(Uint32)event.p[2];
			Uint16 flags = (Uint16)event.p[3];

			playerKilled(p, k);
		}
		break;
	case EVENT_BannerChanged:
		{
			Player *p = (Player*)event.p[0];
		}
		break;
	case EVENT_PlayerScore:
		{
			Player *p = (Player*)event.p[0];
		}
		break;
	case EVENT_PlayerPrize:
		{
			Player *p = (Player*)event.p[0];
			int prize = *(int *)&event.p[1];
		}
		break;
	case EVENT_PlayerShip:
		{
			Player *p = (Player*)event.p[0];
			Uint16 oldship = (Uint16)(Uint32)event.p[1];
			Uint16 oldteam = (Uint16)(Uint32)event.p[2];

			shipChanged(p, oldship, oldteam);
		}
		break;
	case EVENT_PlayerSpec:
		{
			Player *p = (Player*)event.p[0];
			Uint16 oldteam = (Uint16)(Uint32)event.p[1];
			Uint16 oldship = (Uint16)(Uint32)event.p[2];

			char logmsg[256];
			sprintf(logmsg, "Player %s changed to spec mode", p->name);
			Logger::log(logmsg);

			shipChanged(p, oldship, oldteam);
		}
		break;
	case EVENT_PlayerTeam:
		{
			Player *p = (Player*)event.p[0];
			Uint16 oldteam = (Uint16)(Uint32)event.p[1];
			Uint16 oldship = (Uint16)(Uint32)event.p[2];

			char logmsg[256];
			sprintf(logmsg, "Player %s changed freq from %d to %d", p->name, oldteam, p->team);
			Logger::log(logmsg);
			shipChanged(p, oldship, oldteam);
		}
		break;
	case EVENT_PlayerLeaving:
		{
			Player *p = (Player*)event.p[0];

			killTags(p);
			playerLeftArena(p);
		}
		break;
//////// Selfish ////////
	case EVENT_SelfShipReset:
		{
		}
		break;
	case EVENT_SelfPrize:
		{
			Uint16 prize = (Uint16)(Uint32)event.p[0];
			Uint16 count = (Uint16)(Uint32)event.p[1];
		}
		break;
	case EVENT_SelfUFO:
		{
		}
		break;
	case EVENT_PositionHook:
		{
		}
		break;
//////// Bricks ////////
	case EVENT_BrickDropped:
		{
			int x1 = *(int*)&event.p[0];
			int y1 = *(int*)&event.p[1];
			int x2 = *(int*)&event.p[2];
			int y2 = *(int*)&event.p[3];
			int team = *(int*)&event.p[4];
		}
		break;
//////// LVZ ////////
	case EVENT_ObjectToggled:
		{
			objectInfo obj;
			obj.n = (Uint16)(Uint32)event.p[0];
		}
		break;
//////// Turret ////////
	case EVENT_CreateTurret:
		{
			Player *turreter = (Player*)event.p[0];
			Player *turretee = (Player*)event.p[1];
		}
		break;
	case EVENT_DeleteTurret:
		{
			Player *turreter = (Player*)event.p[0];
			Player *turretee = (Player*)event.p[1];
		}
		break;
//////// Chat ////////
	case EVENT_Chat:
		{
			int type  = *(int*)&event.p[0];
			int sound = *(int*)&event.p[1];
			Player *p = (Player*)event.p[2];
			char *msg = (char*)event.p[3];

			switch (type)
			{
			case MSG_Arena:
				{
				}
				break;
			case MSG_PublicMacro:		if (!p) break;
				{
				}
				break;
			case MSG_Public:			if (!p) break;
				{
					char output[1024];
					sprintf(output, "%s> %s", p->name, msg);
					Logger::log("Public message received:");
					Logger::log(output);

					if(strcmp(msg, "!about") == 0)
					{
						Command c = Command("about");
						gotCommand(p, &c);
					}

					if(strcmp(msg, "!duels") == 0)
					{
						Command c = Command("duels");
						gotCommand(p, &c);
					}

					if(strcmp(msg, "!duel") == 0)
					{
						Command c = Command("duel");
						gotCommand(p, &c);
					}

					if(strcmp(msg, "!resign") == 0)
					{
						Command c = Command("resign");
						gotCommand(p, &c);
					}

					if(strcmp(msg, "!clean") == 0)
					{
						Command c = Command("clean");
						gotCommand(p, &c);
					}

					if(strcmp(msg, "!debug") == 0)
					{
						Command c = Command("debug");
						gotCommand(p, &c);
					}

					if(strcmp(msg, "!help") == 0)
					{
						sendPrivate(p, "!box !duel !duels !max !resign !about");
						sendPrivate(p, "for detailed help, type ::!help <command> (e.g. ::!help box)");
						sendPrivate(p, "most commands can be typed in the public chat, except for !max");
					}

					if(strcmp(msg, "!hi") == 0)
					{
						sendPrivate(p, "Hello!");
					}

					/* Todo: change to something that's not horrible like this */
					if(strcmp(msg, "!box 1") == 0)
					{
						Command c = Command("box 1");
						gotCommand(p, &c);
					}

					if(strcmp(msg, "!box 2") == 0)
					{
						Command c = Command("box 2");
						gotCommand(p, &c);
					}

					if(strcmp(msg, "!box 3") == 0)
					{
						Command c = Command("box 3");
						gotCommand(p, &c);
					}

					if(strcmp(msg, "!box 4") == 0)
					{
						Command c = Command("box 4");
						gotCommand(p, &c);
					}
				}
				break;
			case MSG_Team:				if (!p) break;
				{
				}
				break;
			case MSG_TeamPrivate:		if (!p) break;
				{
				}
				break;
			case MSG_Private:			if (!p) break;
				{
					char outputPrivate[1024];
					sprintf(outputPrivate, "%s> %s", p->name, msg);

					Logger::log("Private message received:");
					Logger::log(outputPrivate);

					if(msg[0] != '!')
					{
						sendPrivate(p, "I don't know wtf to do");
					}
				}
				break;
			case MSG_PlayerWarning:		if (!p) break;
				{
				}
				break;
			case MSG_RemotePrivate:
				{
				}
				break;
			case MSG_ServerError:
				{
				}
				break;
			case MSG_Channel:
				{
				}
				break;
			};
		}
		break;
	case EVENT_LocalCommand:
		{
			Player *p = (Player*)event.p[0];
			Command *c = (Command*)event.p[1];

			gotCommand(p, c);
		}
		break;
	case EVENT_LocalHelp:
		{
			Player *p = (Player*)event.p[0];
			Command *c = (Command*)event.p[1];

			gotHelp(p, c);
		}
		break;
	case EVENT_RemoteCommand:
		{
			char *p = (char*)event.p[0];
			Command *c = (Command*)event.p[1];
			Operator_Level o = *(Operator_Level*)&event.p[2];

			gotRemote(p, c, o);
		}
		break;
	case EVENT_RemoteHelp:
		{
			char *p = (char*)event.p[0];
			Command *c = (Command*)event.p[1];
			Operator_Level o = *(Operator_Level*)&event.p[2];

			gotRemoteHelp(p, c, o);
		}
		break;
//////// Containment ////////
	case EVENT_Init:
		{
			int major	= HIWORD(*(int*)&event.p[0]);
			int minor	= LOWORD(*(int*)&event.p[0]);
			callback	= (CALL_COMMAND)event.p[1];
			playerlist	= (CALL_PLIST)event.p[2];
			flaglist	= (CALL_FLIST)event.p[3];
			map			= (CALL_MAP)event.p[4];
			bricklist	= (CALL_BLIST)event.p[5];

			if (major > CORE_MAJOR_VERSION)
			{
				tell(makeEcho("DLL plugin cannot connect.  This plugin is out of date."));

				CONNECTION_DENIED = true;

				return;
			}
			else if ((major < CORE_MAJOR_VERSION) || (minor < CORE_MINOR_VERSION))
			{
				tell(makeEcho("DLL plugin cannot connect.  This plugin requires the latest version of MERVBot."));

				CONNECTION_DENIED = true;

				return;
			}
			else
				tell(makeEcho("DLL plugin connected."));

		}
		break;
	case EVENT_Term:
		{
			tell(makeEcho("DLL plugin disconnected."));
		}
		break;
	};
}


//////// DLL export ////////

_declspec(dllexport) void __stdcall talk(BotEvent &event)
{
	botInfo *bot;

	bot = findBot(event.handle);

	if (!bot)
	{
		bot = new botInfo(event.handle);
		botlist.append(bot);
	}

	if (bot) bot->gotEvent(event);
}


//////// Tag emulation ////////

int botInfo::get_tag(Player *p, int index)
{
	_listnode <PlayerTag> *parse = taglist.head;
	PlayerTag *tag;

	while (parse)
	{
		tag = parse->item;

		if (tag->p == p)
		if (tag->index == index)
			return tag->data;

		parse = parse->next;
	}

	return 0;
}

void botInfo::set_tag(Player *p, int index, int data)
{
	_listnode <PlayerTag> *parse = taglist.head;
	PlayerTag *tag;

	while (parse)
	{
		tag = parse->item;

		if (tag->p == p)
		if (tag->index == index)
		{
			tag->data = data;
			return;
		}

		parse = parse->next;
	}

	tag = new PlayerTag;
	tag->p = p;
	tag->index = index;
	tag->data = data;
	taglist.append(tag);
}

void botInfo::killTags(Player *p)
{
	_listnode <PlayerTag> *parse = taglist.head;
	PlayerTag *tag;

	while (parse)
	{
		tag = parse->item;
		parse = parse->next;

		if (tag->p != p) continue;

		taglist.kill(tag);
	}
}

void botInfo::killTags()
{
	taglist.clear();
}


//////// LVZ Object Toggling ////////

void botInfo::clear_objects()
{
	num_objects = 0;
}

void botInfo::object_target(Player *p)
{
	object_dest = p;
}

void botInfo::toggle_objects()
{
	Player *p = object_dest;

	if (!p)	tell(makeToggleObjects(UNASSIGNED, (Uint16 *)object_array, num_objects));
	else	tell(makeToggleObjects(p->ident, (Uint16 *)object_array, num_objects));

	num_objects = 0;
}

void botInfo::queue_enable(int id)
{
	if (num_objects == MAX_OBJECTS)
		toggle_objects();

	object_array[num_objects].id = id;
	object_array[num_objects].disabled = false;
	++num_objects;
}

void botInfo::queue_disable(int id)
{
	if (num_objects == MAX_OBJECTS)
		toggle_objects();

	object_array[num_objects].id = id;
	object_array[num_objects].disabled = true;
	++num_objects;
}


//////// Chatter ////////

void botInfo::sendPrivate(Player *player, BYTE snd, char *msg)
{
	tell(makeSay(MSG_Private, snd, player->ident, msg));
}

void botInfo::sendPrivate(Player *player, char *msg)
{
	tell(makeSay(MSG_Private, 0, player->ident, msg));
}

void botInfo::sendTeam(char *msg)
{
	tell(makeSay(MSG_Team, 0, 0, msg));
}

void botInfo::sendTeam(BYTE snd, char *msg)
{
	tell(makeSay(MSG_Team, snd, 0, msg));
}

void botInfo::sendTeamPrivate(Uint16 team, char *msg)
{
	tell(makeSay(MSG_TeamPrivate, 0, team, msg));
}

void botInfo::sendTeamPrivate(Uint16 team, BYTE snd, char *msg)
{
	tell(makeSay(MSG_TeamPrivate, snd, team, msg));
}

void botInfo::sendPublic(char *msg)
{
	tell(makeSay(MSG_Public, 0, 0, msg));
}

void botInfo::sendArena(char *msg)
{
	tell(makeSay(MSG_Arena, 0, 0, msg));
}

void botInfo::sendPublic(BYTE snd, char *msg)
{
	tell(makeSay(MSG_Public, snd, 0, msg));
}

void botInfo::sendPublicMacro(char *msg)
{
	tell(makeSay(MSG_PublicMacro, 0, 0, msg));
}

void botInfo::sendPublicMacro(BYTE snd, char *msg)
{
	tell(makeSay(MSG_PublicMacro, snd, 0, msg));
}

void botInfo::sendChannel(char *msg)
{
	tell(makeSay(MSG_Channel, 0, 0, msg));
}

void botInfo::sendRemotePrivate(char *msg)
{
	tell(makeSay(MSG_RemotePrivate, 0, 0, msg));
}

void botInfo::sendRemotePrivate(char *name, char *msg)
{
	String s;
	s += ":";
	s += name;
	s += ":";
	s += msg;

	sendRemotePrivate(s);
}

//////// Primacy Bot Auxiliary Functions ////////

char *botInfo::getShipName(int id)
{
	switch(id)
	{
		case SHIP_Warbird:		return "Warbird";
		case SHIP_Javelin:		return "Javelin";
		case SHIP_Spider:		return "Spider";
		case SHIP_Leviathan:	return "Leviathan";
		case SHIP_Terrier:		return "Terrier";
		case SHIP_Weasel:		return "Weasel";
		case SHIP_Lancaster:	return "Lancaster";
		case SHIP_Shark:		return "Shark";
		case SHIP_Spectator:	return "Spectator";
	}
	return "";
};

void botInfo::warpTo(Player *p, int x, int y)
{
	char out[255];
	sprintf(out, "*warpto %d %d", x, y);
	sendPrivate(p, out);
}

void botInfo::givePrizes(Player *p)
{
	sendPrivate(p, "*prize #13"); //full charge
	sendPrivate(p, "*prize #-21"); //remove repel
	sendPrivate(p, "*prize #-21"); //remove repel
	sendPrivate(p, "*prize #-22"); //remove burst
	sendPrivate(p, "*prize #-22"); //remove burst
	sendPrivate(p, "*prize #-23"); //remove decoy
	sendPrivate(p, "*prize #-23"); //remove decoy
	sendPrivate(p, "*prize #-24"); //remove thor
	sendPrivate(p, "*prize #-24"); //remove thor
	sendPrivate(p, "*prize #-24"); //remove thor
	sendPrivate(p, "*prize #-26"); //remove brick
	sendPrivate(p, "*prize #-26"); //remove brick
	sendPrivate(p, "*prize #-27"); //remove rocket
	sendPrivate(p, "*prize #-28"); //remove portal
	sendPrivate(p, "*prize #-28"); //remove portal
}

void botInfo::announceScore(int idx)
{
	int p1_score = boxes[idx].player_1_score;
	int p2_score = boxes[idx].player_2_score;

	char msg[100];
	if (p1_score > p2_score)
	{
		sprintf(msg, "%d-%d to %s", p1_score, p2_score, boxes[idx].player_1->name);
	} 
	else if (p1_score < p2_score)
	{
		sprintf(msg, "%d-%d to %s", p2_score, p1_score, boxes[idx].player_2->name);
	}
	else
	{
		sprintf(msg, "Tied %d-%d", p1_score, p2_score);
	}
	Logger::log(msg);

	sendPrivate(boxes[idx].player_1, msg);
	sendPrivate(boxes[idx].player_2, msg);
}

void botInfo::announceWinner(int idx)
{
	char msg[256];
	sprintf(msg, "Attempting to finish the match in box %i and announce a winner...", (idx + 1));
	Logger::log(msg);

	int p1_score = boxes[idx].player_1_score;
	int p2_score = boxes[idx].player_2_score;

	char msgaw[256];
	if (p1_score > p2_score)
	{
		sprintf(msgaw, "*arena %s wins! %i-%i", boxes[idx].player_1->name, p1_score, p2_score);
	}
	else if (p2_score > p1_score)
	{
		sprintf(msgaw, "*arena %s wins! %i-%i", boxes[idx].player_2->name, p2_score, p1_score);
	}
	else if (p1_score == p2_score)
	{
		sprintf(msgaw, "*arena Tie! %i-%i", p1_score, p2_score);
	}

	//TODO: The announcement of the winner should come after the last kill private message is sent,
	//Sleep is an idea but delays everything.
	Sleep(1000);
	sendPublic(msgaw);
	Logger::log(msgaw);

	boxes[idx].locked = false;
	boxes[idx].player_1 = NULL;
	boxes[idx].player_2 = NULL;
	boxes[idx].player_1_score = 0;
	boxes[idx].player_2_score = 0;
	boxes[idx].limit = 10;
	boxes[idx].timer = -1;
	
	char msgf[50];
	sprintf(msgf, "*arena Box %i is now free!", (idx + 1));
	sendPublic(msgf);
	Logger::log(msgf);
}

bool botInfo::enterBox(int idx, Player *p)
{
	char msgAssign[100];
	if(boxes[idx].player_1 == NULL)
	{
		boxes[idx].player_1 = p;
		sprintf(msgAssign, "%s assigned to slot #1 in box %d", p->name, (idx + 1));
		Logger::log(msgAssign);
		return true;
	}
	else if(boxes[idx].player_2 == NULL)
	{
		boxes[idx].player_2 = p;
		sprintf(msgAssign, "%s assigned to slot #2 in box %d", p->name, (idx + 1));
		Logger::log(msgAssign);
		return true;
	}
	else
	{
		sprintf(msgAssign, "Unable to assign %s to box %d, box is full.", p->name, (idx + 1));
		Logger::log(msgAssign);
		return false;
	}
}

bool botInfo::leaveBox(int idx, Player *p)
{
	char msgLeave[100];
	if(boxes[idx].player_1 == p)
	{
		sprintf(msgLeave, "%s left box %d on slot #1", p->name, (idx + 1));
		Logger::log(msgLeave);
		boxes[idx].player_1 = NULL;
		return true;
	}
	else if(boxes[idx].player_2 == p)
	{
		sprintf(msgLeave, "%s left box %d on slot #2", p->name, (idx + 1));
		Logger::log(msgLeave);
		boxes[idx].player_2 = NULL;
		return true;
	}
	else
	{
		sprintf(msgLeave, "%s not in box %d, can't leave.", p->name, (idx + 1));
		return false;
	}
}

int botInfo::playerInBox(Player *p)
{
	char msg[50];
	sprintf(msg, "Looking for player %s", p->name);
	Logger::log(msg);
	for(int i = 0; i < sizeof(boxes); i++)
	{
		if(boxes[i].player_1 == p || boxes[i].player_2 == p)
		{
			return i;
			break;
		}
	}
	return -1;
}


//////// Primacy Bot Commands ////////

void botInfo::assignToBox(Player *p, int selectedBox)
{
	char msg[100];
	sprintf(msg, "Attempting to assign player %s to box %d", p->name, selectedBox);
	Logger::log(msg);

	if(selectedBox > 0 && selectedBox <= sizeof(boxes) && p->ship != SHIP_Spectator)
	{
		int idx = selectedBox - 1;
		int p_idx = playerInBox(p);
		bool boxAssigned = false;
		
		if(p_idx != -1)
		{
			if(p_idx == idx)
			{
				sprintf(msg, "Player %s is already in box %d", p->name, selectedBox);
				Logger::log(msg);
				sendPrivate(p, "You're already in that box");
			}
			else
			{
				if(boxes[p_idx].locked)
				{
					Logger::log("Player is already in a duel, can't assign to new box.");
					sendPrivate(p, "You're already in a duel, use !resign to be able to change boxes.");
				}
				else
				{
					sprintf(msg, "Player %s is already in box %d, but no duel started, changing.", p->name, (p_idx + 1));
					Logger::log(msg);
					bool leftBox = leaveBox(p_idx, p);
					boxAssigned = enterBox(idx, p);
				}
			}
		}
		else
		{
			sprintf(msg, "Player %s not in a box, assigning to box %d", p->name, selectedBox);
			Logger::log(msg);
			boxAssigned = enterBox(idx, p);
		}

		if(!boxAssigned)
		{
			sprintf(msg, "Box is full! Unable to assign player %s to box %d.", p->name, selectedBox);
			Logger::log(msg);
			sendPrivate(p, "Unable to assign to box. Try another box.");
		}
		else
		{
			sprintf(msg, "Assigned player %s to box %d", p->name, selectedBox);
			Logger::log(msg);
			warpTo(p, boxes[idx].x, boxes[idx].y);

			sprintf(msg, "Box #%d", selectedBox);
			sendPrivate(p, msg);
		}
	}
	else
	{
		sprintf(msg, "Invalid box/ship. Box: %d Player: %s Ship: %s", selectedBox, p->name, getShipName(p->ship));
		Logger::log(msg);
		sendPrivate(p, "Invalid box/ship.");
	}
}

void botInfo::duel(Player *p)
{
	char msg[100];
	sprintf(msg, "%s is attempting to start a duel.", p->name);
	Logger::log(msg);
	int idx = playerInBox(p);

	if(idx != -1)
	{
		if(boxes[idx].locked)
		{
			sprintf(msg, "Duel already in progress in box %d", (idx + 1));
			Logger::log(msg);
			sendPrivate(p, msg);
		}
		else
		{
			if(boxes[idx].player_1 == NULL || boxes[idx].player_2 == NULL)
			{
				sprintf(msg, "Need two players to start a duel!");
				Logger::log(msg);
				sendPrivate(p, msg);
			}
			else
			{
				boxes[idx].locked = true;
				warpTo(boxes[idx].player_1, boxes[idx].p1_safe_x, boxes[idx].p1_safe_y);
				warpTo(boxes[idx].player_2, boxes[idx].p2_safe_x, boxes[idx].p2_safe_y);
				givePrizes(boxes[idx].player_1);
				givePrizes(boxes[idx].player_2);

				sprintf(msg, "*arena Box %d is now being used for %s vs %s", (idx + 1), boxes[idx].player_1->name, boxes[idx].player_2->name);
				sendPublic(msg);
				Logger::log(msg);
			}
		}
	}
	else
	{
		Logger::log("Player not in a box, can't start duel.");
		sendPrivate(p, "You're not in a box!");
	}
}

void botInfo::aboutBot(Player *p)
{
	char logmsg[100];
	sprintf(logmsg, "Sending !about or !version info to %s", p->name);
	Logger::log(logmsg);
	
	char aboutMsg[100];
	sprintf(aboutMsg, "%s by VanHelsing. Version: %s", botName, botVersion);
	char aboutMsg2[100];
	sprintf(aboutMsg2, "[name: %s] [vanhelsing44@gmail.com]", botName);

	sendPrivate(p, aboutMsg);
	sendPrivate(p, aboutMsg2);
}

void botInfo::listDuels()
{
	Logger::log("Retrieving duels...");
	bool output = false;

	for(int i = 0; i < sizeof(boxes); i++)
	{
		if(boxes[i].locked == true)
		{
			output = true;
			char msg[100];
			sprintf(msg, "*arena Box %d: %s vs %s (%d-%d)", (i + 1), boxes[i].player_1->name, boxes[i].player_2->name, boxes[i].player_1_score, boxes[i].player_2_score);
			sendPublic(msg);
		}
	}

	if(!output)
	{
		sendPublic("*arena No duels in progress");
		Logger::log("No duels in progress");
	}
}

void botInfo::setLimit(Player *p, int newLimit)
{
	char msg[100];
	sprintf(msg, "Attempting to set limit to %d", newLimit);
	Logger::log(msg);

	int idx = playerInBox(p);

	if(idx != -1)
	{
		if(newLimit > boxes[idx].player_1_score && newLimit > boxes[idx].player_2_score)
		{
			boxes[idx].limit = newLimit;
			sprintf(msg, "Limit set to %d (box %d)", newLimit, (idx + 1));

			if(boxes[idx].player_1 != NULL)
			{
				sendPrivate(boxes[idx].player_1, msg);
			}
			if(boxes[idx].player_2 != NULL)
			{
				sendPrivate(boxes[idx].player_2, msg);
			}

			Logger::log(msg);
		}
		else
		{
			sprintf(msg, "New limit must be greater than current score");
			Logger::log(msg);
			sendPrivate(p, msg);
		}
	}
	else
	{
		sprintf(msg, "Not in a box. Can't change limit.");
		Logger::log(msg);
		sendPrivate(p, msg);
	}
}

void botInfo::cleanBoxes()
{
	Logger::log("Cleaning boxes...");
	for(int i = 0; i < sizeof(boxes); i++)
	{
		boxes[i].player_1 = NULL;
		boxes[i].player_2 = NULL;
		boxes[i].player_1_score = 0;
		boxes[i].player_2_score = 0;
		boxes[i].limit = 10;
		boxes[i].timer = -1;
		boxes[i].locked = false;
	}
	Logger::log("Boxes reset.");
	sendPublic("*arena Boxes reset.");
}

void botInfo::resign(Player *p)
{
	char msg[100];
	sprintf(msg, "Player %s forfeits. Resigning.", p->name);
	Logger::log(msg);
	int idx = playerInBox(p);

	if(idx != -1)
	{
		bool scoreReset = false;
		if(boxes[idx].player_1 == p)
		{
			boxes[idx].player_1_score = 0;
			scoreReset = true;
		}
		if(boxes[idx].player_2 == p)
		{
			boxes[idx].player_2_score = 0;
			scoreReset = true;
		}

		if(scoreReset)
		{
			sendPrivate(p, "Successfully forfeited.");
			Logger::log("Successfully forfeited.");
			announceWinner(idx);
		}
	}
	else
	{
		sprintf(msg, "Not in a box. Can't resign.");
		sendPrivate(p, msg);
		Logger::log(msg);
	}
}

///// Primacy Events /////

void botInfo::playerKilled(Player *p, Player *k)
{
	char killMsg[100];
	sprintf(killMsg, "Player %s killed by %s", p->name, k->name);
	Logger::log(killMsg);

	int idx = playerInBox(p);

	if(idx != -1)
	{
		char msgp[100];
		char msgk[100];

		sprintf(msgk, "You had %d energy when you killed %s", k->energy, p->name);
		sprintf(msgp, "%s had %d energy when you died", k->name, k->energy);

		sendPrivate(p, msgp);
		sendPrivate(k, msgk);

		if(boxes[idx].player_1 == k)
		{
			boxes[idx].player_1_score += 1;
		}
		else if (boxes[idx].player_2 == k)
		{
			boxes[idx].player_2_score += 1;
		}
		
		announceScore(idx);
		setTimer(4, idx);
		
		if(boxes[idx].player_1_score >= boxes[idx].limit || boxes[idx].player_2_score >= boxes[idx].limit)
		{
			announceWinner(idx);
		}
	}
}

void botInfo::shipChanged(Player *p, int oldship, int oldteam)
{
	char logmsg[100];
	sprintf(logmsg, "Spec or change ships triggered from %s", p->name);
	Logger::log(logmsg);

	int idx = playerInBox(p);

	if(idx != -1)
	{
		if(boxes[idx].locked == true)
		{
			if(oldship != p->ship)
			{
				if(p->ship != SHIP_Spectator)
				{
					char logcs[200];
					sprintf(logcs, "%s changed ships from %s to %s (%d)\n", p->name, getShipName(oldship), getShipName(p->ship), p->ship);
					Logger::log(logcs);

					char msgs[100];
					sprintf(msgs, "*arena %s changed ship to %s", p->name, getShipName(p->ship));
					sendPublic(msgs);
					setTimer(2, idx);
				}
				else
				{
					char msgsc[100];
					sprintf(msgsc, "*arena %s changed ship to spectator, finishing match.", p->name);
					sendPublic(msgsc);
					announceWinner(idx);
				}
			}

			if(oldteam != p->team && p->team != 9999)
			{
				char logcf[150];
				sprintf(logcf, "%s changed freq from %d to %d", p->name, oldteam, p->team);
				Logger::log(logcf);
				
				if(boxes[idx].player_1->team == boxes[idx].player_2->team)
				{
					char msgSameFreq[50] = "Both players are on the same freq. Ending.";
					sendPrivate(boxes[idx].player_1, msgSameFreq);
					sendPrivate(boxes[idx].player_2, msgSameFreq);
					Logger::log(msgSameFreq);
					announceWinner(idx);
				}
				else
				{
					char msgFreqChange[50] = "Somebody changed freq, resetting.";
					sendPrivate(boxes[idx].player_1, msgFreqChange);
					sendPrivate(boxes[idx].player_2, msgFreqChange);
					Logger::log(msgFreqChange);
					setTimer(2, idx);
				}
			}
		}
		else
		{
			if(p->ship == SHIP_Spectator)
			{
				sprintf(logmsg, "%s specced while assigned to a box but not already in a match", p->name);
				Logger::log(logmsg);
				bool boxLeft = leaveBox(idx, p);
			}
			else
			{
				sprintf(logmsg, "%s changed ships to %s (%d) while not in a match", p->name, getShipName(p->ship), p->ship);
				Logger::log(logmsg);
				warpTo(p, boxes[idx].x, boxes[idx].y);
			}
		}
	}
}

void botInfo::playerLeftArena(Player *p)
{
	int idx = playerInBox(p);
	
	if(idx != -1)
	{
		char msg[100];
		sprintf(msg, "*arena %s left the arena", p->name);

		Logger::log(msg);
		sendPublic(msg);
		announceWinner(idx);
	}
}

void botInfo::setTimer(int secs, int idx)
{
	boxes[idx].timer = secs;
}

void botInfo::timerExpired(int idx)
{
	char msg[100];
	sprintf(msg, "Timer expired function called on box %i", (idx + 1));
	Logger::log(msg);
	if(idx != -1)
	{
		if(boxes[idx].player_1 != NULL && boxes[idx].player_2 != NULL)
		{
			Logger::log("Warping players to safe locations...");
			warpTo(boxes[idx].player_1, boxes[idx].p1_safe_x, boxes[idx].p1_safe_y);
			warpTo(boxes[idx].player_2, boxes[idx].p2_safe_x, boxes[idx].p2_safe_y);
			givePrizes(boxes[idx].player_1);
			givePrizes(boxes[idx].player_2);
			Logger::log("Players warped.");
			boxes[idx].timer = -1;
			Logger::log("Timer reset.");
		}
	}
}

void botInfo::debug()
{
	Logger::log("Debug called, printing status of boxes...");
	for(int i = 0; i < 4; i++)
	{
		char logdbg[512];
		sprintf(logdbg, "\nBox %i:\nPlayer 1: %s\nPlayer 2: %s\nTimer: %i\nLimit: %i\nLocked: %i\n", (i + 1), boxes[i].player_1 ? boxes[i].player_1->name : "NULL", boxes[i].player_2 ? boxes[i].player_2->name : "NULL", boxes[i].timer, boxes[i].limit, boxes[i].locked);
		Logger::log(logdbg);
	}
}

///// Logging /////

void Logger::log(const char *msg)
{
	time_t now = time(0);
	tm* timeInfo = localtime(&now);
	char timestamp[20];
	strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", timeInfo);

	std::ofstream logFile("primbot.log", std::ios::app);
	if(logFile.is_open())
	{
		logFile << "[" << timestamp << "]  " << msg << std::endl;
		logFile.close();
	}
	printf("%s\n", msg);
}