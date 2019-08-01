CC=/usr/bin/clang++
PWD=/Users/samlerner/Projects/NIProtocol

ipcopenhook.dylib: ipcopenhook.o 
	$(CC) -arch x86_64 -o ipcopenhook.dylib ipcopenhook.o \
		-lobjc -framework Cocoa -framework Carbon \
		-Wl,-F/System/Library/PrivateFrameworks \
		-framework CoreSymbolication -dynamiclib \
		-I$(PWD) -L$(PWD)

ipcopenhook.o: ipcopenhook.mm
	$(CC) -arch x86_64 -o ipcopenhook.o -c ipcopenhook.mm \
		-Wno-deprecated-declarations \
		-Wc++11-compat-deprecated-writable-strings \
		-I$(PWD)

clean:
	rm ipcopenhook.o ipcopenhook.dylib
