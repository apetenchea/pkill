# pkill

Windows Driver that can kill any process

## Build

- First, you need to install Visual Studio. I have used Visual Studio 2019 version 16.11.16. Other versions might work as
well.  
- In order to build the driver component, you also need to install the Windows Driver Kit (WDK).  Here, I have
used [WDK for Windows 10, version 2004](https://go.microsoft.com/fwlink/?linkid=2128854)
(don't worry, it's not from the year 2004).  
- Check out Microsoft's
[other-wdk-downloads page](https://learn.microsoft.com/en-us/windows-hardware/drivers/other-wdk-downloads).

### Driver

- Load the `driver.sln` solution in Visual Studio.
- Set the configuration to `Release` and the platform to `x64`.
- Before building the solution, run `bcdedit.exe -set TESTSIGNING ON` in an elevated command prompt. This allows the
  driver to be loaded with a real signature, and is going to be automatically signed with a test certificate during the
  build process. In case you're using BitLocker, be sure to have your recovery key at hand before rebooting.

## Install

- Allow the loading of test-signed drivers `bcdedit.exe -set TESTSIGNING ON`.
- In case you're using BitLocker, be sure to have your recovery key at hand before rebooting.

### Load the driver
```
sc create pkill type=kernel binPath="path_to_your_driver.sys"
sc start pkill
```

### Kill a process
```
python3 pkill.py 1234
python3 pkill.py Notepad.exe
```

### Unload the driver
```
sc stop pkill
sc delete pkill
```

## References
- [apriorit.com](https://www.apriorit.com/dev-blog/791-driver-windows-driver-model)
