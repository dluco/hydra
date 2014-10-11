#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdarg.h>
#include <gdk/gdkkeysyms.h>
#include <webkit/webkit.h>
#include <glib/gstdio.h>
#include "args.h"
#include "config.h"
#include "browser.h"
#include "tab.h"
#include "callbacks.h"
#include "utils.h"

gboolean browser_key_press_event(GtkWidget *widget, GdkEventKey *event, Browser *b)
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
			browser_show_uribar(b);
			return TRUE;
		case GDK_KEY_f:
			browser_toggle_widget(b, b->searchbar);
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
			tab_new(b, FALSE);
			gtk_widget_grab_focus(b->bar);
			return TRUE;
		case GDK_KEY_w:
			browser_close_tab(b, t);
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
			tab_load_uri(t, g_strconcat(DEFAULT_SEARCH, gtk_entry_get_text(GTK_ENTRY(b->bar)), NULL));
			return TRUE;
		default:
			return FALSE;
		}
	}

	if (gtk_widget_has_focus(b->bar) && g == GDK_KEY_Escape) {
		gtk_widget_grab_focus(GTK_WIDGET(b->notebook)); 
		return TRUE; 
	}

	if (gtk_widget_has_focus(b->searchbar)) {
		if (g == GDK_KEY_Escape) {
			browser_toggle_widget(b, b->searchbar);
			browser_focus_tab_view(b); 
			return TRUE; 
		}

		if ((g == GDK_KEY_Return) && (event->state & GDK_MOD1_MASK) == GDK_MOD1_MASK) { 
			tab_search_reverse(t, b->searchbar);
		}
	}
	
	return FALSE; 
}

/* link hovering callback */
void browser_link_hover(WebKitWebView *page, const gchar *title, const gchar *link, Browser *b)
{
	gtk_statusbar_push(GTK_STATUSBAR(b->status), 0, (link) ? link : ""); 
}

/* uri-bar callback */
void browser_uribar_activated(GtkWidget *entry, Browser *b)
{
	Tab *t = browser_get_current_tab(b);

	tab_load_uri(t, g_strdup(gtk_entry_get_text(GTK_ENTRY(b->bar))));
	gtk_widget_grab_focus(GTK_WIDGET(t->view));
 
	if (b->hide) {
		gtk_widget_hide(b->bar);
	}
	
	b->hide = FALSE;
}

Tab *browser_get_tab(Browser *b, int tab_num)
{
	return (Tab *)g_object_get_qdata(G_OBJECT(gtk_notebook_get_nth_page(b->notebook, tab_num)), b->term_data_id);
}

Tab *browser_get_current_tab(Browser *b)
{
	return browser_get_tab(b, gtk_notebook_get_current_page(b->notebook));
}

/* get the tab number containing the tab specified */
int browser_get_tab_num(Browser *b, Tab *t)
{
	return gtk_notebook_page_num(b->notebook, t->scroll);
}

int browser_get_current_tab_num(Browser *b)
{
	return browser_get_tab_num(b, browser_get_current_tab(b));
}

/* close tab, and quit if there are no tabs */
void browser_close_tab(Browser *b, Tab *t)
{
	gtk_notebook_remove_page(b->notebook, gtk_notebook_get_current_page(b->notebook));
	g_free(t);

	if (gtk_notebook_get_n_pages(b->notebook) == 1) {
		/* hide notebook tabs when only one tab */
		gtk_notebook_set_show_tabs(b->notebook, FALSE);
		browser_focus_tab_view(b);
	} else if (gtk_notebook_get_n_pages(b->notebook) == 0) { 
		/* exit if no tabs remaining */
		gtk_main_quit(); 
	}
}

/* switch tabs forward or backwards */
void browser_switch_tab(Browser *b, gboolean forward)
{
	int tab_num;
	int current = gtk_notebook_get_current_page(b->notebook);
	int n = gtk_notebook_get_n_pages(b->notebook);

	tab_num = (forward) ? mod(current + 1, n) : mod(current - 1, n);

	gtk_notebook_set_current_page(b->notebook, tab_num);
}

