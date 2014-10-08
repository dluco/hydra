#ifndef TAB_H
#define TAB_H

typedef struct _tab { 
	GtkWidget *scroll; 
	GtkWidget *label; 
	gchar *main_title; 
	gchar *url_entry;
	gint load_progress; 
	guint status_context_id;
	WebKitWebView *view; 
} Tab;

#endif /* TAB_H */
