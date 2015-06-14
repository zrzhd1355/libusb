/*
 * Windows backend for libusb 1.0
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

#include "windows_common.h"
#include "windows_nt_common.h"

#if defined(_MSC_VER)
// disable /W4 MSVC warnings that are benign
#pragma warning(disable:4100)  // unreferenced formal parameter
#pragma warning(disable:4127)  // conditional expression is constant
#pragma warning(disable:4201)  // nameless struct/union
#pragma warning(disable:4214)  // bit field types other than int
#pragma warning(disable:4996)  // deprecated API calls
#pragma warning(disable:28159) // more deprecated API calls
#endif

// Missing from MSVC6 setupapi.h
#ifndef SPDRP_ADDRESS
#define SPDRP_ADDRESS		28
#endif
#ifndef SPDRP_INSTALL_STATE
#define SPDRP_INSTALL_STATE	34
#endif

#define MAX_CTRL_BUFFER_LENGTH	4096
#define MAX_USB_DEVICES		256
#define MAX_USB_STRING_LENGTH	128
#define MAX_HID_REPORT_SIZE	1024
#define MAX_HID_DESCRIPTOR_SIZE	256
#define MAX_GUID_STRING_LENGTH	40
#define MAX_PATH_LENGTH		128
#define MAX_KEY_LENGTH		256
#define LIST_SEPARATOR		';'

// Handle code for HID interface that have been claimed ("dibs")
#define INTERFACE_CLAIMED	((HANDLE)(intptr_t)0xD1B5)
// Additional return code for HID operations that completed synchronously
#define LIBUSB_COMPLETED	(LIBUSB_SUCCESS + 1)

/*
 * Multiple USB API backend support
 */
#define USB_API_UNSUPPORTED	0
#define USB_API_HUB		1
#define USB_API_COMPOSITE	2
#define USB_API_WINUSBX		3
#define USB_API_HID		4
#define USB_API_MAX		5
// The following is used to indicate if the HID or composite extra props have already been set.
#define USB_API_SET		(1 << USB_API_MAX)

// Sub-APIs for WinUSB-like driver APIs (WinUSB, libusbK, libusb-win32 through the libusbK DLL)
// Must have the same values as the KUSB_DRVID enum from libusbk.h
#define SUB_API_NOTSET		-1
#define SUB_API_LIBUSBK		0
#define SUB_API_LIBUSB0		1
#define SUB_API_WINUSB		2
#define SUB_API_MAX		3

#define WINUSBX_DRV_NAMES	{ "libusbK", "libusb0", "WinUSB" }

struct windows_usb_api_backend {
	const uint8_t id;
	const char *designation;
	const char **driver_name_list; // Driver name, without .sys, e.g. "usbccgp"
	const uint8_t nb_driver_names;
	int (*init)(int sub_api, struct libusb_context *ctx);
	int (*exit)(int sub_api);
	int (*open)(int sub_api, struct libusb_device_handle *dev_handle);
	void (*close)(int sub_api, struct libusb_device_handle *dev_handle);
	int (*configure_endpoints)(int sub_api, struct libusb_device_handle *dev_handle, int iface);
	int (*claim_interface)(int sub_api, struct libusb_device_handle *dev_handle, int iface);
	int (*set_interface_altsetting)(int sub_api, struct libusb_device_handle *dev_handle, int iface, int altsetting);
	int (*release_interface)(int sub_api, struct libusb_device_handle *dev_handle, int iface);
	int (*clear_halt)(int sub_api, struct libusb_device_handle *dev_handle, unsigned char endpoint);
	int (*reset_device)(int sub_api, struct libusb_device_handle *dev_handle);
	int (*submit_bulk_transfer)(int sub_api, struct usbi_transfer *itransfer);
	int (*submit_iso_transfer)(int sub_api, struct usbi_transfer *itransfer);
	int (*submit_control_transfer)(int sub_api, struct usbi_transfer *itransfer);
	int (*abort_control)(int sub_api, struct usbi_transfer *itransfer);
	int (*abort_transfers)(int sub_api, struct usbi_transfer *itransfer);
	int (*copy_transfer_data)(int sub_api, struct usbi_transfer *itransfer, uint32_t io_size);
};

