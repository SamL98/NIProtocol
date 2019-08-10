CC=/usr/bin/clang++
PWD=/Users/samlerner/Projects/NIProtocol/parser

parser.dylib: parser.o
	$(CC) -g -o parser.dylib parser.o \
		-dynamiclib -framework CoreFoundation \
		-install_name libniparser.dylib

	cp $(PWD)/parser.dylib /usr/local/lib/libniparser.dylib
	cp $(PWD)/parser.h /usr/local/include/niparser.h

parser.o:
	$(CC) -o parser.o -c parser.cpp -I$(PWD)

clean:
	rm parser.o parser.dylib
