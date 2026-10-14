/* window.c - the keyboard window: placing, show/hide, look */

#include "qsk.h"

#include <stdio.h>

#define FOCUS_POLL_MS	200

GtkWidget *win, *kb_box;
int unit = 80;			/* width of a normal key, pixels */

static guint focus_id;
static Window shown_focus;	/* input focus when the keyboard was shown */

static gboolean
focus_poll(gpointer data)
{
	if (current_focus() != shown_focus) {
		focus_id = 0;
		set_visible(FALSE);
		return G_SOURCE_REMOVE;
	}
	return G_SOURCE_CONTINUE;
}

static void
place_window(void)
{
	GdkDisplay *d = gdk_display_get_default();
	GdkMonitor *m = gdk_display_get_primary_monitor(d);
	GdkRectangle g;
	int h;

	if (m == NULL)
		m = gdk_display_get_monitor(d, 0);
	gdk_monitor_get_geometry(m, &g);
	unit = (int)((g.width - 16) / max_row_units());
	h = g.width * 26 / 100;
	if (h > g.height * 45 / 100)
		h = g.height * 45 / 100;
	gtk_widget_set_size_request(win, g.width, h);
	gtk_window_resize(GTK_WINDOW(win), g.width, h);
	gtk_window_move(GTK_WINDOW(win), g.x, g.y + g.height - h);
}

void
set_visible(gboolean v)
{
	repeat_stop();
	if (!v) {
		alt_release();
		if (hk_mode) {
			hk_mode = FALSE;
			g_idle_add(rebuild_idle, NULL);
		}
	}
	if (v) {
		shown_focus = current_focus();
		place_window();
		gtk_widget_show_all(win);
		if (gtk_widget_get_window(win) != NULL)
			gdk_window_raise(gtk_widget_get_window(win));
		if (focus_id == 0)
			focus_id = g_timeout_add(FOCUS_POLL_MS, focus_poll,
			    NULL);
	} else {
		gtk_widget_hide(win);
		if (focus_id != 0) {
			g_source_remove(focus_id);
			focus_id = 0;
		}
	}
}

static void
load_css(void)
{
	char css[1024];
	GtkCssProvider *p = gtk_css_provider_new();

	snprintf(css, sizeof(css),
	    "window { background: #000000; }"
	    "button, button:hover, button:active, button:focus,"
	    " button:checked, button:backdrop {"
	    " font-size: %dpx; padding: 0; min-height: 0; min-width: 0;"
	    " color: #ffffff; background: #000000; background-image: none;"
	    " border: 1px solid #333333; border-radius: 8px;"
	    " box-shadow: none; text-shadow: none; outline: none;"
	    " transition: none; animation: none; }"
	    "button.shift-on { border-color: #ffffff; }",
	    (int)(unit * 0.38));
	gtk_css_provider_load_from_data(p, css, -1, NULL);
	gtk_style_context_add_provider_for_screen(gdk_screen_get_default(),
	    GTK_STYLE_PROVIDER(p), GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
	g_object_unref(p);
}

/* Create the (hidden) window and fill it with keys. */
void
window_init(void)
{
	win = gtk_window_new(GTK_WINDOW_POPUP);	/* no WM, never gets focus */
	gtk_window_set_accept_focus(GTK_WINDOW(win), FALSE);
	gtk_window_set_focus_on_map(GTK_WINDOW(win), FALSE);
	gtk_container_set_border_width(GTK_CONTAINER(win), 4);
	kb_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 4);
	gtk_box_set_homogeneous(GTK_BOX(kb_box), TRUE);
	gtk_container_add(GTK_CONTAINER(win), kb_box);
	place_window();
	load_css();
	build_keys();
}
