#ifndef _TAB_H_
#define _TAB_H_

typedef struct _tab { 
	gpointer parent;			/* back pointer to parent Browser */
	GtkWidget *scroll;			/* scrolled window */
	WebKitWebView *view;		/* webview - child of scroll */
	GtkWidget *hbox;			/* notebook label container */
	GtkWidget *label;			/* notebook text label */
	GtkWidget *spinner;			/* progress indicator */
	gchar *title;				/* title of current page */
	double progress; 				/* progress of page being loaded */
	guint status_context_id;	/* statusbar id */
} Tab;

#define TAB(obj) (Tab *)(obj)

#define TAB_TITLE_MAX 25
#define NEW_TAB_TITLE "New Tab"

#include "browser.h"

Tab *tab_new(Browser *b, char *title);
void tab_load_uri(Tab *t, gchar *uri);
void tab_forward(Tab *t);
void tab_back(Tab *t);
void tab_reload(Tab *t, gboolean bypass);
void tab_home(Tab *t);
void tab_zoom(Tab *t, gboolean in);
void tab_zoom_reset(Tab *t);
void tab_update_title(Tab *t);
void tab_view_source(Tab *t);
void tab_search_forward(Tab *t, const char *str, gboolean case_sensitive);
void tab_search_reverse(Tab *t, const char *str, gboolean case_sensitive);

#endif /* _TAB_H_ */
