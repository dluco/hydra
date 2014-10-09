#ifndef CONFIG_H
#define CONFIG_H

#define DOWNLOAD_LOCATION g_get_home_dir()
#define DOWNLOAD_COMMAND "xterm -bg black -fg white -e wget -P %s -o %s %s"
#define DEFAULT_SEARCH "https://www.google.com/search?q="
#define DEFAULT_HOME "https://www.google.ca/"
#define DEFAULT_FONT "san-serif"
#define DEFAULT_TAB_LENGTH 25
#define DEFAULT_HISTORY_FILE ".hydra_history"
#define DEFAULT_COOKIE_FILE ".hydra_cookies"
#define DEFAULT_STYLE_SHEET ".hydra_stylesheet"
#define DEFAULT_ZOOM_LEVEL 1.0
#define ZOOM_INCREMENT .05
#define DEFAULT_HEIGHT 900
#define DEFAULT_WIDTH 800

#endif /* CONFIG_H */
