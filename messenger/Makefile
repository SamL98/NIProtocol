CC=/usr/bin/clang++
PWD=/Users/samlerner/Projects/NIProtocol/Messenger

messenger.dylib: messenger.o
	$(CC) -o messenger.dylib messenger.o \
		-dynamiclib -framework CoreFoundation \

	ln -s $(PWD)/messenger.dylib /usr/local/lib/libnimessenger.dylib
	ln -s $(PWD)/messenger.h /usr/local/include/nimessenger.h

messenger.o:
	$(CC) -o messenger.o -c messenger.cpp -I$(PWD)

clean:
	rm messenger.o messenger.dylib