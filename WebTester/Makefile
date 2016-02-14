all:
webtest: webtest.cpp
	g++ -o webtest webtest.cpp
client: client.cpp
	g++ -o client client.cpp
server: server.cpp
	g++ -std=c++11 -o server server.cpp -pthread
exampleEpoll: exampleEpoll.cpp
	g++ -o exampleEpoll exampleEpoll.cpp
cleantwebtest:
	rm webtest
cleanclient:
	rm client
cleanserver:
	rm server
cleanepoll:
	rm epoll