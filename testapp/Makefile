CC=/usr/bin/clang++

testapp:
	$(CC) -g -o app testapp.cpp \
		-framework Foundation \
		-lniparser -lninotifier -lnimessenger

clean:
	rm app
