// SPDX-License-Identifier: GPL-3.0-or-later

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include "ews.h"

#ifdef _CONSOLE
#define ews_printf printf
#define ews_printf_debug printf
#else
#define ews_printf
#define ews_printf_debug(...)
#endif

#pragma warning(disable:4101)
#pragma warning(disable:4267)
#pragma warning(disable:4996)
#include "EmbeddableWebServer.h"

#pragma comment(lib, "ws2_32")

struct _EWS_SERVER
{
	struct Server server;
	HANDLE thread;
	uint16_t port;
	char root[MAX_PATH];
};

static DWORD WINAPI server_thread_func(LPVOID param)
{
	EWS_SERVER* ws = (EWS_SERVER*)param;
	acceptConnectionsUntilStoppedFromEverywhereIPv4(&ws->server, ws->port);
	return 0;
}

struct Response *
createResponseForRequest(const struct Request* request, struct Connection* connection)
{
	EWS_SERVER* ws = (EWS_SERVER*)connection->server->tag;
	ews_printf("Request: %s\n", request->path);
	if (strcmp(request->path, "/") == 0)
	{
		struct HeapString connectionDebugInfo = connectionDebugStringCreate(connection);
		struct Response* response = responseAllocWithFormat(200, "OK", "text/html; charset=UTF-8",
			"<html><head><title>EWS</title></head>"
			"<body>"
			"<h2>Embedded C Web Server by Forrest Heller</h2>"
			"<h2>Version %s</h2>"
			"<h2>Browse Files</h2>"
			"<a href=\"/?\">ROOT</a><br>"
			"<h2>Server Status</h2>"
			"<table border=\"1\">\n"
			"<tr><td>Active connections</td><td>%" PRId64 "</td></tr>\n"
			"<tr><td>Total connections</td><td>%" PRId64 "</td></tr>\n"
			"<tr><td>Total bytes sent</td><td>%" PRId64 "</td></tr>\n"
			"<tr><td>Total bytes received</td><td>%" PRId64 "</td></tr>\n"
			"<tr><td>Heap string allocations</td><td>%" PRId64 "</td></tr>\n"
			"<tr><td>Heap string reallocations</td><td>%" PRId64 "</td></tr>\n"
			"<tr><td>Heap string frees</td><td>%" PRId64 "</td></tr>\n"
			"<tr><td>Heap string total bytes allocated</td><td>%" PRId64 "</td></tr>\n"
			"</table>"
			"<h2>Connection Debug Info</h2><pre>%s</pre>"
			"</body></html>",
			EMBEDDABLE_WEB_SERVER_VERSION_STRING,
			counters.activeConnections,
			counters.totalConnections,
			counters.bytesSent,
			counters.bytesReceived,
			counters.heapStringAllocations,
			counters.heapStringReallocations,
			counters.heapStringFrees,
			counters.heapStringTotalBytesReallocated,
			connectionDebugInfo.contents);
		heapStringFreeContents(&connectionDebugInfo);
		return response;
	}

	if (strcmp(request->path, "/?") == 0)
		return responseAllocServeFileFromRequestPath("/", "/", "/", ws->root);

	return responseAllocServeFileFromRequestPath("/", request->path, request->pathDecoded, ws->root);
}

EWS_SERVER* ews_start(uint16_t port, const char* root)
{
	EWS_SERVER* ws = (EWS_SERVER*)malloc(sizeof(EWS_SERVER));
	if (!ws)
		return NULL;
	memset(ws, 0, sizeof(EWS_SERVER));
	ws->port = port;
	strcpy_s(ws->root, MAX_PATH, root);
	ws->root[MAX_PATH - 1] = '\0';

	serverInit(&ws->server);
	ws->server.tag = ws;

#ifdef _CONSOLE
	server_thread_func(ws);
#else
	ws->thread = CreateThread(NULL, 0, server_thread_func, ws, 0, NULL);
	if (ws->thread == NULL)
	{
		serverDeInit(&ws->server);
		free(ws);
		return NULL;
	}
#endif
	return ws;
}

void ews_stop(EWS_SERVER* ws)
{
	if (!ws)
		return;
	serverStop(&ws->server);
	if (ws->thread)
	{
		WaitForSingleObject(ws->thread, INFINITE);
		CloseHandle(ws->thread);
	}
	serverDeInit(&ws->server);
	free(ws);
}
