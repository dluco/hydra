#include <gdk/gdkkeysyms.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <webkit/webkit.h>
#include <glib/gstdio.h>
#include "defaults.h"
#include "browser.h"
#include "tab.h"
#include "callbacks.h"

/* download callback */
void cb_download(WebKitWebView *web_view, GObject *d, gpointer user_data)
{
	gchar *command;

	const gchar *download_url = webkit_download_get_uri(WEBKIT_DOWNLOAD(d));
	const gchar *requested_name = webkit_download_get_suggested_filename(WEBKIT_DOWNLOAD(d));

	command = g_new0(gchar, strlen(DEFAULT_DOWNLOAD_COMMAND) + strlen(DEFAULT_DOWNLOAD_LOCATION) + strlen(requested_name) + strlen(download_url) + 1);
	g_sprintf(command, DEFAULT_DOWNLOAD_COMMAND, DEFAULT_DOWNLOAD_LOCATION, requested_name, download_url);
	g_spawn_command_line_async(command, NULL);

	g_free(command);
}
