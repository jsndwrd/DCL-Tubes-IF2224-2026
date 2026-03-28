CXX = g++
CXXFLAGS = -std=c++17 -Wall

SRC = src/main.cpp src/lib/lexer.cpp
BIN = bin
OUT = $(BIN)/lexer

all: $(BIN) $(OUT)

$(BIN):
	mkdir -p $(BIN)

$(OUT):
	$(CXX) $(CXXFLAGS) $(SRC) -o $(OUT)

clean:
	rm -rf $(BIN)