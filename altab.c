/* altab.c - Alt+Tab: Alt stays held for ALT_HOLD_MS after the last press */

#include "qsk.h"

#include <X11/extensions/XTest.h>
#include <unistd.h>

#define ALT_HOLD_MS	3000	/* Alt is held this long after the last !AT */

/*
 * The first press holds Alt down and taps Tab. Every press (re)starts a
 * ALT_HOLD_MS timer; when it runs out, Alt is released.
 */
static guint alt_id;
static gboolean alt_held;
static KeyCode alt_kc;

void
alt_release(void)
{
	if (alt_id != 0) {
		g_source_remove(alt_id);
		alt_id = 0;
	}
	if (alt_held) {
		XTestFakeKeyEvent(xd, alt_kc, False, 0);
		XFlush(xd);
		alt_held = FALSE;
	}
}

static gboolean
alt_timeout(gpointer data)
{
	alt_id = 0;
	alt_release();
	return G_SOURCE_REMOVE;
}

void
send_alt_tab(void)
{
	KeyCode tab = XKeysymToKeycode(xd, XK_Tab);

	if (tab == 0)
		return;
	if (!alt_held) {
		alt_kc = XKeysymToKeycode(xd, XK_Alt_L);
		if (alt_kc == 0)
			return;
		XTestFakeKeyEvent(xd, alt_kc, True, 0);
		XFlush(xd);
		usleep(30000);
		alt_held = TRUE;
	}
	XTestFakeKeyEvent(xd, tab, True, 0);
	XTestFakeKeyEvent(xd, tab, False, 0);
	XFlush(xd);

	if (alt_id != 0)
		g_source_remove(alt_id);
	alt_id = g_timeout_add(ALT_HOLD_MS, alt_timeout, NULL);
}
