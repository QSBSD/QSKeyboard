/*
 * QSKeyboard - declarations shared between the modules.
 * Program description: see main.c.
 */

#ifndef QSK_H
#define QSK_H

#include <gtk/gtk.h>
#include <gdk/gdk.h>
#include <X11/Xlib.h>
#include <X11/keysym.h>

/* main.c */
void die(const char *msg);

/* xsend.c: own X connection and synthetic input */
extern Display *xd;
extern int xi_opcode;
void x_open(void);
void send_special(KeySym ks);
void send_text(const char *s);
Window current_focus(void);

/* altab.c: Alt+Tab with Alt held for a while */
void alt_release(void);
void send_alt_tab(void);

/* hotkey.c: hotkey.conf and hotkey combinations */
void hotkey_ensure_file(void);
GPtrArray *hotkey_load(void);
void send_combo(const char *line);

/* touch.c: raw touch / click events, gesture detection */
void touch_init(void);

/* lang/: key tables of the layouts */
#define NROWS		5
struct lang {
	const char *label;	/* on the !lang key while the previous one is active */
	const char *rows[2][NROWS];	/* [shifted][row] */
};
extern const struct lang lang_en, lang_ru;
extern const struct lang *const langs[];	/* in !lang cycling order */
extern const int nlangs;

/* layout.c: key widgets */
extern int layout;		/* index in langs[] */
extern gboolean shifted;
extern gboolean hk_mode;	/* hotkey layout is shown */
gboolean is_special(const char *t);
double max_row_units(void);
void build_keys(void);

/* actions.c: what the keys do */
void repeat_stop(void);
gboolean rebuild_idle(gpointer data);
gboolean on_touch(GtkWidget *w, GdkEvent *ev, gpointer data);
gboolean on_button(GtkWidget *w, GdkEventButton *ev, gpointer data);

/* window.c: the keyboard window */
extern GtkWidget *win, *kb_box;
extern int unit;		/* width of a normal key, pixels */
void window_init(void);
void set_visible(gboolean v);

#endif /* QSK_H */
