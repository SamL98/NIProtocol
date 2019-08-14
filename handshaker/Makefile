CC=/usr/bin/clang++
PWD=/Users/samlerner/Projects/NIProtocol/handshaker

handshaker.dylib: handshaker.o callbacks.o
	$(CC) -o handshaker.dylib handshaker.o callbacks.o \
		-I$(PWD) -L$(PWD) \
		-dynamiclib -framework CoreFoundation \
		-lniparser -lnimessenger -lninotifier \
		-install_name libnihandshaker.dylib

	cp $(PWD)/handshaker.dylib /usr/local/lib/libnihandshaker.dylib
	cp $(PWD)/handshaker.h /usr/local/include/nihandshaker.h

handshaker.o:
	$(CC) -o handshaker.o -c handshaker.cpp -I$(PWD)

callbacks.o:
	$(CC) -o callbacks.o -c callbacks.cpp

clean:
	rm handshaker.o callbacks.o handshaker.dylib