#include "spawn.h"

#include "..\algorithms.h"

#include "..\dllcore.cpp"
#include "..\datatypes.cpp"
#include "..\algorithms.cpp"
#include "..\player.cpp"
#include "..\commtypes.cpp"

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
			for (int i = 0; i < 4; ++i)
				--countdown[i];
		}
		break;
//////// Arena ////////
	case EVENT_ArenaEnter:
		{
			arena = (char*)event.p[0];
			me = (Player*)event.p[1];	// if(me) {/*we are in the arena*/}
			bool biller_online = *(bool*)&event.p[2];

			sendPublic("?grplogin sysop <PWD>"); // this should come from an .ini file, but for now we'll do it here

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
					parseCommand(msg);
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

void botInfo::warpTo(Player* p, int x, int y)
{
	char out[255];
	sprintf(out, "*warpto %d %d", x, y);
	sendPrivate(p, out);
}

void botInfo::parseCommand(char* command)
{
	if(*command == '.')
	{
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
			}

			sendPublic(token);

			if (strcmp(commandName, ".squads") == 0)
			{
				setSquads(commandArgs);
			}
			else if (strcmp(commandName, ".freqs") == 0)
			{
				setFreqs(commandArgs);
			}
			else if (strcmp(commandName, ".status") == 0)
			{
				printf("Getting status...\n");
				getStatus();
			}
			else if (strcmp(commandName, ".start") == 0)
			{
				startMatch();
			}
			else if (strcmp(commandName, ".end") == 0)
			{
				//gameEnd();
			}
			else
			{
				char out[255];
				sprintf(out, "Unknown command: %s", commandName);
				sendPublic(out);
			}

			token = strtok(NULL, delim);
		}
	}
}

void botInfo::findPlayersInFreqs()
{
    _listnode <Player> *parse = playerlist->head;

    while (parse)
    {
        Player *p = parse->item;

		if(p->team == match.teams[0] && p->ship != SHIP_Spectator)
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
			mp.player = p;
			match.teams[0].players.push_back(mp);
		}

		if(p->team == match.teams[1] && p->ship != SHIP_Spectator)
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
			mp.player = p;
			match.teams[1].players.push_back(mp);
		}

        parse = parse->next;
    }

    sendPublic(match.teams[0].players[0].name);
    sendPublic(match.teams[1].players[1].name);
}

//////// League Bot Commands ////////

void botInfo::setSquads(char* squadStr)
{
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
	sendPublic(out);
}

void botInfo::setFreqs(const char* freqStr)
{
	int freqA = match.teams[0].freq;
	int freqB = match.teams[1].freq;

	sscanf(freqStr, "-a=%d -b=%d", &freqA, &freqB);

	match.teams[0].freq = freqA;
	match.teams[1].freq = freqB;

	char out[255];
	sprintf(out, "%s: %d  %s: %d", match.teams[0].squad, match.teams[0].freq, match.teams[1].squad, match.teams[1].freq);
	sendPublic(out);
}

void botInfo::startMatch()
{
	sendPublic("*lock");
	sendPublic("Starting in 10 seconds...");
	//set a timer, skipping for now
	prepareMatch();
}

void botInfo::prepareMatch()
{
	char out[255];
	sprintf(out, "<----- %s vs %s ----->", match.teams[0].squad, match.teams[1].squad);
	sendPublic(out);

	for(int i = 0; i < match.teams[0].players.size(); i++)
	{
		MatchPlayer p = match.teams[0].players[i];
		sendPublic(p.name);
		warpTo(p.player, match.sideAX, match.sideAY);
	}

	for(int j = 0; i < match.teams[1].players.size(); j++)
	{
		MatchPlayer p = match.teams[1].players[j];
		sendPublic(p.name);
		warpTo(p.player, match.sideBX, match.sideBY);
	}
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
}

void botInfo::getStatus()
{
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

	sendPublic(gameTxt);
	sendPublic(freqTxt);
	sendPublic(playTxt);
	sendPublic(durTxt);
	sendPublic(infoTxt);
	sendPublic(typeTxt);
}