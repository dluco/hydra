#include <string.h>
#include <gtk/gtk.h>
#include <webkit/webkit.h>
#include <glib/gprintf.h>
#include "browser.h"
#include "tab.h"
#include "utils.h"
#include "config.h"

#define DOWNLOAD_LOCATION g_get_home_dir()

/* callbacks */
static void tab_search_entry_activated_cb(GtkWidget *entry, Tab *t);
static void tab_search_previous_cb(GtkWidget *widget, Tab *t);
static void tab_search_next_cb(GtkWidget *widget, Tab *t);
static void tab_search_highlight_cb(GtkWidget *widget, Tab *t);
static void tab_search_case_cb(GtkWidget *widget, Tab *t);
static WebKitWebView *tab_new_requested(WebKitWebView *v, WebKitWebFrame *f, Tab *t);
static void tab_download_cb(WebKitWebView *web_view, GObject *d, gpointer user_data);
static void tab_title_changed(WebKitWebView *view, GParamSpec *pspec, Tab *t);
static void tab_load_status_changed(WebKitWebView *view, GParamSpec *pspec, Tab *t);
static void tab_progress_changed_cb(WebKitWebView *view, GParamSpec *pspec, Tab *t);
static void tab_search(Tab *t, const char *str, gboolean forward, gboolean case_sensitive);

/* search-entry callback */
static void tab_search_entry_activated_cb(GtkWidget *entry, Tab *t)
{
	tab_search_forward(t, gtk_entry_get_text(GTK_ENTRY(t->search_entry)),
		gtk_toggle_tool_button_get_active(GTK_TOGGLE_TOOL_BUTTON(t->search_case)));
}

static void tab_search_entry_text_inserted_cb(GtkEditable *editable, gchar *new_text, gint new_length, gpointer position, Tab *t)
{
	/* update text matches */
	tab_update_search_highlight(t);
}

static void tab_search_entry_text_deleted_cb(GtkEditable *editable, gint start, gint end, Tab *t)
{
	/* update text matches */
	tab_update_search_highlight(t);
}

static void tab_search_previous_cb(GtkWidget *widget, Tab *t)
{
	tab_search_reverse(t, gtk_entry_get_text(GTK_ENTRY(t->search_entry)),
		gtk_toggle_tool_button_get_active(GTK_TOGGLE_TOOL_BUTTON(t->search_case)));
}

static void tab_search_next_cb(GtkWidget *widget, Tab *t)
{
	tab_search_forward(t, gtk_entry_get_text(GTK_ENTRY(t->search_entry)),
		gtk_toggle_tool_button_get_active(GTK_TOGGLE_TOOL_BUTTON(t->search_case)));
}

static void tab_search_highlight_cb(GtkWidget *widget, Tab *t)
{
	/* update text matches */
	tab_update_search_highlight(t);
	/* toggle highlighting */
	webkit_web_view_set_highlight_text_matches(t->view,
		gtk_toggle_tool_button_get_active(GTK_TOGGLE_TOOL_BUTTON(t->search_highlight)));
}

static void tab_search_case_cb(GtkWidget *widget, Tab *t)
{
	/* update text matches */
	tab_update_search_highlight(t);
}

