#include <stdlib.h>
#include <gtk/gtk.h>
#include <webkit/webkit.h>
#include "browser.h"
#include "utils.h"
#include "config.h"

/* main entry-point */
int main(int argc, char *argv[])
{
	gboolean version = FALSE;
	gboolean fullscreen = FALSE;
	gchar *geometry = NULL;
	gchar **uris = NULL;
	GError *error = NULL;

	GOptionEntry entries[] = {
		{ "version", 'v', 0, G_OPTION_ARG_NONE, &version, "Print version number", NULL },
		{ "fullscreen", 0, 0, G_OPTION_ARG_NONE, &fullscreen, "Fullscreen mode", NULL },
		{ "geometry", 'g', 0, G_OPTION_ARG_STRING, &geometry, "X geometry specification", "GEOMETRY" },
		{ G_OPTION_REMAINING, 0, 0, G_OPTION_ARG_STRING_ARRAY, &uris, "Addresses", NULL },
		{ NULL }
	};

	/* initialize GTK and parse commandline options */
	if (!gtk_init_with_args(&argc, &argv, "[addresses]", entries, NULL, &error)) {
		die("%s\n", error->message);
	}

	if (version) {
		printf("hydra %s, 2014 David Luco\n", VERSION);
		exit(EXIT_SUCCESS);
	}

	/* create first browser */
	Browser *b = browser_new();

	if (fullscreen) {
		gtk_window_fullscreen(GTK_WINDOW(b->window));
	}

	/*
	if (argc == 2) {
		tab_load_uri(browser_get_current_tab(b), argv[1]);
	}
	*/

	/* load homepage on startup */
	tab_load_uri(browser_get_current_tab(b), DEFAULT_HOME);

	gtk_main();

	return 0;
}
