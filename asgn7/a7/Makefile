run: libsocket.a user1 user2 

libsocket.a: rsocket.o
	ar -rcs libsocket.a rsocket.o
	rm rsocket.o
	
socket.o: rsocket.h
	gcc -Wall -c rsocket.h

user1: libsocket.a user1.c
	gcc user1.c -o user1 -L. -lsocket -lpthread

user2: libsocket.a user2.c libsocket.a
	gcc user2.c -o user2 -L. -lsocket -lpthread

clean:
	rm user1 user2 libsocket.a

