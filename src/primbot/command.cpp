#include "spawn.h"

#include "..\algorithms.h"

void botInfo::gotHelp(Player *p, Command *c)
{
	// List commands

	if (!*c->final)
	{
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
			{	// Owner-level commands
//				sendPrivate(p, "Ext: ");
			}
			break;
		case OP_SysOp:
			{	// SysOp-level commands
//				sendPrivate(p, "Ext: ");
			}
			break;
		case OP_SuperModerator:
			{	// SuperModerator-level commands
//				sendPrivate(p, "Ext: ");
			}
			break;
		case OP_Moderator:
			{	// Moderator-level commands
//				sendPrivate(p, "Ext: ");
			}
			break;
		case OP_Limited:
			{	// Limited-level commands
			}
		case OP_Player:
			{	// Player-level commands
				sendPrivate(p, "!about (query me about my function)");
			}
		}

		return;
	}

	// Specific command help

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
		{	// Owner-level commands
			if (c->checkParam("owner") || c->checkParam("all"))
			{
//				sendPrivate(p, "Ext: ");
			}
		}
	case OP_SysOp:
		{	// SysOp-level commands
			if (c->checkParam("sop") || c->checkParam("all"))
			{
//				sendPrivate(p, "Ext: ");
			}
		}
	case OP_SuperModerator:
		{	// SuperModerator-level commands
			if (c->checkParam("smod") || c->checkParam("all"))
			{
//				sendPrivate(p, "Ext: ");
			}
		}
	case OP_Moderator:
		{	// Moderator-level commands
			if (c->checkParam("mod") || c->checkParam("all"))
			{
//				sendPrivate(p, "Ext: ");
			}
		}
	case OP_Limited:
		{	// Limited-level commands
		}
	case OP_Player:
		{	// Player-level commands
			if (c->checkParam("about"))
			{
				sendPrivate(p, "!about (query me about my function)");
			}

			if (c->checkParam("limit"))
			{
				sendPrivate(p, "Sets a limit to the duel, Defaults to 10.");
				sendPrivate(p, "Usage: !limit <new_limit>");
			}

			if (c->checkParam("box"))
			{
				sendPrivate(p, "Select a box");
				sendPrivate(p, "Usage: !box <box number>");
			}

			if (c->checkParam("duels"))
			{
				sendPrivate(p, "Get a list of ongoing duels");
				sendPrivate(p, "Usage: !duels");
			}

			if (c->checkParam("duel"))
			{
				sendPrivate(p, "Start a duel while already in a box");
				sendPrivate(p, "Usage: !duel");
			}

			if (c->checkParam("resign"))
			{
				sendPrivate(p, "Forfeit the current match");
				sendPrivate(p, "Usage: !resign");
			}
		}
	}
}

void botInfo::gotCommand(Player *p, Command *c)
{
	if (!p) return;
	if (!c) return;

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
		{	// Owner-level commands
		}
	case OP_SysOp:
		{	// SysOp-level commands
		}
	case OP_SuperModerator:
		{	// SuperModerator-level commands
		}
	case OP_Moderator:
		{	// Moderator-level commands
			if (c->check("clean"))
			{
				cleanBoxes();
			}

			if(c->check("debug"))
			{
				debug();
			}
		}
	case OP_Limited:
		{	// Limited-level commands
		}
	case OP_Player:
		{	// Player-level commands
			if (c->check("about") || c->check("version"))
			{
				aboutBot(p);
			}

			if (c->check("box"))
			{
				if (isNumeric(c->final))
				{
					int box = atoi(c->final);
					assignToBox(p, box);
				}
				else
				{
					sendPrivate(p, "Usage: !box <box_number>");
				}
			}

			if (c->check("duels"))
			{
				listDuels();
			}

			if (c->check("duel"))
			{
				duel(p);
			}

			if (c->check("limit"))
			{
				if (isNumeric(c->final))
				{
					int limit = atoi(c->final);
					setLimit(p, limit);
				}
				else
				{
					sendPrivate(p, "Usage: !limit <new_limit>");
				}
			}
		}
	}
}

void botInfo::gotRemoteHelp(char *p, Command *c, Operator_Level l)
{
	// List commands

	if (!*c->final)
	{
		switch (l)
		{
		case OP_Duke:
		case OP_Baron:
		case OP_King:
		case OP_Emperor:
		case OP_RockStar:
		case OP_Q:
		case OP_God:
		case OP_Owner:
			{	// Owner-level commands
			}
		case OP_SysOp:
			{	// SysOp-level commands
			}
		case OP_SuperModerator:
			{	// SuperModerator-level commands
			}
		case OP_Moderator:
			{	// Moderator-level commands
			}
		case OP_Limited:
			{	// Limited-level commands
			}
		case OP_Player:
			{	// Player-level commands
			}
		}

		return;
	}

	// Specific command help

	switch (l)
	{
	case OP_Duke:
	case OP_Baron:
	case OP_King:
	case OP_Emperor:
	case OP_RockStar:
	case OP_Q:
	case OP_God:
	case OP_Owner:
		{	// Owner-level commands
		}
	case OP_SysOp:
		{	// SysOp-level commands
		}
	case OP_SuperModerator:
		{	// SuperModerator-level commands
		}
	case OP_Moderator:
		{	// Moderator-level commands
		}
	case OP_Limited:
		{	// Limited-level commands
		}
	case OP_Player:
		{	// Player-level commands
		}
	}
}

void botInfo::gotRemote(char *p, Command *c, Operator_Level l)
{
	if (!c) return;

	switch (l)
	{
	case OP_Duke:
	case OP_Baron:
	case OP_King:
	case OP_Emperor:
	case OP_RockStar:
	case OP_Q:
	case OP_God:
	case OP_Owner:
		{	// Owner-level commands
		}
	case OP_SysOp:
		{	// SysOp-level commands
		}
	case OP_SuperModerator:
		{	// SuperModerator-level commands
		}
	case OP_Moderator:
		{	// Moderator-level commands
		}
	case OP_Limited:
		{	// Limited-level commands
		}
	case OP_Player:
		{	// Player-level commands
		}
	}
}
