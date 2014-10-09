#ifndef CALLBACKS_H
#define CALLBACKS_H

void cb_download(WebKitWebView *web_view, GObject *download, gpointer user_data);
void cb_load_status(WebKitWebView *view, GParamSpec *pspec, Browser *b);

#endif /* CALLBACKS_H */
