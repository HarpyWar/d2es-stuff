#include "../../d2hackit/includes/ClientCore.cpp"
#include "AutoStocker.h"

#define CONFIG_FILE ".\\plugin\\autostockerMisc.ini"

void WriteConfig();
void ReadConfig();

AutoStocker autoStocker;

int ticksSkipped = 0;
int speed = 0;
bool settingHotkey = false;
int hotkey = 0;

CLIENTINFO
(
	0,1,
	"",
	"",
	"Auto Stocker",
	""
)

BOOL PRIVATE Start(char** argv, int argc)
{
	bool useChat = false;

	if(argc >= 3)
	{
		for(int i = 2; i < argc; i++)
		{
			if(_stricmp(argv[i], "chat") == 0)
				useChat = true;
		}
	}

	autoStocker.Start(useChat);

	return TRUE;
}


BOOL PRIVATE StartRares(char** argv, int argc)
{
	bool useChat = false;
	bool uniques = false;
	bool rares = false;
	bool sets = false;

	if(argc < 3)
		return FALSE;

	for(int i = 2; i < argc; i++)
	{
		if(_stricmp(argv[i], "sets") == 0 || _stricmp(argv[i], "set") == 0)
		{
			sets = true;
		}
		else if(_stricmp(argv[i], "rares") == 0 || _stricmp(argv[i], "rare") == 0)
		{
			rares = true;
		}
		else if(_stricmp(argv[i], "uniques") == 0 || _stricmp(argv[i], "unique") == 0)
		{
			uniques = true;
		}
		else if(_stricmp(argv[i], "chat") == 0)
		{
			useChat = true;
		}
	}

	autoStocker.StartRares(sets, rares, uniques, useChat);

	return TRUE;
}

BOOL PRIVATE Speed(char** argv, int argc)
{
	if(argc == 3)
	{
		speed = atoi(argv[2]);

		if(speed < 0)
			speed = 0;

		server->GameStringf("�c:Autostocker�c0: Setting speed to %d", speed);
		return TRUE;
	}

	return FALSE;
}

//DWORD EXPORT OnGamePacketBeforeReceived(BYTE* aPacket, DWORD aLen)
//{
//	if(aPacket[0] == 0x2c)
//	{
//		if(aPacket[6] == 0x04 && aPacket[7] == 0x00)
//		{
//			autoStocker.OnTransmute();
//		}
//	}
//
//	return aLen;
//}

VOID EXPORT OnGameJoin(THISGAMESTRUCT* thisgame)
{
	ReadConfig();
}

DWORD EXPORT OnGameTimerTick()
{
	if(ticksSkipped++ < speed)
		return 0;

	autoStocker.OnTick();
	ticksSkipped = 0;

	return 0;
}

VOID EXPORT OnThisPlayerMessage(UINT nMessage, WPARAM wParam, LPARAM lParam)
{
	if(nMessage == PM_UICLOSED)
	{
		autoStocker.Abort();
	}
}

VOID EXPORT OnUnitMessage(UINT nMessage, LPCGAMEUNIT lpUnit, WPARAM wParam, LPARAM lParam)
{
	if(nMessage == UM_ITEMEVENT)
	{
		ITEM *item = (ITEM *)lParam;

		if(wParam == 0x04 && item->iStorageID == 0x04)
		{
			autoStocker.OnItemToCube(item->dwItemID);
		}
		else if(wParam == 0x04 && item->iStorageID == 0x01)
		{
			autoStocker.OnItemToInventory(item->dwItemID);
		}
		else if(wParam == 0x05 && item->iStorageID == 0x01)
		{
			autoStocker.OnItemFromStorage(item->dwItemID);
		}
		else if(wParam == 0x05 && item->iStorageID == 0x04)
		{
			autoStocker.OnItemFromCube(item->dwItemID);
		}
	}
}

BOOL PRIVATE SetHotkey(char** argv, int argc)
{
	settingHotkey = true;

	server->GameStringf("�c:Autostocker�c0: Next key pressed will be the hotkey...");

	return TRUE;
}


BYTE EXPORT OnGameKeyDown(BYTE iKeyCode)
{
	if(settingHotkey)
	{
		settingHotkey = false;

		if(iKeyCode == 0 || iKeyCode == VK_ESCAPE || iKeyCode == VK_RETURN)
		{
			server->GameStringf("�c:Autostocker�c0: Unbound hotkey");
			hotkey = 0;
		}
		else
		{
			hotkey = iKeyCode;
			server->GameStringf("�c:Autostocker�c0: Set hotkey to %d", iKeyCode);
		}

		WriteConfig();
	}
	else
	{
		if(iKeyCode == hotkey)
		{
			Start(NULL, 0);
		}
	}

	switch(iKeyCode)
	{
		case VK_SPACE:
			autoStocker.Abort();
			break;
	}

	return iKeyCode;
}

void ReadConfig()
{
	hotkey = GetPrivateProfileInt("Autostocker", "Hotkey", 0, CONFIG_FILE);
}

void WriteConfig()
{
	char configBuff[64];

	sprintf_s(configBuff, sizeof(configBuff)/sizeof(configBuff[0]), "%d", hotkey);
	WritePrivateProfileString("Autostocker", "Hotkey", configBuff, CONFIG_FILE);
}

MODULECOMMANDSTRUCT ModuleCommands[]=
{
	{
		"help",
		OnGameCommandHelp,
		"help�c0 List commands available in this module.\n"
		"<command> help�c0 Shows detailed help for <command> in this module."
	},
	{
		"Start",
		Start,
		"Usage: Start [chat]",
	},
	{
		"Start_rares",
		StartRares,
		"Usage: Start_rares [sets] [rares] [uniques] [chat]"
	},
	{
		"Speed",
		Speed,
		"Speed # where # is the number of ticks to wait per command. Each tick is 100ms"
	},
	{
		"Hotkey",
		SetHotkey,
		"Sets the hotkey to start autostocker with default settings (.as start)"
	},
	{NULL}
};

