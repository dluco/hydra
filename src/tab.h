#ifndef TAB_H
#define TAB_H

#include "browser.h"

typedef struct _tab { 
	Browser *parent;			/* back pointer to Browser */
	GtkWidget *scroll;			/* scrolled window */
	WebKitWebView *view;		/* webview - child of scroll */
	GtkWidget *label;			/* notebook label */
	gchar *main_title;			/* ... */
	gchar *url_entry;			/* ... */
	gint load_progress; 		/* progress of page being loaded */
	guint status_context_id;	/* statusbar id */
} Tab;

Tab *tab_new(Browser *b, gboolean background);
void tab_load_uri(Tab *t, gchar *uri);
void tab_forward(Tab *t);
void tab_back(Tab *t);
void tab_reload(Tab *t, gboolean bypass);
void tab_zoom(Tab *t, gboolean in);
void tab_zoom_reset(Tab *t);
WebKitWebView *tab_new_requested(WebKitWebView *v, WebKitWebFrame *f, Browser *b);
void tab_title_changed(WebKitWebView *v, WebKitWebFrame *f, const char *title, Browser *b);
void tab_view_source(Tab *t);
void tab_search_forward(Tab *t, GtkWidget *entry);
void tab_search_reverse(Tab *t, GtkWidget *entry);

#endif /* TAB_H */
