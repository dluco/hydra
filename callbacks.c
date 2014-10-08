#include <gdk/gdkkeysyms.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <webkit/webkit.h>
#include <glib/gstdio.h>
#include "defaults.h"
#include "callbacks.h"

#include "hydra.h"

/* key press callback function. NEEDED: a config.h where keys are defined */
gboolean cb_keypress(GtkWidget *widget, GdkEventKey *event)
{
	guint(g) = event->keyval;

	if ((event->state & GDK_CONTROL_MASK) == GDK_CONTROL_MASK ) {
		switch(g) {
		case GDK_KEY_l:
			grab_bar();
			return TRUE;
		case GDK_KEY_f:
			show_search(TRUE);
			return TRUE;
		case GDK_KEY_g:
			tab_and_go();
			return TRUE;
		case GDK_KEY_Back:
			cb_go(FALSE);
			return TRUE;
		case GDK_KEY_Forward:
			cb_go(TRUE);
			return TRUE;
		case GDK_KEY_comma:
			cb_go(FALSE);
			return TRUE;
		case GDK_KEY_period:
			cb_go(TRUE);
			return TRUE;
		case GDK_KEY_o:
			history_command();
			return TRUE;
		case GDK_KEY_h:
			toggle();
			focus_view();
			return TRUE;
		case GDK_KEY_Page_Up:
			tab_switch(FALSE);
			return TRUE;
		case GDK_KEY_Page_Down:
			tab_switch(TRUE);
			return TRUE;
		case GDK_KEY_t:
			tab_new(FALSE);
			gtk_widget_grab_focus(w.bar);
			return TRUE;
		case GDK_KEY_w:
			tab_close();
			return TRUE;
		case GDK_KEY_bracketright:
			tab_zoom(TRUE);
			return TRUE;
		case GDK_KEY_bracketleft:
			tab_zoom(FALSE);
			return TRUE;
		case GDK_KEY_r:
			tab_reload(TRUE);
			return TRUE;
		case GDK_KEY_e:
			tab_reload(FALSE);
			return TRUE;
		case GDK_KEY_s:
			tab_view_source();
			return TRUE;
		case GDK_KEY_Return:
			load_uri(g_strconcat(DEFAULT_SEARCH, gtk_entry_get_text(GTK_ENTRY(w.bar)), NULL));
			return TRUE;
		default:
			return FALSE;
		}
	}

	if (gtk_widget_has_focus(w.bar) && g == GDK_KEY_Escape) {
		gtk_widget_grab_focus(GTK_WIDGET(w.notebook)); 
		return TRUE; 
	}

	if (gtk_widget_has_focus(w.searchbar)) {
		if (g == GDK_KEY_Escape) {
			show_search(FALSE); 
			focus_view(); 
			return TRUE; 
		}

		if ((g == GDK_KEY_Return) && (event->state & GDK_MOD1_MASK) == GDK_MOD1_MASK) { 
			search(NULL, FALSE); 
		}
	}
	
	return FALSE; 
}

/* download callback */
void cb_download(WebKitWebView *web_view, GObject *d, gpointer user_data)
{
	gchar *command;

	const gchar *download_url = webkit_download_get_uri(WEBKIT_DOWNLOAD(d));
	const gchar *requested_name = webkit_download_get_suggested_filename(WEBKIT_DOWNLOAD(d));

	command = g_new0(gchar, strlen(DEFAULT_DOWNLOAD_COMMAND) + strlen(DEFAULT_DOWNLOAD_LOCATION) + strlen(requested_name) + strlen(download_url) + 1);
	g_sprintf(command, DEFAULT_DOWNLOAD_COMMAND, DEFAULT_DOWNLOAD_LOCATION, requested_name, download_url);
	g_spawn_command_line_async(command, NULL);

	g_free(command);
}

/* entry callback */
void cb_entry(GtkWidget* entry, gpointer data)
{
	struct tab *t = get_tab(NULL, gtk_notebook_get_current_page(w.notebook));

	load_uri(g_strdup(gtk_entry_get_text(GTK_ENTRY(w.bar))));
	gtk_widget_grab_focus(GTK_WIDGET(t->view));
 
	if (w.hide) {
		gtk_widget_hide(w.bar);
	}
	
	w.hide = FALSE;
}

/* link hovering callback */
void cb_link_hover(WebKitWebView* page, const gchar* title, const gchar* link, gpointer data)
{
	if (link != NULL) { 
		gtk_statusbar_push(GTK_STATUSBAR(w.status), 0, link);	
	} else { 
		gtk_statusbar_push(GTK_STATUSBAR(w.status), 0, ""); 
	}
}

/* go forward or backwards, simple enough */
void cb_go(gboolean b)
{
	struct tab *t = get_tab(NULL, gtk_notebook_get_current_page(w.notebook));

	if (b) {
		webkit_web_view_go_forward(t->view); 
	} else {
		webkit_web_view_go_back(t->view);
	}
}


/* when the page load is commited, call this function */
void cb_load_status(GObject* object, GParamSpec* pspec, tab *t)
{
	const gchar* uri = webkit_web_view_get_uri(t->view);
	WebKitLoadStatus status = webkit_web_view_get_load_status(t->view);
	
	switch(status) {
	case WEBKIT_LOAD_PROVISIONAL:
		break;
	case WEBKIT_LOAD_COMMITTED:
		if (gtk_notebook_get_current_page(w.notebook) ==	gtk_notebook_page_num(w.notebook, t->scroll)) {
			gtk_entry_set_text(GTK_ENTRY(w.bar), uri); 
		}
		
		FILE *history	= fopen(g_build_filename(g_get_home_dir(), DEFAULT_HISTORY_FILE, NULL), "a+");
		
		if (history) {
			fprintf(history, "%s \n", uri);
			fclose(history);
		}

		break;
	case WEBKIT_LOAD_FIRST_VISUALLY_NON_EMPTY_LAYOUT:
		break;
	case WEBKIT_LOAD_FINISHED:
		break;
	case WEBKIT_LOAD_FAILED:
		break;
	default:
		break;
	}
}

/* title change callback */
void cb_title_changed(WebKitWebView *v, WebKitWebFrame *f, const char *title, tab *t)
{
	gchar *tabtitle;

	if (gtk_notebook_get_current_page(w.notebook) ==	gtk_notebook_page_num(w.notebook, t->scroll)) {
		gtk_window_set_title(GTK_WINDOW(w.win), g_strconcat(title, NULL));
	}

	if (strlen(title) <	DEFAULT_TAB_LENGTH ) {
		tabtitle = g_strdup(title);
	} else {
		tabtitle = g_strndup(title, DEFAULT_TAB_LENGTH);
		 strcat(tabtitle, "...");
	}

	gtk_label_set_label(GTK_LABEL(t->label), tabtitle);
}
