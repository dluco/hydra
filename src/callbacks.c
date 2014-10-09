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

/* when the page load is commited, call this function */
void cb_load_status(WebKitWebView *view, GParamSpec *pspec, Browser *b)
{
	const gchar *uri = webkit_web_view_get_uri(view);
	WebKitLoadStatus status = webkit_web_view_get_load_status(view);
	
	switch(status) {
	case WEBKIT_LOAD_PROVISIONAL:
		break;
	case WEBKIT_LOAD_COMMITTED:
		/*
		if (gtk_notebook_get_current_page(b->notebook) == gtk_notebook_page_num(b->notebook, t->scroll)) {
			gtk_entry_set_text(GTK_ENTRY(b->bar), uri); 
		}*/

		/* set uri-bar if this is the current tab */
		if ((browser_get_current_tab(b))->view == view) {
			gtk_entry_set_text(GTK_ENTRY(b->bar), uri); 
		}
		
		FILE *history = fopen(g_build_filename(g_get_home_dir(), DEFAULT_HISTORY_FILE, NULL), "a+");
		
		if (history) {
			fprintf(history, "%s \n", uri);
			fclose(history);
		}

		break;
	case WEBKIT_LOAD_FIRST_VISUALLY_NON_EMPTY_LAYOUT:
		break;
	case WEBKIT_LOAD_FINISHED:
		break;
	case WEBKIT_LOAD_FAILED:
		break;
	default:
		break;
	}
}
