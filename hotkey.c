/* hotkey.c - ~/.config/QSKeyboard/hotkey.conf and hotkey combinations */

#include "qsk.h"

#include <glib/gstdio.h>
#include <X11/extensions/XTest.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

/* Name from hotkey.conf -> keycode, 0 if unknown. */
static KeyCode
hotkey_keycode(const char *name)
{
	static const struct {
		const char *name;
		KeySym ks;
	} alias[] = {
		{ "ctrl", XK_Control_L }, { "control", XK_Control_L },
		{ "alt", XK_Alt_L }, { "shift", XK_Shift_L },
		{ "super", XK_Super_L }, { "win", XK_Super_L },
		{ "meta", XK_Super_L }, { "enter", XK_Return },
		{ "esc", XK_Escape }, { "space", XK_space },
		{ "tab", XK_Tab }, { "del", XK_Delete },
		{ "bksp", XK_BackSpace },
	};
	KeySym ks = NoSymbol;
	gchar *low = g_ascii_strdown(name, -1);
	size_t i;

	for (i = 0; i < sizeof(alias) / sizeof(alias[0]); i++)
		if (strcmp(low, alias[i].name) == 0)
			ks = alias[i].ks;
	g_free(low);
	if (ks == NoSymbol)
		ks = XStringToKeysym(name);
	if (ks == NoSymbol) {
		gchar *cap = g_strdup(name);

		cap[0] = g_ascii_toupper(cap[0]);
		ks = XStringToKeysym(cap);
		g_free(cap);
	}
	if (ks == NoSymbol)
		return 0;
	return XKeysymToKeycode(xd, ks);
}

/* One combination, e.g. "ctrl+alt+t": press all in order, release reversed. */
void
send_combo(const char *line)
{
	KeyCode kcs[8];
	int n = 0, i;
	gchar **part = g_strsplit(line, "+", -1);

	for (i = 0; part[i] != NULL && n < 8; i++) {
		KeyCode kc;

		g_strstrip(part[i]);
		if (part[i][0] == '\0')
			continue;
		kc = hotkey_keycode(part[i]);
		if (kc == 0) {
			fprintf(stderr, "QSKeyboard: hotkey.conf: unknown "
			    "key \"%s\"\n", part[i]);
			n = 0;
			break;
		}
		kcs[n++] = kc;
	}
	g_strfreev(part);
	for (i = 0; i < n; i++) {
		XTestFakeKeyEvent(xd, kcs[i], True, 0);
		XFlush(xd);
		usleep(30000);
	}
	for (i = n - 1; i >= 0; i--) {
		XTestFakeKeyEvent(xd, kcs[i], False, 0);
		XFlush(xd);
		usleep(30000);
	}
}

static gchar *
hotkey_path(void)
{
	return g_build_filename(g_get_user_config_dir(), "QSKeyboard",
	    "hotkey.conf", NULL);
}

/* Create ~/.config/QSKeyboard/hotkey.conf if it does not exist yet. */
void
hotkey_ensure_file(void)
{
	static const char def[] =
	    "# QSKeyboard: the !HK key opens a layout with one button per line.\n"
	    "# Pressing a button sends that combination and closes the keyboard.\n"
	    "# One combination per line, keys joined with \"+\".\n"
	    "# Modifiers: ctrl, alt, shift, super. Other keys: X11 keysym names\n"
	    "# (Tab, Return, Escape, F4, Left, t, ...).\n"
	    "# Up to 29 lines are used. Lines starting with # are ignored.\n"
	    "# The file is re-read every time the !HK layout is opened.\n"
	    "#\n"
	    "# Examples:\n"
	    "# super+Tab\n"
	    "# ctrl+alt+t\n"
	    "# alt+F4\n";
	gchar *path = hotkey_path();
	gchar *dir = g_path_get_dirname(path);

	if (!g_file_test(path, G_FILE_TEST_EXISTS)) {
		g_mkdir_with_parents(dir, 0755);
		if (!g_file_set_contents(path, def, -1, NULL))
			fprintf(stderr, "QSKeyboard: cannot create %s\n", path);
	}
	g_free(dir);
	g_free(path);
}

/* Non-empty, non-comment lines of hotkey.conf (array of gchar *). */
GPtrArray *
hotkey_load(void)
{
	GPtrArray *a = g_ptr_array_new_with_free_func(g_free);
	gchar *path = hotkey_path();
	gchar *text = NULL;
	gchar **lines;
	int i;

	if (!g_file_get_contents(path, &text, NULL, NULL)) {
		g_free(path);
		hotkey_ensure_file();
		return a;
	}
	g_free(path);
	lines = g_strsplit(text, "\n", -1);
	g_free(text);
	for (i = 0; lines[i] != NULL; i++) {
		gchar *l = g_strstrip(lines[i]);

		if (l[0] != '\0' && l[0] != '#')
			g_ptr_array_add(a, g_strdup(l));
	}
	g_strfreev(lines);
	return a;
}
