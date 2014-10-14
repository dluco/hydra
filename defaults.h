#define DEFAULT_DOWNLOAD_COMMAND "xterm -bg black -fg white -e wget -P %s -o %s %s"
#define DEFAULT_DOWNLOAD_LOCATION g_get_home_dir()
#define DEFAULT_SEARCH "http://www.google.com/search?q="
#define DEFAULT_FONT "san-serif"
#define DEFAULT_TAB_LENGTH 25
#define DEFAULT_HISTORY_FILE ".sb_history"
#define DEFAULT_COOKIE_FILE ".sb_cookies"
#define DEFAULT_STYLE_SHEET ".sb_stylesheet"
#define DEFAULT_ZOOM_INCREMENT .05
#define DEFAULT_ZOOM_LEVEL 1.0
#define DEFAULT_HEIGHT 900
#define DEFAULT_WIDTH 800

GQuark term_data_id;
#define get_tab(x, page_idx ) (struct tab*)g_object_get_qdata(G_OBJECT(gtk_notebook_get_nth_page(w.notebook, page_idx ) ), term_data_id);

typedef struct tab { 
	GtkWidget *scroll; 
	GtkWidget *label; 
	gchar *main_title; 
	gchar *url_entry;
	gint load_progress; 
	guint status_context_id;
	WebKitWebView *view; 
} tab;

struct {	
	WebKitWebSettings *webkitsettings;
	WebKitWebWindowFeatures *webkitwindowfeatures;
	SoupSession *session;
	SoupCookieJar *jar;
	gboolean hide;
	GtkWidget *win;
	GtkWidget *bar;	
	GtkWidget *searchbar; 
	GtkWidget *vbox;
	GtkWidget *hbox;
	GtkNotebook *notebook; 
	GtkWidget *status;
	GtkWidget *status_info;
} w;
