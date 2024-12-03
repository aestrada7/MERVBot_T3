#include "spawn.h"

#include "..\algorithms.h"

#include "..\dllcore.cpp"
#include "..\datatypes.cpp"
#include "..\algorithms.cpp"
#include "..\player.cpp"
#include "..\commtypes.cpp"

#include <cstdlib>
#include <iostream>
#include <fstream>
#include <ctime>
#include <stdexcept>

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
			if(match.countdown > -1)
			{
				match.countdown--;
				if(match.countdown > 0 && match.countdown <= 3)
				{
					char ctd[10];
					sprintf(ctd, "*arena %d", match.countdown);
					sendPublic(ctd);
				}
				else if(match.countdown == 0)
				{
					Logger::log("Countdown expired. Preparing match.");
					sendPublic("*arena Go!");
					prepareMatch();
				}
			}

			if(match.timer > -1)
			{
				match.timer--;
				match.elapsed++;
			}

			for(int i = 0; i < 2; i++)
			{
				for(int j = 0; j < match.teams[i].players.size(); j++)
				{
					MatchPlayer* mp = &match.teams[i].players[j];
					if(mp->timer > -1)
					{
						mp->timer--;

						if(mp->timer == 0)
						{
							mp->shipLocked = true;
							char out[256];
							sprintf(out, "Player %s's ship change timer expired.", mp->name);
							Logger::log(out);
						}
					}
					if(mp->specTimer > -1)
					{
						mp->specTimer--;

						if(mp->specTimer == 0)
						{
							char out[256];
							sprintf(out, "Player %s's spec timer expired.", mp->name);
							Logger::log(out);
							playerSpecced(mp->player);
						}
					}
					if(mp->repTimer > -1)
					{
						mp->repTimer--;
					}
				}
			}

			devalueDamage();
		}
		break;
