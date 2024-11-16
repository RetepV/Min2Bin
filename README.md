# min2bin

NOTE: There used to be a few resources for the Minato 1866 and 1866A eprom programmers here, but they have been moved to their own branch: https://github.com/RetepV/Minato-1866-1866A.

Small commandline tool to quickly convert Minato 1866 papertape dump formatted text to a binary file and vice-versa. Useful for uploading and downloading data when using the Minato 1866 with a serial connection.

There is also a `hex2bin` tool that converts from/to a 'hexdump' format. It is similar to `min2bin`. But I have no clue anymore why I wrote it. This code was written in a hurry, about 15 years ago.

From the help file:

```
Help:

  This program converts binary rom image files from
  binary to Minato upload format for use with Minato
  eprom programmers.

Run as:

  min2bin [/<options>] <inputfile> <outputfile>

Options:

  /?    Show this help.
  /[mM]    Minato format to binary (default).
  /[bB]    Binary to Minato format.
  /[fF]    Fill remaining space in eprom with 00's
           or some other given value (add /c XX).\n" );
  /[cC] XX  Give a different value for filling.
           The value is a HEX number.
```

# Using Minato 1866 over a serial connection

* Set the Minato large dipswitch block to following dipswitch settings (9600 n,8,1 xon/xoff):
  * Switch 1-2-3: off-on-on (9600 buad)
  * Switch 4-5: off-off (no parity)
  * Switch 6-7: on-off (1 stopbit)
  * Switch 8: 1 (xon/xoff)
* Make sure the eprom programmer is off. Connect the eprom programmer to an RS-232 serial port using a serial<->USB cable, or to an old computer with an RS-232C port. I use a noname cable with a Prolific RS-232 chip on it (I think a PL2303).
NOTE: The programmer communicates with RS-232C V24, which means that it uses +12V/-12V signals. Make sure that the serial port that you use can support that. The programmer itself does work with +5V/-5V signals, so if your serial<->USB cable is +12V/-12V tolerant, but itself provides +5V/-5V, it will work.
* On your host computer, start a terminal emulator (e.g. Tera Term: https://teratermproject.github.io/index-en.html) listening to the serial port.
* Switch on the programmer. Wait for it to finish its self-test.
* On the programmer's keyboard, select REM and press START. In your terminal program, you now should see a '#' appear.
* You can now enter commands. NOTE: all entered letters *must* be uppercase, or the programmer will throw an error.
* Useful commands:\
  * *S*: Select a data format. E.g.: `S 0 5`, this chooses 'Minato Hex. format', which `min2bin` parses. Terminal will show `#S.05`.
  * *N*: Select a device. E.g.: `N 0 3`, this chooses a TMS2532. Terminal will show `#N-00-03`.
  * *W*: Program a device from the programmer's memory.
  * *OP*: Copy data from a device into the programmer's memory.
  * *B*: Blank check device.
  * *P[start],[end]*: 'Punch' to paper tape (terminal/tty). This is used to dump the programmer's memory to the terminal, to basically download what's in the programmer's memory.
  * *RL*: Read from paper tape (terminal/tty). This is used to upload data from th =e host computer to the programmer's memory.

Typical reading from a TMS2532 eprom:

* Programmer is reset, connection is there, Tera Term is started and waiting for serial data.
* `REM`, `START` -> `#` appears in the terminal
* `S05[enter]` -> `#S.05[CR]#` appears in the terminal.
* `N03[enter]` -> `#N-00-03[CR]#` appears in the terminal.
* `OP[enter]` -> `#OP` appears in the terminal, after a few seconds, `#` appears.
* `P0,FFF[enter]` -> A dump in Minato format will be output to the terminal, `[#0000 F3 C3 0B E0 ...`. Copy it from the terminak window and save it to a file (preferably with extension `.min`, e.g. `2532.min`).

You can now convert the `.min` file to binary with the command: `min2bin /m 2532.min 2532.bin`.

Typical writing a TMS2532 eprom:

You will probably have a binary image, and that will first need to be converted to Minato format. Let's say that the binary is called `2532.bin`, then it can be converted to `.min` like so: `min2bin /b 2532.bin 2532.min`.

* Programmer is reset, connection is there, terminal is started and waiting for serial data.
* `REM`, `START` -> `#` appears in the terminal
* `S05[enter]` -> `#S.05[CR]#` appears in the terminal.
* `N03[enter]` -> `#N-00-03[CR]#` appears in the terminal.
* `RL[enter]` -> Programmer is now waiting for data from the terminal.
NOTE: This times out if no data is received with a few seconds, so you must be a bit fast with the next steps. I prefer to first copy the full path to the file I want to upload (e.g. `c:\Projects\UnknownZ80Computer\2532.min`) to the paste buffer so that I won't have to browse but can just paste the file path to Tera Term.
* In Tera Term open `File->Send file ...`.
* Paste the file path in Filename, and press Ok.
* The file is now being uploaded. The terminal freezes and does not show anything, but you can check the display of the Minato to see the bytes coming in.
* When all is uploaded, the Terminal will show a `#`, a `?`, and another `#`. This looks as if an error occurred, but is probably due to that there is an extra empty line at the end of the `.min` file. It is of no consequence and can be ignored.
* Insert the empty 2532
* `B[enter]` -> Terminal should respond with `#`. If it responds with a `?`, the eprom is not empty.
* `W[enter]` -> The eprom is now being written.
