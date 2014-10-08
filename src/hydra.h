#ifndef HYDRA_H
#define HYDRA_H

typedef struct _browser {	
	WebKitWebSettings *webkitsettings;
	WebKitWebWindowFeatures *webkitwindowfeatures;
	SoupSession *session;
	SoupCookieJar *jar;
	gboolean hide;
	GtkWidget *win;
	GtkWidget *bar;	
	GtkWidget *searchbar; 
	GtkWidget *vbox;
	GtkWidget *hbox;
	GtkNotebook *notebook; 
	GtkWidget *status;
	GtkWidget *status_info;
} Browser;

void history_command();
void grab_bar();
void load_uri(gchar *uri);
void tab_new(gboolean b);
void tab_zoom(gboolean b);
void tab_close();
void tab_and_go();
void tab_focus(GtkNotebook *notebook, GtkWidget *page, guint page_num, gpointer user_data);
void tab_view_source();
void tab_reload(gboolean b);
void window_setup();
void search(GtkEntry *entry, gboolean b);
void toggle();
void show_search(gboolean b);
void focus_view();
WebKitWebView *tab_new_requested(WebKitWebView *v, WebKitWebFrame *f);
void tab_switch(gboolean b);

#endif /* HYDRA_H */
