# ![IOSync Logo](/IOSync/IOSync.ico) IOSync
IOSync is a console-application which allows you to synchronize input events from several computers to a single host machine.
The main purpose of this application is to send keyboard and gamepad messages to a remote machine, in order to play local multiplayer games over the internet.

This is an early version of the project; some features haven't been thoroughly tested. Compiled binaries may be found in the **["releases" section](https://github.com/Regal-Internet-Brothers/IOSync/releases)**; use at your own risk. If you would like to help, feel free to make a pull request. Just realize that I may fix the formatting of submissions.

Couple this with a quick enough streaming solution (See: ["Streaming (Audio and Video)"](https://github.com/Regal-Internet-Brothers/IOSync/wiki/Streaming-(Audio-and-Video))), and any games that support keyboard, or gamepad (XInput, DirectInput; see below) input, and you should have an experience almost as good as [Steam's in-home streaming](http://store.steampowered.com/streaming). The catch is, this can be done over the internet, rather than a home network. No VPN required, just a bit of setup, and a decent internet connection. Setting up audio and video solutions can be a daunting task for some, but the outcome can definitely be worth it. This isn't a replacement for "in-home streaming" solutions, but it is a replacement for tricking Steam with a VPN service. This project may not be as elegant as Valve's solutions, but it has some major benefits.

##Getting Started##

###Using IOSync:###
Simply open the IOSync binary you've retrieved (Compiled releases may be [found here](https://github.com/Regal-Internet-Brothers/IOSync/releases), or you may compile it yourself), then enter the information requested. In order to host a server with IOSync, you will need to open at least one port in your router's firewall (UDP). This can be done by using your router's configuration functionality, or if your router supports UPnP, you can use tools like ['portmapper'](https://github.com/kaklakariada/portmapper) (By [kaklakariada](https://github.com/kaklakariada)). That particular solution requires you to install [an appropriate Java runtime](http://www.oracle.com/technetwork/java/javase/downloads/jre8-downloads-2133155.html). Of course, that tool can also be useful for other applications. Under some situations, port forwarding may not be required. In addition, **clients should not be required to open ports for their firewalls**, most routers will allow for UDP I/O as long as their end initiates the first packet (Connecting to the remote machine). Once the host machine has their desired port forwarded, they may use it to initiate the host application. The default port (Specified by typing "default") is currently 5029; this port does not need to be used, however, it is an option. Clients simply need the IP address (Currently IPV4-only) of the remote machine, and the port it will be using. The host can find its IP address a number of ways, but the easiest is probably ["whatismyip.com"](http://www.whatismyip.com).

IOSync works immediately when a server is started. This means any client can connect and have **direct control** over the host machine's keyboard. This includes system keys. The ideal setup for remote-managed local-multiplayer games is to either, map controls to portions of the keyboard, or to utilize the gamepad functionality. For more information on restricting device-access, [click here](https://github.com/Regal-Internet-Brothers/IOSync/wiki/Configuring-IOSync).

####Virtual 'XInput' Gamepad Support:####
'XInput' integration is partially automated by default. If a local controller is plugged in, IOSync will see it, and then immediately attempt to connect the virtual device to the remote machine. Virtual 'XInput' devices are only available to programs which use 'XInput', and are able to be easily "injected" into. The latest versions of IOSync may request a "PID" when starting up. You may use this to inject one of the "injector" DLLs/modules. For details on injection options, [click here](https://github.com/Regal-Internet-Brothers/IOSync/wiki/Configuring-IOSync). The exact module used is dependent on the process's architecture/platform. This will be resolved automatically. In the event remote injection does not work well (Actively changing module/memory state, original module gets unloaded, etc), you may also use the "injection modules" as wrappers/"shims" for the executable directly. This can be done by copying and/or moving the appropriate DLL (Correct architecture; probably x86) into the same folder as the targeted executable, then renaming the module to be the correct 'XInput' module-name ("xinput1_X.dll", "xinput9_1_0.dll", etc). If the program does not use XInput, injection will not work. Other injection options are available, besides the initial injection request. As the license says, I'm not held responsible for what happens when you use this software.

*If you are having trouble getting IOSync to see your local gamepad, you may need to use an input alternative, like [x360ce](https://github.com/x360ce/x360ce)*.

#### vJoy Gamepad Support (DirectInput, etc):####
As of version 1.0.3, IOSync may be used to "feed" re-encoded data into vJoy for simulation purposes. vJoy is a DirectInput "simulation driver", which allows external applications to read and write to virtual devices. Unlike IOSync's XInput functionality, vJoy works at the kernel's level as a driver. This means injection is not needed. XInput injection is still the preferred method for gamepad functionality (If XInput is used), but a number of applications rely upon DirectInput and/or an alternative that does not work with XInput. By default, vJoy support is enabled, and will work using either a local interface-module, or your vJoy installation's interface-module. This process is automated. For details on configuring IOSync's vJoy functionality, [please view this page](https://github.com/Regal-Internet-Brothers/IOSync/wiki/Configuring-IOSync).

To utilize IOSync's vJoy functionality, **[you'll first need to download it](http://vjoystick.sourceforge.net/site/)**. Once you've downloaded and installed vJoy, run "vJoyConf" ("Configure vJoy"). From there, select a device-number, then click "Add Device". Once your device is connected, configuration is mostly up to you.

If you'd like to map an Xbox 360 controller (Or similar), then the number of buttons should be set to either 16 or 32 (Most buttons are unused, currently). To accuractely represent an Xbox 360 controller, you should ensure the **X**, **Y**, **Z**, **Rx**, and **Ry** axes are all enabled. IOSync should safely disregard disabled axes, in the event you decide to restrict or disable an axis. IOSync does not currently support the **Rz** axis, the *triggers* are currently mapped to the **Z** axis. IOSync currently supports both 4-directional and continuous POV modes. For most cases, **1 continuous POV switch** is ideal; IOSync does not support more than one at this time. All unmentioned axes supported by vJoy are unused or undocumented.

*Exact configuration procedures may change in the future*. As [stated here](http://vjoystick.sourceforge.net/site/index.php/download-a-install/80-install-x86x64), vJoy is now an officially signed driver.

###Streaming (Audio and Video; Gameplay):###
* [Please visit the wiki page regarding streaming](https://github.com/Regal-Internet-Brothers/IOSync/wiki/Streaming-(Audio-and-Video)).

###Compiling the project(s):###
This project provides both source code, and compiled binaries. Pre-compiled binaries may be found in the ["releases" section](https://github.com/Regal-Internet-Brothers/IOSync/releases) (Use at your own risk). If you wish to compile the project, read on.

This project is currently Windows-only. Compilation has only been attempted using [Visual Studio 2013 (Community Edition)](https://www.visualstudio.com/en-us/products/visual-studio-community-vs.aspx). Builds should work for both the x86 (8086), and x64 (AMD64) architectures. The only ***required*** external dependency this project has is 'QuickLib'. [The experimental version in the 'QuickLib' repository should work](https://github.com/Regal-Internet-Brothers/QuickLib). Optionally, IOSync may be built with vJoy support. In order to do this, you will need [the vJoy SDK](http://sourceforge.net/projects/vjoystick/files/Beta%202.x/SDK/). The current include-folder used for vJoy is "vJoy". IOSync does not statically link with an interface-module, however. Like the shared XInput code, vJoy is dynamically handled by dedicated wrapper-code. Using vJoy directly will result in problems at link-time, unless otherwise configured. By disabling 'GAMEPAD_VJOY_DYNAMIC_LINK' with the preprocessor, vJoy's interface-module will be statically linked. However, the existing wrapper-code should stil be used, as it will route calls accordingly.

Building the injection/wrapper DLLs is pretty straight forward, as long as you have the correct version of Visual Studio/MSVC, then the project(s) should compile. An appropriate XInput header is required to build IOSync. This should already be available from MSVC2013 by default.

####Other Notes:####
* **DO NOT** move any of the injection DLLs/modules into your system-folder ("System32"/"SysWOW64", etc) on Windows. You probably won't get any issues, but if you rename an injection-DLL to one of the 'XInput' DLL names, you could run into problems. If you're going to copy over an injection DLL/module, please copy to the actual location, not your system-folder.
* Some emulators (Dolphin specifically) have very 'dynamic' ways of managing 'XInput'. Remote DLL injection may not work well with these applications. Under such a situation, it's best to simply move over the appropriate injection DLL, then rename it. Please keep track of any injector-modules you move or rename. Compatibility issues are likely in the long run.

#####Special Thanks:#####
* Matthew Diamond (For initially helping with DLL injection, as well as making the logo)
* MainMemory (For his really basic "jump injection" command; [original source here](https://github.com/sonicretro/sadx-mod-loader/blob/49cbca9ffecbcdc9541ac63ed2bc88ae52bfcfbf/include/ModLoader/MemAccess.h#L148))
* SonicFreak94 (Michael), MetalSonicMK72 (Josh), and Kirbeh (Tristan) for helping with testing.

##History##
IOSync was a project I started a few years ago in BlitzMax using the old 'keybd_event' Windows API command, and some hacked together input detection.
Today, it's not a lot better, but at least it's solid from a networking front.
The original project used MaxGUI (BlitzMax's official GUI module) for basic windows and buttons.

At this time, this version of the project is completely command-line oriented.

##Technical##
This project currently only supports Microsoft Windows, with groundwork put in to support other backends and platforms down the road.

The entire "device framework" provided is completely capable of supporting other types of input and output devices.
Keyboard input is currently handled using both Microsoft's "Raw Input" API, and the 'SendInput' functionality. Gamepad input is detected using **XInput** (Some controllers may be unsupported; see below), and simulated using a custom, user-level (DLL) XInput injector. Optionally, newer versions of IOSync support vJoy for DirectInput device-simulation.

If your gamepad is unsupported by IOSync, you'll need to either use an XInput-compatible gamepad/controller, or use an alternative. One well regarded DirectInput-to-XInput alternative is [x360ce, a user-level alternative, similar to IOSync's injection functionality](https://github.com/x360ce/x360ce).

All source code provided assumes C++11 as a minimum standard.
*This means 'auto' is used, and several standard library features are, or will be implemented.*

This software is being developed for Microsoft's Visual C++ compiler, support for other C++ implementations (Such as MinGW) has not been tested.

##Other##
This application relies on a (Currently experimental) version of the 'QuickSock' project (Networking), as well as the QuickINI library.
IPV6 support is largely experimental and likely disabled at this time.

When initialized on Windows, the client-portion of this program will use the raw device-flag ['RIDEV_INPUTSINK'](https://msdn.microsoft.com/en-us/library/windows/desktop/ms645565%28v=vs.85%29.aspx), which allows for background-input detection.
This can be thought of as similar to key-logging; ALL keyboard messages will be forwarded to the remote host's system.
At this time, this includes keys such as the CTRL, ALT, and SHIFT keys. Keyboard scan codes are used for simulation, so most applications should be capable of reading remote keyboard-input.

When connecting to a host, you can rest assured that input is being detected, but not simulated.
However, *hosts* simulate input at this time. [Explicit "virtual device" configuration](https://github.com/Regal-Internet-Brothers/IOSync/wiki/Configuring-IOSync) may be used to change this behavior. Groundwork has been done to support intermediate network-nodes, which carry traffic back and forth. This has not been fully implemented at this time.

**Supported "devices":**
* Keyboard (I/O)
* Gamepads (I/O):
  * Virtual 'XInput' device integration. (I/O)
  * vJoy feeder support. (Optional; uses standard gamepad behavior)

**TO-DO List:**
* Rumble simulation for 'XInput' gamepads.
* Explicit 'XInput' gamepad/device support synchronization.
* DirectInput gamepad-detection support. (Clients)
