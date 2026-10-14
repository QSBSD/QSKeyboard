/* actions.c - what the keys do: tokens, auto-repeat, press handling */

#include "qsk.h"

#include <string.h>

#define DOUBLE_US	(40 * 1000)	/* touch + emulated click */
#define REPEAT_FIRST_MS	450
#define REPEAT_NEXT_MS	50

static guint repeat_id;
static KeySym repeat_sym;

void
repeat_stop(void)
{
	if (repeat_id != 0) {
		g_source_remove(repeat_id);
		repeat_id = 0;
	}
}

static gboolean
repeat_next(gpointer data)
{
	send_special(repeat_sym);
	return G_SOURCE_CONTINUE;
}

static gboolean
repeat_first(gpointer data)
{
	send_special(repeat_sym);
	repeat_id = g_timeout_add(REPEAT_NEXT_MS, repeat_next, NULL);
	return G_SOURCE_REMOVE;
}

static void
repeat_start(KeySym ks)
{
	repeat_stop();
	repeat_sym = ks;
	repeat_id = g_timeout_add(REPEAT_FIRST_MS, repeat_first, NULL);
}

gboolean
rebuild_idle(gpointer data)
{
	build_keys();
	return G_SOURCE_REMOVE;
}

/* Combo is sent after the window is really unmapped and focus is back. */
static gboolean
combo_cb(gpointer data)
{
	send_combo((const char *)data);
	g_free(data);
	return G_SOURCE_REMOVE;
}

/* Returns the keysym to auto-repeat, or NoSymbol. */
static KeySym
handle_token(const char *t)
{
	if (strcmp(t, "@at") != 0)
		alt_release();
	if (strncmp(t, "@c:", 3) == 0) {
		gchar *combo = g_strdup(t + 3);

		set_visible(FALSE);
		XSync(xd, False);
		g_timeout_add(250, combo_cb, combo);
		return NoSymbol;
	}
	if (!is_special(t)) {
		send_text(t);
		if (shifted) {
			shifted = FALSE;
			g_idle_add(rebuild_idle, NULL);
		}
		return NoSymbol;
	}
	if (strcmp(t, "@bksp") == 0) {
		send_special(XK_BackSpace);
		return XK_BackSpace;
	} else if (strcmp(t, "@left") == 0) {
		send_special(XK_Left);
		return XK_Left;
	} else if (strcmp(t, "@right") == 0) {
		send_special(XK_Right);
		return XK_Right;
	} else if (strcmp(t, "@enter") == 0) {
		send_special(XK_Return);
	} else if (strcmp(t, "@tab") == 0) {
		send_special(XK_Tab);
	} else if (strcmp(t, "@at") == 0) {
		send_alt_tab();
	} else if (strcmp(t, "@hk") == 0) {
		hk_mode = TRUE;
		g_idle_add(rebuild_idle, NULL);
	} else if (strcmp(t, "@back") == 0) {
		hk_mode = FALSE;
		g_idle_add(rebuild_idle, NULL);
	} else if (strcmp(t, "@space") == 0) {
		send_special(XK_space);
	} else if (strcmp(t, "@shift") == 0) {
		shifted = !shifted;
		g_idle_add(rebuild_idle, NULL);
	} else if (strcmp(t, "@lang") == 0) {
		layout = (layout + 1) % nlangs;
		shifted = FALSE;
		g_idle_add(rebuild_idle, NULL);
	} else if (strcmp(t, "@quit") == 0) {
		gtk_main_quit();
	}
	return NoSymbol;
}

static void
key_down(GtkWidget *w)
{
	const char *t = g_object_get_data(G_OBJECT(w), "tok");
	static GtkWidget *last_w;
	static gint64 last_us;
	gint64 now = g_get_monotonic_time();
	KeySym rep;

	if (t == NULL)
		return;
	/* GDK may deliver both the touch and the click emulated from it */
	if (w == last_w && now - last_us < DOUBLE_US)
		return;
	last_w = w;
	last_us = now;
	rep = handle_token(t);
	if (rep != NoSymbol)
		repeat_start(rep);
}

static void
key_up(GtkWidget *w)
{
	repeat_stop();
}

gboolean
on_touch(GtkWidget *w, GdkEvent *ev, gpointer data)
{
	switch (ev->type) {
	case GDK_TOUCH_BEGIN:
		key_down(w);
		return TRUE;
	case GDK_TOUCH_END:
	case GDK_TOUCH_CANCEL:
		key_up(w);
		return TRUE;
	default:
		return FALSE;
	}
}

gboolean
on_button(GtkWidget *w, GdkEventButton *ev, gpointer data)
{
	if (ev->type == GDK_BUTTON_PRESS)
		key_down(w);
	else if (ev->type == GDK_BUTTON_RELEASE)
		key_up(w);
	return TRUE;
}
