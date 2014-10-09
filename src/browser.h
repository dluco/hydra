#ifndef BROWSER_H
#define BROWSER_H

typedef struct _browser {	
	WebKitWebSettings *webkitsettings;
	WebKitWebWindowFeatures *webkitwindowfeatures;
	SoupSession *session;
	SoupCookieJar *jar;
	GtkWidget *window;
	GtkWidget *bar;	
	GtkWidget *searchbar; 
	GtkWidget *vbox;
	GtkNotebook *notebook; 
	GtkWidget *status;
	GtkWidget *status_info;
	GQuark term_data_id;
	gboolean hide;
} Browser;

#include "tab.h"

Tab *browser_get_tab(Browser *b, int tab_num);
Tab *browser_get_current_tab(Browser *b);
void browser_close_tab(Browser *b, Tab *t);
void browser_switch_tab(Browser *b, gboolean forward);
void browser_history(Browser *b);
void browser_show_bar(Browser *b);
void tab_and_go();
void browser_link_hover(WebKitWebView *page, const gchar *title, const gchar *link, Browser *b);
void browser_tab_switched(GtkNotebook *notebook, GtkWidget *page, guint page_num, Browser *b);
void browser_toggle_widget(Browser *b, GtkWidget *widget);
void browser_focus_tab_view(Browser *b);
Browser *browser_new(void);

#endif /* BROWSER_H */
