#ifndef _TAB_H_
#define _TAB_H_

Tab *tab_new(Browser *b, char *title);
void tab_close(Tab *t);
void tab_load_uri(Tab *t, gchar *uri);
void tab_forward(Tab *t);
void tab_back(Tab *t);
void tab_reload(Tab *t, gboolean bypass);
void tab_stop_loading(Tab *t);
void tab_home(Tab *t);
void tab_zoom(Tab *t, gboolean in);
void tab_zoom_reset(Tab *t);
void tab_update_title(Tab *t);
void tab_update_search_highlight(Tab *t);
void tab_view_source(Tab *t);
void tab_show_search_entry(Tab *t);
void tab_search_forward(Tab *t, const char *str, gboolean case_sensitive);
void tab_search_reverse(Tab *t, const char *str, gboolean case_sensitive);

#endif /* _TAB_H_ */
