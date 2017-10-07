# kio slave ADB

use dolphin to copy files from and to your android phone using the android debug bridge commands.

## setup

* enable android debugging on your phone (tab build version several times)
* enable usb debugging on your phone (enable the option under settings -> development options)
* copy or softlink the adb binary programm to a reachable position ( ~/bin if this is in your path, /usr/local/bin or /usr/bin )
* install libkde4-devel, rpmbuild and dependencis
* make rpm
* rpm -i ~/rpmbuild/RPMS/kio_adb...rpm

## using

* enter _adb:/// in the dolphin address bar
* preview images: images bigger than 3MB wont display a preview image. Check it in the Dolphins settings.

## TODO

* usefull logging
* usefull error messages
* delete, mkdir, rename
* Kaffeine support
* root explorer

## TESTING

 kdeinit5 # or just re-login
 kioclient5 exec gdrive:/

## KNOWN BUGS

* idiotic adb seems to delete spaces in command args
  '  ' => ' '
  Filenames containing two spaces are not handled correct
* missing copy/move directory
* missing rm directory
