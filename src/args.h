#ifndef ARGS_H
#define ARGS_H

typedef struct _args {
	gboolean version;
	gboolean fullscreen;
	char *geometry;
} Args;

Args *args_parse(int argc, char *argv[]);

#endif /* ARGS_H */
