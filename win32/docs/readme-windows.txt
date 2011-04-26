Snes9x: The Portable Super Nintendo Entertainment System Emulator
=================================================================
Files included in the Snes9x archive:
  Snes9x.exe
  fmod.dll
  readme-windows.txt
  faqs-windows.txt
  changes.txt
  snes9x-license.txt

version 1.53  April, 2011
Home page: http://www.snes9x.com/



Contents
========

Introduction
Getting Started
Requirements
Controllers Support
Game Saving
Cheat Support
Movie Support
Netplay Support
Miscellaneous
Compatibility
Problems
Technical Information
Credits



Introduction
============

What is Snes9x?
---------------
Snes9x is a portable, freeware Super Nintendo Entertainment System (SNES)
emulator. It basically allows you to play most games designed for the SNES and
Super Famicom Nintendo game systems on your Mac, Linux, Windows and so on. The
games include some real gems that were only ever released in Japan.

The original Snes9x project was founded by Gary Henderson and Jerremy Koot as a
collaboration of their earlier attempts at SNES emulation (Snes96 and Snes97.)
Over the years the project has grown and has collected some of the greatest
talent in the emulation community (at least of the SNES variety) some of which
have been listed in the credits section, others have helped but have been loss
in the course of time.

Why Emulate the SNES?
---------------------
Well, there are many reasons for this. The main reason is for nostalgic
purposes. In this day and age, it's hard to find an SNES and many good games.
Plus, many of us over the course of time have lost our beloved consoles (may
they R.I.P) but still have our original carts. With no other means to play them,
we turn to emulators. Besides this there are many conveniences of doing this on
the computer instead of dragging out your old system.

Advantages consist of:
- ability to save in any location of the game, despite how the game was
  designed. It's amazingly useful when you don't want to redo the same level
  over and over.
- built-in peripherals. This is anything from multi-taps, to super scopes, to
  cheat devices.
- ability to rip sprites and music for your own personal use.
- easier to organize and no stacks of cartridges.
- filters can be used to enhance graphics and sounds on old games.

As with all things there are disadvantages though:
- if you have an ancient computer, you aren't likely to get a playable
  experience.
- some games are still unemulated (though this a very tiny minority.)
- the emulator can be difficult for new users to configure.



Getting Started
===============

Launch Snes9x using the Windows explorer to locate the directory where you
un-zipped the snes9x.exe and the fmod.dll files and double-click on the
snes9x.exe executable. You could create a shortcut to Snes9x and drag that icon
out onto your desktop.

Loading Games
-------------
Use the Open option from the File menu to open the ROM load dialog. The dialog
allows you to browse your computer to locate the directory where you have stored
your SNES games. Single-click and then press Load to load and start the game.

SNES ROM images come in lots of different formats. Snes9x supports zipped ROMs
as long as there is only 1 per zip file. Also Snes9x can open gzip and jma
compressed files.

Game Color System
-----------------
Snes9x displays the ROM information when a ROM is first loaded. Depending on the
colors used you can tell whether or not a ROM is a good working ROM, or if it's
been altered or is corrupted.

  white   the ROM should be a perfect working copy.
  green   the ROM is mode 1 interleaved.
  orange  the ROM is mode 2 interleaved.
  aqua    the ROM is Game Doctor 24M interleaved.
  yellow  the ROM has probably been altered. Either it's a translation, PD ROM,
          hacked, or possibly a bad ROM. It may also be an overdumped ROM.
  red     the ROM is definitely hacked and that a proper version should be
          exist. Some ROM Tools such as NSRT can also fix these ROMs.

When asking for help on the Snes9x forums, please list the color and CRC32 that
is displayed. This will help to find out what the problem is.

These colors do NOT signify whether a game will work or not. It is just a means
for reference so we can understand what may or may not be a problem. Most often
the problem with games that don't work it's because they are corrupt or are a
bad dump and should be redumped.

