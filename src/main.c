#include <stdlib.h>
#include <stdio.h>
#include "hydra.h"
#include "config.h"
#include "browser.h"
#include "tab.h"
#include "utils.h"

/* main entry-point */
int main(int argc, char *argv[])
{
	gboolean version = FALSE;
	gboolean fullscreen = FALSE;
	char *geometry = NULL;
	char **uris = NULL;
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
	
	if (uris) {
		/* load first uri in first tab (already created) */
		tab_load_uri(browser_get_current_tab(b), *uris++);
		/* load other uris (if any) in new tabs */
		while (uris && *uris) {
			tab_load_uri(tab_new(b, "Loading..."), *uris++);
		}
	} else {
		/* load homepage on startup */
		tab_load_uri(browser_get_current_tab(b), DEFAULT_HOME);
	}

	gtk_main();

	return 0;
}
