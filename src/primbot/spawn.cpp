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
			for(int i = 0; i < 3; i++)
			{
				if(boxes[i].timer == 0)
				{
					printf("Box %i timer expired\n", i);
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

			printf("Player %s changed from ship %s\n", p->name, getShipName(oldship));
			shipChanged(p, oldship, oldteam);
		}
		break;
	case EVENT_PlayerSpec:
		{
			Player *p = (Player*)event.p[0];
			Uint16 oldteam = (Uint16)(Uint32)event.p[1];
			Uint16 oldship = (Uint16)(Uint32)event.p[2];

			printf("Player %s changed to spec mode\n", p->name);
			shipChanged(p, oldship, oldteam);
		}
		break;
	case EVENT_PlayerTeam:
		{
			Player *p = (Player*)event.p[0];
			Uint16 oldteam = (Uint16)(Uint32)event.p[1];
			Uint16 oldship = (Uint16)(Uint32)event.p[2];

			printf("Player %s changed frequency from %d\n", p->name, oldteam);
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

			printf("Chat event received\n");

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
					printf("Public message received\n");
					printf("Message: %s\n", msg);

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
						sendPrivate(p, "!box !duel !duels !limit !resign !about");
						sendPrivate(p, "for detailed help, type ::!help <command> (e.g. ::!help box)");
						sendPrivate(p, "most commands can be typed in the public chat, except for !limit");
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
					printf("Private message received\n");
					printf("Message: %s\n", msg);

					printf("From: %s\n", p->name);

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
	std::stringstream sstm;
	sstm << "*warpto " << x << " " << y;
	std::string str = sstm.str();
	char *warpToMsg = &str[0];

	sendPrivate(p, warpToMsg);
}

void botInfo::announceScore(int idx)
{
	int p1_score = boxes[idx].player_1_score;
	int p2_score = boxes[idx].player_2_score;

	std::stringstream sstm;
	if (p1_score > p2_score)
	{
		sstm << p1_score << "-" << p2_score << " to " << boxes[idx].player_1->name;
	} 
	else if (p1_score < p2_score)
	{
		sstm << p2_score << "-" << p1_score << " to " << boxes[idx].player_2->name;
	}
	else
	{
		sstm << "Tied " << p1_score << "-" << p2_score;
	}
	std::string str = sstm.str();
	char *msg = &str[0];

	sendPrivate(boxes[idx].player_1, msg);
	sendPrivate(boxes[idx].player_2, msg);
}

void botInfo::announceWinner(int idx)
{
	printf("Attempting to finish the match and announce a winner...\n");
	int p1_score = boxes[idx].player_1_score;
	int p2_score = boxes[idx].player_2_score;

	std::stringstream sstmaw;
	if (p1_score > p2_score)
	{
		sstmaw << "*arena " << boxes[idx].player_1->name << " wins! " << p1_score << "-" << p2_score;
	}
	else if (p2_score > p1_score)
	{
		sstmaw << "*arena " << boxes[idx].player_2->name << " wins! " << p2_score << "-" << p1_score;
	}
	else if (p1_score == p2_score)
	{
		sstmaw << "*arena Tie! " << p1_score << "-" << p2_score;
	}
	std::string straw = sstmaw.str();
	char *msgaw = &straw[0];

	//TODO: The announcement of the winner should come after the last kill private message is sent,
	//Sleep is an idea but delays everything.
	Sleep(1000);
	sendPublic(msgaw);

	boxes[idx].locked = false;
	boxes[idx].player_1 = NULL;
	boxes[idx].player_2 = NULL;
	boxes[idx].player_1_score = 0;
	boxes[idx].player_2_score = 0;
	boxes[idx].limit = 10;
	boxes[idx].timer = -1;

	std::stringstream sstmf;
	sstmf << "*arena Box " << idx + 1 << " is now free!";
	std::string strf = sstmf.str();
	char *msgf = &strf[0];

	sendPublic(msgf);
}

bool botInfo::enterBox(int idx, Player *p)
{
	if(boxes[idx].player_1 == NULL)
	{
		boxes[idx].player_1 = p;
		printf("Assigned to slot 1\n");
		return true;
	}
	else if(boxes[idx].player_2 == NULL)
	{
		boxes[idx].player_2 = p;
		printf("Assigned to slot 2\n");
		return true;
	}
	else
	{
		printf("Box full! \n");
		return false;
	}
}

bool botInfo::leaveBox(int idx, Player *p)
{
	if(boxes[idx].player_1 == p)
	{
		printf("Left box %d on slot 1\n", (idx + 1));
		boxes[idx].player_1 = NULL;
		return true;
	}
	else if(boxes[idx].player_2 == p)
	{
		printf("Left box %d on slot 2\n", (idx + 1));
		boxes[idx].player_2 = NULL;
		return true;
	}
	else
	{
		printf("Not in box %d\n", (idx + 1));
		return false;
	}
}

int botInfo::playerInBox(Player *p)
{
	printf("looking for player %s\n", p->name);
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
	printf("Attempting to assign player %s to box %d\n", p->name, selectedBox);
	if(selectedBox > 0 && selectedBox <= sizeof(boxes) && p->ship != SHIP_Spectator)
	{
		int idx = selectedBox - 1;
		int p_idx = playerInBox(p);
		bool boxAssigned = false;
		
		if(p_idx != -1)
		{
			if(p_idx == idx)
			{
				printf("Player is already in box %d\n", selectedBox);
				sendPrivate(p, "You are already in that box");
			}
			else
			{
				if(boxes[p_idx].locked)
				{
					sendPrivate(p, "You're already in a duel, use !resign to be able to change boxes.");
				}
				else
				{
					printf("Player in a box, but no duel started, changing. %d\n", p_idx);
					bool leftBox = leaveBox(p_idx, p);
					boxAssigned = enterBox(idx, p);
				}
			}
		}
		else
		{
			printf("Player not in a box, assigning to box %d\n", selectedBox);
			boxAssigned = enterBox(idx, p);
		}

		if(!boxAssigned)
		{
			printf("Box is full!\n");
			sendPrivate(p, "Box is full!");
		}
		else
		{
			printf("Assigned. Warping to box %d\n", selectedBox);
			warpTo(p, boxes[idx].x, boxes[idx].y);

			std::stringstream sstmb;
			sstmb << "Box #" << selectedBox;
			std::string strb = sstmb.str();
			char *boxMsg = &strb[0];
			sendPrivate(p, boxMsg);
		}
	}
	else
	{
		sendPrivate(p, "Invalid box/ship.");
	}
}

void botInfo::duel(Player *p)
{
	printf("Attempting to start a duel with %s\n", p->name);
	int idx = playerInBox(p);

	if(idx != -1)
	{
		if(boxes[idx].locked)
		{
			printf("Box %d is locked!\n", idx);
			std::stringstream sstmd;
			sstmd << "Duel already in progress for box " << idx + 1;
			std::string strd = sstmd.str();
			char *msgd = &strd[0];
			sendPrivate(p, msgd);
		}
		else
		{
			if(boxes[idx].player_1 == NULL || boxes[idx].player_2 == NULL)
			{
				printf("Need two players to start a duel!\n");
				sendPrivate(p, "Need two players to start a duel!");
			}
			else
			{
				boxes[idx].locked = true;
				warpTo(boxes[idx].player_1, boxes[idx].p1_safe_x, boxes[idx].p1_safe_y);
				warpTo(boxes[idx].player_2, boxes[idx].p2_safe_x, boxes[idx].p2_safe_y);
				std::stringstream sstmdd;
				sstmdd << "*arena Box " << idx + 1 << " is now being used for " << boxes[idx].player_1->name << " vs " << boxes[idx].player_2->name;
				std::string strdd = sstmdd.str();
				char *msgdd = &strdd[0];
				sendPublic(msgdd);
			}
		}
	}
	else
	{
		sendPrivate(p, "You're not in a box!");
	}
}

void botInfo::aboutBot(Player *p)
{
	printf("Sending !about or !version info...\n");
	sendPrivate(p, "Primacy Bot by VanHelsing. Version: 0.4.2 (2024/09/11)");
	sendPrivate(p, "[name: primbot.dll] [vanhelsing44@gmail.com]");
}

void botInfo::listDuels()
{
	printf("Retrieving duels...\n");
	bool output = false;

	for(int i = 0; i < sizeof(boxes); i++)
	{
		if(boxes[i].locked == true)
		{
			output = true;
			std::stringstream sstma;
			sstma << "*arena Box " << (i + 1) << ": " << boxes[i].player_1->name << " vs " << boxes[i].player_2->name;
			std::string stra = sstma.str();
			char *msga = &stra[0];
			sendPublic(msga);
		}
	}

	if(!output)
	{
		sendPublic("*arena No duels in progress");
	}
}

void botInfo::setLimit(Player *p, int newLimit)
{
	printf("Setting limit to %d\n", newLimit);

	int idx = playerInBox(p);

	if(idx != -1)
	{
		if(newLimit > boxes[idx].player_1_score && newLimit > boxes[idx].player_2_score)
		{
			boxes[idx].limit = newLimit;
			std::stringstream sstml;
			sstml << "Limit set to " << newLimit;
			std::string strml = sstml.str();
			char *msgl = &strml[0];

			if(boxes[idx].player_1 != NULL)
			{
				sendPrivate(boxes[idx].player_1, msgl);
			}
			if(boxes[idx].player_2 != NULL)
			{
				sendPrivate(boxes[idx].player_2, msgl);
			}
		}
		else
		{
			sendPrivate(p, "New limit must be greater than current score");
		}
	}
	else
	{
		sendPrivate(p, "Not in a box. Can't change limit.");
	}
}

void botInfo::cleanBoxes()
{
	printf("Cleaning boxes...\n");
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
	sendPublic("*arena Boxes reset.");
}

void botInfo::resign(Player *p)
{
	printf("Player %s forfeits. Resigning.\n", p->name);
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
			announceWinner(idx);
		}
	}
	else
	{
		sendPrivate(p, "Not in a box. Can't resign.");
	}
}

