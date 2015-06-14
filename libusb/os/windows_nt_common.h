/*
 * Windows backend common header for libusb 1.0
 *
 * This file brings together header code common between
 * the desktop Windows backends.
 * Copyright © 2012-2013 RealVNC Ltd.
 * Copyright © 2009-2012 Pete Batard <pete@akeo.ie>
 * With contributions from Michael Plante, Orin Eman et al.
 * Parts of this code adapted from libusb-win32-v1 by Stephan Meyer
 * Major code testing contribution by Xiaofan Chen
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
 */

#pragma once

#include "windows_shared_types.h"

 /* Windows versions */
enum windows_version {
	WINDOWS_UNDEFINED = -1,
	WINDOWS_UNSUPPORTED = 0,
	WINDOWS_XP = 0x51,
	WINDOWS_2003 = 0x52,	// Also XP x64
	WINDOWS_VISTA = 0x60,
	WINDOWS_7 = 0x61,
	WINDOWS_8 = 0x62,
	WINDOWS_8_1_OR_LATER = 0x63,
	WINDOWS_MAX
};

extern enum windows_version windows_version;

struct windows_backend {
	int (*init)(struct libusb_context *ctx);
	void (*exit)(struct libusb_context *ctx);
	int (*get_device_list)(struct libusb_context *ctx,
		struct discovered_devs **discdevs);
	int (*open)(struct libusb_device_handle *dev_handle);
	void (*close)(struct libusb_device_handle *dev_handle);
	int (*get_device_descriptor)(struct libusb_device *device, unsigned char *buffer);
	int (*get_active_config_descriptor)(struct libusb_device *device,
		unsigned char *buffer, size_t len);
	int (*get_config_descriptor)(struct libusb_device *device,
		uint8_t config_index, unsigned char *buffer, size_t len);
	int(*get_config_descriptor_by_value)(struct libusb_device *device,
		uint8_t bConfigurationValue, unsigned char **buffer);
	int (*get_configuration)(struct libusb_device_handle *dev_handle, int *config);
	int (*set_configuration)(struct libusb_device_handle *dev_handle, int config);
	int (*claim_interface)(struct libusb_device_handle *dev_handle, int interface_number);
	int (*release_interface)(struct libusb_device_handle *dev_handle, int interface_number);
	int (*set_interface_altsetting)(struct libusb_device_handle *dev_handle,
		int interface_number, int altsetting);
	int (*clear_halt)(struct libusb_device_handle *dev_handle,
		unsigned char endpoint);
	int (*reset_device)(struct libusb_device_handle *dev_handle);
	void (*destroy_device)(struct libusb_device *dev);
	int (*submit_transfer)(struct usbi_transfer *itransfer);
	int (*cancel_transfer)(struct usbi_transfer *itransfer);
	void (*clear_transfer_priv)(struct usbi_transfer *itransfer);
	int (*copy_transfer_data)(struct usbi_transfer *itransfer, uint32_t io_size);
	struct winfd *(*get_fd)(struct usbi_transfer *itransfer);
	void (*get_overlapped_result)(struct usbi_transfer *itransfer, struct winfd *,
		DWORD *io_result, DWORD *io_size);
};

struct windows_context_priv {
	const struct windows_backend *backend;
};

union windows_device_priv {
	struct usbdk_device_priv usbdk_priv;
	struct winusb_device_priv winusb_priv;
};

union windows_device_handle_priv {
	struct usbdk_device_handle_priv usbdk_priv;
	struct winusb_device_handle_priv winusb_priv;
};

union windows_transfer_priv {
	struct usbdk_transfer_priv usbdk_priv;
	struct winusb_transfer_priv winusb_priv;
};

extern const struct windows_backend usbdk_backend;
extern const struct windows_backend winusb_backend;

unsigned long htab_hash(const char *str);

#if defined(ENABLE_LOGGING)
const char *windows_error_str(DWORD error_code);
#endif
