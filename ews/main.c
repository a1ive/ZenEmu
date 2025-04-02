// SPDX-License-Identifier: GPL-3.0-or-later

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdlib.h>
#include <stdio.h>

#include "ews.h"

int main(int argc, char* argv[])
{
	uint16_t port = 80;
	const char* root = ".";
	for (int i = 1; i < argc; i++)
	{
		if (strcmp(argv[i], "--help") == 0 ||
			strcmp(argv[i], "-h") == 0 ||
			strcmp(argv[i], "/?") == 0)
		{
			printf("Usage: %s [--port=PORT] [--root=ROOT]\n", argv[0]);
			return 0;
		}
		if (strncmp(argv[i], "--port=", 7) == 0)
		{
			port = atoi(argv[i] + 7);
			continue;
		}
		if (strncmp(argv[i], "--root=", 7) == 0)
		{
			root = argv[i] + 7;
			continue;
		}
	}
	EWS_SERVER* ws = ews_start(port, root);
	ews_stop(ws);
	return 0;
}
