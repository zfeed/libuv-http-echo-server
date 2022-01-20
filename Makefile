main: server.o main.cpp
	g++ -I./deps/libuv/include -I./ -o main main.cpp ./deps/libuv/.libs/libuv.a ./server.o

server.o: deps/libuv/.libs/libuv.a server.cpp server.h
	g++ -I./deps/libuv/include -c -o server.o server.cpp

deps/libuv/.libs/libuv.a: deps/libuv/Makefile
	$(MAKE) -C ./deps/libuv/

deps/libuv/Makefile:
	cd ./deps/libuv/ && sh autogen.sh && ./configure

clean:
	rm -f ./server.o ./main
	$(MAKE) -C ./deps/libuv/ clean
