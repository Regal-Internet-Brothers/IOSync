# ![IOSync Logo](/IOSync/IOSync.ico) IOSync
IOSync is a console-application which allows you to synchronize input events from several computers to a single host machine.
The main purpose of this application is to send keyboard and gamepad messages to a remote machine, in order to play local multiplayer games over the internet. To learn how to get started, [click here](https://github.com/Regal-Internet-Brothers/IOSync/wiki).

This is an early version of the project; some features haven't been thoroughly tested. Compiled binaries may be found in the **["releases" section](https://github.com/Regal-Internet-Brothers/IOSync/releases)**; use at your own risk. If you would like to help, feel free to make a pull request. Just realize that I may fix the formatting of submissions.

Couple this with a quick enough streaming solution (See: ["Streaming (Audio and Video)"](https://github.com/Regal-Internet-Brothers/IOSync/wiki/Streaming-(Audio-and-Video))), and any games that support keyboard, or gamepad (XInput, DirectInput; see below) input, and you should have an experience almost as good as [Steam's in-home streaming](http://store.steampowered.com/streaming). The catch is, this can be done over the internet, rather than a home network. No VPN required, just a bit of setup, and a decent internet connection. Setting up audio and video solutions can be a daunting task for some, but the outcome can definitely be worth it. This isn't a replacement for "in-home streaming" solutions, but it is a replacement for tricking Steam with a VPN service. This project may not be as elegant as Valve's solutions, but it has some major benefits.

##Getting Started##
You can download the [latest versions from the "releases" section](https://github.com/Regal-Internet-Brothers/IOSync/releases). After downloading IOSync, please follow [the documentation, found on the wiki](https://github.com/Regal-Internet-Brothers/IOSync/wiki).

###Compiling the project(s):###
This project provides both source code, and compiled binaries. Pre-compiled binaries may be found in the ["releases" section](https://github.com/Regal-Internet-Brothers/IOSync/releases) (Use at your own risk). **If you wish to compile the project, read on. If not, [please read the wiki](https://github.com/Regal-Internet-Brothers/IOSync/wiki).**

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