#define PRINT_UNSUPPORTED_API(fname)				\
	usbi_dbg("unsupported API call for '"			\
		#fname "' (unrecognized device driver)");	\
	return LIBUSB_ERROR_NOT_SUPPORTED;

/*
 * private structures definition
 * with inline pseudo constructors/destructors
 */

// TODO (v2+): move hid desc to libusb.h?
struct libusb_hid_descriptor {
	uint8_t bLength;
	uint8_t bDescriptorType;
	uint16_t bcdHID;
	uint8_t bCountryCode;
	uint8_t bNumDescriptors;
	uint8_t bClassDescriptorType;
	uint16_t wClassDescriptorLength;
};

#define LIBUSB_DT_HID_SIZE		9
#define HID_MAX_CONFIG_DESC_SIZE (LIBUSB_DT_CONFIG_SIZE + LIBUSB_DT_INTERFACE_SIZE \
	+ LIBUSB_DT_HID_SIZE + 2 * LIBUSB_DT_ENDPOINT_SIZE)
#define HID_MAX_REPORT_SIZE		1024
#define HID_IN_EP			0x81
#define HID_OUT_EP			0x02
#define LIBUSB_REQ_RECIPIENT(request_type)	((request_type) & 0x1F)
#define LIBUSB_REQ_TYPE(request_type)		((request_type) & (0x03 << 5))
#define LIBUSB_REQ_IN(request_type)		((request_type) & LIBUSB_ENDPOINT_IN)
#define LIBUSB_REQ_OUT(request_type)		(!LIBUSB_REQ_IN(request_type))

#ifndef METHOD_BUFFERED
#define METHOD_BUFFERED				0
#endif
#ifndef METHOD_IN_DIRECT
#define METHOD_IN_DIRECT			1
#endif
#ifndef METHOD_OUT_DIRECT
#define METHOD_OUT_DIRECT			2
#endif
#ifndef METHOD_NEITHER
#define METHOD_NEITHER				3
#endif
#ifndef FILE_ANY_ACCESS
#define FILE_ANY_ACCESS				0x00000000
#endif
#ifndef FILE_DEVICE_UNKNOWN
#define FILE_DEVICE_UNKNOWN			0x00000022
#endif
#ifndef FILE_DEVICE_KEYBOARD
#define FILE_DEVICE_KEYBOARD			0x0000000b
#endif
#ifndef FILE_DEVICE_USB
#define FILE_DEVICE_USB				FILE_DEVICE_UNKNOWN
#endif

#ifndef CTL_CODE
#define CTL_CODE(DeviceType, Function, Method, Access) \
	(((DeviceType) << 16) | ((Access) << 14) | ((Function) << 2) | (Method))
#endif

// The following are used for HID reports IOCTLs
#define HID_CTL_CODE(id) \
	CTL_CODE(FILE_DEVICE_KEYBOARD, (id), METHOD_NEITHER, FILE_ANY_ACCESS)
#define HID_BUFFER_CTL_CODE(id) \
	CTL_CODE(FILE_DEVICE_KEYBOARD, (id), METHOD_BUFFERED, FILE_ANY_ACCESS)
#define HID_IN_CTL_CODE(id) \
	CTL_CODE(FILE_DEVICE_KEYBOARD, (id), METHOD_IN_DIRECT, FILE_ANY_ACCESS)
#define HID_OUT_CTL_CODE(id) \
	CTL_CODE(FILE_DEVICE_KEYBOARD, (id), METHOD_OUT_DIRECT, FILE_ANY_ACCESS)

#define IOCTL_HID_GET_FEATURE		HID_OUT_CTL_CODE(100)
#define IOCTL_HID_GET_INPUT_REPORT	HID_OUT_CTL_CODE(104)
#define IOCTL_HID_SET_FEATURE		HID_IN_CTL_CODE(100)
#define IOCTL_HID_SET_OUTPUT_REPORT	HID_IN_CTL_CODE(101)

