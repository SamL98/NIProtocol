CC=/usr/bin/clang++
PWD=/Users/samlerner/Projects/NIProtocol/server

server: callbacks
	$(CC) -g -o m2server server.cpp callbacks.o \
		-I$(PWD) -L$(PWD) \
		-framework Foundation \
		-lnimessenger -lninotifier -lniparser -lnihandshaker

callbacks:
	$(CC) -g -o callbacks.o -c callbacks.cpp

clean:
	rm m2server callbacks.o
