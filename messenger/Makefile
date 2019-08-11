CC=/usr/bin/clang++
PWD=/Users/samlerner/Projects/NIProtocol/messenger

messenger.dylib: messenger.o
	$(CC) -o messenger.dylib messenger.o \
		-dynamiclib -framework CoreFoundation \
		-lniparser \
		-install_name libnimessenger.dylib

	cp $(PWD)/messenger.dylib /usr/local/lib/libnimessenger.dylib
	cp $(PWD)/messenger.h /usr/local/include/nimessenger.h

messenger.o:
	$(CC) -o messenger.o -c messenger.cpp -I$(PWD)

clean:
	rm messenger.o messenger.dylib
