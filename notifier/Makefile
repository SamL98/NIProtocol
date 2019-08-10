CC=/usr/bin/clang++
PWD=/Users/samlerner/Projects/NIProtocol/notifier

notifier.dylib: notifier.o
	$(CC) -g -o notifier.dylib notifier.o \
		-dynamiclib -framework CoreFoundation \
		-install_name libninotifier.dylib

	cp $(PWD)/notifier.dylib /usr/local/lib/libninotifier.dylib
	cp $(PWD)/notifier.h /usr/local/include/ninotifier.h

notifier.o:
	$(CC) -g -o notifier.o -c notifier.cpp -I$(PWD)

clean:
	rm notifier.o notifier.dylib
