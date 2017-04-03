all: libusb-1.0.la libusb-1.0.dll

AUTOMAKE_OPTIONS = subdir-objects

lib_LTLIBRARIES = libusb-1.0.la

POSIX_PLATFORM_SRC = os/events_posix.h os/events_posix.c \
		     os/threads_posix.h os/threads_posix.c
WINDOWS_PLATFORM_SRC = os/events_windows.h os/events_windows.c \
		       os/threads_windows.h os/threads_windows.c
DARWIN_USB_SRC = os/darwin_usb.h os/darwin_usb.c
HAIKU_USB_SRC = os/haiku_usb.h os/haiku_usb_backend.cpp \
		os/haiku_usb_raw.h os/haiku_usb_raw.cpp os/haiku_pollfs.cpp
LINUX_USBFS_SRC = os/linux_usbfs.h os/linux_usbfs.c
NETBSD_USB_SRC = os/netbsd_usb.c
OPENBSD_USB_SRC = os/openbsd_usb.c
SUNOS_USB_SRC = os/sunos_usb.h os/sunos_usb.c
WINCE_USB_SRC = os/wince_usb.h os/wince_usb.c
WINDOWS_COMMON_SRC = os/windows_nt_common.h os/windows_nt_common.c \
		     os/windows_common.h libusb-1.0.rc libusb-1.0.def
WINDOWS_WINUSB_SRC = os/windows_winusb.h os/windows_winusb.c
WINDOWS_USBDK_SRC = os/windows_usbdk.h os/windows_usbdk.c

EXTRA_DIST = $(POSIX_PLATFORM_SRC) $(WINDOWS_PLATFORM_SRC) \
	$(DARWIN_USB_SRC) $(HAIKU_USB_SRC) \
	$(LINUX_USBFS_SRC) $(NETBSD_USB_SRC) \
	$(OPENBSD_USB_SRC) $(SUNOS_USB_SRC) \
	$(WINCE_USB_SRC) $(WINDOWS_COMMON_SRC) \
	$(WINDOWS_WINUSB_SRC) $(WINDOWS_USBDK_SRC) \
	os/linux_netlink.c os/linux_udev.c

if OS_DARWIN
OS_SRC = $(DARWIN_USB_SRC)
AM_CFLAGS_EXT = -no-cpp-precomp
endif

if OS_HAIKU
noinst_LTLIBRARIES = libusb_haiku.la
libusb_haiku_la_SOURCES = $(HAIKU_USB_SRC)
libusb_1_0_la_LIBADD = libusb_haiku.la
endif

if OS_LINUX

if USE_UDEV
OS_SRC = $(LINUX_USBFS_SRC) os/linux_udev.c
else
OS_SRC = $(LINUX_USBFS_SRC) os/linux_netlink.c
endif

endif

if OS_NETBSD
OS_SRC = $(NETBSD_USB_SRC)
endif

if OS_OPENBSD
OS_SRC = $(OPENBSD_USB_SRC)
endif

if OS_SUNOS
OS_SRC = $(SUNOS_USB_SRC)
endif

if OS_WINDOWS

OS_SRC = $(WINDOWS_COMMON_SRC)

if USE_USBDK
OS_SRC += $(WINDOWS_USBDK_SRC)
else
OS_SRC += $(WINDOWS_WINUSB_SRC)
endif

.rc.lo:
	$(AM_V_GEN)$(LIBTOOL) $(AM_V_lt) $(AM_LIBTOOLFLAGS) $(LIBTOOLFLAGS) --tag=RC --mode=compile $(RC) $(RCFLAGS) -i $< -o $@

libusb-1.0.rc: version.h version_nano.h
endif

libusb-1.0.dll: libusb-1.0.def libusb-1.0.la
if CREATE_IMPORT_LIB
# Rebuild the import lib from the .def so that MS and MinGW DLLs can be interchanged
	$(AM_V_GEN)$(DLLTOOL) $(DLLTOOLFLAGS) --kill-at --input-def $(srcdir)/libusb-1.0.def --dllname $@ --output-lib .libs/$@.a
endif

if PLATFORM_POSIX
PLATFORM_SRC = $(POSIX_PLATFORM_SRC)
else
PLATFORM_SRC = $(WINDOWS_PLATFORM_SRC)
endif

libusb_1_0_la_CFLAGS = $(AM_CFLAGS)
libusb_1_0_la_LDFLAGS = $(LTLDFLAGS)
libusb_1_0_la_SOURCES = libusbi.h libusb.h version.h version_nano.h \
	core.c descriptor.c hotplug.h hotplug.c io.c strerror.c sync.c \
	$(PLATFORM_SRC) $(OS_SRC)

hdrdir = $(includedir)/libusb-1.0
hdr_HEADERS = libusb.h
