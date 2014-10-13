#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdarg.h>
#include <gdk/gdkkeysyms.h>
#include <webkit/webkit.h>
#include <glib/gstdio.h>
#include "config.h"
#include "browser.h"
#include "tab.h"
#include "utils.h"

static gboolean browser_key_press_event_cb(GtkWidget *widget, GdkEventKey *event, Browser *b);
static void browser_tab_switched_cb(GtkNotebook *notebook, GtkWidget *page, guint page_num, Browser *b);
static void browser_uri_entry_activated_cb(GtkWidget *entry, Browser *b);

static gboolean browser_key_press_event_cb(GtkWidget *widget, GdkEventKey *event, Browser *b)
{
	if (event->type != GDK_KEY_PRESS) {
		return FALSE;
	}

	/* get current tab */
	Tab *t = browser_get_current_tab(b);

	/* Check if Caps-Lock is enabled - change keyval
	   to work with upper/lowercase. */
	if (gdk_keymap_get_caps_lock_state(gdk_keymap_get_default())) {
		event->keyval = gdk_keyval_to_upper(event->keyval);
	}

	/* FIXME: cleanup */
	guint(g) = event->keyval;

	if ((event->state & GDK_CONTROL_MASK) == GDK_CONTROL_MASK ) {
		switch(g) {
		case GDK_KEY_l:
			browser_show_uri_entry(b);
			return TRUE;
		case GDK_KEY_f:
			tab_show_search_entry(t);
			return TRUE;
		case GDK_KEY_g:
			tab_and_go(b);
			return TRUE;
		case GDK_KEY_Back:
			tab_back(t);
			return TRUE;
		case GDK_KEY_Forward:
			tab_forward(t);
			return TRUE;
		case GDK_KEY_comma:
			tab_back(t);
			return TRUE;
		case GDK_KEY_period:
			tab_forward(t);
			return TRUE;
		case GDK_KEY_o:
			browser_history(b);
			return TRUE;
		case GDK_KEY_Page_Up:
			browser_switch_tab(b, FALSE);
			return TRUE;
		case GDK_KEY_Page_Down:
			browser_switch_tab(b, TRUE);
			return TRUE;
		case GDK_KEY_t:
			tab_new(b, "New Tab");
			gtk_widget_grab_focus(b->uri_entry);
			return TRUE;
		case GDK_KEY_w:
			browser_close_tab(b, t);
			return TRUE;
		case GDK_KEY_q:
			browser_close(b);
			return TRUE;
		case GDK_KEY_bracketright:
			tab_zoom(t, TRUE);
			return TRUE;
		case GDK_KEY_bracketleft:
			tab_zoom(t, FALSE);
			return TRUE;
		case GDK_KEY_r:
			tab_reload(t, FALSE);
			return TRUE;
		case GDK_KEY_e:
			tab_reload(t, TRUE);
			return TRUE;
		case GDK_KEY_s:
			tab_view_source(t);
			return TRUE;
		case GDK_KEY_Return:
			tab_load_uri(t, g_strconcat(DEFAULT_SEARCH, gtk_entry_get_text(GTK_ENTRY(b->uri_entry)), NULL));
			return TRUE;
		default:
			return FALSE;
		}
	}

	/* Esc : change focus to webview, hide searchbar if currently focused */
	if (g == GDK_KEY_Escape) {
		if (gtk_widget_has_focus(t->search_entry)) {
			gtk_widget_hide(t->searchbar);
		}
		browser_focus_tab_view(b);
	}

	/* Ctrl+Shift+g : reverse search */
	if ((g == GDK_KEY_Return) && (event->state & GDK_MOD1_MASK) == GDK_MOD1_MASK) {
		tab_search_reverse(t, gtk_entry_get_text(GTK_ENTRY(t->search_entry)),
			gtk_toggle_tool_button_get_active(GTK_TOGGLE_TOOL_BUTTON(t->search_case)));
		return TRUE;
	}
	
	return FALSE; 
}

/* focus on tab after switching, aka title, statusbar, view, etc */
static void browser_tab_switched_cb(GtkNotebook *notebook, GtkWidget *page, guint page_num, Browser *b)
{
	Tab *t = browser_get_tab(b, page_num);
	const char *uri = webkit_web_view_get_uri(t->view);
	const char *title = webkit_web_view_get_title(t->view);

	/* reset browser state for new tab */
	gtk_window_set_title(GTK_WINDOW(b->window), (title) ? title : DEFAULT_BROWSER_TITLE);
	gtk_entry_set_text(GTK_ENTRY(b->uri_entry), (uri) ? uri : "");
	gtk_statusbar_pop(GTK_STATUSBAR(b->statusbar), b->status_context_id);
	gtk_statusbar_push(GTK_STATUSBAR(b->statusbar), b->status_context_id, "");

	/* update toolbar buttons */
	gtk_widget_set_sensitive(GTK_WIDGET(b->back_button), webkit_web_view_can_go_back(t->view));
	gtk_widget_set_sensitive(GTK_WIDGET(b->forward_button), webkit_web_view_can_go_forward(t->view));
}

