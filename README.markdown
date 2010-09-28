nl2xml
======
Converts Nightingale <http://www.ngale.com/> Notelist files into MusicXML <http://www.musicxml.org/> (partwise) files.
Simply reads Notelist from stdin and writes MusicXML to stdout.

Building 
--------
(Tested at various times on Linux x86, Linux PPC, Windows/Cygwin)

something like:
$ gcc -o nl2xml nl*.c

Should also be easily imported into and build in the development enviroment of your choice.

Usage
-----
single file:
$ ./nl2xml notelistfile.nl > musicxmlfile.xml
 
batch:
$ all_nl2xml.sh notelist/*.nl

Output Validation
-----------------
(requires Ant, defaults to validation against MusicXML schema)
$ ant 

Author
------
Geoff Chirgwin
<geoff at chirgwin.com>

Based on an example Notelist parsing program (that ships with Nightingale) by John Gibson, Donald Byrd, and Tim Crawford.
 
