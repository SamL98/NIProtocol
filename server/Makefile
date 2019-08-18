CC=/usr/bin/clang++
PWD=/Users/samlerner/Projects/NIProtocol/server

server: mach_callbacks sock_callbacks bonjour_callbacks
	$(CC) -g -o m2server server.cpp mach_callbacks.o sock_callbacks.o bonjour_callbacks.o \
		-I$(PWD) -L$(PWD) \
		-framework Foundation -framework CFNetwork \
		-lnimessenger -lninotifier -lniparser -lnihandshaker

mach_callbacks:
	$(CC) -g -o mach_callbacks.o -c mach_callbacks.cpp

sock_callbacks:
	$(CC) -g -o sock_callbacks.o -c sock_callbacks.cpp

bonjour_callbacks:
	$(CC) -g -o bonjour_callbacks.o -c bonjour_callbacks.cpp

clean:
	rm m2server mach_callbacks.o sock_callbacks.o bonjour_callbacks.o