/* create new Tab with parent Browser */
Tab *tab_new(Browser *b, char *title)
{
	Tab *t;
	GtkWidget *hbox, *ebox, *close_button;
	GtkRcStyle *rcstyle;
	GtkToolItem *tool_item;

	t = g_new0(Tab, 1);

	t->parent = b;
	t->vbox = gtk_vbox_new(FALSE, 0);

	/* tab notebook label */
	ebox = gtk_event_box_new();
	gtk_widget_set_events(ebox, GDK_BUTTON_PRESS_MASK);
	hbox = gtk_hbox_new(FALSE, 0);
	gtk_container_add(GTK_CONTAINER(ebox), hbox);
	/* tab spinner */
	t->spinner = gtk_spinner_new();
	/* tab label */
	t->label = gtk_label_new(title);
	gtk_widget_set_size_request(t->label, TAB_LABEL_WIDTH, -1);
	gtk_label_set_ellipsize(GTK_LABEL(t->label), PANGO_ELLIPSIZE_END);
	/* align label to left */
	gtk_misc_set_alignment(GTK_MISC(t->label), 0.0, 0.5);
	/* tab close button */
	close_button = gtk_button_new();
	gtk_button_set_relief(GTK_BUTTON(close_button), GTK_RELIEF_NONE);
	gtk_button_set_focus_on_click(GTK_BUTTON(close_button), FALSE);
	gtk_container_add(GTK_CONTAINER(close_button), gtk_image_new_from_stock(GTK_STOCK_CLOSE, GTK_ICON_SIZE_MENU));
	/* make button as small as possible */
	rcstyle = gtk_rc_style_new();
	rcstyle->xthickness = rcstyle->ythickness = 0;
	gtk_widget_modify_style(close_button, rcstyle);
	/* pack and show all */
	gtk_box_pack_start(GTK_BOX(hbox), t->spinner, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(hbox), t->label, TRUE, TRUE, 6);
	gtk_box_pack_start(GTK_BOX(hbox), close_button, FALSE, FALSE, 0);
	gtk_widget_show_all(ebox);
	/* hide spinner initially */
	gtk_widget_hide(t->spinner);

	/* scrolled webview */
	t->scroll = gtk_scrolled_window_new(NULL, NULL);
	t->view = WEBKIT_WEB_VIEW(webkit_web_view_new());
	gtk_container_add(GTK_CONTAINER(t->scroll), GTK_WIDGET(t->view));

	gtk_box_pack_start(GTK_BOX(t->vbox), t->scroll, TRUE, TRUE, 0);

	/* searchbar */
	t->searchbar = gtk_toolbar_new();
	gtk_orientable_set_orientation(GTK_ORIENTABLE(t->searchbar), GTK_ORIENTATION_HORIZONTAL);
	gtk_toolbar_set_icon_size(GTK_TOOLBAR(t->searchbar), GTK_ICON_SIZE_MENU);
	gtk_toolbar_set_show_arrow(GTK_TOOLBAR(t->searchbar), FALSE);
	/* search label */
	t->search_label = gtk_label_new("Search:");
	tool_item = gtk_tool_item_new();
	gtk_container_set_border_width(GTK_CONTAINER(tool_item), 5);
	gtk_container_add(GTK_CONTAINER(tool_item), t->search_label);
	gtk_toolbar_insert(GTK_TOOLBAR(t->searchbar), GTK_TOOL_ITEM(tool_item), -1);
	/* search entry */
	t->search_entry = gtk_entry_new();
	tool_item = gtk_tool_item_new();
	gtk_container_set_border_width(GTK_CONTAINER(tool_item), 1);
	gtk_container_add(GTK_CONTAINER(tool_item), t->search_entry);
	gtk_tool_item_set_expand(tool_item, TRUE);	// allow search_entry to expand
	gtk_toolbar_insert(GTK_TOOLBAR(t->searchbar), GTK_TOOL_ITEM(tool_item), -1);
	/* search previous button */
	t->search_previous = gtk_tool_button_new_from_stock(GTK_STOCK_GO_UP);
	gtk_toolbar_insert(GTK_TOOLBAR(t->searchbar), GTK_TOOL_ITEM(t->search_previous), -1);
	/* search next button */
	t->search_next = gtk_tool_button_new_from_stock(GTK_STOCK_GO_DOWN);
	gtk_toolbar_insert(GTK_TOOLBAR(t->searchbar), GTK_TOOL_ITEM(t->search_next), -1);
	/* toggle highlight all matches button */
	t->search_highlight = gtk_toggle_tool_button_new();
	gtk_tool_button_set_label(GTK_TOOL_BUTTON(t->search_highlight), "Highlight All");
	gtk_toolbar_insert(GTK_TOOLBAR(t->searchbar), GTK_TOOL_ITEM(t->search_highlight), -1);
	/* toggle search case sensitivity button */
	t->search_case = gtk_toggle_tool_button_new();
	gtk_tool_button_set_label(GTK_TOOL_BUTTON(t->search_case), "Match Case");
	gtk_toolbar_insert(GTK_TOOLBAR(t->searchbar), GTK_TOOL_ITEM(t->search_case), -1);
	/* separator item - separates search items from close button */
	tool_item = gtk_separator_tool_item_new();
	gtk_tool_item_set_expand(tool_item, TRUE);
	gtk_toolbar_insert(GTK_TOOLBAR(t->searchbar), GTK_TOOL_ITEM(tool_item), -1);
	/* searchbar hide button */
	t->search_hide = gtk_tool_button_new_from_stock(GTK_STOCK_CLOSE);
	gtk_toolbar_insert(GTK_TOOLBAR(t->searchbar), GTK_TOOL_ITEM(t->search_hide), -1);

	gtk_box_pack_start(GTK_BOX(t->vbox), t->searchbar, FALSE, FALSE, 0);

	/*callbacks*/
	g_signal_connect(G_OBJECT(t->view), "notify::title", G_CALLBACK(tab_title_changed), t);
	g_signal_connect(G_OBJECT(t->view), "notify::load-status", G_CALLBACK(tab_load_status_changed), t);
	g_signal_connect(G_OBJECT(t->view), "notify::progress", G_CALLBACK(tab_progress_changed_cb), t);
	g_signal_connect(G_OBJECT(t->view), "hovering-over-link", G_CALLBACK(browser_link_hover_cb), b);
	g_signal_connect(G_OBJECT(t->view), "create-web-view", G_CALLBACK(tab_new_requested), t);
	g_signal_connect(G_OBJECT(t->view), "download-requested", G_CALLBACK(tab_download_cb), t->view);
	g_signal_connect(G_OBJECT(t->search_entry), "activate", G_CALLBACK(tab_search_entry_activated_cb), t);
	g_signal_connect_after(G_OBJECT(t->search_entry), "insert-text", G_CALLBACK(tab_search_entry_text_inserted_cb), t);
	g_signal_connect_after(G_OBJECT(t->search_entry), "delete-text", G_CALLBACK(tab_search_entry_text_deleted_cb), t);
	g_signal_connect(G_OBJECT(t->search_previous), "clicked", G_CALLBACK(tab_search_previous_cb), t);
	g_signal_connect(G_OBJECT(t->search_next), "clicked", G_CALLBACK(tab_search_next_cb), t);
	g_signal_connect(G_OBJECT(t->search_highlight), "toggled", G_CALLBACK(tab_search_highlight_cb), t);
	g_signal_connect(G_OBJECT(t->search_case), "toggled", G_CALLBACK(tab_search_case_cb), t);
	g_signal_connect_swapped(G_OBJECT(t->search_hide), "clicked", G_CALLBACK(gtk_widget_hide), t->searchbar);
	
	/* apply webkit settings */
	webkit_web_view_set_settings(t->view, b->webkit_settings);
	webkit_web_view_set_highlight_text_matches(t->view, TRUE);
	webkit_web_view_set_zoom_level(t->view, DEFAULT_ZOOM_LEVEL);
	
	/* scrolled window settings */
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(t->scroll), GTK_POLICY_NEVER, GTK_POLICY_NEVER);
	gtk_scrolled_window_set_shadow_type(GTK_SCROLLED_WINDOW(t->scroll), GTK_SHADOW_NONE);

	/* add to notebook */
	int index = gtk_notebook_append_page(GTK_NOTEBOOK(b->notebook), t->vbox, ebox);
	gtk_notebook_set_tab_reorderable(GTK_NOTEBOOK(b->notebook), t->vbox, TRUE);

	/* setup widgets, automatically focus the addressbar */
	g_object_set_qdata(G_OBJECT(gtk_notebook_get_nth_page(GTK_NOTEBOOK(b->notebook), index)), b->term_data_id, t);
	gtk_statusbar_pop(GTK_STATUSBAR(b->statusbar), b->status_context_id);
	gtk_statusbar_push(GTK_STATUSBAR(b->statusbar), b->status_context_id, "");

	/* hide searchbar initally */
	gtk_widget_show_all(t->vbox);
	gtk_widget_hide(t->searchbar);
	
	/* TODO: switch to the new tab if it has not been opened in background */
	/*
	if (!background) {
		gtk_notebook_set_current_page(GTK_NOTEBOOK(b->notebook), index); 
	}
	*/