/* uri-bar callback */
static void browser_uri_entry_activated_cb(GtkWidget *entry, Browser *b)
{
	Tab *t = browser_get_current_tab(b);

	tab_load_uri(t, g_strdup(gtk_entry_get_text(GTK_ENTRY(b->uri_entry))));
}

/* link hovering callback */
void browser_link_hover_cb(WebKitWebView *page, const gchar *title, const gchar *link, Browser *b)
{
	gtk_statusbar_pop(GTK_STATUSBAR(b->statusbar), b->status_context_id); 
	if (link) {
		gtk_statusbar_push(GTK_STATUSBAR(b->statusbar), b->status_context_id, link); 
	}
}

Tab *browser_get_tab(Browser *b, int tab_num)
{
	return (Tab *)g_object_get_qdata(G_OBJECT(gtk_notebook_get_nth_page(GTK_NOTEBOOK(b->notebook), tab_num)), b->term_data_id);
}

Tab *browser_get_current_tab(Browser *b)
{
	return browser_get_tab(b, gtk_notebook_get_current_page(GTK_NOTEBOOK(b->notebook)));
}

/* get the tab number containing the tab specified */
int browser_get_tab_num(Browser *b, Tab *t)
{
	return gtk_notebook_page_num(GTK_NOTEBOOK(b->notebook), t->scroll);
}

int browser_get_current_tab_num(Browser *b)
{
	return browser_get_tab_num(b, browser_get_current_tab(b));
}

void browser_close(Browser *b)
{
	int i;
	int n = gtk_notebook_get_n_pages(GTK_NOTEBOOK(b->notebook));

	for (i = 0; i < n; i++) {
		browser_close_tab(b, browser_get_current_tab(b));
	}
}

/* close tab, and quit if there are no tabs */
void browser_close_tab(Browser *b, Tab *t)
{
	gtk_notebook_remove_page(GTK_NOTEBOOK(b->notebook), gtk_notebook_get_current_page(GTK_NOTEBOOK(b->notebook)));
	g_free(t);

	if (gtk_notebook_get_n_pages(GTK_NOTEBOOK(b->notebook)) == 1) {
		/* hide notebook tabs when only one tab */
		gtk_notebook_set_show_tabs(GTK_NOTEBOOK(b->notebook), FALSE);
		browser_focus_tab_view(b);
	} else if (gtk_notebook_get_n_pages(GTK_NOTEBOOK(b->notebook)) == 0) { 
		/* exit if no tabs remaining */
		gtk_main_quit(); 
	}
}

/* switch tabs forward or backwards */
void browser_switch_tab(Browser *b, gboolean forward)
{
	int tab_num;
	int current = gtk_notebook_get_current_page(GTK_NOTEBOOK(b->notebook));
	int n = gtk_notebook_get_n_pages(GTK_NOTEBOOK(b->notebook));

	tab_num = (forward) ? mod(current + 1, n) : mod(current - 1, n);

	gtk_notebook_set_current_page(GTK_NOTEBOOK(b->notebook), tab_num);
}

/* an alternative to the regular tab command, combines tabbing and history command into one */
void tab_and_go(Browser *b)
{
	gchar *returned;

	g_spawn_command_line_sync(g_strconcat("sh -c 'sort ",
		g_build_filename(g_get_home_dir(), DEFAULT_HISTORY_FILE, NULL),	
		" | dmenu -l 15'", NULL), &returned, NULL, NULL, NULL);	

	if (strcmp(returned, "") == 0) {
		browser_focus_tab_view(b); 
	} else { 
		Tab *t = tab_new(b, "Loading...");
		tab_load_uri(t, returned);
		g_free(returned);
	}
}

void browser_go_back(Browser *b)
{
	tab_back(browser_get_current_tab(b));
}

void browser_go_forward(Browser *b)
{
	tab_forward(browser_get_current_tab(b));
}

void browser_reload(Browser *b)
{
	tab_reload(browser_get_current_tab(b), FALSE);
}

void browser_go_home(Browser *b)
{
	tab_home(browser_get_current_tab(b));
}

void browser_show_uri_entry(Browser *b)
{
	if (!gtk_widget_get_visible(b->uri_entry)) {
		gtk_widget_show(b->uri_entry);
	}
 
	gtk_widget_grab_focus(GTK_WIDGET(b->uri_entry));
}

/* call the history command. should we do it ASYNC?*/
void browser_history(Browser *b)
{
	gchar *file; 
	gchar *command;
	gchar *returned;

	file =	g_build_filename(g_get_home_dir(), DEFAULT_HISTORY_FILE, NULL);
	command = g_strconcat("sh -c 'sort ", file, " | dmenu -l 15'", NULL);

	g_spawn_command_line_sync(command, &returned, NULL, NULL, NULL);	
	
	if (strcmp(returned, "") == 0) {
		browser_focus_tab_view(b); 
	} else {
		Tab *t = browser_get_current_tab(b);
		tab_load_uri(t, returned); 
	}

	g_free(file);
	g_free(command);
	g_free(returned);
}

/* focus on	current tab's webview */
void browser_focus_tab_view(Browser *b)
{
	Tab *t = browser_get_current_tab(b);
	gtk_widget_grab_focus(GTK_WIDGET(t->view));	
}