/* toggle visibility of browser widget and grab focus if shown */
void browser_toggle_widget(Browser *b, GtkWidget *widget)
{
	if (!gtk_widget_get_visible(widget)) {
		gtk_widget_show(widget);
		gtk_widget_grab_focus(widget);
	} else {
		gtk_widget_hide(widget);
	}
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
		Tab *t = tab_new(b, FALSE);
		tab_load_uri(t, returned);
		g_free(returned);
	}
}

void browser_back(Browser *b)
{
	tab_back(browser_get_current_tab(b));
}

void browser_forward(Browser *b)
{
	tab_forward(browser_get_current_tab(b));
}

void browser_reload(Browser *b)
{
	tab_reload(browser_get_current_tab(b), FALSE);
}

void browser_home(Browser *b)
{
	tab_home(browser_get_current_tab(b));
}

/* if the bar isn't visible, show it and set the b->hide flag to TRUE*/
void browser_show_uribar(Browser *b)
{
	if (!gtk_widget_get_visible(b->bar)) {
		gtk_widget_show(b->bar);
		b->hide = TRUE;
	}
 
	gtk_widget_grab_focus(GTK_WIDGET(b->bar));
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

/* focus on tab after switching, aka title, statusbar, view, etc */
void browser_tab_switched(GtkNotebook *notebook, GtkWidget *page, guint page_num, Browser *b)
{
	Tab *t = browser_get_tab(b, page_num);
	const char *uri = webkit_web_view_get_uri(t->view);
	const char *title = webkit_web_view_get_title(t->view);

	/* reset browser state for new tab */
	gtk_window_set_title(GTK_WINDOW(b->window), (title) ? title : DEFAULT_BROWSER_TITLE);
	gtk_entry_set_text(GTK_ENTRY(b->bar), (uri) ? uri : "");
	gtk_statusbar_push(GTK_STATUSBAR(b->status), 0, "");

	/* update toolbar buttons */
	gtk_widget_set_sensitive(GTK_WIDGET(b->back_button), webkit_web_view_can_go_back(t->view));
	gtk_widget_set_sensitive(GTK_WIDGET(b->forward_button), webkit_web_view_can_go_forward(t->view));
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
	
	b->term_data_id = g_quark_from_static_string("s");

	b->window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	b->vbox = gtk_vbox_new(FALSE, 0);
	b->bar = gtk_entry_new();
	b->notebook = GTK_NOTEBOOK(gtk_notebook_new());
	b->searchbar = gtk_entry_new();
	b->status = gtk_statusbar_new();

	/* toolbar */
	b->toolbar = gtk_toolbar_new();
	b->back_button = gtk_tool_button_new_from_stock(GTK_STOCK_GO_BACK);
	b->forward_button = gtk_tool_button_new_from_stock(GTK_STOCK_GO_FORWARD);
	b->refresh_button = gtk_tool_button_new_from_stock(GTK_STOCK_REFRESH);
	tool_item = gtk_tool_item_new();
	b->home_button = gtk_tool_button_new_from_stock(GTK_STOCK_HOME);

	gtk_toolbar_insert(GTK_TOOLBAR(b->toolbar), GTK_TOOL_ITEM(b->back_button), -1);
	gtk_toolbar_insert(GTK_TOOLBAR(b->toolbar), GTK_TOOL_ITEM(b->forward_button), -1);
	gtk_toolbar_insert(GTK_TOOLBAR(b->toolbar), GTK_TOOL_ITEM(b->refresh_button), -1);
	gtk_container_add(GTK_CONTAINER(tool_item), b->bar);
	gtk_toolbar_insert(GTK_TOOLBAR(b->toolbar), GTK_TOOL_ITEM(tool_item), -1);
	gtk_toolbar_insert(GTK_TOOLBAR(b->toolbar), GTK_TOOL_ITEM(b->home_button), -1);
	
	gtk_box_pack_start(GTK_BOX(b->vbox), b->toolbar, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(b->vbox), GTK_WIDGET(b->notebook), TRUE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(b->vbox), b->searchbar, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(b->vbox), b->status, FALSE, FALSE, 0);
	gtk_container_add(GTK_CONTAINER(b->window), b->vbox);

	/* basic settings */
	gtk_window_set_wmclass(GTK_WINDOW(b->window), "hydra", "Hydra");
	gtk_window_set_role(GTK_WINDOW(b->window), "Hydra");
	gtk_window_set_default_size(GTK_WINDOW(b->window), DEFAULT_HEIGHT, DEFAULT_WIDTH);
	gtk_entry_set_has_frame(GTK_ENTRY(b->bar), FALSE);
	gtk_orientable_set_orientation(GTK_ORIENTABLE(b->toolbar), GTK_ORIENTATION_HORIZONTAL);
	gtk_tool_item_set_expand(tool_item, TRUE);	// allow uri-bar to expand
	gtk_notebook_set_scrollable(b->notebook, TRUE);
	gtk_notebook_set_show_border(b->notebook, FALSE);
	gtk_widget_set_sensitive(GTK_WIDGET(b->back_button), FALSE);
	gtk_widget_set_sensitive(GTK_WIDGET(b->forward_button), FALSE);

	b->webkit_settings = webkit_web_settings_new();

	/* basic webkit settings */
	g_object_set(G_OBJECT(b->webkit_settings),
		"user_agent", DEFAULT_USER_AGENT, NULL);

	b->session = webkit_get_default_session();
	b->jar = soup_cookie_jar_text_new(g_build_filename(g_get_home_dir(),	DEFAULT_COOKIE_FILE, NULL), FALSE);
	soup_session_add_feature(b->session, SOUP_SESSION_FEATURE(b->jar));

	t = tab_new(b, FALSE);

	/* signals */
	g_signal_connect_swapped(G_OBJECT(b->back_button), "clicked", G_CALLBACK(browser_back), b);
	g_signal_connect_swapped(G_OBJECT(b->forward_button), "clicked", G_CALLBACK(browser_forward), b);
	g_signal_connect_swapped(G_OBJECT(b->refresh_button), "clicked", G_CALLBACK(browser_reload), b);
	g_signal_connect(G_OBJECT(b->bar), "activate", G_CALLBACK(browser_uribar_activated), b);
	g_signal_connect_swapped(G_OBJECT(b->home_button), "clicked", G_CALLBACK(browser_home), b);
	g_signal_connect_swapped(G_OBJECT(b->searchbar), "activate", G_CALLBACK(tab_search_forward), t);
	g_signal_connect(G_OBJECT(b->notebook), "switch-page", G_CALLBACK(browser_tab_switched), b);
	g_signal_connect(G_OBJECT(b->window), "destroy", G_CALLBACK(gtk_main_quit), NULL);
	g_signal_connect(G_OBJECT(b->window), "key-press-event", G_CALLBACK(browser_key_press_event), b);

	gtk_widget_show_all(b->window);
	gtk_widget_hide(b->searchbar);
	gtk_widget_grab_focus(b->bar);

	return b;
}

/* main entry-point */
int main(int argc, char *argv[])
{
	Args *args;
	Browser *b;

	/* initialize GTK */
	gtk_init(&argc, &argv);

	args = args_parse(argc, argv);

	b = browser_new();

	if (args->fullscreen) {
		gtk_window_fullscreen(GTK_WINDOW(b->window));
	}

	if (argc == 2) {
		tab_load_uri(browser_get_current_tab(b), argv[1]);
	}

	gtk_main();

	return 0;
}
