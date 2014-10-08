#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdarg.h>
#include <gdk/gdkkeysyms.h>
#include <webkit/webkit.h>
#include <glib/gstdio.h>
#include "args.h"
#include "defaults.h"
#include "callbacks.h"
#include "hydra.h"

gchar *tab_get_tab_postition()
{
	gchar *page_info = NULL;

	sprintf(page_info, "[ %d / %d ]", gtk_notebook_get_current_page(w.notebook), gtk_notebook_get_n_pages(w.notebook));
	puts(page_info);

	return page_info;
}

void search(GtkEntry *entry, gboolean b) {

	struct tab *t = get_tab(NULL, gtk_notebook_get_current_page(w.notebook));

	webkit_web_view_search_text(t->view, gtk_entry_get_text(GTK_ENTRY(w.searchbar)), FALSE, b, TRUE);
}

void show_search(gboolean b)
{
	if (b) {
		gtk_widget_show(w.searchbar);
		gtk_widget_grab_focus(w.searchbar);
	} else {
		gtk_widget_hide(w.searchbar);
	}
}

/* Basic reload function */
void tab_reload(gboolean b)
{
	struct tab *t = get_tab(NULL, gtk_notebook_get_current_page(w.notebook));

	if (b) {
		webkit_web_view_reload(t->view);	
	} else {
		webkit_web_view_reload_bypass_cache(t->view);
	}
}

/* close tab, and quit if there are no tabs */
void tab_close()
{
	struct tab *t = get_tab(NULL, gtk_notebook_get_current_page(w.notebook));

	gtk_notebook_remove_page(w.notebook, gtk_notebook_get_current_page(w.notebook));
	g_free(t);

	if (gtk_notebook_get_n_pages(w.notebook) == 1) {
		gtk_notebook_set_show_tabs(w.notebook, FALSE);
		gtk_widget_grab_focus(gtk_notebook_get_nth_page(w.notebook, gtk_notebook_get_current_page(w.notebook)));
		focus_view();
	}

	if (gtk_notebook_get_n_pages(w.notebook) == 0) { 
		gtk_main_quit(); 
	}
}

/* an alternative to the regular tab command, combines tabbing and history command into one */
void tab_and_go()
{
	gchar *returned;

	g_spawn_command_line_sync(g_strconcat("sh -c 'sort ", g_build_filename(g_get_home_dir(), DEFAULT_HISTORY_FILE, NULL),	
																				" | dmenu -l 15'", NULL), &returned, NULL, NULL, NULL);	

	if (strcmp(returned, "") == 0) {
		focus_view(); 
	} else { 
		tab_new(FALSE);
		load_uri(returned);
		g_free(returned);
	}
}

/* loads the uri, check for the protocol sign */
void load_uri(gchar *uri)
{
	gchar *u;
	struct tab *t = get_tab(NULL, gtk_notebook_get_current_page(w.notebook));

	/*Borrowed from surf, no point creating another method, this seems to work well
	 * these few lines of code were distributed under the MIT/X license by the 
	 * suckless project. */

	u = g_strrstr(uri, "://") ? g_strdup(uri) : g_strdup_printf("http://%s", uri);
	webkit_web_view_load_uri(t->view, u);

	g_free(u);
}

/*increase or decrease the zoom of the page */
void tab_zoom(gboolean b)
{
	struct tab *t = get_tab(NULL, gtk_notebook_get_current_page(w.notebook));

	if (b) {
		webkit_web_view_set_zoom_level(t->view,(webkit_web_view_get_zoom_level(t->view) +	DEFAULT_ZOOM_INCREMENT)); 
	} else {
		webkit_web_view_set_zoom_level(t->view,(webkit_web_view_get_zoom_level(t->view) -	DEFAULT_ZOOM_INCREMENT)); 
	}
}

/* if the bar isn't visible, show it and set the w.hide flag to TRUE*/
void grab_bar( )
{
	if (!gtk_widget_get_visible(w.bar)) {
		gtk_widget_show(w.bar);
		w.hide = TRUE;
	}
 
	gtk_widget_grab_focus(GTK_WIDGET(w.bar));
}

