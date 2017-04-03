#ifndef LIBUSB_EVENTS_WINDOWS_H
#define LIBUSB_EVENTS_WINDOWS_H

#define USBI_OS_HANDLE_DESC		"HANDLE"
#define USBI_OS_HANDLE_FORMAT_SPECIFIER	"%p"
#define USBI_EVENT_MASK			0

typedef struct {
	HANDLE handle;
} usbi_event_t;

#define USBI_EVENT_GET_SOURCE(event)	((event)->handle)
#define USBI_INVALID_EVENT		{ INVALID_HANDLE_VALUE }

typedef HANDLE usbi_timer_t;

#define USBI_INVALID_TIMER		INVALID_HANDLE_VALUE

#endif /* LIBUSB_EVENTS_WINDOWS_H */