enum libusb_hid_request_type {
	HID_REQ_GET_REPORT = 0x01,
	HID_REQ_GET_IDLE = 0x02,
	HID_REQ_GET_PROTOCOL = 0x03,
	HID_REQ_SET_REPORT = 0x09,
	HID_REQ_SET_IDLE = 0x0A,
	HID_REQ_SET_PROTOCOL = 0x0B
};

enum libusb_hid_report_type {
	HID_REPORT_TYPE_INPUT = 0x01,
	HID_REPORT_TYPE_OUTPUT = 0x02,
	HID_REPORT_TYPE_FEATURE = 0x03
};

struct hid_device_priv {
	uint16_t vid;
	uint16_t pid;
	uint8_t config;
	uint8_t nb_interfaces;
	bool uses_report_ids[3]; // input, ouptput, feature
	uint16_t input_report_size;
	uint16_t output_report_size;
	uint16_t feature_report_size;
	WCHAR string[3][MAX_USB_STRING_LENGTH];
	uint8_t string_index[3]; // man, prod, ser
};

// used to match a device driver (including filter drivers) against a supported API
struct driver_lookup {
	char list[MAX_KEY_LENGTH + 1]; // REG_MULTI_SZ list of services (driver) names
	const DWORD reg_prop;          // SPDRP registry key to use to retrieve list
	const char* designation;       // internal designation (for debug output)
};

/*
 * Windows DDK API definitions. Most of it copied from MinGW's includes
 */
typedef DWORD DEVNODE, DEVINST;
typedef DEVNODE *PDEVNODE, *PDEVINST;
typedef DWORD RETURN_TYPE;
typedef RETURN_TYPE CONFIGRET;

#define CR_SUCCESS				0x00000000

#ifndef USB_GET_DESCRIPTOR_FROM_NODE_CONNECTION
#define USB_GET_DESCRIPTOR_FROM_NODE_CONNECTION	260
#endif
#ifndef USB_GET_NODE_CONNECTION_INFORMATION_EX
#define USB_GET_NODE_CONNECTION_INFORMATION_EX	274
#endif
#ifndef USB_GET_NODE_CONNECTION_INFORMATION_EX_V2
#define USB_GET_NODE_CONNECTION_INFORMATION_EX_V2	279
#endif

