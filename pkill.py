import argparse
import ctypes
import psutil
import win32file


FILE_DEVICE_UNKNOWN = 0x00000022
METHOD_BUFFERED = 0
FILE_ACCESS = 0x0002 | 0x0001
IOCTL_PKILL_TERMINATE = (FILE_DEVICE_UNKNOWN << 16) | (FILE_ACCESS << 14) | (0x800 << 2) | METHOD_BUFFERED


def get_pid_by_process_name(process_name):
    for process in psutil.process_iter():
        process_info = process.as_dict(attrs=['pid', 'name'])
        if process_name == process_info['name']:
            return process_info['pid']
    return None

def main(args):
    if args.pid.isdigit():
        pid = int(args.pid)
    else:
        pid = get_pid_by_process_name(args.pid)
        if pid is None:
            print('Process not found')
            return

    handle = win32file.CreateFile(
        r'\\.\PKILL',
        win32file.GENERIC_READ | win32file.GENERIC_WRITE,
        0,
        None,
        win32file.OPEN_EXISTING,
        0,
        None)
    if handle is None or handle == win32file.INVALID_HANDLE_VALUE:
        print('Failed to open device')
        return

    pid = ctypes.c_ulong(pid)
    out_buffer = ctypes.c_ulong(0)
    bytes_returned = ctypes.c_ulong(0)
    status = ctypes.windll.kernel32.DeviceIoControl(
        handle.handle,
        IOCTL_PKILL_TERMINATE,
        ctypes.byref(pid), ctypes.sizeof(pid),
        ctypes.byref(out_buffer), ctypes.sizeof(out_buffer),
        ctypes.byref(bytes_returned),
        None
    )

    if status and out_buffer.value == 0:
        print("DeviceIoControl succeeded")
    else:
        print("DeviceIoControl failed:", ctypes.windll.kernel32.GetLastError())

    win32file.CloseHandle(handle)


if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument('pid', help='PID or name of the process to kill')
    main(parser.parse_args())
