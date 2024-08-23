# min2bin

Smaal commandline tool to quickly convert Minato 1866 papertape dump formatted text to a binary file and vice-versa. Useful for uploading and downloading data when using the Minato 1866 with a serial connection.

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
