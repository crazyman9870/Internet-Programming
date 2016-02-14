all:
server: server.cpp
	g++ -std=c++11 -o server server.cpp -pthread
exthread: exampleThreading.c
	gcc -o exthread exampleThreading.c -pthread
exthread2: exampleThreading2.cpp
	g++ -o exthread2 exampleThreading2.cpp -pthread
cleanexthread:
	rm exthread
cleanexthread2:
	rm exthread2
clean:
	rm server