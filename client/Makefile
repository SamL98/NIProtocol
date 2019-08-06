CC=/usr/bin/clang++
PWD=/Users/samlerner/Projects/NIProtocol/client

client: callbacks messenger
	$(CC) -g -o m2client client.cpp callbacks.o messenger.o \
		-framework CoreFoundation \
		-I$(PWD) -L$(PWD)

callbacks: messenger
	$(CC) -o callbacks.o -c callbacks.cpp \
		-I$(PWD)

messenger:
	$(CC) -o messenger.o -c messenger.cpp

clean:
	rm m2client callbacks.o messenger.o