//	gtk_notebook_set_current_page(GTK_NOTEBOOK(b->notebook), index); 

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

void tab_stop_loading(Tab *t)
{
	webkit_web_view_stop_loading(t->view);
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

void tab_update_search_highlight(Tab *t)
{
	/* update highlighted matches, if button is set */
	if (gtk_toggle_tool_button_get_active(GTK_TOGGLE_TOOL_BUTTON(t->search_highlight))) {
		/* remove previous highlighting */
		webkit_web_view_unmark_text_matches(t->view);
		/* highlight new text */
		webkit_web_view_mark_text_matches(t->view, gtk_entry_get_text(GTK_ENTRY(t->search_entry)),
			gtk_toggle_tool_button_get_active(GTK_TOGGLE_TOOL_BUTTON(t->search_case)), 0);
		webkit_web_view_set_highlight_text_matches(t->view, TRUE);
	}
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

/* show searchbar and give focus to entry */
void tab_show_search_entry(Tab *t)
{
	gtk_widget_show(t->searchbar);
	gtk_widget_grab_focus(t->search_entry);
}

static void tab_search(Tab *t, const char *str, gboolean forward, gboolean case_sensitive)
{
	webkit_web_view_search_text(t->view, str, case_sensitive, forward, TRUE);
}

void tab_search_forward(Tab *t, const char *str, gboolean case_sensitive)
{
	tab_search(t, str, TRUE, case_sensitive);
}

void tab_search_reverse(Tab *t, const char *str, gboolean case_sensitive)
{
	tab_search(t, str, FALSE, case_sensitive);
}
