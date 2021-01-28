Arduino compiles a sketch by copiing the sketch folder content in a temp build folder.
Because of this, we cannot #include files outside the sketch folder (eg ..\path.h).
In order to shared code between the arduino and the host (windows), we make a symlink (an NTFS junction) to a parent folder...

see: setup_common_junction.bat