/* toggle visibility */
void toggle()
{
	if (gtk_widget_get_visible(w.bar)) {
		gtk_widget_hide(w.bar);
		gtk_widget_hide(w.status);
		gtk_notebook_set_show_tabs(w.notebook, FALSE);
	} else {
		gtk_widget_show_all(w.vbox);
		gtk_widget_hide(w.searchbar);
		
		if (!gtk_notebook_get_n_pages(w.notebook) == 1)	{
			gtk_notebook_set_show_tabs(w.notebook, TRUE); 
		}
	}
}

/* rotate tabs forward or backwards */
void tab_switch(gboolean b)
{
	gint(current) = gtk_notebook_get_current_page(w.notebook);

	if (b) {
		if (current == gtk_notebook_get_n_pages(w.notebook) -1 ) {
			current = 0; 
		} else { 
			current	= current + 1;
		}
	} else {
		if (current == 0) {
			current = gtk_notebook_get_n_pages(w.notebook) - 1;
		} else { 
			current = current -1; 
		}
	}

	gtk_notebook_set_current_page(w.notebook, current);
}

/* when a new tab is requested, return the t->view */
WebKitWebView *tab_new_requested(WebKitWebView *v, WebKitWebFrame *f)
{
	tab_new(FALSE); 
	struct tab *t = get_tab(NULL, gtk_notebook_get_current_page(w.notebook));
	
	return t->view;
}

/* switch to view source mode - stays in mode until reverted */
void tab_view_source()
{
	struct tab *t = get_tab(NULL, gtk_notebook_get_current_page(w.notebook));

	if (webkit_web_view_get_view_source_mode(t->view)) { 
		webkit_web_view_set_view_source_mode(t->view, FALSE); 
	} else { 
		webkit_web_view_set_view_source_mode(t->view, TRUE);
	}

	webkit_web_view_reload(t->view);
}

/* create a tab */
void tab_new(gboolean b)
{
	gchar *stylesheet;
	tab *t;
	t = g_new0(tab, 1);

	t->scroll = gtk_scrolled_window_new(NULL, NULL);
	t->view = WEBKIT_WEB_VIEW(webkit_web_view_new());
	t->label = gtk_label_new("new tab");


	/*reset status bar*/
	gtk_container_add(GTK_CONTAINER(t->scroll), GTK_WIDGET(t->view));

	int index = gtk_notebook_append_page(w.notebook, t->scroll, t->label);
	gtk_notebook_set_tab_reorderable(w.notebook, t->scroll, TRUE);

	if (index == 0) {
		gtk_notebook_set_show_tabs(w.notebook, FALSE);
	} else {
		gtk_notebook_set_show_tabs(w.notebook, TRUE); 
	}

	/*callbacks*/
	g_object_connect(G_OBJECT(t->view), "signal::title-changed", G_CALLBACK(cb_title_changed), t,
										"signal::notify::load-status", G_CALLBACK(cb_load_status), t,
										"signal::hovering-over-link", G_CALLBACK(cb_link_hover), t->view,
										"signal::download-requested", G_CALLBACK(cb_download), t->view,
										"signal::create-web-view", G_CALLBACK(tab_new_requested), NULL, NULL);
	
	/*settings*/
	webkit_web_view_set_highlight_text_matches(t->view, TRUE);
	webkit_web_view_set_zoom_level(t->view, DEFAULT_ZOOM_LEVEL);
	
	/* this doesn't actually work - because webkit draws them not GTK or something.. */
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(t->scroll), GTK_POLICY_NEVER, GTK_POLICY_NEVER);

	stylesheet = g_strconcat("file://", g_get_home_dir(), "/", DEFAULT_STYLE_SHEET, NULL);

	g_object_set(G_OBJECT(w.webkitsettings),
		"enable-page-cache", TRUE,
		"enable-java-applet", FALSE,
		"user-stylesheet-uri", stylesheet, NULL);					 

	g_free(stylesheet);

	/* setup widgets, automatically focus the addressbar */

	gtk_statusbar_push(GTK_STATUSBAR(w.status), 0, "");
	g_object_set_qdata_full(G_OBJECT(gtk_notebook_get_nth_page((GtkNotebook*)w.notebook, index)), term_data_id, t, NULL);

	gtk_widget_show_all(w.win);
	gtk_widget_hide(w.searchbar);
	
	if (!b) {
		gtk_notebook_set_current_page(w.notebook, index); 
	}

	gtk_widget_grab_focus(w.bar);
}


