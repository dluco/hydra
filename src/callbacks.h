#ifndef CALLBACKS_H
#define CALLBACKS_H

/* all functions should use this format cb_CALLBACK */

gboolean cb_keypress(GtkWidget *widget, GdkEventKey *event);
void cb_entry(GtkWidget *entry, gpointer data);
void cb_link_hover(WebKitWebView *page, const gchar *title, const gchar *link, gpointer data);
void cb_go(gboolean b);
void cb_download(WebKitWebView *web_view, GObject *download, gpointer user_data);
void cb_title_changed(WebKitWebView *v, WebKitWebFrame *f, const char *title, tab *t);
void cb_load_status(GObject *object, GParamSpec *pspec, tab *t);

#endif /* CALLBACKS_H */
