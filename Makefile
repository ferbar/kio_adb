
all: /usr/local/lib64/kde4/kio_adb.so /usr/local/share/kde4/services/adb.protocol
	sudo mv /usr/local/lib64/kde4/kio_adb.so /usr/lib64/kde4/kio_adb.so
	sudo mv /usr/local/share/kde4/services/adb.protocol /usr/share/kde4/services/adb.protocol

/usr/local/lib64/kde4/kio_adb.so: build adb.cpp adb.h
	cd build && make && sudo make install

uninstall:
	sudo rm /usr/lib64/kde4/kio_adb.so /usr/share/kde4/services/adb.protocol

clean:
	rm -r build

build:
	mkdir build && cd build && cmake -DCMAKE_BUILD_TYPE=debugfull ..

version=0.1
release=1

rpm:
	tar --exclude=build --transform "s|^.|kio_adb-${version}/|" -czvf ~/rpmbuild/SOURCES/kio_adb-${version}.tar.gz .
	rpmbuild -ba kio_adb.spec 
