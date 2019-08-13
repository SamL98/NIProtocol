CC=/usr/bin/clang++

ipcopenhook.dylib: ipcopenhook.o 
	$(CC) -arch x86_64 -o ipcopenhook.dylib ipcopenhook.o \
		-lobjc -framework Cocoa -framework Carbon \
		-Wl,-F/System/Library/PrivateFrameworks \
		-dynamiclib

ipcopenhook.o: ipcopenhook.mm
	$(CC) -arch x86_64 -o ipcopenhook.o -c ipcopenhook.mm \
		-Wno-deprecated-declarations \
		-Wc++11-compat-deprecated-writable-strings

clean:
	rm ipcopenhook.o ipcopenhook.dylib
