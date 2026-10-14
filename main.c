/*
 * QSKeyboard - on-screen keyboard for X11 that runs in the background.
 *
 *  - show:             two quick taps with two fingers on a touchscreen
 *                      (XInput2 raw touch events; the mouse never shows it)
 *  - hide:             a touch or click outside the keyboard, or the input
 *                      focus moving to another window
 *  - look:             plain black, no press animations
 *  - typing:           XTest; characters go through a spare keycode that is
 *                      remapped on the fly (the way xdotool does it), so
 *                      Cyrillic works whatever the X keyboard layout is
 *  - !AT key:          Alt+Tab; Alt stays held for 3 s after the last press
 *                      (each press taps Tab and restarts the 3 s); Alt is
 *                      released by the timeout, by any other key, when the
 *                      keyboard hides and when the program exits
 *  - !HK key:          switches to a hotkey layout: one button per line of
 *                      ~/.config/QSKeyboard/hotkey.conf (created on the
 *                      first start; e.g. "super+Tab", "ctrl+alt+t"; "#"
 *                      starts a comment, 5 rows x 6 slots, the last slot is
 *                      "ABC"). Pressing a button emulates that combination
 *                      and hides the keyboard; "ABC" goes back to letters
 *  - Close key:        quits the program
 *
 * Usage: QSKeyboard [-f]     (-f: stay in the foreground)
 * SIGUSR2 shows the keyboard, SIGUSR1 hides it, SIGTERM quits.
 *
 * Modules:
 *   main.c     start-up, daemonizing, signals
 *   xsend.c    X connection, typing characters and special keys
 *   altab.c    Alt+Tab
 *   hotkey.c   hotkey.conf and hotkey combinations
 *   touch.c    raw touch/click events: show gesture, hide on outside
 *   lang/      key tables of the layouts (en.c, ru.c, list in lang.c)
 *   layout.c   key widgets
 *   actions.c  key press handling, auto-repeat
 *   window.c   the keyboard window, show/hide, CSS
 */

#include "qsk.h"

#include <glib-unix.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static gboolean
sig_show(gpointer data)
{
	set_visible(TRUE);
	return G_SOURCE_CONTINUE;
}

static gboolean
sig_hide(gpointer data)
{
	set_visible(FALSE);
	return G_SOURCE_CONTINUE;
}

static gboolean
sig_quit(gpointer data)
{
	gtk_main_quit();
	return G_SOURCE_REMOVE;
}

void
die(const char *msg)
{
	fprintf(stderr, "QSKeyboard: %s\n", msg);
	exit(1);
}

int
main(int argc, char **argv)
{
	if (!(argc > 1 && strcmp(argv[1], "-f") == 0)) {
		pid_t pid = fork();

		if (pid < 0)
			die("fork failed");
		if (pid > 0) {
			printf("QSKeyboard: started, pid %d\n", (int)pid);
			return 0;
		}
		setsid();
		if (freopen("/dev/null", "r", stdin) == NULL ||
		    freopen("/dev/null", "w", stdout) == NULL)
			die("cannot redirect stdio");
	}

	atexit(alt_release);
	gtk_init(&argc, &argv);

	x_open();
	touch_init();
	hotkey_ensure_file();
	window_init();

	g_unix_signal_add(SIGUSR1, sig_hide, NULL);
	g_unix_signal_add(SIGUSR2, sig_show, NULL);
	g_unix_signal_add(SIGTERM, sig_quit, NULL);
	g_unix_signal_add(SIGINT, sig_quit, NULL);
	g_unix_signal_add(SIGHUP, sig_quit, NULL);

	gtk_main();
	alt_release();
	return 0;
}
