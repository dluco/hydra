#include "tab.h"
#include "config.h"

/* create new Tab */
void tab_new(gboolean b)
{
	gchar *stylesheet;
	Tab *t;
	t = g_new0(Tab, 1);

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

	/* load user stylesheet */
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

/* close tab, and quit if there are no tabs */
void tab_close()
{
	Tab *t = get_tab(NULL, gtk_notebook_get_current_page(w.notebook));

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

void tab_forward(void)
{
	Tab *t = get_tab(NULL, gtk_notebook_get_current_page(w.notebook));

	webkit_web_view_go_forward(t->view); 
}

void tab_back(void)
{
	Tab *t = get_tab(NULL, gtk_notebook_get_current_page(w.notebook));

	webkit_web_view_go_back(t->view);
}

/* basic reload function */
void tab_reload(gboolean bypass)
{
	Tab *t = get_tab(NULL, gtk_notebook_get_current_page(w.notebook));

	if (bypass) {
		webkit_web_view_reload(t->view);	
	} else {
		webkit_web_view_reload_bypass_cache(t->view);
	}
}

/* increase or decrease the zoom of the page */
void tab_zoom(gboolean in)
{
	Tab *t = get_tab(NULL, gtk_notebook_get_current_page(w.notebook));

	if (in) {
		webkit_web_view_set_zoom_level(t->view, (webkit_web_view_get_zoom_level(t->view) + ZOOM_INCREMENT)); 
	} else {
		webkit_web_view_set_zoom_level(t->view, (webkit_web_view_get_zoom_level(t->view) - ZOOM_INCREMENT)); 
	}
}

/* reset the zoom of the page */
void tab_zoom(gboolean in)
{
	Tab *t = get_tab(NULL, gtk_notebook_get_current_page(w.notebook));

	webkit_web_view_set_zoom_level(t->view, DEFAULT_ZOOM_LEVEL); 
}

/* when a new tab is requested, return the t->view */
WebKitWebView * tab_new_requested(WebKitWebView *v, WebKitWebFrame *f)
{
	tab_new(FALSE); 
	Tab *t = get_tab(NULL, gtk_notebook_get_current_page(w.notebook));
	
	return t->view;
}

/* switch to view source mode - stays in mode until reverted */
void tab_view_source()
{
	Tab *t = get_tab(NULL, gtk_notebook_get_current_page(w.notebook));

	webkit_web_view_set_view_source_mode(t->view, !webkit_web_view_get_view_source_mode(t->view)); 
	webkit_web_view_reload(t->view);
}
