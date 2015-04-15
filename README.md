# IOSync
IOSync is a console-application which allows you to synchronize input events from several computers to a single host machine.
The main purpose of this application is to send keyboard and gamepad messages to a remote machine, in order to play local multiplayer games over the internet.

Couple this with a quick enough streaming solution (See: ["Streaming (Audio and Video)"](https://github.com/Regal-Internet-Brothers/IOSync/wiki/Streaming-(Audio-and-Video))), and any games that support keyboard input, and you should have an experience almost as good as [Steam's in-home streaming](http://store.steampowered.com/streaming). The catch is, this can be done over the internet, rather than a home network. No VPN required, just a bit of setup, and a decent internet connection. Setting up audio and video solutions can be a daunting task for some, but the outcome can definitely be worth it. This isn't a replacement for "in-home streaming", but it is a replacement for tricking Steam with a VPN service. This project may not be as elegant as Valve's solutions, but it has some major benefits.

This is an early version of the project; some features haven't been thoroughly tested. This repository is mainly meant to "showcase" the source code thus far. If you would like to help, feel free to make a pull request. Just realize that I may fix the formatting of submissions.

**Supported "devices":**
* Keyboard (I/O)
* Virtual 'XInput' device integration. (Experimental; I/O)

**TO-DO List:**
* Rumble simulation for 'XInput' gamepads.
* Explicit 'XInput' gamepad/device support synchronization.
* DirectInput integration.

##Getting Started##

###Using IOSync:###
Simply open the IOSync binary you've retrieved (Either through an external distribution, or compiling it yourself), then enter the information requested. In order to host a server with IOSync, you will need to open at least one port in your router's firewall (UDP). This can be done by using your router's configuration functionality, or if your router supports UPnP, you can use tools like ['portmapper'](https://github.com/kaklakariada/portmapper) (By [kaklakariada](https://github.com/kaklakariada)). That particular solution requires you to install [an appropriate Java runtime](http://www.oracle.com/technetwork/java/javase/downloads/jre8-downloads-2133155.html). Of course, this tool can also be useful for other applications. Under some situations, port forwarding may not be required. In addition, **clients should not be required to open ports for their firewalls**, most routers will allow for UDP I/O as long as their end initiates the first packet (Connecting to the remote machine). Once the host machine has their desired port forwarded, they may use it to initiate the host application. The default port (Specified by typing "default") is currently 5029; this port does not need to be used, however, it is an option. Clients simply need the IP address (Currently IPV4-only) of the remote machine, and the port it will be using. The host can find its IP address a number of ways, but the easiest is probably ["whatismyip.com"](http://www.whatismyip.com).

IOSync works immediately when a server is started. This means any client can connect and have **direct control** over the host machine's keyboard. This includes system keys. The ideal setup for remote-managed local-multiplayer games is to either, map controls to portions of the keyboard, or to utilize the experimental 'XInput' functionality.

####Virtual 'XInput' Gamepad Support:####
'XInput' integration is partially automated by default. If a local controller is plugged in, IOSync will see it, and then immediately attempt to connect the virtual device to the remote machine. Virtual 'XInput' devices are only available to programs which use 'XInput', and are able to be easily "injected" into. Injection has not been standardized, so it is not an explicit feature. Despite this, the latest versions of IOSync may request a "PID" when starting up. You may use this to inject one of the "injector" DLLs/modules. The exact module used is dependent on the process's architecture/platform. This will be resolved automatically. In the event remote injection does not work well (Actively changing module/memory state, original module gets unloaded, etc), you may also use the "injection modules" as wrappers/"shims" for the executable directly. This can be done by copying and/or moving the appropriate DLL (Correct architecture; probably x86) into the same folder as the targeted executable, then renaming the module to be the correct 'XInput' module-name ("xinput1_X", "xinput9_1_0.dll", etc). If the program does not use XInput, injection will not work. Injection has also not been tested with DotNET processes; use with caution. As the license says, I'm not held responsible for what happens when you use this software.

###Streaming (Audio and Video; Gameplay):###
* [Please visit the wiki page regarding streaming](https://github.com/Regal-Internet-Brothers/IOSync/wiki/Streaming-(Audio-and-Video)).

###Compiling the project(s):###
This repository only provides source code at this time. This means you're going to have to compile the project.
This project is currently Windows-only, and it has only been tested with Visual Studio 2013 Community Edition. Builds should work for both x86 and x64.
The only external dependency this project has is 'QuickSock'. [The experimental version in the 'QuickLib' repository should work](https://github.com/Regal-Internet-Brothers/QuickLib). That being said, all you should need is Visual Studio 2013 (Community Edition).

Building the injection/wrapper DLLs is pretty straight forward, as long as you have MSVC 2013, then the project(s) should compile.

####Other Notes:####
* DO NOT move any of the injection DLLs/modules into your system-folder ("System32"/"SysWOW64", etc) on Windows. You probably won't get any issues, but if you rename an injection-DLL to one of the 'XInput' DLL names, you could run into problems. If you're going to copy over an injection DLL/module, please copy to the actual location, not your system-folder.
* Some emulators (Dolphin specifically) have very 'dynamic' ways of managing 'XInput'. Remote DLL injection may not work well with these applications. It's best to simply move over the appropriate injection DLL, then rename it.

#####Special Thanks:#####
* Matthew Diamond (For initially helping with DLL injection)
* MainMemory (For his really basic "jump injection" command; [original source here](https://github.com/sonicretro/sadx-mod-loader/blob/49cbca9ffecbcdc9541ac63ed2bc88ae52bfcfbf/include/ModLoader/MemAccess.h#L148))
* SonicFreak94 (Michael), and Kirbeh (Tristan) for helping with testing.

##History##
IOSync was a project I started a few years ago in BlitzMax using the old 'keybd_event' Windows API command, and some hacked together input detection.
Today, it's not a lot better, but at least it's solid from a networking front.
The original project used MaxGUI (BlitzMax's official GUI module) for basic windows and buttons.

At this time, this version of the project is completely command-line oriented.

##Technical##
This project currently only supports Microsoft Windows, with groundwork put in to support other backends and platforms down the road.
The entire "device framework" provided is completely capable of supporting other types of input and output devices.
Keyboard input is currently handled using both Microsoft's "Raw Input" API, and the 'SendInput' functionality.

All source code provided assumes C++11 as a minimum standard.
*This means 'auto' is used, and several standard library features are, or will be implemented.*

This software is being developed for Microsoft's Visual C++ compiler, support for other projects (Such as MinGW) has not been tested.

##Other##
This application relies on a (Currently experimental) version of the 'QuickSock' project (Networking).
IPV6 support is largely experimental and likely disabled at this time.

When initialized on Windows, the client-portion of this program will use the raw device-flag ['RIDEV_INPUTSINK'](https://msdn.microsoft.com/en-us/library/windows/desktop/ms645565%28v=vs.85%29.aspx), which allows for background-input detection.
This can be thought of as similar to key-logging; ALL keyboard messages will be forwarded to the remote host's system.
At this time, this includes keys such as the CTRL, ALT, and SHIFT keys.

When connecting to a host, you can rest assured that input is being detected, but not simulated.
However, all *hosts* simulate input at this time. Groundwork has been done to support intermediate network-nodes, which carry traffic back and forth. This has not been fully implemented at this time.

There is very little in the way of security at this time. Work may be done later to ensure better control over the device-simulation aspects of the program. Honestly, the mayhem of having 4+ people share a keyboard can actually be entertaining by itself.