SNES Joypad Emulation
---------------------
The default key mapping for joypad 1 is as follows:

  'up arrow'     Up direction
  'down arrow'   Down direction
  'left arrow'   Left direction
  'right arrow'  Right direction
  'V'            A button
  'C'            B button
  'D'            X button
  'X'            Y button
  'A'            L button
  'S'            R button
  'Enter'        Select button
  'Space'        Start button



Requirements
============

System Requirements
-------------------
Windows 98/2000/XP/Vista/7.
DirectX 6.1b or later.
300MHz processor BARE MINIMUM (1GHz+ rec for best settings.)
16MB RAM BARE MINIMUM.
DirectSound capable sound card.

Certain games use added hardware which will require a faster machine. The specs
listed above is the minimum to use Snes9x in any playable form. It is
recommended that you get a semi-modern machine with a 800MHz CPU if you want
good results. A 1GHz CPU is recommended for those that want a near perfect
experience.

Software
--------
You will need access to SNES ROM images otherwise you will have nothing to run!
Some home-brewed ROM images can be downloaded from http://www.zophar.com/.
Please note, it is illegal in most countries to have commercial ROM images
without also owning the actual SNES ROM cartridges, and even then you may be
liable under various EULAs.


CG Shaders
--------
If you want to use CG Shaders in Snex9x for windows you need to install the
CG Toolkit from nvidia's developer zone:
http://developer.nvidia.com/object/cg_download.html

CG shaders work in both D3D and OpenGL. Various shaders can be found in
Themaister's Emulator Shader Pack:
https://github.com/Themaister/Emulator-Shader-Pack


Controllers Support
===================

The real SNES has two ports to connect input devices. Usually 1P and 2P SNES
joypads are connected but various devices and adopters can be plugged.

Multi Player 5
  known as Multi Tap; a five player adapter, allowing up to five people to play
  at once on games that supported it.

SNES Mouse
  a 2-button mouse, originally supplied with a paint program.

Super Scope
  a light-gun; it used infrared to provide wireless communication between the
  gun and the console unit.

Justifier
  a gun similar to Super Scope, supported with one gun-shooting game.

Snes9x can emulate those input devices with the keyboard, mouse and gamepad.

Configuring Keyboard and Gamepad
--------------------------------
Add support for your gamepad and calibrate it using Windows' applet from the
Windows control panel BEFORE starting Snes9x, then use Joy-pad Configuration
dialog in Snes9x to customize the keyboard/gamepad to SNES joypad mappings.

