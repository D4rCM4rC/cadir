.DEFAULT_GOAL := compile

compile:
	c++ -Wall main.cpp -o cadir -lcrypto -lssl -std=c++17 -lstdc++fs