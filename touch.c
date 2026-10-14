/* touch.c - raw touch / click events: show gesture and hide-on-outside */

#include "qsk.h"

#include <X11/extensions/XInput2.h>

#define TAP_MAX_US	(400 * 1000)	/* a tap lasts at most this long */
#define TAP_GAP_US	(500 * 1000)	/* max pause between the two taps */
#define OUTSIDE_MS	60		/* pointer settles after a touch */
#define STALE_US	(5 * G_USEC_PER_SEC)	/* lost touch-end events */
#define X_TICK_MS	100
#define MAXTOUCH	10

static guint outside_id;

static struct {
	int dev;
	int id;
} touches[MAXTOUCH];		/* fingers currently down on touchscreens */
static int ntouch;
static gint64 seq_start_us, last_tap_end_us, last_touch_ev_us;
static gboolean seq_two;	/* two fingers were down in this sequence */
static int taps;		/* two-finger taps done so far (0..1) */

static gboolean
is_direct_touch(int deviceid)
{
	XIDeviceInfo *info;
	int n, i;
	gboolean r = FALSE;

	info = XIQueryDevice(xd, deviceid, &n);
	if (info == NULL)
		return FALSE;
	for (i = 0; i < info->num_classes; i++) {
		if (info->classes[i]->type == XITouchClass &&
		    ((XITouchClassInfo *)info->classes[i])->mode ==
		    XIDirectTouch)
			r = TRUE;
	}
	XIFreeDeviceInfo(info);
	return r;
}

/* A touch or click landed outside the keyboard: hide it. */
static gboolean
outside_check(gpointer data)
{
	Window rw, cw;
	int rx, ry, wx, wy, x, y;
	unsigned int m;

	outside_id = 0;
	if (!gtk_widget_get_visible(win) || gtk_widget_get_window(win) == NULL)
		return G_SOURCE_REMOVE;
	if (!XQueryPointer(xd, DefaultRootWindow(xd), &rw, &cw, &rx, &ry, &wx,
	    &wy, &m))
		return G_SOURCE_REMOVE;
	gdk_window_get_origin(gtk_widget_get_window(win), &x, &y);
	if (rx < x || rx >= x + gtk_widget_get_allocated_width(win) ||
	    ry < y || ry >= y + gtk_widget_get_allocated_height(win))
		set_visible(FALSE);
	return G_SOURCE_REMOVE;
}

static void
check_outside_soon(void)
{
	if (gtk_widget_get_visible(win) && outside_id == 0)
		outside_id = g_timeout_add(OUTSIDE_MS, outside_check, NULL);
}

static void
touch_add(int dev, int id)
{
	int i;
	gint64 now = g_get_monotonic_time();

	/* a lost touch-end must not block the gesture forever */
	if (ntouch > 0 && now - last_touch_ev_us > STALE_US) {
		ntouch = 0;
		taps = 0;
	}
	last_touch_ev_us = now;
	for (i = 0; i < ntouch; i++)
		if (touches[i].dev == dev && touches[i].id == id)
			return;
	if (ntouch >= MAXTOUCH)
		return;
	if (ntouch == 0) {
		seq_start_us = now;
		seq_two = FALSE;
	}
	touches[ntouch].dev = dev;
	touches[ntouch].id = id;
	ntouch++;
	if (ntouch >= 2)
		seq_two = TRUE;
}

/* All fingers up: was that a quick two-finger tap, and the second in a row? */
static void
touch_sequence_done(void)
{
	gint64 now = g_get_monotonic_time();

	if (!seq_two || now - seq_start_us > TAP_MAX_US) {
		taps = 0;
		return;
	}
	if (taps == 1 && seq_start_us - last_tap_end_us <= TAP_GAP_US) {
		taps = 0;
		if (!gtk_widget_get_visible(win))
			set_visible(TRUE);
		return;
	}
	taps = 1;
	last_tap_end_us = now;
}

static void
touch_del(int dev, int id)
{
	int i;

	last_touch_ev_us = g_get_monotonic_time();
	for (i = 0; i < ntouch; i++) {
		if (touches[i].dev == dev && touches[i].id == id) {
			touches[i] = touches[ntouch - 1];
			ntouch--;
			if (ntouch == 0)
				touch_sequence_done();
			return;
		}
	}
}

static void
process_x_events(void)
{
	XEvent ev;
	XIRawEvent *re;

	while (XPending(xd) > 0) {
		XNextEvent(xd, &ev);
		if (ev.type != GenericEvent ||
		    ev.xcookie.extension != xi_opcode ||
		    !XGetEventData(xd, &ev.xcookie))
			continue;
		re = ev.xcookie.data;
		if (ev.xcookie.evtype == XI_RawTouchBegin) {
			if (is_direct_touch(re->sourceid)) {
				touch_add(re->sourceid, re->detail);
				check_outside_soon();
			}
		} else if (ev.xcookie.evtype == XI_RawTouchEnd) {
			touch_del(re->sourceid, re->detail);
		} else if (ev.xcookie.evtype == XI_RawButtonPress) {
			/* real mouse clicks only, not touch emulation */
			if (!(re->flags & XIPointerEmulated))
				check_outside_soon();
		}
		XFreeEventData(xd, &ev.xcookie);
	}
}

static gboolean
x_watch_cb(GIOChannel *src, GIOCondition cond, gpointer data)
{
	process_x_events();
	return TRUE;
}

static gboolean
x_tick(gpointer data)
{
	process_x_events();
	return G_SOURCE_CONTINUE;
}

/* Subscribe to raw events on the root window and hook into the main loop. */
void
touch_init(void)
{
	unsigned char bits[XIMaskLen(XI_LASTEVENT)] = { 0 };
	XIEventMask mask;
	GIOChannel *ch;

	mask.deviceid = XIAllDevices;
	mask.mask_len = sizeof(bits);
	mask.mask = bits;
	XISetMask(bits, XI_RawTouchBegin);
	XISetMask(bits, XI_RawTouchEnd);
	XISetMask(bits, XI_RawButtonPress);
	XISelectEvents(xd, DefaultRootWindow(xd), &mask, 1);
	XFlush(xd);

	ch = g_io_channel_unix_new(ConnectionNumber(xd));
	g_io_add_watch(ch, G_IO_IN, x_watch_cb, NULL);
	g_timeout_add(X_TICK_MS, x_tick, NULL);
}
