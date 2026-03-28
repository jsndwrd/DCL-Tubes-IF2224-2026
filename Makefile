CXX      = g++
CXXFLAGS = -std=c++17 -Wall
SRC      = src/main.cpp src/lib/lexer.cpp
BIN      = bin

ifeq ($(OS), Windows_NT)
	OUT = $(BIN)/lexer.exe
	RUN = $(OUT)
	MKDIR = if not exist $(BIN) mkdir $(BIN)
	CLEAN = if exist $(BIN) rmdir /s /q $(BIN)
else
	OUT = $(BIN)/lexer
	RUN = ./$(OUT)
	MKDIR = mkdir -p $(BIN)
	CLEAN = rm -rf $(BIN)
endif

.PHONY: all build run clean

all: build

build: $(OUT)

$(OUT):
	$(MKDIR)
	$(CXX) $(CXXFLAGS) $(SRC) -o $(OUT)

run: build
	$(RUN)

clean:
	$(CLEAN)