//////// Arena ////////
	case EVENT_ArenaEnter:
		{
			arena = (char*)event.p[0];
			me = (Player*)event.p[1];	// if(me) {/*we are in the arena*/}
			bool biller_online = *(bool*)&event.p[2];

			botVersion = "0.4.5 (2024/12/2)";
			botName = "T3 League Bot";
			botDLL = "leaguebot.dll";

			char logVersion[256];
			sprintf(logVersion, "%s by VanHelsing. Version: %s\n[name: %s] [vanhelsing44@gmail.com]", botName, botVersion, botDLL);
			Logger::log("Bot connected to arena.");
			Logger::log(logVersion);

			sendPublic("?grplogin sysop <PWD>"); // this should come from an .ini file, but for now we'll do it here
			sendPrivate(me, "!ownbot on");

			Team teamA;
			teamA.freq = 0;
			teamA.squad = "Squad A";
			teamA.score = 0;

			Team teamB;
			teamB.freq = 1;
			teamB.squad = "Squad B";
			teamB.score = 0;

			match.sideAX = 250;
			match.sideAY = 380;
			match.sideBX = 792;
			match.sideBY = 695;
			match.numPlayers = 3;
			match.lives = 3;
			match.duration = 20;
			match.lagEnforcing = true;
			match.gameType = "Unofficial";
			match.locked = false;
			match.countdown = -1;
			match.timer = -1;
			match.elapsed = 0;
			match.teams[0] = teamA;
			match.teams[1] = teamB;
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

			if(wi.type == PROJ_Repel)
			{
				checkForcedRepel(p);
			}
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
			
			trackDamage(p, k, damage);
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
		}
		break;
	case EVENT_PlayerSpec:
		{
			Player *p = (Player*)event.p[0];
			Uint16 oldteam = (Uint16)(Uint32)event.p[1];
			Uint16 oldship = (Uint16)(Uint32)event.p[2];

			setSpecTimer(p);
		}
		break;
	case EVENT_PlayerTeam:
		{
			Player *p = (Player*)event.p[0];
			Uint16 oldteam = (Uint16)(Uint32)event.p[1];
			Uint16 oldship = (Uint16)(Uint32)event.p[2];
		}
		break;
	case EVENT_PlayerLeaving:
		{
			Player *p = (Player*)event.p[0];

			killTags(p);
			playerSpecced(p);
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
			/* move bot to center of map */
			tell(makeFollowing(false));
			tell(makeFlying(true));
			me->move(512 * 16, 600 * 16);
			tell(makeSendPosition(true));
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

					parseCommand(p, msg);
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

//////// League Bot Auxiliary Functions ////////

void botInfo::warpTo(Player *p, int x, int y)
{
	char out[255];
	sprintf(out, "*warpto %d %d", x, y);
	sendPrivate(p, out);
}

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

char *botInfo::getReadableElapsed(bool showSeconds)
{
	char* timeStr = new char[6];
	int minutes = match.elapsed / 60;
	if(showSeconds)
	{
		int seconds = match.elapsed % 60;
		sprintf(timeStr, "%02d:%02d", minutes, seconds);
	}
	else
	{
		sprintf(timeStr, "%d", minutes);
	}
	return timeStr;
}

//////// League Bot Common Functions ////////

void botInfo::parseCommand(Player *p, char* command)
{
	if(*command == '.')
	{
		bool isMod = false;
		bool isLimited = false;

		switch (p->access)
		{
			case OP_Duke:
			case OP_Baron:
			case OP_King:
			case OP_Emperor:
			case OP_RockStar:
			case OP_Q:
			case OP_God:
			case OP_Owner:
			case OP_SysOp:
			case OP_SuperModerator:
			case OP_Moderator:
			{
				isMod = true;
			}
			case OP_Limited:
			{
				isLimited = true;
			}
		}

		char cmd[200];
		const char delim[2] = "|";

		strcpy(cmd, command);

		char *token;
		token = strtok(cmd, delim);

		while(token != NULL)
		{
			char commandName[15];
			char commandArgs[100];

			size_t leadingSpaces = strspn(token, " ");
			if (leadingSpaces > 0)
			{
				memmove(token, token + leadingSpaces, strlen(token + leadingSpaces) + 1);
			}

			char *space = strchr(token, ' ');

			if(space != NULL)
			{
				strncpy(commandName, token, space - token);
				commandName[space - token] = '\0';
				strcpy(commandArgs, space + 1);
			}
			else
			{
				strcpy(commandName, token);
				commandArgs[0] = '\0';
			}

			char msg[255];
			sprintf(msg, "Command '%s' discovered with args '%s'", commandName, commandArgs);
			Logger::log(msg);

			if (strcmp(commandName, ".squads") == 0)
			{
				if(isMod)
				{
					setSquads(p, commandArgs);
				}
				else
				{
					Logger::log("Permission denied (not a mod).");
				}
			}
			else if (strcmp(commandName, ".freqs") == 0)
			{
				if(isMod)
				{
					setFreqs(p, commandArgs);
				}
				else
				{
					Logger::log("Permission denied (not a mod).");
				}
			}
			else if (strcmp(commandName, ".status") == 0)
			{
				getStatus(p);
			}
			else if (strcmp(commandName, ".start") == 0)
			{
				if(isMod || isLimited)
				{
					startMatch();
				}
				else
				{
					Logger::log("Permission denied (not a mod/limited).");
				}
			}
			else if (strcmp(commandName, ".end") == 0)
			{
				if(isMod || isLimited)
				{
					gameEnd();
				}
				else
				{
					Logger::log("Permission denied (not a mod/limited).");
				}
			}
			else if (strcmp(commandName, ".sc") == 0)
			{
				shipChange(p, commandArgs);
			}
			else if (strcmp(commandName, ".announce") == 0)
			{
				if(isMod || isLimited)
				{
					announce();
				}
				else
				{
					Logger::log("Permission denied (not a mod/limited).");
				}
			}
			else if (strcmp(commandName, ".about") == 0)
			{
				aboutBot(p);
			}
			else
			{
				char out[255];
				sprintf(out, "Unknown command: %s", commandName);
				Logger::log(out);
				sendPrivate(p, out);
			}

			token = strtok(NULL, delim);
		}
	}
}

void botInfo::findPlayersInFreqs()
{
	Logger::log("Finding players in freqs...");
	char out[255];

	_listnode <Player> *parse = playerlist->head;

	while (parse)
	{
		Player *p = parse->item;
		bool playerFound = false;

		if(p->team == match.teams[0].freq && p->ship != SHIP_Spectator)
		{
			MatchPlayer mp;
			mp.name = p->name;
			mp.lives = 3;
			mp.kills = 0;
			mp.assists = 0;
			mp.deaths = 0;
			mp.lagouts = 0;
			mp.forcedReps = 0;
			mp.teamkills = 0;
			mp.mvpPoints = 0;
			mp.shipLocked = true;
			mp.lagged = false;
			mp.timer = -1;
			mp.specTimer = -1;
			mp.repTimer = -1;
			mp.player = p;

			match.teams[0].players.push_back(mp);

			sprintf(out, "Found player %s in freq %d.", p->name, p->team);
			Logger::log(out);
			
			playerFound = true;
			sendPrivate(p, "*watchdamage 1");
		}

		if(p->team == match.teams[1].freq && p->ship != SHIP_Spectator)
		{
			MatchPlayer mp;
			mp.name = p->name;
			mp.lives = 3;
			mp.kills = 0;
			mp.assists = 0;
			mp.deaths = 0;
			mp.lagouts = 0;
			mp.forcedReps = 0;
			mp.teamkills = 0;
			mp.mvpPoints = 0;
			mp.lagged = false;
			mp.shipLocked = true;
			mp.timer = -1;
			mp.specTimer = -1;
			mp.repTimer = -1;
			mp.player = p;

			match.teams[1].players.push_back(mp);

			sprintf(out, "Found player %s in freq %d.", p->name, p->team);
			Logger::log(out);

			playerFound = true;
			sendPrivate(p, "*watchdamage 1");
		}

		if(!playerFound && p->ship != SHIP_Spectator)
		{
			sendPrivate(p, "*setship 9");
		}

		parse = parse->next;
	}
}

Team* botInfo::playerTeam(Player *p)
{
	for(int i = 0; i < 2; i++)
	{
		for(int j = 0; j < match.teams[i].players.size(); j++)
		{
			MatchPlayer mp = match.teams[i].players[j];
			if(strcmp(p->name, mp.name) == 0)
			{
				return &match.teams[i];
			}
		}
	}
	Team t;
	return &t;
}

MatchPlayer* botInfo::findPlayer(char* playerName)
{
	char out[255];
	sprintf(out, "Looking for player %s", playerName);
	Logger::log(out);

	for(int i = 0; i < 2; i++)
	{
		for(int j = 0; j < match.teams[i].players.size(); j++)
		{
			MatchPlayer* mp = &match.teams[i].players[j];
			if(strcmp(playerName, mp->name) == 0)
			{
				sprintf(out, "Found player %s in freq %d.", mp->name, mp->player->team);
				Logger::log(out);
				return mp;
			}
		}
	}

	Logger::log("Player not found.");
	throw std::runtime_error("Player not found.");
	MatchPlayer* empty;
	return empty;
}

//////// League Bot Commands ////////

void botInfo::setSquads(Player *p, const char* squadStr)
{
	Logger::log("Setting squad names...");
	//todo: allow reuse of current squad names
	//todo: allow only one parameter (either -a=x or -b=y)
	//todo: allow spaces in squad a

	char squadA[50] = "Team A";
	char squadB[50] = "Team B";
	//std::string squadA = match.teams[0].squad;
	//std::string squadB = match.teams[1].squad;

	//sscanf(squadStr, "-a=%[a-zA-Z0-9 ] -b=%[^\n]", squadA, squadB);
	sscanf(squadStr, "-a=%[a-zA-Z0-9] -b=%[^\n]", squadA, squadB);

	match.teams[0].squad = new char[strlen(squadA) + 1];
	strcpy(match.teams[0].squad, squadA);
	match.teams[1].squad = new char[strlen(squadB) + 1];
	strcpy(match.teams[1].squad, squadB);

	char out[255];
	sprintf(out, "%s vs %s", match.teams[0].squad, match.teams[1].squad);
	Logger::log(out);
	sendPrivate(p, out);
}

void botInfo::setFreqs(Player *p, const char* freqStr)
{
	Logger::log("Setting freqs...");
	int freqA = match.teams[0].freq;
	int freqB = match.teams[1].freq;

	sscanf(freqStr, "-a=%d -b=%d", &freqA, &freqB);

	match.teams[0].freq = freqA;
	match.teams[1].freq = freqB;

	char out[255];
	sprintf(out, "%s: %d  %s: %d", match.teams[0].squad, match.teams[0].freq, match.teams[1].squad, match.teams[1].freq);
	Logger::log(out);
	sendPrivate(p, out);
}

void botInfo::startMatch()
{
	Logger::log("Start command receieved. Starting match.");
	
	//subgame locking
	sendPublic("*lock");
	//asss locking
	sendPublic("?lockarena -n");

	sendPublic("*arena Starting in 10 seconds...");
	findPlayersInFreqs();

	match.countdown = 10;
}

void botInfo::prepareMatch()
{
	char out[255];
	sprintf(out, "*arena <----- %s vs %s ----->", match.teams[0].squad, match.teams[1].squad);
	sendPublic(out);
	Logger::log(out);

	match.timer = match.duration * 60;

	Logger::log("Warping players to their starting spots...");
	for(int i = 0; i < match.teams[0].players.size(); i++)
	{
		MatchPlayer p = match.teams[0].players[i];
		sprintf(out, "Warping %s", p.name);
		Logger::log(out);
		warpTo(p.player, match.sideAX, match.sideAY);
	}

	for(int j = 0; j < match.teams[1].players.size(); j++)
	{
		MatchPlayer px = match.teams[1].players[j];
		sprintf(out, "Warping %s", px.name);
		Logger::log(out);
		warpTo(px.player, match.sideBX, match.sideBY);
	}
	match.locked = true;
	Logger::log("Match locked and started!");
}

void botInfo::endMatch()
{
	Team teamA;
	teamA.freq = match.teams[0].freq;
	teamA.squad = match.teams[0].squad;
	teamA.score = 0;

	Team teamB;
	teamB.freq = match.teams[1].freq;
	teamB.squad = match.teams[1].squad;
	teamB.score = 0;

	match.teams[0] = teamA;
	match.teams[1] = teamB;

	match.countdown = -1;
	match.timer = -1;
	match.elapsed = 0;

	match.damageTracker.clear();
	sendPublic("*watchdamage 0");
}

void botInfo::getStatus(Player *p)
{
	Logger::log("Getting status...");
	char gameTxt[255];
	char freqTxt[255];
	char playTxt[255];
	char durTxt[255];
	char infoTxt[255] = "Lag limit enforcing: Enabled";
	char typeTxt[255];

	sprintf(gameTxt, "Game: %s vs %s", match.teams[0].squad, match.teams[1].squad);
	sprintf(freqTxt, "On freqs: %d and %d", match.teams[0].freq, match.teams[1].freq);
	sprintf(playTxt, "Deaths: %d Number of Players: %d", match.lives, match.numPlayers);
	sprintf(durTxt, "Duration: %d Score: %d - %d", match.duration, match.teams[0].score, match.teams[1].score);
	sprintf(typeTxt, "Game type: %s", match.gameType);

	sendPrivate(p, gameTxt);
	sendPrivate(p, freqTxt);
	sendPrivate(p, playTxt);
	sendPrivate(p, durTxt);
	sendPrivate(p, infoTxt);
	sendPrivate(p, typeTxt);

	char out[512];
	sprintf(out, "\n%s\n%s\n%s\n%s\n%s\n%s\n", gameTxt, freqTxt, playTxt, durTxt, infoTxt, typeTxt);
	Logger::log(out);
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

void botInfo::shipChange(Player *p, const char* shipStr)
{
	if(match.locked)
	{
		char out[255];
		sprintf(out, "Player %s attempting to change ship to %s", p->name, shipStr);
		Logger::log(out);

		MatchPlayer* mp = findPlayer(p->name);

		if(!mp->shipLocked)
		{
			int shipNumber = -1;
			shipNumber = atoi(shipStr);
			sprintf(out, "Discovered ship number: %d", shipNumber);
			Logger::log(out);

			if(shipNumber >= SHIP_Warbird && shipNumber < SHIP_Spectator)
			{
				sprintf(out, "*setship %d", shipNumber);
				sendPrivate(p, out);

				sprintf(out, "*arena %s changes ships", p->name);
				sendPublic(out);

				sprintf(out, "Player %s changed ship to %s", p->name, getShipName(shipNumber - 1));
				Logger::log(out);

				mp->shipLocked = true;
			}
			else
			{
				sprintf(out, "Invalid ship number.");
				sendPrivate(p, out);
				Logger::log(out);
			}
		}
		else
		{
			sprintf(out, "Ship change failed. 10 seconds have passed.");
			sendPrivate(p, out);
			Logger::log(out);
		}
	}
}

void botInfo::announce()
{
	char out[255];
	sprintf(out, "*zone %s vs %s will be held in ?go league", match.teams[0].squad, match.teams[1].squad);
	Logger::log(out);
	sendPublic(out);
}

//////// League Bot Events ////////

void botInfo::playerKilled(Player *p, Player *k)
{
	const int SHIP_CHANGE_TIME = 10;

	if(match.locked)
	{
		char out[255];
		sprintf(out, "%s killed by %s", p->name, k->name);
		Logger::log(out);

		Team* teamKilled = playerTeam(p);
		Team* teamKiller = playerTeam(k);
		MatchPlayer* killed;
		MatchPlayer* killer;

		try
		{
			//if player not in match it kills the program
			//throws an error on the findPlayer method to handle the use case of a player not in the match
			sprintf(out, "Retrieving MatchPlayer object for %s (killed)", p->name);
			Logger::log(out);
			killed = findPlayer(p->name);

			sprintf(out, "Retrieving MatchPlayer object for %s (killer)", k->name);
			Logger::log(out);
			killer = findPlayer(k->name);
		}
		catch(const std::exception& e)
		{
			return;
		}

		char tkTxt[20] = " - Teamkill! ";
		bool isTeamkill = false;

		char asTxt[50];
		bool hasAssist = false;

		Assist *assist = findAssist(p, k);
		if(assist->assistLevel != ASSIST_TYPE_NONE)
		{
			hasAssist = true;
			MatchPlayer* assistPlayer = findPlayer(assist->player->name);
			assistPlayer->assists += 1;

			if(assist->assistLevel == ASSIST_TYPE_ROBBED)
			{
				assistPlayer->mvpPoints += 2;
				sprintf(asTxt, " - Robbed! from %s ", assist->player->name);
			}
			else if(assist->assistLevel == ASSIST_TYPE_STANDARD)
			{
				assistPlayer->mvpPoints += 1;
				sprintf(asTxt, " - Assist! from %s ", assist->player->name);
			}
			else if(assist->assistLevel == ASSIST_TYPE_LITE)
			{
				assistPlayer->mvpPoints += 0.5;
				sprintf(asTxt, " - Lite Assist! from %s ", assist->player->name);
			}
		}
		resetDamage(p);

		if(teamKilled->freq != teamKiller->freq)
		{
			killer->kills += 1;
			killer->mvpPoints += 2;

			teamKiller->score += 1;
		}
		else
		{
			isTeamkill = true;
			killer->teamkills += 1;
			killer->mvpPoints -= 0.5;

			if(match.teams[0].freq != teamKilled->freq)
			{
				match.teams[0].score += 1;
			}
			else if(match.teams[1].freq != teamKilled->freq)
			{
				match.teams[1].score += 1;
			}
		}

		killed->deaths += 1;
		killed->lives -= 1;

		char msg[255];
		if(killed->lives == 0)
		{
			int countOut = 0;

			for(int i = 0; i < 2; i++)
			{
				for(int j = 0; j < match.teams[i].players.size(); j++)
				{
					MatchPlayer p = match.teams[i].players[j];
					if(p.lives == 0)
					{
						countOut += 1;
					}
				}
			}

			if(countOut == 1)
			{
				killed->mvpPoints -= 2;
			}
			sprintf(msg, "*arena (OUT) %s kb %s%s%s- Kill time: %s", killed->name, killer->name, (isTeamkill ? tkTxt : " "), (hasAssist ? asTxt : " "), getReadableElapsed(true));
			sendPrivate(p, "*setship 9");
			sendPublic(msg);
			Logger::log(msg);
		}
		else
		{
			sprintf(msg, "*arena (%d/%d) %s kb %s%s%s- Kill time: %s", killed->deaths, match.lives, killed->name, killer->name, (isTeamkill ? tkTxt : " "), (hasAssist ? asTxt : " "), getReadableElapsed(true));
			sendPublic(msg);
			Logger::log(msg);

			sprintf(msg, "You have 10 seconds to change ship, type .sc # to do so.");
			sendPrivate(p, msg);

			killed->shipLocked = false;
			killed->timer = SHIP_CHANGE_TIME;
		}

		announceScore();
		checkRemainingPlayers(teamKilled);
	}
}

void botInfo::playerSpecced(Player *p)
{
	if(match.locked)
	{
		char out[255];
		sprintf(out, "%s Specced (since arena is locked, likely lagged out, or a mod went in)", p->name);
		Logger::log(out);

		Team* team = playerTeam(p);
		MatchPlayer* mp;

		sprintf(out, "*arena %s Lagged out!", p->name);
		sendPublic(out);
		
		try
		{
			mp = findPlayer(p->name);
			mp->lagouts += 1;
			mp->lagged = true;

			if(team->players.size() > 0)
			{
				checkRemainingPlayers(team);
			}
		}
		catch(const std::exception& e)
		{
			Logger::log("Error caught finding player in playerSpecced");
			return;
		}
	}
}

void botInfo::setSpecTimer(Player *p)
{
	if(match.locked)
	{
		MatchPlayer* mp;
		
		try
		{
			mp = findPlayer(p->name);
			mp->specTimer = 5;
		}
		catch(const std::exception& e)
		{
			Logger::log("Error caught finding player in setSpecTimer");
		}
	}
}

void botInfo::setRepelTimer(Player *p)
{
	if(match.locked)
	{
		MatchPlayer* mp;
		
		try
		{
			mp = findPlayer(p->name);
			mp->repTimer = 3;
		}
		catch(const std::exception& e)
		{
			Logger::log("Error caught finding player in setRepelTimer");
		}
	}
}

void botInfo::checkRemainingPlayers(Team* team)
{
	char out[255];
	sprintf(out, "Player died/lagged, checking remaining players in freq %d (%s)", team->freq, team->squad);
	Logger::log(out);
	bool gameFinished = true;

	for(int i = 0; i < team->players.size(); i++)
	{
		MatchPlayer* p = &team->players[i];
		sprintf(out, "Found player %s in freq %d with %d lives", p->name, team->freq, p->lives);
		Logger::log(out);

		if(p->lives > 0)
		{
			if(!p->lagged)
			{
				Logger::log("Player still alive and not spectating, continuing match...");
				gameFinished = false;
			}
		}
	}

	if(gameFinished)
	{
		Logger::log("All players dead/specced, ending match.");
		gameEnd();
	}
}

void botInfo::trackDamage(Player *p, Player* k, int damage)
{
	if(match.locked)
	{
		char out[255];

		sprintf(out, "Tracking damage: %s <- %s %d", p->name, k->name, damage);
		Logger::log(out);

		bool trackerExists = false;

		for(int i = 0; i < match.damageTracker.size(); i++)
		{
			if(match.damageTracker[i].player == p && match.damageTracker[i].attacker == k)
			{
				trackerExists = true;
				match.damageTracker[i].damage += damage;

				sprintf(out, "Adding damage to existing tracker: %s <- %s %d (total: %d)", p->name, k->name, damage, match.damageTracker[i].damage);
				Logger::log(out);
			}
		}

		if(!trackerExists)
		{
			sprintf(out, "Adding damage to new tracker: %s <- %s %d", p->name, k->name, damage);
			Logger::log(out);

			DamageTracker dt;
			dt.player = p;
			dt.attacker = k;
			dt.damage = damage;
			match.damageTracker.push_back(dt);
		}
	}
}

void botInfo::devalueDamage()
{
	const int DEVALUE_DAMAGE_PER_SECOND = 80;

	if(match.locked)
	{
		for(int i = 0; i < match.damageTracker.size(); i++)
		{
			if(match.damageTracker[i].damage > 0)
			{
				match.damageTracker[i].damage -= DEVALUE_DAMAGE_PER_SECOND;	
			}

			if(match.damageTracker[i].damage < 0)
			{
				match.damageTracker[i].damage = 0;
			}
		}
	}
}

void botInfo::resetDamage(Player* p)
{
	for(int i = 0; i < match.damageTracker.size(); i++)
	{
		if(match.damageTracker[i].player == p)
		{
			match.damageTracker[i].damage = 0;
		}
	}
}

Assist *botInfo::findAssist(Player* p, Player *k)
{
	Assist* a = new Assist();

	if(match.locked)
	{
		char out[255];
		sprintf(out, "Player %s died, looking for assist", p->name);
		Logger::log(out);

		const int THRESHOLD_ROBBED = 2000;
		const int THRESHOLD_ASSIST = 1000;
		const int THRESHOLD_LITE = 500;

		int mostDamage = 0;

		for(int i = 0; i < match.damageTracker.size(); i++)
		{
			if(match.damageTracker[i].player == p && match.damageTracker[i].attacker != k)
			{
				if(match.damageTracker[i].damage > mostDamage)
				{
					mostDamage = match.damageTracker[i].damage;
					a->player = match.damageTracker[i].attacker;
				}
			}
		}

		if(mostDamage > 0)
		{
			sprintf(out, "Most damage (%d) delivered to %s by %s", mostDamage, p->name, a->player->name);
			Logger::log(out);
		}
		else
		{
			sprintf(out, "No assist.");
			Logger::log(out);
		}

		if(mostDamage > THRESHOLD_ROBBED)
		{
			a->assistLevel = ASSIST_TYPE_ROBBED;
		}
		else if(mostDamage > THRESHOLD_ASSIST)
		{
			a->assistLevel = ASSIST_TYPE_STANDARD;
		}
		else if(mostDamage > THRESHOLD_LITE)
		{
			a->assistLevel = ASSIST_TYPE_LITE;
		}
		else
		{
			a->assistLevel = ASSIST_TYPE_NONE;
		}
	}

	return a;
}

void botInfo::checkForcedRepel(Player* p)
{
	if(match.locked)
	{
		/* Ensure this only gets fired once */
		MatchPlayer* mpt;
		mpt = findPlayer(p->name);
		if(mpt->repTimer != -1)
		{
			return;
		}

		setRepelTimer(p);
		/* Continue code execution as normal */

		char out[255];
		sprintf(out, "Repel triggered by %s, checking for forced repel...", p->name);
		Logger::log(out);

		const int THRESHOLD_FORCED_REPEL = 1000;
		int mostDamage = 0;
		char attackerName[50];

		for(int i = 0; i < match.damageTracker.size(); i++)
		{
			if(match.damageTracker[i].player == p)
			{
				if(match.damageTracker[i].damage > mostDamage)
				{
					mostDamage = match.damageTracker[i].damage;
					Player *k = match.damageTracker[i].attacker;

					sprintf(attackerName, "%s", k->name);
				}
			}
		}

		sprintf(out, "Most damage (%d) delivered to %s by %s", mostDamage, p->name, attackerName);
		Logger::log(out);

		if(strlen(attackerName) > 0)
		{
			if(mostDamage > THRESHOLD_FORCED_REPEL)
			{
				sprintf(out, "Forcing repel for %s", attackerName);
				Logger::log(out);
				MatchPlayer* mp = findPlayer(attackerName);
				mp->forcedReps += 1;
				mp->mvpPoints += 0.25;
			}
		}
		else
		{
			sprintf(out, "No one has damaged %s recently", p->name);
			Logger::log(out);
		}
	}
}

void botInfo::gameEnd()
{
	Logger::log("Ending game...");
	char* winner;
	char* loser;
	int winnerScore;
	int loserScore;
	char out[255];
	char* mvp;
	float mvpPoints = -5;
	int mvpK = 0;
	int mvpD = 0;
	bool isTie = false;
	bool mvpFound = false;

	if(match.teams[0].score > match.teams[1].score)
	{
		winner = match.teams[0].squad;
		loser = match.teams[1].squad;
		winnerScore = match.teams[0].score;
		loserScore = match.teams[1].score;
	}
	else if(match.teams[1].score > match.teams[0].score)
	{
		winner = match.teams[1].squad;
		loser = match.teams[0].squad;
		winnerScore = match.teams[1].score;
		loserScore = match.teams[0].score;
	}
	else
	{
		isTie = true;
		winner = match.teams[0].squad;
		loser = match.teams[1].squad;
		winnerScore = match.teams[0].score;
		loserScore = match.teams[1].score;
	}

	for(int i = 0; i < 2; i++)
	{
		printScoreBoxTop(match.teams[i].squad);
		for(int j = 0; j < match.teams[i].players.size(); j++)
		{
			MatchPlayer p = match.teams[i].players[j];
			printPlayerData(p);
			if(p.mvpPoints > mvpPoints)
			{
				mvpPoints = p.mvpPoints;
				mvp = p.name;
				mvpK = p.kills;
				mvpD = p.deaths;
				mvpFound = true;
			}
		}
		printTeamData(match.teams[i]);
	}

	sendPublic("*arena Game Over!");

	//subgame unlocking
	sendPublic("*lock");
	//asss unlocking
	sendPublic("?unlockarena -n");

	match.locked = false;
	
	if(isTie)
	{
		sprintf(out, "*arena Tie between %s and %s Final Score: %d - %d in %s minutes.", winner, loser, winnerScore, loserScore, getReadableElapsed(false));
	}
	else
	{
		sprintf(out, "*arena %s defeats %s Final Score: %d - %d in %s minutes.", winner, loser, winnerScore, loserScore, getReadableElapsed(false));
	}
	sendPublic(out);
	Logger::log(out);

	if(mvpFound)
	{
		sprintf(out, "*arena MVP: %s (%.2f pts, %d-%d)", mvp, mvpPoints, mvpK, mvpD);
		Logger::log(out);
		sendPublic(out);
	}
	else
	{
		Logger::log("No MVP.");
	}

	endMatch();
}

void botInfo::printScoreBoxTop(char* squadName)
{
	char c = '-';
	int totalLength = 127;
	int startLength = 22;
	char out[128];
	char squad[50];

	sprintf(squad, " %s ", squadName);

	memset(out, c, totalLength);
	out[totalLength] = '\0';
	strncpy(out + startLength, squad, strlen(squad));

	char finalOutput[300];
	sprintf(finalOutput, "*arena %s", out);
	sendPublic(finalOutput);
}

void botInfo::printPlayerData(MatchPlayer p)
{
	char out[255];
	sprintf(out, "*arena Name: %-22s Kills: %-3d Deaths: %-3d TeamKills: %-3d Lagouts: %-3d Assists: %-3d Forced Reps: %-3d MVP Points: %.2f", p.name, p.kills, p.deaths, p.teamkills, p.lagouts, p.assists, p.forcedReps, p.mvpPoints);
	sendPublic(out);
}

void botInfo::printTeamData(Team t)
{
	char out[255];
	int kills = 0;
	int deaths = 0;
	int teamkills = 0;
	int lagouts = 0;
	int assists = 0;
	int forcedReps = 0;

	for(int i = 0; i < t.players.size(); i++)
	{
		MatchPlayer p = t.players[i];
		kills += p.kills;
		deaths += p.deaths;
		teamkills += p.teamkills;
		lagouts += p.lagouts;
		assists += p.assists;
		forcedReps += p.forcedReps;
	}

	sprintf(out, "*arena TEAM TOTALS -                Kills: %-3d Deaths: %-3d TeamKills: %-3d Lagouts: %-3d Assists: %-3d Forced Reps: %-3d", kills, deaths, teamkills, lagouts, assists, forcedReps);
	sendPublic(out);
}


void botInfo::announceScore()
{
	char* leadingTeam;
	char msg[255];
	int leadingScore;
	int trailingScore;

	if(match.teams[0].score > match.teams[1].score)
	{
		leadingTeam = match.teams[0].squad;
		leadingScore = match.teams[0].score;
		trailingScore = match.teams[1].score;
	}
	else if(match.teams[1].score > match.teams[0].score)
	{
		leadingTeam = match.teams[1].squad;
		leadingScore = match.teams[1].score;
		trailingScore = match.teams[0].score;
	}
	else
	{
		leadingTeam = "Tie";
		leadingScore = match.teams[0].score;
		trailingScore = leadingScore;
	}
	sprintf(msg, "*arena Score %d - %d %s", leadingScore, trailingScore, leadingTeam);
	Logger::log(msg);
	sendPublic(msg);
}

///// Logging /////

void Logger::log(const char *msg)
{
	time_t now = time(0);
	tm* timeInfo = localtime(&now);
	char timestamp[20];
	strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", timeInfo);

	std::ofstream logFile("leaguebot.log", std::ios::app);
	if(logFile.is_open())
	{
		logFile << "[" << timestamp << "]  " << msg << std::endl;
		logFile.close();
	}
	printf("%s\n", msg);
}

//todo: implement lagout functionality and give 1 minute to return
//todo: could we announce to a discord bot?