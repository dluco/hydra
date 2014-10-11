#include <string.h>
#include <gtk/gtk.h>
#include <webkit/webkit.h>
#include "config.h"
#include "browser.h"
#include "tab.h"
#include "callbacks.h"
#include "utils.h"

/* create new Tab with parent Browser */
Tab *tab_new(Browser *b, gboolean background)
{
	Tab *t;
//	gchar *stylesheet;

	t = g_new0(Tab, 1);

	t->scroll = gtk_scrolled_window_new(NULL, NULL);
	t->view = WEBKIT_WEB_VIEW(webkit_web_view_new());
	t->label = gtk_label_new(NEW_TAB_TITLE);
	t->parent = b;

	gtk_container_add(GTK_CONTAINER(t->scroll), GTK_WIDGET(t->view));

	int index = gtk_notebook_append_page(b->notebook, t->scroll, t->label);
	gtk_notebook_set_tab_reorderable(b->notebook, t->scroll, TRUE);

	/* hide tabs if only one */
	gtk_notebook_set_show_tabs(b->notebook, (index > 0));

	/*callbacks*/
//	g_object_connect(G_OBJECT(t->view), "signal::title-changed", G_CALLBACK(tab_title_changed), b,
//										"signal::notify::load-status", G_CALLBACK(tab_load_status_changed), b,
//										"signal::hovering-over-link", G_CALLBACK(browser_link_hover), b,
//										"signal::download-requested", G_CALLBACK(cb_download), t->view,
//										"signal::create-web-view", G_CALLBACK(tab_new_requested), b, NULL);

	g_signal_connect(G_OBJECT(t->view), "notify::title", G_CALLBACK(tab_title_changed), t);
	g_signal_connect(G_OBJECT(t->view), "notify::load-status", G_CALLBACK(tab_load_status_changed), t);
	g_signal_connect(G_OBJECT(t->view), "notify::progress", G_CALLBACK(tab_progress_changed), t);
	g_signal_connect(G_OBJECT(t->view), "hovering-over-link", G_CALLBACK(browser_link_hover), b);
	g_signal_connect(G_OBJECT(t->view), "create-web-view", G_CALLBACK(tab_new_requested), t);
	
	/*settings*/
	webkit_web_view_set_settings(t->view, b->webkit_settings);
	webkit_web_view_set_highlight_text_matches(t->view, TRUE);
	webkit_web_view_set_zoom_level(t->view, DEFAULT_ZOOM_LEVEL);
	
//	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(t->scroll), GTK_POLICY_NEVER, GTK_POLICY_NEVER);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(t->scroll),
		GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);

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
	gtk_statusbar_push(GTK_STATUSBAR(b->status), 0, "");
	g_object_set_qdata_full(G_OBJECT(gtk_notebook_get_nth_page(b->notebook, index)), b->term_data_id, t, NULL);

	gtk_widget_show_all(b->window);
	gtk_widget_hide(b->searchbar);
	
	/* switch to the new tab if it has not been opened in background */
	if (!background) {
		gtk_notebook_set_current_page(b->notebook, index); 
	}

	gtk_widget_grab_focus(b->bar);

	return t;
}

/* check for protocol and load uri */
void tab_load_uri(Tab *t, gchar *uri)
{
	gchar *uri_real;

	uri_real = g_strrstr(uri, "://") ? g_strdup(uri) : g_strdup_printf("http://%s", uri);
	webkit_web_view_load_uri(t->view, uri_real);

	g_free(uri_real);
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
void tab_reload(Tab *t, gboolean bypass)
{
	if (!bypass) {
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
WebKitWebView *tab_new_requested(WebKitWebView *v, WebKitWebFrame *f, Tab *t)
{
	Browser *b = BROWSER(t->parent);
	Tab *t_new = tab_new(b, FALSE); 
	return t_new->view;
}

/* title change callback */
void tab_title_changed(WebKitWebView *view, GParamSpec *pspec, Tab *t)
{
	const gchar *title = webkit_web_view_get_title(view);;

	if (title) {
		t->title = str_copy(&t->title, title);
		tab_update_title(t);
	}
}

void tab_update_title(Tab *t)
{
	gchar *tab_title;
	Browser *b = BROWSER(t->parent);

	/* update window title if this is the current tab */
	if (browser_get_current_tab_num(b) == browser_get_tab_num(b, t)) {
		b->title = g_strdup(t->title);
		gtk_window_set_title(GTK_WINDOW(b->window), (t->title) ? t->title : DEFAULT_BROWSER_TITLE);
	}

	/* truncate title if over max and append ellipsis */
	if (strlen(t->title) < TAB_TITLE_MAX) {
		tab_title = g_strdup((t->title) ? t->title : "");
	} else {
		tab_title = g_strndup(t->title, TAB_TITLE_MAX - 3);
		strcat(tab_title, "...");
	}

	/* set tab label */
	gtk_label_set_label(GTK_LABEL(t->label), tab_title);
	g_free(tab_title);
}

void tab_load_status_changed(WebKitWebView *view, GParamSpec *pspec, Tab *t)
{
	const gchar *uri = webkit_web_view_get_uri(view);
	WebKitLoadStatus status = webkit_web_view_get_load_status(view);
	Browser *b = BROWSER(t->parent);
	
	switch(status) {
	case WEBKIT_LOAD_PROVISIONAL:
		break;
	case WEBKIT_LOAD_COMMITTED:
		/* update uri-bar if this is the current tab */
		if (browser_get_current_tab_num(b) == browser_get_tab_num(b, t)) {
			gtk_entry_set_text(GTK_ENTRY(b->bar), uri); 
		}
		
		FILE *history = fopen(g_build_filename(g_get_home_dir(), DEFAULT_HISTORY_FILE, NULL), "a+");
		
		if (history) {
			fprintf(history, "%s \n", uri);
			fclose(history);
		}

		break;
	case WEBKIT_LOAD_FIRST_VISUALLY_NON_EMPTY_LAYOUT:
		break;
	case WEBKIT_LOAD_FINISHED:
		t->progress = 100;
		break;
	case WEBKIT_LOAD_FAILED:
		break;
	default:
		break;
	}

	/* update toolbar buttons */
	gtk_widget_set_sensitive(GTK_WIDGET(b->back_button), webkit_web_view_can_go_back(t->view));
	gtk_widget_set_sensitive(GTK_WIDGET(b->forward_button), webkit_web_view_can_go_forward(t->view));
}

void tab_progress_changed(WebKitWebView *view, GParamSpec *pspec, Tab *t)
{
	t->progress = webkit_web_view_get_progress(t->view) * 100;
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

void tab_search_forward(Tab *t, GtkWidget *entry)
{
	tab_search(t, gtk_entry_get_text(GTK_ENTRY(entry)), TRUE);
}

void tab_search_reverse(Tab *t, GtkWidget *entry)
{
	tab_search(t, gtk_entry_get_text(GTK_ENTRY(entry)), FALSE);
}
