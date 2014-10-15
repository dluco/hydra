#ifndef _BROWSER_H_
#define _BROWSER_H_

typedef struct _Browser {	
	WebKitWebSettings *webkit_settings;
	SoupSession *session;
	SoupCookieJar *jar;
	GtkWidget *window;
	GtkWidget *vbox;
	GtkWidget *uri_entry;	
	GtkWidget *toolbar;
	GtkToolItem *back_button, *forward_button, *refresh_button, *home_button;
	GtkWidget *notebook; 
	GtkWidget *new_tab_button;
	GtkWidget *statusbar;
	GQuark term_data_id;
	gchar *title;
	int status_context_id;
	gboolean hide;
} Browser;

#include "tab.h"

#define BROWSER(obj) (Browser *)(obj)

#define DEFAULT_BROWSER_TITLE "Hydra"

void browser_link_hover_cb(WebKitWebView *page, const gchar *title, const gchar *link, Browser *b);
Tab *browser_get_tab(Browser *b, int tab_num);
Tab *browser_get_current_tab(Browser *b);
int browser_get_tab_num(Browser *b, Tab *t);
int browser_get_current_tab_num(Browser *b);
void browser_new_tab(Browser *b);
void browser_close(Browser *b);
void browser_switch_tab(Browser *b, gboolean forward);
void browser_history(Browser *b);
void browser_show_uri_entry(Browser *b);
void tab_and_go();
void browser_go_back(Browser *b);
void browser_go_forward(Browser *b);
void browser_go_home(Browser *b);
void browser_reload(Browser *b);
void browser_show_search_entry(Browser *b);
void browser_focus_tab_view(Browser *b);
Browser *browser_new(void);

#endif /* _BROWSER_H_ */
