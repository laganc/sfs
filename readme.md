# Simple File System

## Description
This program implements a simple file system (SFS) which takes a .dmg or .img file
as an argument, which you can run multiple commands on. 

## Commands
diskinfo: This command shows the super block information and FAT information for the file uploaded. It takes one argument, which is the .dmg or .img file.

Example: ./diskinfo test.dmg

disklist: This command shows all of the files that exist within the file system. It takes one argument, which is the .dmg or .img file.

Example: ./disklist test.dmg

diskget: This command gets a file from the file system and copies it into the current directory it is called from. It takes three arguments, the first being the .dmg or .img file, the second being the directory and the name of the file you want to save it as, and the third being the file you want to find in the file system.

Example: ./disklist test.dmg /foo_new.txt foo.txt
