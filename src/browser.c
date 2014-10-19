#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdarg.h>
#include <gdk/gdkkeysyms.h>
#include <webkit/webkit.h>
#include <glib/gstdio.h>
#include "hydra.h"
#include "config.h"
#include "browser.h"
#include "tab.h"
#include "utils.h"

static gboolean browser_key_press_event_cb(GtkWidget *widget, GdkEventKey *event, Browser *b);
static void browser_tab_switched_cb(GtkNotebook *notebook, GtkWidget *page, guint page_num, Browser *b);
static void browser_new_tab_cb(GtkWidget *widget, Browser *b);
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
		case GDK_KEY_f:			/* Ctrl+f : open search bar */
			tab_show_search_entry(t);
			return TRUE;
		case GDK_KEY_g:			/* Ctrl+g : open page from history in new tab */
			tab_and_go(b);
			return TRUE;
		case GDK_KEY_Left:		/* Ctrl+Left : go back */
			tab_back(t);
			return TRUE;
		case GDK_KEY_Right:		/* Ctrl+Right : go forward */
			tab_forward(t);
			return TRUE;
		case GDK_KEY_h:			/* Ctrl+h : search history */
			browser_history(b);
			return TRUE;
		case GDK_KEY_Page_Up:	/* Ctrl+PgDn : switch to next tab */
			browser_switch_tab(b, FALSE);
			return TRUE;
		case GDK_KEY_Page_Down:	/* Ctrl+PgUp : switch to previous tab */
			browser_switch_tab(b, TRUE);
			return TRUE;
		case GDK_KEY_t:			/* Ctrl+t : new tab */
			browser_new_tab(b);
			return TRUE;
		case GDK_KEY_w:			/* Ctrl+w : close tab */
			browser_close_tab(b, t);
			return TRUE;
		case GDK_KEY_n:			/* Ctrl+t : new browser */
			browser_new();
			return TRUE;
		case GDK_KEY_q:			/* Ctrl+q : close browser */
			browser_close(b);
			return TRUE;
		case GDK_KEY_plus:		/* Ctrl++ : zoom in */
			tab_zoom(t, TRUE);
			return TRUE;
		case GDK_KEY_minus:		/* Ctrl+- : zoom out */
			tab_zoom(t, FALSE);
			return TRUE;
		case GDK_KEY_0:			/* Ctrl+0 : reset zoom */
			tab_zoom_reset(t);
			return TRUE;
		case GDK_KEY_r:			/* Ctrl+r : reload page */
			tab_reload(t, FALSE);
			return TRUE;
		case GDK_KEY_e: 		/* Ctrl+e : reload page (bypass cache) */
			tab_reload(t, TRUE);
			return TRUE;
		case GDK_KEY_u:			/* Ctrl+u : view page source */
			tab_view_source(t);
			return TRUE;
		case GDK_KEY_Return:
			tab_load_uri(t, g_strconcat(DEFAULT_SEARCH, gtk_entry_get_text(GTK_ENTRY(b->uri_entry)), NULL));
			return TRUE;
		default:
			return FALSE;
		}
	}

	/* Esc : change focus to webview, hide searchbar if currently focused, or stop loading page */
	if (g == GDK_KEY_Escape) {
		if (gtk_widget_has_focus(t->search_entry)) {
			gtk_widget_hide(t->searchbar);
		} else if (gtk_widget_has_focus(GTK_WIDGET(t->view))) {
			tab_stop_loading(t);
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
	gtk_widget_set_visible(GTK_WIDGET(b->refresh_button), (t->progress == 1.0 || t->progress == 0.0));
	gtk_widget_set_visible(GTK_WIDGET(b->stop_button), (t->progress < 1.0 && t->progress > 0.0));
}

static void browser_new_tab_cb(GtkWidget *widget, Browser *b)
{
	/* create new tab */
	browser_new_tab(b);
}

/* uri-bar callback */
static void browser_uri_entry_activated_cb(GtkWidget *entry, Browser *b)
{
	Tab *t = browser_get_current_tab(b);

	tab_load_uri(t, g_strdup(gtk_entry_get_text(GTK_ENTRY(b->uri_entry))));
}

/* link hovering callback */
void browser_link_hover_cb(WebKitWebView *page, const char *title, const char *link, Browser *b)
{
	gtk_statusbar_pop(GTK_STATUSBAR(b->statusbar), b->status_context_id); 
	if (link) {
		gtk_statusbar_push(GTK_STATUSBAR(b->statusbar), b->status_context_id, link); 
	}
}

Tab *browser_get_tab(Browser *b, int tab_num)
{
	/* get child widget from notebook at tab_num */
	GtkWidget *widget = gtk_notebook_get_nth_page(GTK_NOTEBOOK(b->notebook), tab_num);
	/* search b->tabs for parent tab struct of widget */
	GList *list;
	for (list = b->tabs; list != NULL; list = list->next) {
		if (TAB(list->data)->vbox == widget) {
			return TAB(list->data);
		}
	}
	/* error */
	return NULL;
}

Tab *browser_get_current_tab(Browser *b)
{
	return browser_get_tab(b, gtk_notebook_get_current_page(GTK_NOTEBOOK(b->notebook)));
}

/* get the tab number containing the tab specified */
int browser_get_tab_num(Browser *b, Tab *t)
{
	return gtk_notebook_page_num(GTK_NOTEBOOK(b->notebook), t->vbox);
}

int browser_get_current_tab_num(Browser *b)
{
	return browser_get_tab_num(b, browser_get_current_tab(b));
}

void browser_new_tab(Browser *b)
{
	/* create new tab, change notebook page, and focus uri-bar */
	Tab *t = tab_new(b, "New Tab");
	gtk_notebook_set_current_page(GTK_NOTEBOOK(b->notebook), browser_get_tab_num(b, t));
	gtk_widget_grab_focus(b->uri_entry);
}

void browser_close_tab(Browser *b, Tab *t)
{
	/* close tab */
	tab_close(t);

	/* exit if no tabs remaining */
	if (g_list_length(b->tabs) == 0) {
		browser_close(b);
	} else {
		browser_focus_tab_view(b);
	}
}

void browser_close(Browser *b)
{
	/* close and free all tabs */
	int i;
	int n = g_list_length(b->tabs);

	for (i = 0; i < n; i++) {
		tab_close(g_list_nth_data(b->tabs, 0));
	}

	/* free tabs list */
	g_list_free(b->tabs);

	/* remove browser from browser list */
	hydra->browsers = g_list_remove(hydra->browsers, b);

	/* block signal handler for b->window:destroy */
	g_signal_handlers_block_by_func(G_OBJECT(b->window), G_CALLBACK(browser_close), b);
	/* free browser widgets elements */
	gtk_widget_destroy(b->window);
	g_free(b);

	if (g_list_length(hydra->browsers) == 0) {
		/* no more windows open - kill program */
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
	char *returned;

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

void browser_stop(Browser *b)
{
	tab_stop_loading(browser_get_current_tab(b));
}

void browser_go_home(Browser *b)
{
	tab_home(browser_get_current_tab(b));
}

/* call the history command. should we do it ASYNC?*/
void browser_history(Browser *b)
{
	char *file; 
	char *command;
	char *returned;

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
	GtkWidget *align;
	GtkRcStyle *rcstyle;
	
	b = g_new0(Browser, 1);
	
	b->window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	b->vbox = gtk_vbox_new(FALSE, 0);
	b->notebook = gtk_notebook_new();

	/* toolbar */
	b->toolbar = gtk_toolbar_new();
	gtk_orientable_set_orientation(GTK_ORIENTABLE(b->toolbar), GTK_ORIENTATION_HORIZONTAL);
	b->back_button = gtk_tool_button_new_from_stock(GTK_STOCK_GO_BACK);
	b->forward_button = gtk_tool_button_new_from_stock(GTK_STOCK_GO_FORWARD);
	b->refresh_button = gtk_tool_button_new_from_stock(GTK_STOCK_REFRESH);
	b->stop_button = gtk_tool_button_new_from_stock(GTK_STOCK_STOP);
	b->uri_entry = gtk_entry_new();
	tool_item = gtk_tool_item_new();
	gtk_tool_item_set_expand(tool_item, TRUE);	// allow uri-bar to expand
	b->home_button = gtk_tool_button_new_from_stock(GTK_STOCK_HOME);

	gtk_toolbar_insert(GTK_TOOLBAR(b->toolbar), GTK_TOOL_ITEM(b->back_button), -1);
	gtk_toolbar_insert(GTK_TOOLBAR(b->toolbar), GTK_TOOL_ITEM(b->forward_button), -1);
	gtk_toolbar_insert(GTK_TOOLBAR(b->toolbar), GTK_TOOL_ITEM(b->refresh_button), -1);
	gtk_toolbar_insert(GTK_TOOLBAR(b->toolbar), GTK_TOOL_ITEM(b->stop_button), -1);
	gtk_container_add(GTK_CONTAINER(tool_item), b->uri_entry);
	gtk_toolbar_insert(GTK_TOOLBAR(b->toolbar), GTK_TOOL_ITEM(tool_item), -1);
	gtk_toolbar_insert(GTK_TOOLBAR(b->toolbar), GTK_TOOL_ITEM(b->home_button), -1);

	/* notebook "new tab" button */
	b->new_tab_button = gtk_button_new();
	gtk_button_set_relief(GTK_BUTTON(b->new_tab_button), GTK_RELIEF_NONE);
	gtk_button_set_focus_on_click(GTK_BUTTON(b->new_tab_button), FALSE);
	gtk_container_add(GTK_CONTAINER(b->new_tab_button),
		gtk_image_new_from_stock(GTK_STOCK_ADD, GTK_ICON_SIZE_MENU));
	/* pack button in alignment widget and remove padding */
	align = gtk_alignment_new(0.5, 0.5, 0.0, 0.0);
	gtk_alignment_set_padding(GTK_ALIGNMENT(align), 0, 0, 0, 0);
	gtk_container_add(GTK_CONTAINER(align), b->new_tab_button);
	/* pack and show button */
	gtk_notebook_set_action_widget(GTK_NOTEBOOK(b->notebook), align, GTK_PACK_START);
	gtk_widget_show_all(align);

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
	rcstyle = gtk_rc_style_new();
	rcstyle->xthickness = rcstyle->ythickness = 1;
	gtk_widget_modify_style(b->notebook, rcstyle);

	gtk_widget_set_sensitive(GTK_WIDGET(b->back_button), FALSE);
	gtk_widget_set_sensitive(GTK_WIDGET(b->forward_button), FALSE);

	b->webkit_settings = webkit_web_settings_new();

	/* basic webkit settings */
	g_object_set(G_OBJECT(b->webkit_settings),
		"user_agent", DEFAULT_USER_AGENT,
		"enable-plugins", TRUE, NULL);

	b->status_context_id = gtk_statusbar_get_context_id(GTK_STATUSBAR(b->statusbar), "link-hover");

	b->soup_session = webkit_get_default_session();
	b->cookie_jar = soup_cookie_jar_text_new(g_build_filename(g_get_home_dir(), DEFAULT_COOKIE_FILE, NULL), FALSE);
	soup_session_add_feature(b->soup_session, SOUP_SESSION_FEATURE(b->cookie_jar));

	/* create first tab and add to tabs list */
	t = tab_new(b, "New Tab");

	/* signals */
	g_signal_connect_swapped(G_OBJECT(b->back_button), "clicked", G_CALLBACK(browser_go_back), b);
	g_signal_connect_swapped(G_OBJECT(b->forward_button), "clicked", G_CALLBACK(browser_go_forward), b);
	g_signal_connect_swapped(G_OBJECT(b->refresh_button), "clicked", G_CALLBACK(browser_reload), b);
	g_signal_connect_swapped(G_OBJECT(b->stop_button), "clicked", G_CALLBACK(browser_stop), b);
	g_signal_connect(G_OBJECT(b->uri_entry), "activate", G_CALLBACK(browser_uri_entry_activated_cb), b);
	g_signal_connect_swapped(G_OBJECT(b->home_button), "clicked", G_CALLBACK(browser_go_home), b);
	g_signal_connect(G_OBJECT(b->notebook), "switch-page", G_CALLBACK(browser_tab_switched_cb), b);
	g_signal_connect(G_OBJECT(b->new_tab_button), "clicked", G_CALLBACK(browser_new_tab_cb), b);
	g_signal_connect_swapped(G_OBJECT(b->window), "destroy", G_CALLBACK(browser_close), b);
	g_signal_connect(G_OBJECT(b->window), "key-press-event", G_CALLBACK(browser_key_press_event_cb), b);

	gtk_widget_show_all(b->window);
	gtk_widget_hide(t->searchbar);

	gtk_widget_grab_focus(b->uri_entry);

	return b;
}
