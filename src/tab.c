#include <string.h>
#include <gtk/gtk.h>
#include <webkit/webkit.h>
#include <glib/gprintf.h>
#include "browser.h"
#include "tab.h"
#include "utils.h"
#include "config.h"

#define DOWNLOAD_LOCATION g_get_home_dir()

static WebKitWebView *tab_new_requested(WebKitWebView *v, WebKitWebFrame *f, Tab *t);
static void tab_download_cb(WebKitWebView *web_view, GObject *d, gpointer user_data);
static void tab_title_changed(WebKitWebView *view, GParamSpec *pspec, Tab *t);
static void tab_load_status_changed(WebKitWebView *view, GParamSpec *pspec, Tab *t);
static void tab_progress_changed_cb(WebKitWebView *view, GParamSpec *pspec, Tab *t);
static void tab_search(Tab *t, const char *str, gboolean forward);

/* create new Tab with parent Browser */
Tab *tab_new(Browser *b, char *title)
{
	Tab *t;
//	gchar *stylesheet;

	t = g_new0(Tab, 1);

	t->scroll = gtk_scrolled_window_new(NULL, NULL);
	t->view = WEBKIT_WEB_VIEW(webkit_web_view_new());
	t->hbox = gtk_hbox_new(FALSE, 0);
	t->label = gtk_label_new(title);
	t->parent = b;

	/* label */
	t->spinner = gtk_spinner_new();
	gtk_box_pack_start(GTK_BOX(t->hbox), t->spinner, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(t->hbox), t->label, FALSE, FALSE, 5);

	gtk_widget_show_all(t->hbox);
	gtk_widget_hide(t->spinner);

	gtk_container_add(GTK_CONTAINER(t->scroll), GTK_WIDGET(t->view));

	int index = gtk_notebook_append_page(GTK_NOTEBOOK(b->notebook), t->scroll, t->hbox);
	gtk_notebook_set_tab_reorderable(GTK_NOTEBOOK(b->notebook), t->scroll, TRUE);
	gtk_label_set_max_width_chars(GTK_LABEL(t->label), TAB_TITLE_MAX);
	gtk_label_set_ellipsize(GTK_LABEL(t->label), PANGO_ELLIPSIZE_END);

	/* hide tabs if only one */
	gtk_notebook_set_show_tabs(GTK_NOTEBOOK(b->notebook), (index > 0));

	/*callbacks*/
	g_signal_connect(G_OBJECT(t->view), "notify::title", G_CALLBACK(tab_title_changed), t);
	g_signal_connect(G_OBJECT(t->view), "notify::load-status", G_CALLBACK(tab_load_status_changed), t);
	g_signal_connect(G_OBJECT(t->view), "notify::progress", G_CALLBACK(tab_progress_changed_cb), t);
	g_signal_connect(G_OBJECT(t->view), "hovering-over-link", G_CALLBACK(browser_link_hover_cb), b);
	g_signal_connect(G_OBJECT(t->view), "create-web-view", G_CALLBACK(tab_new_requested), t);
	g_signal_connect(G_OBJECT(t->view), "download-requested", G_CALLBACK(tab_download_cb), t->view);
	
	/* apply webkit settings */
	webkit_web_view_set_settings(t->view, b->webkit_settings);
	webkit_web_view_set_highlight_text_matches(t->view, TRUE);
	webkit_web_view_set_zoom_level(t->view, DEFAULT_ZOOM_LEVEL);
	
	/* scrolled window settings */
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(t->scroll), GTK_POLICY_NEVER, GTK_POLICY_NEVER);
	gtk_scrolled_window_set_shadow_type(GTK_SCROLLED_WINDOW(t->scroll), GTK_SHADOW_NONE);

	/* load user stylesheet */
//	stylesheet = g_strconcat("file://", g_get_home_dir(), "/", DEFAULT_STYLE_SHEET, NULL);

	/* FIXME: this should be set by config, not here */
//	g_object_set(G_OBJECT(b->webkitsettings),
//		"user_agent", DEFAULT_USER_AGENT, NULL);
//		"enable-page-cache", TRUE,
//		"enable-java-applet", FALSE,
//		"user-stylesheet-uri", stylesheet, NULL);					 

//	g_free(stylesheet);

	/* setup widgets, automatically focus the addressbar */
	g_object_set_qdata(G_OBJECT(gtk_notebook_get_nth_page(GTK_NOTEBOOK(b->notebook), index)), b->term_data_id, t);
	gtk_statusbar_pop(GTK_STATUSBAR(b->statusbar), b->status_context_id);
	gtk_statusbar_push(GTK_STATUSBAR(b->statusbar), b->status_context_id, "");

	gtk_widget_show_all(b->window);
	gtk_widget_hide(b->searchbar);
	
	/* TODO: switch to the new tab if it has not been opened in background */
	/*
	if (!background) {
		gtk_notebook_set_current_page(GTK_NOTEBOOK(b->notebook), index); 
	}
	*/
	gtk_notebook_set_current_page(GTK_NOTEBOOK(b->notebook), index); 

	gtk_widget_grab_focus(b->uri_entry);

	return t;
}

/* check for protocol and load uri */
void tab_load_uri(Tab *t, gchar *uri)
{
	gchar *uri_real;

	uri_real = g_strrstr(uri, "://") ? g_strdup(uri) : g_strdup_printf("http://%s", uri);
	webkit_web_view_load_uri(t->view, uri_real);

	g_free(uri_real);

	gtk_widget_grab_focus(GTK_WIDGET(t->view));
}

