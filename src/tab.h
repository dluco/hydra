#ifndef TAB_H
#define TAB_H

typedef struct _tab { 
	gpointer parent;			/* back pointer to parent Browser */
	GtkWidget *scroll;			/* scrolled window */
	WebKitWebView *view;		/* webview - child of scroll */
	GtkWidget *label;			/* notebook label */
	gchar *title;				/* title of current page */
	gint progress; 				/* progress of page being loaded */
	guint status_context_id;	/* statusbar id */
} Tab;

#define TAB(obj) (Tab *)(obj)

#define TAB_TITLE_MAX 25
#define NEW_TAB_TITLE "New Tab"

#include "browser.h"

Tab *tab_new(Browser *b, gboolean background);
void tab_load_uri(Tab *t, gchar *uri);
void tab_forward(Tab *t);
void tab_back(Tab *t);
void tab_reload(Tab *t, gboolean bypass);
void tab_home(Tab *t);
void tab_zoom(Tab *t, gboolean in);
void tab_zoom_reset(Tab *t);
WebKitWebView *tab_new_requested(WebKitWebView *v, WebKitWebFrame *f, Tab *t);
void tab_title_changed(WebKitWebView *v, GParamSpec *pspec, Tab *t);
void tab_progress_changed(WebKitWebView *view, GParamSpec *pspec, Tab *t);
void tab_update_title(Tab *t);
void tab_load_status_changed(WebKitWebView *view, GParamSpec *pspec, Tab *t);
void tab_view_source(Tab *t);
void tab_search_forward(Tab *t, GtkWidget *entry);
void tab_search_reverse(Tab *t, GtkWidget *entry);

#endif /* TAB_H */
