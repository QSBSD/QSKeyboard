/* xsend.c - own X connection and synthetic keyboard input (XTest) */

#include "qsk.h"

#include <X11/extensions/XInput2.h>
#include <X11/extensions/XTest.h>
#include <stdio.h>
#include <unistd.h>

Display *xd;		/* own X connection: raw events and XTest */
int xi_opcode;

static KeyCode scratch_kc;

static KeyCode
find_scratch_keycode(void)
{
	int min, max, per, kc, i;
	KeySym *m;

	XDisplayKeycodes(xd, &min, &max);
	m = XGetKeyboardMapping(xd, min, max - min + 1, &per);
	if (m == NULL)
		return 0;
	for (kc = max; kc >= min; kc--) {
		gboolean empty = TRUE;

		for (i = 0; i < per; i++)
			if (m[(kc - min) * per + i] != NoSymbol)
				empty = FALSE;
		if (empty) {
			XFree(m);
			return (KeyCode)kc;
		}
	}
	XFree(m);
	return 0;
}

void
x_open(void)
{
	int ev, err, major = 2, minor = 2;

	xd = XOpenDisplay(NULL);
	if (xd == NULL)
		die("cannot open display");
	if (!XTestQueryExtension(xd, &ev, &err, &ev, &err))
		die("no XTEST extension");
	if (!XQueryExtension(xd, "XInputExtension", &xi_opcode, &ev, &err))
		die("no XInput extension");
	if (XIQueryVersion(xd, &major, &minor) != Success)
		die("XInput 2.2 (touch) is required");

	scratch_kc = find_scratch_keycode();
	if (scratch_kc == 0)
		fprintf(stderr, "QSKeyboard: no spare keycode, typing "
		    "characters will not work\n");
}

static void
send_keycode(KeyCode kc)
{
	XTestFakeKeyEvent(xd, kc, True, 0);
	XTestFakeKeyEvent(xd, kc, False, 0);
	XFlush(xd);
}

Window
current_focus(void)
{
	Window w;
	int revert;

	XGetInputFocus(xd, &w, &revert);
	return w;
}

/* Keys that exist in every layout: BackSpace, Return, Tab, arrows, space. */
void
send_special(KeySym ks)
{
	KeyCode kc = XKeysymToKeycode(xd, ks);

	if (kc != 0)
		send_keycode(kc);
}

/* Any character: map its keysym to the spare keycode and press that. */
static void
send_char(gunichar c)
{
	KeySym ks, map[2];

	if (scratch_kc == 0)
		return;
	ks = gdk_unicode_to_keyval(c);
	map[0] = map[1] = ks;
	XChangeKeyboardMapping(xd, scratch_kc, 2, map, 1);
	XSync(xd, False);
	usleep(8000);
	send_keycode(scratch_kc);
	XSync(xd, False);
}

void
send_text(const char *s)
{
	const char *p;

	for (p = s; *p != '\0'; p = g_utf8_next_char(p))
		send_char(g_utf8_get_char(p));
}
