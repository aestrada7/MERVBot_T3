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
				sendPrivate(p, "!squads !freqs !status");
			}
			break;
		case OP_Limited:
			{	// Limited-level commands
				sendPrivate(p, "!start !end !announce");
			}
		case OP_Player:
			{	// Player-level commands
				sendPrivate(p, "!sc !about !version");
				sendPrivate(p, "for detailed help, type ::!help <command> (e.g. ::!help about)");
				sendPrivate(p, "most commands can be typed in the public chat with a . in front (e.g. '.about')");
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
				sendPrivate(p, "Displays information about the bot.");
			}

			if (c->checkParam("version"))
			{
				sendPrivate(p, "Displays the version of the bot.");
			}

			if (c->checkParam("sc"))
			{
				sendPrivate(p, "Performs a ship change after being killed, usage: !sc #.");
			}

			if (c->checkParam("own"))
			{
				sendPrivate(p, "Provides limited ownership of the bot, allows the owner to start and end matches.");
			}

			if (c->checkParam("give"))
			{
				sendPrivate(p, "Surrenders bot ownership.");
			}

			if (c->checkParam("start"))
			{
				sendPrivate(p, "Starts a match.");
			}

			if (c->checkParam("end"))
			{
				sendPrivate(p, "Ends a match.");
			}

			if (c->checkParam("announce"))
			{
				sendPrivate(p, "Announces a match zone wide.");
			}

			if (c->checkParam("freqs"))
			{
				sendPrivate(p, "Sets frequencies to the provided numbers.");
				sendPrivate(p, "Always use the format below, otherwise it will not work.");
				sendPrivate(p, "Usage: !freqs -a=<FREQ_A> -b=<FREQ_B>");
			}

			if (c->checkParam("squads"))
			{
				sendPrivate(p, "Sets the bot's squads to the provided squad names.");
				sendPrivate(p, "Always use the format below, otherwise it will not work.");
				sendPrivate(p, "Usage: !squads -a=<SQUAD_A> -b=<SQUAD_B>");
			}

			if (c->checkParam("status"))
			{
				sendPrivate(p, "Shows the match status.");
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

			if (c->checkParam("squads"))
			{
				setSquads(p, c->final);
			}

			if (c->checkParam("freqs"))
			{
				setFreqs(p, c->final);
			}

			if (c->checkParam("status"))
			{
				getStatus(p);
			}
		}
	case OP_Limited:
		{	// Limited-level commands
			if (c->check("start"))
			{
				startMatch();
			}

			if(c->check("end"))
			{
				gameEnd();
			}

			if(c->check("announce"))
			{
				announce();
			}
		}
	case OP_Player:
		{	// Player-level commands
			if (c->check("about") || c->check("version"))
			{
				aboutBot(p);
			}

			if (c->check("sc"))
			{
				shipChange(p, c->final);
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