The dialog is easy to use: select which SNES joypad you are configuring using
the combo box (#1 to #5). Make sure that you click the 'enabled' box on that
controller or Snes9x won't recognize a controller being plugged in. Click on the
text box next to 'UP' and then press the key on the keyboard or button on your
gamepad that you would like to perform the UP action. The focus will
automatically move on to the 'RIGHT' text box, press the key or gamepad button
that you want to perform the RIGHT action, and so on until you've customized all
the SNES joypad buttons.

Use of the special diagonal keys should only be used by keyboard users who are
having problems pressing more then one or 2 buttons at a time. First you must
hit 'toggle diagonals' so that you are able to change them.

Using Input Devices
-------------------
SNES Mouse, Super Scope, Justifier and Multi Player 5 are disabled by default,
but you can enable them like so:

First, load your game. Then select the optional controller you want enabled from
the Input menu. Or, the controller is selectable by pressing '7' to cycle to it.

If you use NSRT to add header information to your ROMs, Snes9x will
automatically detect this information and choose the best controller
configuration for you when the game starts up. Incompatible choices will also be
grayed out from the Input menu, but if you really want, they remain selectable
by pressing '7'.

The default key mapping for input devices is as follows:

  '`'  Superscope turbo button.
  '~'  Superscope pause button.
  '7'  rotates between Multi Player 5, SNES mouse on port 1, SNES mouse on port
       2, SuperScope and Justifier emulation. (need to enable special
       controllers in the menu first)



Game Saving
===========

Many SNES games take a very long time to complete from start to finish, so they
allowed your progress to be saved at the predefined places chosen by the game
designers. The game cartridge contains a battery-backed RAM, known as SRAM, and
your save data remain in this SRAM until the battery shutoff.

Snes9x has two methods for saving games. One is the same as of the real SNES
shown above; emulating SRAM. The SRAM contents are saved into a file (.srm) so
you don't need to be worried about the battery shutoff. The other is more
convenient way than the real SNES; 'freezing' or 'snapshotting' the game. It
means saving the whole game state anywhere you want, beyond the game designers'
intent - ideal for saving your game just before a tricky bit!

Freeze files and SRAM files are normally written to and read from the folder
called Saves where your snes9x.exe is located, but sometimes this is not
desirable or possible, especially if it’s a CD-ROM, which is of course is
usually read-only! You can change the folder where Snes9x saves and loads freeze
and SRAM files using the Settings Dialog, available from the Options menu.

Using the SRAM File
-------------------
It's easy enough, just save the game as you do with the real SNES. Snes9x
outputs the contents of the emulated SRAM into a file (.srm) when you load a new
game or quit Snes9x. This file is automatically loaded the next time you play
the game.

Freezing and Defrosting the Game State
--------------------------------------
Snes9x provides 9 slots for freezing the whole of your game state. During the
game, press Shift+F1 to F9 to save a game, and just F1 to F9 to load it again
later.

Real-Time Clock Emulation
-------------------------
Some games have a battery-backed real-time clock (RTC) in their cartridge to
bring a real-time event in the game. Snes9x saves the state of RTC into a file
(.rtc) and also into a freeze file. Note that because it's a 'real-time' clock,
when these files are loaded, the emulated clock is automatically advanced in
reference to your system's time and date.

Fool-Proof System
-----------------
If you quit the game by error without saving your long-time progress, try to
find '.oops' file in the same folder as freeze files. If it exists, try to load
it. It's a freeze file automatically generated if you don't save anything for a
long time.



Cheat Support
=============

Cheat codes allow you to cheat at games. They might give you more lives,
infinite health, enable special powers normally only activated when a special
item is found, and etc. Two major formats are well-known: Game Genie and
Pro-Action Reply (PAR). Many existing Game Genie and PAR codes can be found via
Internet.

Snes9x supports both Game Genie and PAR. Also you can find your own cheat code.
Cheats are saved in .cht files and are automatically loaded the next time a game
with the same filename is loaded.

Technically, a cheat code consists of two elements; an address in SNES memory
map where you want to overwrite, and a value which is overwritten on the
address. Beware of cheat codes designed for a ROM from a different region
compared to the one you are playing or for a different version of the ROM; the
source of the cheats should tell you which region and for which version of the
game they were designed for. If you use a code designed for a different region
or version of your game, the game might crash or do other weird things because
the cheat address might be different between regions and versions.

Cheat Code Entry
----------------
Use the Cheat Code Entry and Editor dialog from the Cheats menu to enter Game
Genie or PAR cheat codes. Type in a Game Genie or PAR code into the 'Enter Cheat
Code' text edit box and press Return key. Be sure to include the '-' when typing
in a Game Genie code. You can then type in an optional short description as a
reminder to yourself of what function the cheat performs. Press Return key again
or click the Add button to add the cheat to the list.

Note that the Add button remains insensitive while 'Enter Cheat Code' text edit
box is empty or contains an invalid code. The cheat code is always translated
into an address and value pair and displayed in the cheat list as such.

It is also possible to enter cheats as an address and value pair. Type in the
address into the 'Address' text edit box then type the value into the 'Value'
text edit box. The value is normally entered in decimal, but if you prefix the
value with a '$' or append an 'h' then you can enter the value in hex.

Double-clicking on an cheat line from the list in the dialog or clicking on the
'En' column toggles an individual cheat on and off. All cheats can be switched
on and off by checking and unchecking the 'Apply cheats' item from the Cheat
menu.

Selecting a cheat from the list causes its details to be filled into the text
edit boxes in the dialog box; the details can then be edited and the Change
button pressed to commit the edits. Note that the 'Enter Cheat Code' text edit
box always redisplays the cheat code as a Pro-Action Replay code regardless of
whether you originally entered it as a Game Genie or Pro-Action Replay code.

Selecting a cheat from the list then pressing the Delete button permanently
removes that cheat.

Cheat Search
------------
Snes9x also allows new cheats to be found using the Search for New Cheats
dialog, again available from the Cheats menu. The easiest way to describe the
dialog is to walk through an example.

Let’s give ourselves infinite health and lives on Ocean's Addams Family platform
game:

Load up the game; keep pressing the start button (Return key by default) to skip
past the title screens until you actually start playing the game. You'll notice
the game starts with 2 health hearts and 5 lives. Remember that information, it
will come in useful later.

Launch the Cheat Search dialog for the first time; Alt+A is its accelerator.
Press the Reset button just in case you've used the dialog before, leave the
Search Type and Data Size radio boxes at their default values and press OK.

Play the game for a while until you loose a life by just keep walking into
baddies, when the game restarts and the life counter displays 4, launch the
Cheat Search dialog again but this time press the Search button rather than
Reset. The number of items in the list will reduce, each line shows a memory
location, its current value and its previous value; what we're looking for is
the memory location where the game stores its life counter.

Look at address line 7E00AC, its current value is 4 and its previous value was
5. Didn't we start with 5 lives? Looks interesting...

Note that some games store the current life counter as displayed on the screen,
while others store current number of lives minus 1. Looks like Addams Family
stores the actual life count as displayed on the screen.

Just to make sure you've found the correct location, press OK on the dialog, and
play the game until you loose another life. Launch the Cheat Search dialog again
after the life counter on screen has been updated and press the Search button.
Now there are even fewer items in the list, but 7E00AC is there again, this time
the current value is 3 and the previous value was 4. Looks very much like we've
found the correct location.

Now that we're happy we've found the correct location, click on the 7E00AC
address line in the list and then press the Add Cheat button. Another dialog,
Cheat Details, will be displayed. Type in a new value of say 5, this will be
number of lives that will be displayed by the lives counter. Don't be greedy;
some games display a junk life counter or might even crash if you enter a value
that's too high; Snes9x keeps the value constant anyway, so even if you do loose
a life and life counter goes down by one, less than 20ms later, Snes9x resets
the counter back to the value you chose!

If the memory location you add a cheat on proves to be wrong, just go to the
Cheat Code Editor dialog and delete the incorrect entry.

Now let’s try and find the Addams Family health counter. While two hearts are
displayed on the screen, visit the Cheat Search dialog and press the Reset
button followed by OK. Play the game until you loose a heart by touching a
baddie, then visit the Cheat Search dialog again.

Press the Search button to update the list with all memory locations that have
gone down in value since the last dialog visit. We're going to have to try and
find the heart memory location now because there were only two hearts to start
with.

Look at address line 7E00C3, its current value is 1 and its previous value was
2. Scrolling through the list doesn't reveal any other likely memory locations,
so let’s try our luck. Click on the 7E00C3 line, press the Add Cheat button and
type in a new value of say 4 into the dialog that appears and press OK. Press OK
on the Search for New Cheats dialog to return to the game.

At first sight it looks like 7E00C3 wasn't the correct memory location because
the number of hearts displayed on screen hasn't gone up, but fear not, some
games don't continually update health and life displays until they think they
need to. Crash into another baddie - instead of dying, the number of hearts
displayed jumps up to 4! We've found the correct location after all!

Now every time you play Addams Family you'll have infinite lives and health.
Have fun finding cheats for other games.



Movie Support
=============

Movie support allows you to record your actions while playing a game. This can
be used for your own personal playback or to show other people that you can do
something without them having to be around when you did it.

Recording the Movie
-------------------
Simply click File menu and click on Movie. Click the Record button. Here you can
decide when to start recording. If you want to record from the very start of a
game, click on 'Record from reset.' If you want to start recording from where
you are already in a game, click 'Record from now.' You can also choose which
controllers to record. If you are playing by yourself leave joypad 1 as the only
one selected. The more controllers you choose to record the larger the file size
will be.

Playing Back the Movie
----------------------
To play back a movie you recorded, click File menu, Movie, Play and select the
file to play. Make sure the movie was recorded with the same ROM that you have
loaded.

Re-recording the Movie
----------------------
If you make a mistake while recording a movie, there is a movie re-record
function. Simply create a freeze file anytime while recording. If you want to
re-record, load the freeze file and it will bring up the message 'movie
re-record'. Loading any freeze file while a movie is playing or recording will
cause this to happen. If you want to watch a video with no chance to
accidentally alter it, check 'Open as read only' when you go to play it.



Netplay Support
===============

Netplay support allows up to five players to sit in front of different computers
and simultaneously play the same game, or just watch someone else play a game.
All the computers have to be connected to a network that allows TCP/IP traffic
to flow between them; this includes a local Ethernet-style network, a
direct-cable connection, or, if you're lucky and have short ping times, the
Internet.

How to Netplay
--------------
It's currently easier if you use Snes9x in windowed mode while using netplay,
mainly because netplay currently displays status information in the window's
title bar, and it might be necessary to setup a separate chat application so you
can talk to the other players when deciding what game to play next.

One machine has to act as a server which other players (client sessions) connect
to. The 'master' player, player 1, uses the server machine; the master decides
what game to play. The server machine should be selected to be the fastest
machine on the fastest connection in the group taking part due to the extra
functions it has to perform.

Load up a game, then select the 'Act as server' option from the Netplay menu to
become a netplay server; the 'network', in whatever form it takes, will need to
be initialized, if necessary, before you do this. Then just wait for other
players to connect...

Clients connect to the server using the 'Connect to server...' dialog, again
available from the Netplay menu. Type in the IP address or host name of the
machine running the Snes9x server session and press OK. The first remote client
to connect will become player 2, and so on. Start Menu->Run->winipcfg will tell
you your current IP address, but note that many ISPs will allocate you a new IP
address each time you sign in.

The server will request the client loads up the correct game first before
joining the game. Then the server will either send the client SRAM data and
reset all players' games if the 'Sync Using Reset Game' option is checked, or
send it a freeze file to get the new client in sync with the other player's
progress in a game.

If the master player loads a different game, the server will request that the
clients load the game. If the master player loads a freeze file, the server will
automatically send that to remote clients.

Client sessions must be able to keep up with the server at all times - if they
can't, either because the machine is just too slow, or its busy, the games will
get out of sync and it will be impossible to successfully play a multi-player
game...

...To make sure this doesn't happen, don't move the Snes9x window unnecessarily
and don't use Ctrl+Alt+Del to display the task manager while playing. Also stop
any unnecessary applications and as many background tasks as possible. Even
something as simple as a text editor might periodically write unsaved data to
the disk, stealing CPU time away from Snes9x causing it to skip a frame or delay
a sound effect; not a problem for most games, but the Bomberman series (the best
multi-player games on the SNES) sync the game to sound samples finishing.



Miscellaneous
=============

Using IPS or UPS Patch
----------------------
Snes9x automatically patches without overwriting the ROM image.

- Put the IPS or UPS file into the same folder as the ROM image.
- Rename the name to the same as the ROM image (except extension, it is '.ips'
  or '.ups').
- (IPS only) If you want to use multiple IPS files at a time, set their
  extensions to '.000.ips', '.001.ips', ...
- Open and load the ROM image.

Additional Keyboard Controls
----------------------------
Snes9x has various functions to play games with fun. The default mapping is as
follows. Go to Input > Customize Hotkeys... to configure these and more.

  'Pause'              pauses or unpauses the emulator.
  'Escape'             shows or hides the menu bar.
  'Alt'+'Enter'        toggles between full-screen and windowed mode.
  'Ctrl'+'Shift'+'R'   resets the game.
  'Shift'+'F1-F9'      saves a freeze file into the slot 1-9.
  'F1-F9'              loads a freeze file from the slot 1-9.
  'F12'                takes a screenshot.
  'Shift'+'Page Down'  toggles turbo on the 'A' button. Note: toggles for all
                       controllers.
  'Shift'+'End'        toggles turbo on the 'B' button...
  'Shift'+'Page Up'    toggles turbo on the 'X' button...
  'Shift'+'Home'       toggles turbo on the 'Y' button...
  'Shift'+'Ins'        toggles turbo on the 'L' button...
  'Shift'+'Del'        toggles turbo on the 'R' button...
  'Shift'+'['          toggles turbo on the 'Select' button...
  'Shift'+']'          toggles turbo on the 'Start' button...
  '6'                  toggles swapping of joypad one and two.
  'Shift'+'='          increases frame rendering skip rate, making the screen
                       updates more jerky but speeding up the game.
  'Shift'+'-'          decreases frame rendering skip rate, making the screen
                       updates more smoothly, but potentially slowing down the
                       game. Repeatedly pressing the key will eventually switch
                       to auto-frame skip rate where the rate is dynamically
                       adjusted to keep a constant game play speed.
  '-'                  increases emulated frame time by 1ms - slowing down the
                       game. (auto-frame skip must be on)
  '='                  decreases emulated frame time by 1ms - speeding up the
                       game. (auto-frame skip must be on)
  '\'                  pauses the game, or slowly advances gameplay if it's
                       already paused. To return to normal, press the 'Pause'
                       key.
  'Tab'                turbo mode.
  ','                  toggles display of input, so you can see which SNES
                       buttons are registering as pressed.
  '.'                  toggles movie frame display on/off. Movie must be open.
  'Shift'+'8'          toggles movie read-only status. Movie must be open.
  '1-4'                toggles background 1-4 on/off.
  '5'                  toggles sprites on/off
  '9'                  (not recommended) toggles transparency effects on/off.
  '8'                  (not recommended) toggles emulation of graphics window
                       effects on/off.
  '0'                  (not recommended) toggles H-DMA emulation on/off.



Compatibility
=============

Compatibility with Other Ports
------------------------------
All the files generated by Snes9x are compatible between platforms, except for
the extension of the freeze files.

Compatibility with Other SNES Emulators
---------------------------------------
Cheat files (.cht) are common between Snes9x and ZSNES. RTC files (.rtc) are
common between Snes9x and bsnes. SRAM files (.srm) should be common among all
SNES emulators.



Problems
========

Problems with ROMs
------------------
If Snes9x just displays a black screen for over 10 seconds after you've loaded a
ROM image, then one of the following could be true:

- You just loaded some random ROM image and it isn't even a SNES game or you
  only have part of the image. Snes9x only emulates games designed for the Super
  NES, not NES, or Master System, or Game Boy, or <insert your favorite old
  games system here>.
- Someone has edited the Nintendo ROM information area inside the ROM image and
  Snes9x can't work out what format the ROM image is in. Try playing around with
  the ROM format options on the ROM load dialog.
- The ROM image is corrupt. If you're loading from CD, I know it might sound
  silly, but is the CD dirty? Clean, unhacked ROM images will display [checksum
  ok] when first loaded, corrupt or hacked ROMs display [bad checksum].
- The original SNES ROM cartridge had additional hardware inside that is not
  emulated yet and might never be.
- You might be using a file that is compressed in a way Snes9x does not
  understand.

The following ROMs are known to currently not to work with any version of
Snes9x:

  SD Gundam GX                     DSP-3
  Hayazashi Nidan Morita Shougi    Seta-11
  Hayazashi Nidan Morita Shougi 2  Seta-18

Problems with Sounds
--------------------
No sound coming from any SNES game using Snes9x? Could be any or all of these:

- If all sound menu options are grayed out, or an error dialog about Snes9x not
  being able to initialize DirectSound is displayed - then DirectSound could not
  initialize itself. Make sure DirectX 6 or above is installed and your sound
  card is supported by DirectX.
  Installing the latest drivers for your sound card might help. Another Windows
  application might have opened DirectSound in exclusive mode or opened the
  Windows WAVE device - WinAmp uses the Windows WAVE device by default - in
  which case you will need to stop that application and then restart Snes9x. It
  is possible to switch WinAmp to use DirectSound, in which case both Snes9x and
  WinAmp output can be heard at the same time.
  If your sound card isn't supported by DirectX very well (or not at all) you
  will have to use FMOD's WAVE output option; but WAVE output introduces a 0.15s
  delay between Snes9x generating sample data and you hearing it. Select FMOD's
  WAVE output by choosing the 'FMOD Windows Multimedia' sound driver option from
  the Sound Settings dialog.
- The sound card's volume level might be set too low. Snes9x doesn't alter the
  card's master volume level so you might need to adjust it using the sound
  card's mixer/volume controls usually available from the task bar or start
  menu.
- Make sure your speakers and turned on, plugged in and the volume controls are
  set to a suitable level.



Technical Information
=====================

What's Emulated?
----------------
- 65c816 main CPU.
- Variable length machine cycles.
- 8 channel DMA and H-DMA.
- H-IRQ, V-IRQ and NMI.
- Sony SPC700 sound CPU.
- Sound DSP, with eight 16-bit, stereo channels, compressed samples, hardware
  envelope processing, echo, pitch modulation and digital FIR sound filter.
- SRAM, a battery-backed RAM.
- All background modes, 0 to 7.
- All screen resolutions including interlace mode.
- Pseudo hi-res mode.
- 8x8, 16x8 and 16x16 tile sizes, flipped in either direction.
- 32x32, 32x64, 64x32 and 64x64 screen tile sizes.
- Vertical and horizontal offset-per-tile.
- 128 8x8, 16x16, 32x32 and 64x64 sprites, flipped in either direction.
- Sub-screen and fixed color blending effects.
- Mosaic effects.
- Mode 7 screen rotation, scaling and screen flipping.
- Single and dual graphic clip windows, with all four logic combination modes.
- Color blending effects only inside or outside a window.
- Palette changes during frame.
- Direct color mode - uses tile and palette-group data directly as RGB value.
- SNES Mouse.
- Super Scope, emulated using computer mouse.
- Justifier, by Konami, similar to the Super Scope and used only in Lethal
  Enforcers.
- Multi Player 5, allowing up to five people to play games simultaneously on
  games that support that many players.
- Super FX, a fast RISC CPU used in several games.
- SA-1, a faster version of main CPU with some functions, used in several games.
- DSP-1, a custom chip used in several games, mainly racing games.
- DSP-2, a custom chip used only in Dungeon Master.
- DSP-4, a custom chip used only in Top Gear 3000.
- C4, a sprite scaler/rotator/line drawer/simple maths co-processor chip used
  only in Megaman X2 and X3.
- Seta-10, a custom chip used only in F1 Race of Champions 2.
- OBC1, a sprite management chip used only in Metal Combat.
- S-DD1, a data decompression chip used only in Star Ocean and Street Fighter 2
  Alpha.
- SPC7110, similar in use to S-DD1, used in a few Hadoson games.
- S-RTC, a real-time clock chip, used only in Dai Kaijyu Monogatari 2.
- Satellaview and BS-X, only partially.

What's Not?
-----------
- Exact sub-cycle timings of communication among most of parts - main CPU, sound
  CPU, DMA, H-DMA, IRQ, NMI, and so on. Snes9x cannot run games that require
  severe timings!
- Any other odd chips that manufactures sometimes placed inside the cartridge to
  enhance games and as a nice side-effect, also act as an anti-piracy measure.
  (DSP-3, Seta-11 and Seta-18, as examples)
- The expansion slot found in many carts.

Custom Chips
------------
Super FX
  The Super FX is a 10.5/21MHz RISC CPU developed by Argonaut Software used as a
  game enhancer by several game titles. Released SNES Super FX games included
  Yoshi's Island, Doom, Winter Gold, Dirt Trax FX, StarFox, Stunt Race FX and
  Vortex.

SA-1
  The SA-1 is a fast, custom 65c816 8/16-bit processor, the same as inside the
  SNES itself, but clocked at 10MHz compared to a maximum of 3.58MHz for the CPU
  inside the SNES. The SA-1 isn't just a CPU; it also contains some extra
  circuits developed by Nintendo which includes some very fast RAM, a memory
  mapper, DMA and, several real-time timers.

C4
  The C4 is custom Capcom chip used only in the Megaman X2 and Megaman X3 games.
  It can scale and rotate images, draw line-vector objects and do some simple
  maths to rotate them.

S-DD1
  The S-DD1 is a custom data decompression chip that can decompress data in
  real-time as the SNES DMA's data from the ROM to RAM. Only two games use the
  chip: Star Ocean and Street Fighter Alpha 2.

SPC7110
  The SPC7110 is a compression and memory mapping chip. It provides a few extra
  features as well. It functions as an RTC interface, and has a multiply/divide
  unit that has more precision than the SNES. The SPC7110 is found only in 4
  games: Super Power League 4, Far East of Eden Zero, Far East of Eden Zero -
  Shounen Jump no Shou and Momotaro Dentetsu Happy.

Others
  Other known custom chips: DSP-1, DSP-2, DSP-3, DSP-4, Seta-10, Seta-11,
  Seta-18, OBC1 and S-RTC.



Credits
=======

- Jerremy Koot for all his hard work on previous versions of Snes96, Snes97 and
  Snes9x.
- Ivar for the original Super FX C emulation, DSP-1 emulation work and
  information on both chips.
- zsKnight and _Demo_ for the Intel Super FX assembler, DSP-1 and C4 emulation
  code.
- zsKnight and _Demo_ for all the other ideas and code I've nicked off them;
  they've nicked lots of my ideas and information too!
- John Weidman and Darkforce for the S-RTC emulation information and code.
- Kreed and Maxim Stepin for excellent image enhancer routines.
- Nose000 for code changes to support various Japanese SNES games.
- Neill Corlett for the IPS patching support code.
- DiskDude's SNES Kart v1.6 document for the Game Genie(TM) and Pro-Action
  Replay cheat system information.
- Lord ESNES for some nice chats and generally useful stuff.
- Lee Hyde (lee@jlp1.demon.co.uk) for his quest for sound information and the
  Windows 95 icon.
- Shawn Hargreaves for the rather good Allegro 3.0 DOS library.
- Robert Grubbs for the SideWinder information - although I didn't use his
  actual driver in the end.
- Steve Snake for his insights into SNES sound sample decompression.
- Vojtech Pavlik for the Linux joystick driver patches.
- Maciej Babinski for the basics of Linux's DGA X server extensions.
- Alexander Larsson for the GGI Linux port code.
- Harald Fielker for the original sound interpolation code (never used directly
  due to problems).
- Takehiro TOMINAGA for many speed up suggestions and bug fixes.
- Predicador for the Windows icon.
- Lindsey Dubb for the mode 7 bi-linear filter code and the improved color
  addition and subtraction code.
- Anti Resonance for his super-human efforts to help get his fast sound CPU core
  and sound DSP core working in Snes9x.
- Brad Martin and TRAC for better and refined sound emulation.
- ernstp and entonne for patches and testing on Linux PPC.
- byuu for the most exact timing information and tons of the newest technical
  findings.
- Blargg for the most accurate timings between sound CPU and DSP and exact sound
  emulation codes.
- pagefault, TRAC, Dark Force, byuu, and others who have donated ideas and/or
  code to the project.



Nintendo is a trademark.
Super NES, Super Famicon, Super Scope and Super FX are trademarks of Nintendo.
Sufami Turbo is a trademark of Bandai Co., Ltd.
Game Genie is a trademark of Lewis Galoob Toys, Inc.
Pro Action Replay is a trademark of Datel Inc.
Macintosh, Mac and Mac OS X are trademarks of Apple Computer, Inc.
UNIX is a trademark of The Open Group.
Linux is a trademark of Linus Torvalds.
Windows is a trademark of Microsoft Corp.
Intel is a trademark of Intel Corp.
PowerPC is a trademark of International Business Machines Corp.
Sony is a trademark of Sony Corp.
Konami and Justifier are trademarks of Konami Corp.
Hudson is a trademark of Husdon Soft Co., Ltd.
Capcom is a trademark of Capcom Co., Ltd.

Gary Henderson

Edited for Windows port by: zones (kasumitokoduck@yahoo.com)
Updated most recently by: 2011/04/11 zones