/* misc functions to help initialization */
Browser *browser_new(void)
{
	Browser *b;
	Tab *t;
	GtkToolItem *tool_item;
	
	b = g_new0(Browser, 1);
	
	b->term_data_id = g_quark_from_static_string("tab");

	b->window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	b->vbox = gtk_vbox_new(FALSE, 0);
	b->notebook = gtk_notebook_new();

	/* toolbar */
	b->toolbar = gtk_toolbar_new();
	gtk_orientable_set_orientation(GTK_ORIENTABLE(b->toolbar), GTK_ORIENTATION_HORIZONTAL);
	b->back_button = gtk_tool_button_new_from_stock(GTK_STOCK_GO_BACK);
	b->forward_button = gtk_tool_button_new_from_stock(GTK_STOCK_GO_FORWARD);
	b->refresh_button = gtk_tool_button_new_from_stock(GTK_STOCK_REFRESH);
	b->uri_entry = gtk_entry_new();
	tool_item = gtk_tool_item_new();
	gtk_tool_item_set_expand(tool_item, TRUE);	// allow uri-bar to expand
	b->home_button = gtk_tool_button_new_from_stock(GTK_STOCK_HOME);

	gtk_toolbar_insert(GTK_TOOLBAR(b->toolbar), GTK_TOOL_ITEM(b->back_button), -1);
	gtk_toolbar_insert(GTK_TOOLBAR(b->toolbar), GTK_TOOL_ITEM(b->forward_button), -1);
	gtk_toolbar_insert(GTK_TOOLBAR(b->toolbar), GTK_TOOL_ITEM(b->refresh_button), -1);
	gtk_container_add(GTK_CONTAINER(tool_item), b->uri_entry);
	gtk_toolbar_insert(GTK_TOOLBAR(b->toolbar), GTK_TOOL_ITEM(tool_item), -1);
	gtk_toolbar_insert(GTK_TOOLBAR(b->toolbar), GTK_TOOL_ITEM(b->home_button), -1);

	/* statusbar */
	b->statusbar = gtk_statusbar_new();
	
	gtk_box_pack_start(GTK_BOX(b->vbox), b->toolbar, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(b->vbox), b->notebook, TRUE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(b->vbox), b->statusbar, FALSE, FALSE, 0);
	gtk_container_add(GTK_CONTAINER(b->window), b->vbox);

	/* basic settings */
	gtk_window_set_wmclass(GTK_WINDOW(b->window), "hydra", "Hydra");
	gtk_window_set_role(GTK_WINDOW(b->window), "Hydra");
	gtk_window_set_default_size(GTK_WINDOW(b->window), DEFAULT_HEIGHT, DEFAULT_WIDTH);

	gtk_notebook_set_scrollable(GTK_NOTEBOOK(b->notebook), TRUE);
	gtk_notebook_set_show_border(GTK_NOTEBOOK(b->notebook), FALSE);

	gtk_widget_set_sensitive(GTK_WIDGET(b->back_button), FALSE);
	gtk_widget_set_sensitive(GTK_WIDGET(b->forward_button), FALSE);

	b->webkit_settings = webkit_web_settings_new();

	/* basic webkit settings */
	g_object_set(G_OBJECT(b->webkit_settings),
		"user_agent", DEFAULT_USER_AGENT,
		"enable-plugins", TRUE, NULL);

	b->status_context_id = gtk_statusbar_get_context_id(GTK_STATUSBAR(b->statusbar), "link-hover");

	b->session = webkit_get_default_session();
	b->jar = soup_cookie_jar_text_new(g_build_filename(g_get_home_dir(), DEFAULT_COOKIE_FILE, NULL), FALSE);
	soup_session_add_feature(b->session, SOUP_SESSION_FEATURE(b->jar));

	t = tab_new(b, "New Tab");

	/* signals */
	g_signal_connect_swapped(G_OBJECT(b->back_button), "clicked", G_CALLBACK(browser_go_back), b);
	g_signal_connect_swapped(G_OBJECT(b->forward_button), "clicked", G_CALLBACK(browser_go_forward), b);
	g_signal_connect_swapped(G_OBJECT(b->refresh_button), "clicked", G_CALLBACK(browser_reload), b);
	g_signal_connect(G_OBJECT(b->uri_entry), "activate", G_CALLBACK(browser_uri_entry_activated_cb), b);
	g_signal_connect_swapped(G_OBJECT(b->home_button), "clicked", G_CALLBACK(browser_go_home), b);
	g_signal_connect(G_OBJECT(b->notebook), "switch-page", G_CALLBACK(browser_tab_switched_cb), b);
	g_signal_connect_swapped(G_OBJECT(b->window), "destroy", G_CALLBACK(browser_close), b);
	g_signal_connect(G_OBJECT(b->window), "key-press-event", G_CALLBACK(browser_key_press_event_cb), b);

	gtk_widget_show_all(b->window);
	gtk_widget_hide(t->searchbar);

	gtk_widget_grab_focus(b->uri_entry);

	return b;
}
