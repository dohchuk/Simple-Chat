all:
	g++ -std=c++11 -pthread -o server server.cpp
	g++ -std=c++11 -pthread -o client client.cpp

