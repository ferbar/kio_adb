# kio_adb
kio-slave ADB

copy files from your android phone using the android debug bridge commands. 

## setup

. enable android debugging on your phone (tab build version several times)
. enable usb debugging on your phone (enable the option under settings -> development options)
. copy or softlink the adb binary programm to a reachable position ( ~/bin if this is in your path, /usr/local/bin or /usr/bin )
. install libkde4-devel, rpmbuild and dependencis
. make rpm
. rpm -i ~/rpmbuild/RPMS/kio_adb...rpm