///// Primacy Events /////

void botInfo::playerKilled(Player *p, Player *k)
{
	printf("Player %s killed by %s\n", p->name, k->name);

	int idx = playerInBox(p);

	if(idx != -1)
	{
		std::stringstream sstmk;
		sstmk << "You had " << k->energy << " energy when you killed " << p->name;
		std::string strk = sstmk.str();
		char *msgk = &strk[0];

		std::stringstream sstmp;
		sstmp << k->name << " had " << k->energy << " energy when you died";
		std::string strp = sstmp.str();
		char *msgp = &strp[0];

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
	printf("Spec or change ships triggered from %s\n", p->name);
	int idx = playerInBox(p);

	if(idx != -1)
	{
		if(boxes[idx].locked == true)
		{
			if(oldship != p->ship)
			{
				printf("%s Changed ships to %s (%d)\n", p->name, getShipName(p->ship), p->ship);

				if(p->ship != SHIP_Spectator)
				{
					std::stringstream sstms;
					sstms << "*arena " << p->name << " changed ship to " << getShipName(p->ship);
					std::string strs = sstms.str();
					char *msgs = &strs[0];

					sendPublic(msgs);
					setTimer(2, idx);
				}
				else
				{
					std::stringstream sstmsc;
					sstmsc << "*arena " << p->name << " changed to spectator, finishing match.";
					std::string strsc = sstmsc.str();
					char *msgsc = &strsc[0];

					sendPublic(msgsc);
					announceWinner(idx);
				}
			}

			if(oldteam != p->team && p->team != 9999)
			{
				printf("Changed freq\n");
				
				if(boxes[idx].player_1->team == boxes[idx].player_2->team)
				{
					sendPrivate(boxes[idx].player_1, "Both players are on the same freq. Ending.");
					sendPrivate(boxes[idx].player_2, "Both players are on the same freq. Ending.");
					announceWinner(idx);
				}
				else
				{
					sendPrivate(boxes[idx].player_1, "Somebody changed freq, resetting.");
					sendPrivate(boxes[idx].player_2, "Somebody changed freq, resetting.");
					setTimer(2, idx);
				}
			}
		}
	}
}

void botInfo::playerLeftArena(Player *p)
{
	int idx = playerInBox(p);
	
	if(idx != -1)
	{
		std::stringstream sstmla;
		sstmla << "*arena " << p->name << " left the arena";
		std::string strla = sstmla.str();
		char *msgla = &strla[0];

		sendPublic(msgla);
		announceWinner(idx);
	}
}

void botInfo::setTimer(int secs, int idx)
{
	boxes[idx].timer = secs;
}

void botInfo::timerExpired(int idx)
{
	printf("Timer expired function called on box %i\n", idx);
	if(idx != -1)
	{
		if(boxes[idx].player_1 != NULL && boxes[idx].player_2 != NULL)
		{
			printf("Warping players to safe locations...\n");
			warpTo(boxes[idx].player_1, boxes[idx].p1_safe_x, boxes[idx].p1_safe_y);
			warpTo(boxes[idx].player_2, boxes[idx].p2_safe_x, boxes[idx].p2_safe_y);
			printf("Players warped.\n");
			boxes[idx].timer = -1;
			printf("Timer reset\n");
		}
	}
}

void botInfo::debug()
{
	for(int i = 0; i < 3; i++)
	{
		printf("Box %i\n", i);
		printf("Player 1: %s\n", boxes[i].player_1 ? boxes[i].player_1->name : "NULL");
		printf("Player 2: %s\n", boxes[i].player_2 ? boxes[i].player_2->name : "NULL");
		printf("Timer: %i\n", boxes[i].timer);
		printf("Limit: %i\n", boxes[i].limit);
		printf("Locked: %i\n", boxes[i].locked);
	}
}