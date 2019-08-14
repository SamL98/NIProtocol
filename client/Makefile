CC=/usr/bin/clang++
PWD=/Users/samlerner/Projects/NIProtocol/client

client: callbacks
	$(CC) -o m2client client.cpp \
		-framework CoreFoundation \
		-lnimessenger -lninotifier -lnihandshaker

clean:
	rm m2client callbacks.o
