CC=/usr/bin/clang++

hahooks.dylib: hahooks.o 
	$(CC) -arch x86_64 -o hahooks.dylib hahooks.o \
		-lobjc -framework Cocoa -framework Carbon \
		-Wl,-F/System/Library/PrivateFrameworks \
		-dynamiclib

hahooks.o: hahooks.mm
	$(CC) -arch x86_64 -o hahooks.o -c hahooks.mm \
		-Wno-deprecated-declarations \
		-Wc++11-compat-deprecated-writable-strings

clean:
	rm hahooks.o hahooks.dylib
