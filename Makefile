.PHONY: clean build

all: kde5

kde4: /usr/local/lib64/kde4/kio_adb.so /usr/local/share/kde4/services/adb.protocol
	sudo mv /usr/local/lib64/kde4/kio_adb.so /usr/lib64/kde4/kio_adb.so
	sudo mv /usr/local/share/kde4/services/adb.protocol /usr/share/kde4/services/adb.protocol

/usr/local/lib64/kde4/kio_adb.so: build src/adb.cpp src/adb.h
	cd build && make && sudo make install

kde5: /usr/local/lib64/plugins/kf5/kio/adb.so
	sudo mv /usr/local/lib64/plugins/kf5/kio/adb.so /usr/lib64/qt5/plugins/kf5/kio/

/usr/local/lib64/plugins/kf5/kio/adb.so: build src/adb.cpp src/adb.h
	cd build && make && sudo make install


uninstall_kde4:
	sudo rm /usr/lib64/kde4/kio_adb.so /usr/share/kde4/services/adb.protocol

clean:
	rm -r build

build:
	mkdir -p build && cd build && cmake -DCMAKE_BUILD_TYPE=debugfull ..

version=0.1
release=1

rpm:
	tar --exclude=build --transform "s|^.|kio_adb-${version}/|" -czvf ~/rpmbuild/SOURCES/kio_adb-${version}.tar.gz .
	rpmbuild -ba kio_adb.spec 