#define USB_CTL_CODE(id) \
	CTL_CODE(FILE_DEVICE_USB, (id), METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_USB_GET_DESCRIPTOR_FROM_NODE_CONNECTION	USB_CTL_CODE(USB_GET_DESCRIPTOR_FROM_NODE_CONNECTION)
#define IOCTL_USB_GET_NODE_CONNECTION_INFORMATION_EX	USB_CTL_CODE(USB_GET_NODE_CONNECTION_INFORMATION_EX)
#define IOCTL_USB_GET_NODE_CONNECTION_INFORMATION_EX_V2	USB_CTL_CODE(USB_GET_NODE_CONNECTION_INFORMATION_EX_V2)

typedef enum USB_CONNECTION_STATUS {
	NoDeviceConnected,
	DeviceConnected,
	DeviceFailedEnumeration,
	DeviceGeneralFailure,
	DeviceCausedOvercurrent,
	DeviceNotEnoughPower,
	DeviceNotEnoughBandwidth,
	DeviceHubNestedTooDeeply,
	DeviceInLegacyHub
} USB_CONNECTION_STATUS, *PUSB_CONNECTION_STATUS;

typedef enum USB_HUB_NODE {
	UsbHub,
	UsbMIParent
} USB_HUB_NODE;

// Most of the structures below need to be packed
#include <pshpack1.h>

typedef struct _USB_DESCRIPTOR_REQUEST {
	ULONG ConnectionIndex;
	struct {
		UCHAR bmRequest;
		UCHAR bRequest;
		USHORT wValue;
		USHORT wIndex;
		USHORT wLength;
	} SetupPacket;
//	UCHAR Data[0];
} USB_DESCRIPTOR_REQUEST, *PUSB_DESCRIPTOR_REQUEST;

typedef struct _USB_CONFIGURATION_DESCRIPTOR_SHORT {
	USB_DESCRIPTOR_REQUEST req;
	USB_CONFIGURATION_DESCRIPTOR desc;
} USB_CONFIGURATION_DESCRIPTOR_SHORT;

typedef struct USB_INTERFACE_DESCRIPTOR {
	UCHAR bLength;
	UCHAR bDescriptorType;
	UCHAR bInterfaceNumber;
	UCHAR bAlternateSetting;
	UCHAR bNumEndpoints;
	UCHAR bInterfaceClass;
	UCHAR bInterfaceSubClass;
	UCHAR bInterfaceProtocol;
	UCHAR iInterface;
} USB_INTERFACE_DESCRIPTOR, *PUSB_INTERFACE_DESCRIPTOR;

typedef struct _USB_NODE_CONNECTION_INFORMATION_EX {
	ULONG ConnectionIndex;
	USB_DEVICE_DESCRIPTOR DeviceDescriptor;
	UCHAR CurrentConfigurationValue;
	UCHAR Speed;
	BOOLEAN DeviceIsHub;
	USHORT DeviceAddress;
	ULONG NumberOfOpenPipes;
	USB_CONNECTION_STATUS ConnectionStatus;
//	USB_PIPE_INFO PipeList[0];
} USB_NODE_CONNECTION_INFORMATION_EX, *PUSB_NODE_CONNECTION_INFORMATION_EX;

typedef union _USB_PROTOCOLS {
	ULONG ul;
	struct {
		ULONG Usb110:1;
		ULONG Usb200:1;
		ULONG Usb300:1;
		ULONG ReservedMBZ:29;
	};
} USB_PROTOCOLS, *PUSB_PROTOCOLS;

typedef union _USB_NODE_CONNECTION_INFORMATION_EX_V2_FLAGS {
	ULONG ul;
	struct {
		ULONG DeviceIsOperatingAtSuperSpeedOrHigher:1;
		ULONG DeviceIsSuperSpeedCapableOrHigher:1;
		ULONG ReservedMBZ:30;
	};
} USB_NODE_CONNECTION_INFORMATION_EX_V2_FLAGS, *PUSB_NODE_CONNECTION_INFORMATION_EX_V2_FLAGS;

typedef struct _USB_NODE_CONNECTION_INFORMATION_EX_V2 {
	ULONG ConnectionIndex;
	ULONG Length;
	USB_PROTOCOLS SupportedUsbProtocols;
	USB_NODE_CONNECTION_INFORMATION_EX_V2_FLAGS Flags;
} USB_NODE_CONNECTION_INFORMATION_EX_V2, *PUSB_NODE_CONNECTION_INFORMATION_EX_V2;

#include <poppack.h>

/* winusb.dll interface */

#define SHORT_PACKET_TERMINATE	0x01
#define AUTO_CLEAR_STALL	0x02
#define PIPE_TRANSFER_TIMEOUT	0x03
#define IGNORE_SHORT_PACKETS	0x04
#define ALLOW_PARTIAL_READS	0x05
#define AUTO_FLUSH		0x06
#define RAW_IO			0x07
#define MAXIMUM_TRANSFER_SIZE	0x08
#define AUTO_SUSPEND		0x81
#define SUSPEND_DELAY		0x83
#define DEVICE_SPEED		0x01
#define LowSpeed		0x01
#define FullSpeed		0x02
#define HighSpeed		0x03

typedef enum _USBD_PIPE_TYPE {
	UsbdPipeTypeControl,
	UsbdPipeTypeIsochronous,
	UsbdPipeTypeBulk,
	UsbdPipeTypeInterrupt
} USBD_PIPE_TYPE;

typedef struct _WINUSB_PIPE_INFORMATION {
	USBD_PIPE_TYPE PipeType;
	UCHAR PipeId;
	USHORT MaximumPacketSize;
	UCHAR Interval;
} WINUSB_PIPE_INFORMATION, *PWINUSB_PIPE_INFORMATION;

#include <pshpack1.h>

typedef struct _WINUSB_SETUP_PACKET {
	UCHAR RequestType;
	UCHAR Request;
	USHORT Value;
	USHORT Index;
	USHORT Length;
} WINUSB_SETUP_PACKET, *PWINUSB_SETUP_PACKET;

#include <poppack.h>

typedef void *WINUSB_INTERFACE_HANDLE, *PWINUSB_INTERFACE_HANDLE;

typedef BOOL (WINAPI *WinUsb_AbortPipe_t)(
	WINUSB_INTERFACE_HANDLE InterfaceHandle,
	UCHAR PipeID
);
typedef BOOL (WINAPI *WinUsb_ControlTransfer_t)(
	WINUSB_INTERFACE_HANDLE InterfaceHandle,
	WINUSB_SETUP_PACKET SetupPacket,
	PUCHAR Buffer,
	ULONG BufferLength,
	PULONG LengthTransferred,
	LPOVERLAPPED Overlapped
);
typedef BOOL (WINAPI *WinUsb_FlushPipe_t)(
	WINUSB_INTERFACE_HANDLE InterfaceHandle,
	UCHAR PipeID
);
typedef BOOL (WINAPI *WinUsb_Free_t)(
	WINUSB_INTERFACE_HANDLE InterfaceHandle
);
typedef BOOL (WINAPI *WinUsb_GetAssociatedInterface_t)(
	WINUSB_INTERFACE_HANDLE InterfaceHandle,
	UCHAR AssociatedInterfaceIndex,
	PWINUSB_INTERFACE_HANDLE AssociatedInterfaceHandle
);
typedef BOOL (WINAPI *WinUsb_GetCurrentAlternateSetting_t)(
	WINUSB_INTERFACE_HANDLE InterfaceHandle,
	PUCHAR AlternateSetting
);
typedef BOOL (WINAPI *WinUsb_GetDescriptor_t)(
	WINUSB_INTERFACE_HANDLE InterfaceHandle,
	UCHAR DescriptorType,
	UCHAR Index,
	USHORT LanguageID,
	PUCHAR Buffer,
	ULONG BufferLength,
	PULONG LengthTransferred
);
typedef BOOL (WINAPI *WinUsb_GetOverlappedResult_t)(
	WINUSB_INTERFACE_HANDLE InterfaceHandle,
	LPOVERLAPPED lpOverlapped,
	LPDWORD lpNumberOfBytesTransferred,
	BOOL bWait
);
typedef BOOL (WINAPI *WinUsb_GetPipePolicy_t)(
	WINUSB_INTERFACE_HANDLE InterfaceHandle,
	UCHAR PipeID,
	ULONG PolicyType,
	PULONG ValueLength,
	PVOID Value
);
typedef BOOL (WINAPI *WinUsb_GetPowerPolicy_t)(
	WINUSB_INTERFACE_HANDLE InterfaceHandle,
	ULONG PolicyType,
	PULONG ValueLength,
	PVOID Value
);
typedef BOOL (WINAPI *WinUsb_Initialize_t)(
	HANDLE DeviceHandle,
	PWINUSB_INTERFACE_HANDLE InterfaceHandle
);
typedef BOOL (WINAPI *WinUsb_QueryDeviceInformation_t)(
	WINUSB_INTERFACE_HANDLE InterfaceHandle,
	ULONG InformationType,
	PULONG BufferLength,
	PVOID Buffer
);
typedef BOOL (WINAPI *WinUsb_QueryInterfaceSettings_t)(
	WINUSB_INTERFACE_HANDLE InterfaceHandle,
	UCHAR AlternateSettingNumber,
	PUSB_INTERFACE_DESCRIPTOR UsbAltInterfaceDescriptor
);
typedef BOOL (WINAPI *WinUsb_QueryPipe_t)(
	WINUSB_INTERFACE_HANDLE InterfaceHandle,
	UCHAR AlternateInterfaceNumber,
	UCHAR PipeIndex,
	PWINUSB_PIPE_INFORMATION PipeInformation
);
typedef BOOL (WINAPI *WinUsb_ReadPipe_t)(
	WINUSB_INTERFACE_HANDLE InterfaceHandle,
	UCHAR PipeID,
	PUCHAR Buffer,
	ULONG BufferLength,
	PULONG LengthTransferred,
	LPOVERLAPPED Overlapped
);
typedef BOOL (WINAPI *WinUsb_ResetPipe_t)(
	WINUSB_INTERFACE_HANDLE InterfaceHandle,
	UCHAR PipeID
);
typedef BOOL (WINAPI *WinUsb_SetCurrentAlternateSetting_t)(
	WINUSB_INTERFACE_HANDLE InterfaceHandle,
	UCHAR AlternateSetting
);
typedef BOOL (WINAPI *WinUsb_SetPipePolicy_t)(
	WINUSB_INTERFACE_HANDLE InterfaceHandle,
	UCHAR PipeID,
	ULONG PolicyType,
	ULONG ValueLength,
	PVOID Value
);
typedef BOOL (WINAPI *WinUsb_SetPowerPolicy_t)(
	WINUSB_INTERFACE_HANDLE InterfaceHandle,
	ULONG PolicyType,
	ULONG ValueLength,
	PVOID Value
);
typedef BOOL (WINAPI *WinUsb_WritePipe_t)(
	WINUSB_INTERFACE_HANDLE InterfaceHandle,
	UCHAR PipeID,
	PUCHAR Buffer,
	ULONG BufferLength,
	PULONG LengthTransferred,
	LPOVERLAPPED Overlapped
);
typedef BOOL (WINAPI *WinUsb_ResetDevice_t)(
	WINUSB_INTERFACE_HANDLE InterfaceHandle
);

/* /!\ These must match the ones from the official libusbk.h */
typedef enum _KUSB_FNID {
	KUSB_FNID_Init,
	KUSB_FNID_Free,
	KUSB_FNID_ClaimInterface,
	KUSB_FNID_ReleaseInterface,
	KUSB_FNID_SetAltInterface,
	KUSB_FNID_GetAltInterface,
	KUSB_FNID_GetDescriptor,
	KUSB_FNID_ControlTransfer,
	KUSB_FNID_SetPowerPolicy,
	KUSB_FNID_GetPowerPolicy,
	KUSB_FNID_SetConfiguration,
	KUSB_FNID_GetConfiguration,
	KUSB_FNID_ResetDevice,
	KUSB_FNID_Initialize,
	KUSB_FNID_SelectInterface,
	KUSB_FNID_GetAssociatedInterface,
	KUSB_FNID_Clone,
	KUSB_FNID_QueryInterfaceSettings,
	KUSB_FNID_QueryDeviceInformation,
	KUSB_FNID_SetCurrentAlternateSetting,
	KUSB_FNID_GetCurrentAlternateSetting,
	KUSB_FNID_QueryPipe,
	KUSB_FNID_SetPipePolicy,
	KUSB_FNID_GetPipePolicy,
	KUSB_FNID_ReadPipe,
	KUSB_FNID_WritePipe,
	KUSB_FNID_ResetPipe,
	KUSB_FNID_AbortPipe,
	KUSB_FNID_FlushPipe,
	KUSB_FNID_IsoReadPipe,
	KUSB_FNID_IsoWritePipe,
	KUSB_FNID_GetCurrentFrameNumber,
	KUSB_FNID_GetOverlappedResult,
	KUSB_FNID_GetProperty,
	KUSB_FNID_COUNT,
} KUSB_FNID;

typedef struct _KLIB_VERSION {
	INT Major;
	INT Minor;
	INT Micro;
	INT Nano;
} KLIB_VERSION;
typedef KLIB_VERSION* PKLIB_VERSION;

typedef BOOL (WINAPI *LibK_GetProcAddress_t)(
	PVOID *ProcAddress,
	ULONG DriverID,
	ULONG FunctionID
);

typedef VOID (WINAPI *LibK_GetVersion_t)(
	PKLIB_VERSION Version
);

struct winusb_interface {
	bool initialized;
	WinUsb_AbortPipe_t AbortPipe;
	WinUsb_ControlTransfer_t ControlTransfer;
	WinUsb_FlushPipe_t FlushPipe;
	WinUsb_Free_t Free;
	WinUsb_GetAssociatedInterface_t GetAssociatedInterface;
	WinUsb_GetCurrentAlternateSetting_t GetCurrentAlternateSetting;
	WinUsb_GetDescriptor_t GetDescriptor;
	WinUsb_GetOverlappedResult_t GetOverlappedResult;
	WinUsb_GetPipePolicy_t GetPipePolicy;
	WinUsb_GetPowerPolicy_t GetPowerPolicy;
	WinUsb_Initialize_t Initialize;
	WinUsb_QueryDeviceInformation_t QueryDeviceInformation;
	WinUsb_QueryInterfaceSettings_t QueryInterfaceSettings;
	WinUsb_QueryPipe_t QueryPipe;
	WinUsb_ReadPipe_t ReadPipe;
	WinUsb_ResetPipe_t ResetPipe;
	WinUsb_SetCurrentAlternateSetting_t SetCurrentAlternateSetting;
	WinUsb_SetPipePolicy_t SetPipePolicy;
	WinUsb_SetPowerPolicy_t SetPowerPolicy;
	WinUsb_WritePipe_t WritePipe;
	WinUsb_ResetDevice_t ResetDevice;
};

/* hid.dll interface */

#define HIDP_STATUS_SUCCESS	0x110000
typedef void * PHIDP_PREPARSED_DATA;

#include <pshpack1.h>

typedef struct _HIDD_ATTIRBUTES {
	ULONG Size;
	USHORT VendorID;
	USHORT ProductID;
	USHORT VersionNumber;
} HIDD_ATTRIBUTES, *PHIDD_ATTRIBUTES;

#include <poppack.h>

typedef USHORT USAGE;
typedef struct _HIDP_CAPS {
	USAGE Usage;
	USAGE UsagePage;
	USHORT InputReportByteLength;
	USHORT OutputReportByteLength;
	USHORT FeatureReportByteLength;
	USHORT Reserved[17];
	USHORT NumberLinkCollectionNodes;
	USHORT NumberInputButtonCaps;
	USHORT NumberInputValueCaps;
	USHORT NumberInputDataIndices;
	USHORT NumberOutputButtonCaps;
	USHORT NumberOutputValueCaps;
	USHORT NumberOutputDataIndices;
	USHORT NumberFeatureButtonCaps;
	USHORT NumberFeatureValueCaps;
	USHORT NumberFeatureDataIndices;
} HIDP_CAPS, *PHIDP_CAPS;

typedef enum _HIDP_REPORT_TYPE {
	HidP_Input,
	HidP_Output,
	HidP_Feature
} HIDP_REPORT_TYPE;

typedef struct _HIDP_VALUE_CAPS {
	USAGE UsagePage;
	UCHAR ReportID;
	BOOLEAN IsAlias;
	USHORT BitField;
	USHORT LinkCollection;
	USAGE LinkUsage;
	USAGE LinkUsagePage;
	BOOLEAN IsRange;
	BOOLEAN IsStringRange;
	BOOLEAN IsDesignatorRange;
	BOOLEAN IsAbsolute;
	BOOLEAN HasNull;
	UCHAR Reserved;
	USHORT BitSize;
	USHORT ReportCount;
	USHORT Reserved2[5];
	ULONG UnitsExp;
	ULONG Units;
	LONG LogicalMin, LogicalMax;
	LONG PhysicalMin, PhysicalMax;
	union {
		struct {
			USAGE UsageMin, UsageMax;
			USHORT StringMin, StringMax;
			USHORT DesignatorMin, DesignatorMax;
			USHORT DataIndexMin, DataIndexMax;
		} Range;
		struct {
			USAGE Usage, Reserved1;
			USHORT StringIndex, Reserved2;
			USHORT DesignatorIndex, Reserved3;
			USHORT DataIndex, Reserved4;
		} NotRange;
	} u;
} HIDP_VALUE_CAPS, *PHIDP_VALUE_CAPS;
