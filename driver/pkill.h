#pragma once

#include <ntddk.h>

// Called when the driver is loaded.
NTSTATUS DriverEntry(PDRIVER_OBJECT pDriverObject, PUNICODE_STRING pRegistryPath);
// Called when the driver is unloaded.
DRIVER_UNLOAD Unload;
// Called when a HANDLE is requested (using CreateFile, for example).
DRIVER_DISPATCH DispatchCreate;
// Called when the last HANDLE has been closed and released (CloseHandle).
DRIVER_DISPATCH DispatchClose;
// Called when a request containing an I/O control code is passed to the device driver.
DRIVER_DISPATCH DispatchDeviceControl;
// Used to indicate unsupported functionality.
DRIVER_DISPATCH UnsupportedFunction;

/*
 * INIT - procedure is needed only for initialization, after which it can be removed from memory.
 * PAGE - code can be paged when the device is not active.
 */
#pragma alloc_text(INIT, DriverEntry)
#pragma alloc_text(PAGE, Unload)
#pragma alloc_text(PAGE, DispatchCreate)
#pragma alloc_text(PAGE, DispatchClose)
#pragma alloc_text(PAGE, UnsupportedFunction)
#pragma alloc_text(PAGE, DispatchDeviceControl)

/*
 * Control code for killing a process.
 * The PID is passed as a DWORD. Returns 0 on success, 1 on failure.
 */
#define IOCTL_PKILL_TERMINATE CTL_CODE(FILE_DEVICE_UNKNOWN, 0x800, METHOD_BUFFERED, FILE_READ_DATA | FILE_WRITE_DATA)

#define DEVICE_NAME L"\\Device\\PKILL"
#define DOS_DEVICE_NAME L"\\DosDevices\\PKILL"