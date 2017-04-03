/*
 * events_windows: event abstraction layer for Windows systems
 * Copyright © 2017 Chris Dickens <christopher.a.dickens@gmail.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 *
 */

#include <config.h>

#include <assert.h>

#include "libusbi.h"

int usbi_create_event(usbi_event_t *event)
{
	event->handle = CreateEvent(NULL, TRUE, FALSE, NULL);
	if (!event->handle) {
		usbi_warn(NULL, "failed to create event (%u)", GetLastError());
		return LIBUSB_ERROR_OTHER;
	}

	return 0;
}

int usbi_signal_event(usbi_event_t *event)
{
	if (!SetEvent(event->handle)) {
		usbi_warn(NULL, "failed to set event (%u)", GetLastError());
		return LIBUSB_ERROR_OTHER;
	}

	return 0;
}

int usbi_clear_event(usbi_event_t *event)
{
	if (!ResetEvent(event->handle)) {
		usbi_warn(NULL, "failed to reset event (%u)", GetLastError());
		return LIBUSB_ERROR_OTHER;
	}

	return 0;
}

int usbi_destroy_event(usbi_event_t *event)
{
	if (!CloseHandle(event->handle)) {
		usbi_warn(NULL, "failed to close event handle (%u)", GetLastError());
		return LIBUSB_ERROR_OTHER;
	}

	return 0;
}

usbi_timer_t usbi_create_timer(void)
{
	return USBI_INVALID_TIMER;
}

int usbi_arm_timer(usbi_timer_t timer, struct timeval *tv)
{
	return LIBUSB_ERROR_NOT_SUPPORTED;
}

int usbi_disarm_timer(usbi_timer_t timer)
{
	return LIBUSB_ERROR_NOT_SUPPORTED;
}

int usbi_destroy_timer(usbi_timer_t timer)
{
	return LIBUSB_ERROR_NOT_SUPPORTED;
}

int usbi_alloc_event_data(struct libusb_context *ctx)
{
	struct usbi_event_source *event_source;
	HANDLE *handles = (HANDLE *)ctx->event_data;
	int i = 0;

	/* Windows is fundamentally different from other platforms in that it imposes a
	 * rather small limit on the number of HANDLEs you can wait for. Since the Windows
	 * backends will alter the event source list for every submitted transfer, we can
	 * optimize a little by allocating the largest array of HANDLEs supported just one
	 * time. On subsequent calls to this function, we will save time by not re-allocating
	 * and instead just update the elements in the array. */
	if (!handles) {
		handles = calloc(MAXIMUM_WAIT_OBJECTS, sizeof(HANDLE));
		if (!handles)
			return LIBUSB_ERROR_NO_MEM;
		ctx->event_data = handles;
	}

	list_for_each_entry(event_source, &ctx->event_sources, list, struct usbi_event_source) {
		if (i == MAXIMUM_WAIT_OBJECTS) {
			usbi_warn(ctx, "too many HANDLEs to wait on, some will be ignored!");
			break;
		}
		handles[i++] = event_source->pollfd.fd;
	}

	return 0;
}

int usbi_handle_events(struct libusb_context *ctx, void *event_data, unsigned int cnt,
	unsigned int internal_cnt, int timeout_ms)
{
	HANDLE *handles = (HANDLE *)event_data;
	DWORD result;
	int r;

	/* make sure to wait for at most MAXIMUM_WAIT_OBJECT HANDLEs */
	cnt = MIN(cnt, MAXIMUM_WAIT_OBJECTS);
	assert(internal_cnt <= cnt);

	usbi_dbg("WaitForMultipleObjects() for %u HANDLEs with timeout in %dms", cnt, timeout_ms);
	result = WaitForMultipleObjects((DWORD)cnt, handles, FALSE, (DWORD)timeout_ms);
	usbi_dbg("WaitForMultipleObjects() returned %d", result);
	if (result == WAIT_TIMEOUT) {
		return usbi_using_timer(ctx) ? 0 : LIBUSB_ERROR_TIMEOUT;
	} else if (result == WAIT_FAILED) {
		usbi_err(ctx, "WaitForMultipleObjects() failed err=%d", GetLastError());
		return LIBUSB_ERROR_IO;
	}

	result -= WAIT_OBJECT_0;

	/* handles[0] is always the event */
	if (result == 0) {
		r = usbi_handle_event_trigger(ctx);
		if (r < 0) {
			/* return error code */
			return r;
		}
	}

	/* on timer configurations, handles[1] is the timer */
	if (usbi_using_timer(ctx) && result == 1) {
		r = usbi_handle_timer_trigger(ctx);
		if (r < 0) {
			/* return error code */
			return r;
		}
	}

	r = usbi_backend->handle_events(ctx, handles + internal_cnt, cnt - internal_cnt, cnt - internal_cnt);
	if (r)
		usbi_err(ctx, "backend handle_events failed with error %d", r);

	return r;
}
