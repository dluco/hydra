#ifndef BROWSER_H
#define BROWSER_H

typedef struct _browser {	
	WebKitWebSettings *webkit_settings;
	SoupSession *session;
	SoupCookieJar *jar;
	GtkWidget *window;
	GtkWidget *vbox;
	GtkWidget *bar;	
	GtkWidget *toolbar;
	GtkToolItem *back_button, *forward_button, *refresh_button, *home_button;
	GtkNotebook *notebook; 
	GtkWidget *searchbar; 
	GtkWidget *status;
	GtkWidget *status_info;
	GQuark term_data_id;
	gchar *title;
	gboolean hide;
} Browser;

#define BROWSER(obj) (Browser *)(obj)

#define DEFAULT_BROWSER_TITLE "Hydra"

#include "tab.h"

Tab *browser_get_tab(Browser *b, int tab_num);
Tab *browser_get_current_tab(Browser *b);
int browser_get_tab_num(Browser *b, Tab *t);
int browser_get_current_tab_num(Browser *b);
void browser_close_tab(Browser *b, Tab *t);
void browser_switch_tab(Browser *b, gboolean forward);
void browser_history(Browser *b);
void browser_show_uribar(Browser *b);
void tab_and_go();
void browser_back(Browser *b);
void browser_forward(Browser *b);
void browser_reload(Browser *b);
void browser_home(Browser *b);
void browser_link_hover(WebKitWebView *page, const gchar *title, const gchar *link, Browser *b);
void browser_tab_switched(GtkNotebook *notebook, GtkWidget *page, guint page_num, Browser *b);
void browser_toggle_widget(Browser *b, GtkWidget *widget);
void browser_focus_tab_view(Browser *b);
Browser *browser_new(void);

#endif /* BROWSER_H */
