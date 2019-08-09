CC=/usr/bin/clang++
PWD=/Users/samlerner/Projects/NIProtocol/client

client: callbacks
	$(CC) -o m2client client.cpp callbacks.o \
		-framework CoreFoundation \
		-I$(PWD) -L$(PWD) \
		-lnimessenger -lninotifier

callbacks:
	$(CC) -o callbacks.o -c callbacks.cpp

clean:
	rm m2client callbacks.o
