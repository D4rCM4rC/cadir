.DEFAULT_GOAL := compile

compile:
	c++ -Wall main.cpp -o cardir -lcrypto -lssl -std=c++17 -lstdc++fs