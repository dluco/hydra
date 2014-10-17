#ifndef _HYDRA_H_
#define _HYDRA_H_

#include <gtk/gtk.h>
#include <webkit/webkit.h>

#ifndef NAME
#define NAME "hydra"
#endif

#ifndef PROG_NAME
#define PROG_NAME "Hydra"
#endif

#define DEFAULT_BROWSER_TITLE PROG_NAME
#define TAB_LABEL_WIDTH 100
#define NEW_TAB_TITLE "New Tab"

typedef struct _Hydra Hydra;
typedef struct _Browser Browser;
typedef struct _Tab Tab;

struct _Hydra {
	GList *browsers;
	GHashTable *settings;
};

struct _Browser {	
	WebKitWebSettings *webkit_settings;
	SoupSession *session;
	SoupCookieJar *jar;
	GtkWidget *window;
	GtkWidget *vbox;
	GtkWidget *uri_entry;	
	GtkWidget *toolbar;
	GtkToolItem *back_button, *forward_button, *refresh_button, *stop_button, *home_button;
	GtkWidget *notebook; 
	GtkWidget *new_tab_button;
	GtkWidget *statusbar;
	GQuark term_data_id;
	gchar *title;
	int status_context_id;
};

struct _Tab { 
	Browser *parent;			/* back pointer to parent Browser */
	GtkWidget *vbox;			/* vbox */
	GtkWidget *scroll;			/* scrolled window - child of vbox */
	WebKitWebView *view;		/* webview - child of scrolled window */
	GtkWidget *label;			/* notebook text label */
	GtkWidget *spinner;			/* progress indicator */
	GtkWidget *searchbar; 
	GtkWidget *search_label; 
	GtkWidget *search_entry; 
	GtkToolItem *search_previous; 
	GtkToolItem *search_next; 
	GtkToolItem *search_highlight;
	GtkToolItem *search_case;
	gchar *title;				/* title of current page */
	double progress; 				/* progress of page being loaded */
	guint status_context_id;	/* statusbar id */
};

#endif /* _HYDRA_H_ */