/* call the history command. should we do it ASYNC?*/
void history_command()
{
	gchar *returned;
	gchar *file; 
	gchar *command;

	file =	g_build_filename(g_get_home_dir(), DEFAULT_HISTORY_FILE, NULL);
	command = g_strconcat("sh -c 'sort ", file, " | dmenu -l 15'", NULL);

	g_spawn_command_line_sync(command, &returned, NULL, NULL, NULL);	
	
	if (strcmp(returned, "") == 0) {
		focus_view(); 
	} else {
		load_uri(returned); 
	}

	g_free(returned);
	g_free(file);
	g_free(command);
}

/*focus on tab after switching, aka title, statusbar, view, etc */
void tab_focus(GtkNotebook *notebook, GtkWidget *page, guint page_num, gpointer user_data)
{
	struct tab *t = get_tab(NULL, page_num);
	const char *url = webkit_web_view_get_uri(t->view);
	const char *title = webkit_web_view_get_title(t->view);

	gtk_statusbar_push(GTK_STATUSBAR(w.status), 0, "");

	if (title == NULL && url == NULL) {
		title = "Hydra";
		url = "";
	}

	gtk_window_set_title(GTK_WINDOW(w.win), title);
	gtk_entry_set_text(GTK_ENTRY(w.bar), url);
}


/* focus on	webview */
void focus_view()
{
	struct tab *t = get_tab(NULL, gtk_notebook_get_current_page(w.notebook));
	gtk_widget_grab_focus(GTK_WIDGET(t->view));	
}


/* misc functions to help initialization */
void window_setup()
{
	term_data_id = g_quark_from_static_string("s");

	w.vbox = gtk_vbox_new(FALSE, 0);
	w.hbox = gtk_hbox_new(FALSE, 0);

	w.notebook = GTK_NOTEBOOK(gtk_notebook_new());
	w.bar = gtk_entry_new();
	w.searchbar = gtk_entry_new();
	w.status = gtk_statusbar_new();

	gtk_entry_set_has_frame(GTK_ENTRY(w.bar), FALSE);
	gtk_notebook_set_scrollable(w.notebook, TRUE);
	gtk_notebook_set_show_border(w.notebook, FALSE);

	gtk_box_pack_start(GTK_BOX(w.vbox), w.bar, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(w.vbox), GTK_WIDGET(w.notebook), TRUE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(w.vbox), w.searchbar, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(w.vbox), w.status, FALSE, FALSE, 0);


	w.win = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_default_size(GTK_WINDOW(w.win), DEFAULT_HEIGHT, DEFAULT_WIDTH);
	
	w.webkitsettings = webkit_web_settings_new();
	w.webkitwindowfeatures = webkit_web_window_features_new();


	w.session =	webkit_get_default_session();
	w.jar = soup_cookie_jar_text_new(g_build_filename(g_get_home_dir(),	DEFAULT_COOKIE_FILE, NULL), FALSE);
	soup_session_add_feature(w.session, SOUP_SESSION_FEATURE(w.jar));

	tab_new(FALSE);

	g_signal_connect(G_OBJECT(w.searchbar), "activate", G_CALLBACK(search), GINT_TO_POINTER(1));
	g_signal_connect(G_OBJECT(w.bar), "activate", G_CALLBACK(cb_entry), NULL);
	g_signal_connect(G_OBJECT(w.notebook), "switch-page", G_CALLBACK(tab_focus), NULL);
	g_signal_connect(G_OBJECT(w.win), "destroy", G_CALLBACK(gtk_main_quit), NULL);
	g_signal_connect(G_OBJECT(w.win), "key-press-event", G_CALLBACK(cb_keypress), NULL);

	gtk_container_add(GTK_CONTAINER(w.win), w.vbox);
	gtk_widget_show_all(w.win);
	gtk_widget_hide(w.searchbar);
	gtk_widget_grab_focus(w.bar);
}


/* main entry-point */
int main(int argc, char *argv[])
{
	Args *args;

	gtk_init(&argc, &argv);

	args = args_parse(argc, argv);

	window_setup();

	if (argc == 2) {
		load_uri(argv[1]);
	}

	gtk_main();

	return 0;
}
