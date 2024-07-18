/*---------------------------------------------------------------------------------

Function findAP is based on the example https://github.com/devkitPro/nds-examples/blob/master/dswifi/ap_search/source/template.c

---------------------------------------------------------------------------------*/
#include <nds.h>
#include <stdio.h>
#include <dswifi9.h>

#include <sys/types.h>
#include <netinet/in.h>

#include <sys/socket.h>
#include <netdb.h>

#include <arpa/inet.h>

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

void removeLeadingZerosFromIP(char *ip)
{
	int octet1, octet2, octet3, octet4;

	sscanf(ip, "%d.%d.%d.%d", &octet1, &octet2, &octet3, &octet4);
	sprintf(ip, "%d.%d.%d.%d", octet1, octet2, octet3, octet4);
}

void removeLeadingZerosFromPort(char *port)
{
	int port_number;
	sscanf(port, "%d", &port_number);
	sprintf(port, "%d", port_number);
}

void setupReceiverIP(char *rec_ip, char *rec_port)
{
	int selected = 0;
	int j;
	int count;
	count = strlen(rec_ip) + strlen(rec_port) + 1;
	char *selected_pt;

	int pressed = 0;
	do
	{
		scanKeys();
		pressed = keysDown();

		selected_pt = (selected < strlen(rec_ip)) ? rec_ip + selected : rec_port + selected - strlen(rec_ip) - 1;

		consoleClear();
		iprintf("Setup the IP and port\nof the receiver\n");
		iprintf("%d, %d, %d, %d, %c\n", strlen(rec_ip), strlen(rec_port), count, selected, *selected_pt);

		iprintf("%s:%s\n", rec_ip, rec_port);

		for (j = 0; j < count; j++)
		{
			iprintf((selected == j) ? "*" : " ");
		}

		if (pressed & KEY_UP)
		{
			if (*selected_pt >= '0' && *selected_pt < '9') // Check if it's between '0' and '8'
			{
				(*selected_pt)++; // Increment the digit
			}
			else if (*selected_pt == '9') // If it's '9', wrap around to '0'
			{
				*selected_pt = '0';
			}
		}

		if (pressed & KEY_DOWN)
		{
			if (*selected_pt > '0' && *selected_pt <= '9') // Check if it's between '1' and '9'
			{
				(*selected_pt)--; // Decrement the digit
			}
			else if (*selected_pt == '0') // If it's '0', wrap around to '9'
			{
				*selected_pt = '9';
			}
		}

		if (pressed & KEY_RIGHT)
		{
			selected = (selected + 1) % count; // Move to the next character
		}

		if (pressed & KEY_LEFT)
		{
			selected = (selected - 1 + count) % count; // Move to the previous character
		}

		swiWaitForVBlank();
	} while (!(pressed & KEY_A));

	consoleClear();

	removeLeadingZerosFromIP(rec_ip);
	removeLeadingZerosFromPort(rec_port);
}

void sendToReceiver(char *rec_ip, char *rec_port)
{
	int sock;
	struct sockaddr_in server;

	iprintf("Connecting to %s:%s\n", rec_ip, rec_port);

	// Create socket
	sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock == -1)
	{
		iprintf("Could not create socket\n");
	}
	else
	{
		iprintf("Socket created\n");
	}

	server.sin_addr.s_addr = inet_addr(rec_ip);
	server.sin_family = AF_INET;
	server.sin_port = htons(atoi(rec_port));

	// Connect to remote server
	if (connect(sock, (struct sockaddr *)&server, sizeof(server)) < 0)
	{
		iprintf("Error: connection failed\n");
		return;
	}

	iprintf("Connected to socket\n");

	uint32 keys_down, last_keys_down = 0;

	consoleSetWindow(NULL, 2, 5, 30, 22);

	// Key press detection and sending loop
	while (1)
	{

		scanKeys();				   // Prepare key state for reading
		keys_down = keysCurrent(); // Get keys currently down

		// only send if the key state has changed
		if (keys_down != last_keys_down)
		{
			iprintf("Sending %ld\n", keys_down);
			send(sock, &keys_down, sizeof(keys_down), 0);
		}

		if (keys_down & KEY_START & KEY_SELECT)
		{ // Use START and SELECT button as exit condition
			break;
		}

		last_keys_down = keys_down; // Store the last key state for comparison
		swiWaitForVBlank();
	}

	close(sock);
	iprintf("Socket closed\n");
}

int main(void)
{
	Wifi_InitDefault(false);

	consoleDemoInit();

	while (1)
	{
		int status = ASSOCSTATUS_DISCONNECTED;

		// setup the window for the text console
		consoleClear();
		consoleSetWindow(NULL, 0, 0, 32, 24);

		Wifi_AccessPoint *ap = findAP();

		consoleClear();

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
			consoleClear();
			iprintf("%s", ASSOCSTATUS_STRINGS[status]);

			scanKeys();

			if (keysDown() & KEY_B)
				break;

			swiWaitForVBlank();
		}

		if (status == ASSOCSTATUS_ASSOCIATED)
		{
			iprintf("\nConnected\n");
			char rec_ip[16] = "192.168.001.001";
			char rec_port[5] = "8888";
			setupReceiverIP(rec_ip, rec_port);
			sendToReceiver(rec_ip, rec_port);
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