#include <ntifs.h>
#include <windef.h>
#include "pkill.h"

NTSTATUS DriverEntry(PDRIVER_OBJECT pDriverObject, PUNICODE_STRING pRegistryPath)
{
	UNREFERENCED_PARAMETER(pRegistryPath);

	NTSTATUS NtStatus = STATUS_SUCCESS;
	PDEVICE_OBJECT pDeviceObject = NULL;

	// DosDeviceName will be used to access the device from user mode.
	UNICODE_STRING DosDeviceName;
	RtlInitUnicodeString(&DosDeviceName, DOS_DEVICE_NAME);

	// Create the DeviceObject, an endpoint point for communication between user-mode and kernel-mode
	UNICODE_STRING DeviceName;
	RtlInitUnicodeString(&DeviceName, DEVICE_NAME);
	NtStatus = IoCreateDevice(pDriverObject, 0, &DeviceName, FILE_DEVICE_UNKNOWN,
		FILE_DEVICE_SECURE_OPEN, FALSE, &pDeviceObject);
	if (!NT_SUCCESS(NtStatus)) {
		return NtStatus;
	}

	// Set up the driver's dispatch table.
	for (DWORD index = 0; index < IRP_MJ_MAXIMUM_FUNCTION; ++index) {
		pDriverObject->MajorFunction[index] = UnsupportedFunction;
	}
	pDriverObject->DriverUnload = Unload;
	pDriverObject->MajorFunction[IRP_MJ_CREATE] = DispatchCreate;
	pDriverObject->MajorFunction[IRP_MJ_CLOSE] = DispatchClose;
	pDriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] = DispatchDeviceControl;

	// The I/O manager will use buffered I/O for I/O requests.
	pDeviceObject->Flags |= DO_BUFFERED_IO;
	// Mark device as being initialized.
	pDeviceObject->Flags &= (~DO_DEVICE_INITIALIZING);

	// Create a user-visible symbolic link.
	NtStatus = IoCreateSymbolicLink(&DosDeviceName, &DeviceName);
	if (!NT_SUCCESS(NtStatus)) {
		IoDeleteDevice(pDeviceObject);
	}

	return NtStatus;
}

VOID Unload(PDRIVER_OBJECT pDriverObject)
{
	UNICODE_STRING DosDeviceName;
	RtlInitUnicodeString(&DosDeviceName, DOS_DEVICE_NAME);
	if (!NT_SUCCESS(IoDeleteSymbolicLink(&DosDeviceName))) {
		DbgPrint("IoDeleteSymbolicLink failed!");
	}
	IoDeleteDevice(pDriverObject->DeviceObject);
}

NTSTATUS DispatchCreate(PDEVICE_OBJECT pDeviceObject, PIRP pIrp)
{
	UNREFERENCED_PARAMETER(pDeviceObject);
	UNREFERENCED_PARAMETER(pIrp);
	return STATUS_SUCCESS;
}

NTSTATUS DispatchClose(PDEVICE_OBJECT pDeviceObject, PIRP pIrp)
{
	UNREFERENCED_PARAMETER(pDeviceObject);
	UNREFERENCED_PARAMETER(pIrp);
	return STATUS_SUCCESS;
}

NTSTATUS DispatchDeviceControl(PDEVICE_OBJECT pDeviceObject, PIRP pIrp)
{
	UNREFERENCED_PARAMETER(pDeviceObject);

	NTSTATUS NtStatus = STATUS_SUCCESS;

	// Entry in the I/O stack associated with this IRP.
	PIO_STACK_LOCATION pIoStackLocation = IoGetCurrentIrpStackLocation(pIrp);
	PVOID pSystemBuffer = pIrp->AssociatedIrp.SystemBuffer;

	switch (pIoStackLocation->Parameters.DeviceIoControl.IoControlCode) {
	case IOCTL_PKILL_TERMINATE:
		if (pIoStackLocation->Parameters.DeviceIoControl.InputBufferLength == sizeof(DWORD)) {
			DWORD dwPid;
			RtlCopyMemory(&dwPid, pSystemBuffer, sizeof(dwPid));

			PEPROCESS eProcess = NULL;
			NtStatus = PsLookupProcessByProcessId((HANDLE)dwPid, &eProcess);
			if (NT_SUCCESS(NtStatus)) {
				__try {
					HANDLE proc = NULL;
					NtStatus = ObOpenObjectByPointer(eProcess, OBJ_KERNEL_HANDLE, NULL, KEY_ALL_ACCESS, NULL, KernelMode, &proc);
					if (NT_SUCCESS(NtStatus)) {
						NtStatus = ZwTerminateProcess(proc, STATUS_FAIL_FAST_EXCEPTION);
						ZwClose(proc);
					}
				} __except (EXCEPTION_EXECUTE_HANDLER) {
					NtStatus = STATUS_INTERNAL_ERROR;
				}
			}
		} else {
			NtStatus = STATUS_INVALID_PARAMETER_1;
		}
		break;
	default:
		NtStatus = STATUS_INVALID_PARAMETER_1;
		break;
	}

	RtlZeroMemory(pSystemBuffer, pIoStackLocation->Parameters.DeviceIoControl.InputBufferLength);
	if (!NT_SUCCESS(NtStatus)) {
		*(DWORD*)pSystemBuffer = (DWORD)1;
	}

	pIrp->IoStatus.Information = sizeof(DWORD);
	pIrp->IoStatus.Status = NtStatus;

	// Return the IRP to the I/O manager.
	IoCompleteRequest(pIrp, IO_NO_INCREMENT);

	return NtStatus;
}

NTSTATUS UnsupportedFunction(PDEVICE_OBJECT pDeviceObject, PIRP pIrp)
{
	UNREFERENCED_PARAMETER(pDeviceObject);
	UNREFERENCED_PARAMETER(pIrp);
	return STATUS_NOT_SUPPORTED;
}