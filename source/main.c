/*---------------------------------------------------------------------------------

Inspired by example https://github.com/devkitPro/nds-examples/blob/master/dswifi/ap_search/source/template.c

---------------------------------------------------------------------------------*/
#include <nds.h>
#include <stdio.h>
#include <dswifi9.h>

#include <sys/types.h>
#include <netinet/in.h>

#include <sys/socket.h>
#include <netdb.h>

Wifi_AccessPoint *findAP(void)
{
	int selected = 0;
	int i;
	int count = 0, displaytop = 0;
	static Wifi_AccessPoint ap;

	Wifi_ScanMode(); // search for APs

	int pressed = 0;
	do
	{

		scanKeys();
		pressed = keysDown();

		if (pressed & KEY_START)
			exit(0);

		// count number of detected APs
		count = Wifi_GetNumAP();

		consoleClear();

		iprintf("%d APs detected\n\n", count);

		int displayend = displaytop + 10;
		if (displayend > count)
			displayend = count;

		// display the APs to the user
		for (i = displaytop; i < displayend; i++)
		{
			// get data from internal structures
			Wifi_GetAPData(i, &ap);

			// display the name of the AP
			iprintf("%s %.28s\n   Security: %s Strenght:%4d\n",
					i == selected ? (ap.flags & WFLAG_APDATA_WEP ? "-x" : "->") : "  ",
					ap.ssid,
					ap.flags & WFLAG_APDATA_WEP ? "WEP " : "Open",
					ap.rssi * 100 / 0xD0);
		}

		// move the selection arrow
		if (pressed & KEY_UP)
		{
			selected--;
			if (selected < 0)
			{
				selected = 0;
			}
			if (selected < displaytop)
				displaytop = selected;
		}

		if (pressed & KEY_DOWN)
		{
			selected++;
			if (selected >= count)
			{
				selected = count - 1;
			}
			displaytop = selected - 9;
			if (displaytop < 0)
				displaytop = 0;
		}

		swiWaitForVBlank();
	} while (!(pressed & KEY_A));

	// user has made a choice so grab the ap and return it
	Wifi_GetAPData(selected, &ap);

	return &ap;
}

void keyPressed(int c)
{
	if (c > 0)
		iprintf("%c", c);
}

int main(void)
{
	Wifi_InitDefault(false);

	consoleDemoInit();

	Keyboard *kb = keyboardDemoInit();
	kb->OnKeyPressed = keyPressed;

	while (1)
	{
		int status = ASSOCSTATUS_DISCONNECTED;

		// setup the window for the text console
		consoleClear();
		consoleSetWindow(NULL, 0, 0, 32, 24); 

		Wifi_AccessPoint *ap = findAP();

		consoleClear();
		consoleSetWindow(NULL, 0, 0, 32, 10);

		iprintf("Connecting to %s\n", ap->ssid);

		// this tells the wifi lib to use dhcp for everything
		Wifi_SetIP(0, 0, 0, 0, 0);
		if (ap->flags & WFLAG_APDATA_WEP)
		{
			iprintf("Error: WEP connection\nUnable to connect\n");
			break;
		}
		else
		{
			Wifi_ConnectAP(ap, WEPMODE_NONE, 0, 0);
		}
		consoleClear();
		while (status != ASSOCSTATUS_ASSOCIATED && status != ASSOCSTATUS_CANNOTCONNECT)
		{

			status = Wifi_AssocStatus();
			int len = strlen(ASSOCSTATUS_STRINGS[status]);
			iprintf("\x1b[0;0H\x1b[K"); // clear the line
			iprintf("%s", ASSOCSTATUS_STRINGS[status]);

			scanKeys();

			if (keysDown() & KEY_B)
				break;

			swiWaitForVBlank();
		}

		if (status == ASSOCSTATUS_ASSOCIATED)
		{
			iprintf("\nConnected\n");
		}
		else
		{
			iprintf("\nFailed to connect\n");
		}

		int quit = 0;
		iprintf("Press A to try again, B to quit.");
		while (1)
		{
			swiWaitForVBlank();
			scanKeys();
			int pressed = keysDown();
			if (pressed & KEY_B)
				quit = 1;
			if (pressed & (KEY_A | KEY_B))
				break;
		}
		if (quit)
			break;
	}
	return 0;
}