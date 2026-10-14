/* layout.c - key sizes and labels, building the key widgets */

#include "qsk.h"

#include <string.h>

#define HK_COLS		6	/* slots per row on the hotkey layout */
#define HK_WEIGHT	2.3	/* slot width, in normal keys */

int layout;			/* index in langs[] */
gboolean shifted;
gboolean hk_mode;		/* hotkey layout is shown */

gboolean
is_special(const char *t)
{
	return t[0] == '@' && t[1] != '\0';
}

static double
key_weight(const char *t)
{
	if (strncmp(t, "@c:", 3) == 0 || strcmp(t, "@back") == 0)
		return HK_WEIGHT;
	if (strcmp(t, "@space") == 0)
		return 5;
	if (strcmp(t, "@bksp") == 0 || strcmp(t, "@enter") == 0 ||
	    strcmp(t, "@shift") == 0)
		return 2;
	if (strcmp(t, "@at") == 0 || strcmp(t, "@hk") == 0)
		return 1;
	if (is_special(t))
		return 1.5;
	return 1;
}

static const char *
key_label(const char *t)
{
	if (strncmp(t, "@c:", 3) == 0)
		return t + 3;
	if (strcmp(t, "@back") == 0)
		return "ABC";
	if (!is_special(t))
		return t;
	if (strcmp(t, "@bksp") == 0)
		return "Bksp";
	if (strcmp(t, "@enter") == 0)
		return "Enter";
	if (strcmp(t, "@shift") == 0)
		return "Shift";
	if (strcmp(t, "@tab") == 0)
		return "Tab";
	if (strcmp(t, "@at") == 0)
		return "!AT";
	if (strcmp(t, "@hk") == 0)
		return "!HK";
	if (strcmp(t, "@left") == 0)
		return "\xe2\x86\x90";
	if (strcmp(t, "@right") == 0)
		return "\xe2\x86\x92";
	if (strcmp(t, "@quit") == 0)
		return "Close";
	if (strcmp(t, "@lang") == 0)
		return langs[(layout + 1) % nlangs]->label;
	return "";	/* space */
}

static void
add_key(GtkWidget *row, const char *t)
{
	GtkWidget *b = gtk_button_new_with_label(key_label(t));

	gtk_widget_set_size_request(b, (int)(unit * key_weight(t)) - 4, -1);
	gtk_widget_set_can_focus(b, FALSE);
	gtk_widget_add_events(b, GDK_TOUCH_MASK | GDK_BUTTON_PRESS_MASK |
	    GDK_BUTTON_RELEASE_MASK);
	g_object_set_data_full(G_OBJECT(b), "tok", g_strdup(t), g_free);
	if (shifted && strcmp(t, "@shift") == 0)
		gtk_style_context_add_class(gtk_widget_get_style_context(b),
		    "shift-on");
	g_signal_connect(b, "touch-event", G_CALLBACK(on_touch), NULL);
	g_signal_connect(b, "button-press-event", G_CALLBACK(on_button), NULL);
	g_signal_connect(b, "button-release-event", G_CALLBACK(on_button),
	    NULL);
	gtk_box_pack_start(GTK_BOX(row), b, TRUE, TRUE, 0);
}

/* Empty slot of the hotkey grid: keeps the slot width, shows nothing. */
static void
add_blank(GtkWidget *row)
{
	GtkWidget *b = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);

	gtk_widget_set_size_request(b, (int)(unit * HK_WEIGHT) - 4, -1);
	gtk_box_pack_start(GTK_BOX(row), b, TRUE, TRUE, 0);
}

/*
 * Hotkey layout: fixed grid of NROWS x HK_COLS slots filled with the
 * combinations from hotkey.conf. The last slot is "ABC" (back to the normal
 * layout), so there is room for NROWS * HK_COLS - 1 combinations. Unused
 * slots stay empty.
 */
static void
build_hk_keys(void)
{
	GPtrArray *a = hotkey_load();
	int r, c, n = a->len;

	for (r = 0; r < NROWS; r++) {
		GtkWidget *row = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 4);

		gtk_box_pack_start(GTK_BOX(kb_box), row, TRUE, TRUE, 0);
		for (c = 0; c < HK_COLS; c++) {
			int i = r * HK_COLS + c;

			if (r == NROWS - 1 && c == HK_COLS - 1) {
				add_key(row, "@back");
			} else if (i < n) {
				gchar *tok = g_strconcat("@c:",
				    (gchar *)g_ptr_array_index(a, i), NULL);

				add_key(row, tok);
				g_free(tok);
			} else {
				add_blank(row);
			}
		}
	}
	g_ptr_array_free(a, TRUE);
	gtk_widget_show_all(kb_box);
}

void
build_keys(void)
{
	int r, i;

	repeat_stop();
	gtk_container_foreach(GTK_CONTAINER(kb_box),
	    (GtkCallback)gtk_widget_destroy, NULL);
	if (hk_mode) {
		build_hk_keys();
		return;
	}
	for (r = 0; r < NROWS; r++) {
		GtkWidget *row = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 4);
		gchar **tok = g_strsplit(langs[layout]->rows[shifted][r], " ", -1);

		gtk_box_pack_start(GTK_BOX(kb_box), row, TRUE, TRUE, 0);
		for (i = 0; tok[i] != NULL; i++)
			if (tok[i][0] != '\0')
				add_key(row, tok[i]);
		g_strfreev(tok);
	}
	gtk_widget_show_all(kb_box);
}

/* Width of the widest row of any layout, in units of a normal key. */
double
max_row_units(void)
{
	static double max;
	int l, sh, r, i;

	if (max > 0)
		return max;
	for (l = 0; l < nlangs; l++)
		for (sh = 0; sh < 2; sh++)
			for (r = 0; r < NROWS; r++) {
				gchar **tok = g_strsplit(langs[l]->rows[sh][r], " ", -1);
				double w = 0;

				for (i = 0; tok[i] != NULL; i++)
					if (tok[i][0] != '\0')
						w += key_weight(tok[i]);
				g_strfreev(tok);
				if (w > max)
					max = w;
			}
	return max;
}
