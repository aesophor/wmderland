CXX=g++
CXXFLAGS=-g -std=c++14 -Wall -lpthread -lX11 -lglog
BIN=Wmderland
SRC=$(wildcard src/*.cpp)

all:
	$(CXX) -o $(BIN) $(SRC) $(CXXFLAGS)

clean:
	rm $(BIN)

install:
	sudo cp $(BIN) /usr/local/bin/.
	sudo chmod +x /usr/local/bin/$(BIN)