void tab_forward(Tab *t)
{
	webkit_web_view_go_forward(t->view); 
}

void tab_back(Tab *t)
{
	webkit_web_view_go_back(t->view);
}

/* basic reload function */
void tab_reload(Tab *t, gboolean bypass_cache)
{
	if (!bypass_cache) {
		webkit_web_view_reload(t->view);	
	} else {
		webkit_web_view_reload_bypass_cache(t->view);
	}
}

void tab_home(Tab *t)
{
	tab_load_uri(t, DEFAULT_HOME);
}

/* increase or decrease the zoom of the page */
void tab_zoom(Tab *t, gboolean zoom_in)
{
	if (zoom_in) {
		webkit_web_view_zoom_in(t->view);
	} else {
		webkit_web_view_zoom_out(t->view);
	}
}

/* reset the zoom of the page */
void tab_zoom_reset(Tab *t)
{
	webkit_web_view_set_zoom_level(t->view, DEFAULT_ZOOM_LEVEL); 
}

/* when a new tab is requested, return the t->view */
static WebKitWebView *tab_new_requested(WebKitWebView *v, WebKitWebFrame *f, Tab *t)
{
	Browser *b = BROWSER(t->parent);
	Tab *t_new = tab_new(b, "Loading...");
	gtk_widget_grab_focus(GTK_WIDGET(t_new->view));
	return t_new->view;
}

/* download callback */
static void tab_download_cb(WebKitWebView *web_view, GObject *d, gpointer user_data)
{
	gchar *command;

	const gchar *download_url = webkit_download_get_uri(WEBKIT_DOWNLOAD(d));
	const gchar *requested_name = webkit_download_get_suggested_filename(WEBKIT_DOWNLOAD(d));

	command = g_new0(gchar, strlen(DOWNLOAD_COMMAND) + strlen(DOWNLOAD_LOCATION) + strlen(requested_name) + strlen(download_url) + 1);
	g_sprintf(command, DOWNLOAD_COMMAND, DOWNLOAD_LOCATION, requested_name, download_url);
	g_spawn_command_line_async(command, NULL);

	g_free(command);
}

/* title change callback */
static void tab_title_changed(WebKitWebView *view, GParamSpec *pspec, Tab *t)
{
	const gchar *title = webkit_web_view_get_title(view);;

	if (title) {
		t->title = str_copy(&t->title, title);
		tab_update_title(t);
	}
}

void tab_update_title(Tab *t)
{
	Browser *b = BROWSER(t->parent);

	/* update window title if this is the current tab */
	if (browser_get_current_tab_num(b) == browser_get_tab_num(b, t)) {
		b->title = str_copy(&b->title, t->title);
		gtk_window_set_title(GTK_WINDOW(b->window), (b->title) ? b->title : "");
	}

	/* set tab label */
	gtk_label_set_text(GTK_LABEL(t->label), (t->title) ? t->title : "");
}

static void tab_load_status_changed(WebKitWebView *view, GParamSpec *pspec, Tab *t)
{
	Browser *b = BROWSER(t->parent);
	const gchar *uri;
	
	switch(webkit_web_view_get_load_status(t->view)) {
	case WEBKIT_LOAD_PROVISIONAL:
		gtk_label_set_text(GTK_LABEL(t->label), "Loading...");
		/* start and show spinner */
		gtk_spinner_start(GTK_SPINNER(t->spinner));
		gtk_widget_show(t->spinner);
		break;
	case WEBKIT_LOAD_COMMITTED:
		uri = webkit_web_view_get_uri(t->view);

		/* update uri-bar if this is the current tab */
		if (browser_get_current_tab_num(b) == browser_get_tab_num(b, t)) {
			gtk_entry_set_text(GTK_ENTRY(b->uri_entry), uri); 
		}
		
		FILE *history = fopen(g_build_filename(g_get_home_dir(), DEFAULT_HISTORY_FILE, NULL), "a+");
		
		if (history) {
			fprintf(history, "%s \n", uri);
			fclose(history);
		}

		break;
	case WEBKIT_LOAD_FINISHED:
		t->progress = 1.0;
		/* hide and stop spinner */
		gtk_widget_hide(t->spinner);
		gtk_spinner_stop(GTK_SPINNER(t->spinner));
		break;
	default:
		break;
	}

	/* update toolbar buttons */
	gtk_widget_set_sensitive(GTK_WIDGET(b->back_button), webkit_web_view_can_go_back(t->view));
	gtk_widget_set_sensitive(GTK_WIDGET(b->forward_button), webkit_web_view_can_go_forward(t->view));
}

static void tab_progress_changed_cb(WebKitWebView *view, GParamSpec *pspec, Tab *t)
{
	t->progress = webkit_web_view_get_progress(t->view);
}

/* toggle view source mode */
void tab_view_source(Tab *t)
{
	webkit_web_view_set_view_source_mode(t->view, !webkit_web_view_get_view_source_mode(t->view)); 
	webkit_web_view_reload(t->view);
}

static void tab_search(Tab *t, const char *str, gboolean forward)
{
	webkit_web_view_search_text(t->view, str, FALSE, forward, TRUE);
}

void tab_search_forward(Tab *t, const char *str)
{
	tab_search(t, str, TRUE);
}

void tab_search_reverse(Tab *t, const char *str)
{
	tab_search(t, str, FALSE);
}
