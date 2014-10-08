#include <glib.h>
#include "args.h"

Args *options_parse(int argc, char *argv[])
{
	Args *args;
	GOptionContext *context;
	GError *gerror = NULL;

	/* Allocate Args */
	args = g_new0(Args, 1);

	GOptionEntry entries[] = {
		{ "version", 'v', 0, G_OPTION_ARG_NONE, &args->version, "Print version number", NULL },
		{ "fullscreen", 0, 0, G_OPTION_ARG_NONE, &args->fullscreen, "Fullscreen mode", NULL },
		{ "geometry", 'g', 0, G_OPTION_ARG_STRING, &args->geometry, "X geometry specification", "GEOMETRY" },
		{ NULL }
	};

	context = g_option_context_new("- web browser");
	g_option_context_add_main_entries(context, entries, NULL);
	g_option_context_add_group(context, gtk_get_option_group(TRUE));
	if (!g_option_context_parse(context, &argc, &argv, &gerror)) {
		die("%s\n", gerror->message);
	}
	g_option_context_free(context);

	/* Print version info and exit */
	if (args->version) {
		printf("hydra %s, 2014 David Luco\n", VERSION);
		exit(EXIT_SUCCESS);
	}

	return args;
}
