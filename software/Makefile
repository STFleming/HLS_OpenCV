CC=gcc
CPP=g++
CFLAGS=-I$(INC_DIR) -lm -lrt -lpthread

LIBS=-lm -lrt

INC_DIR=./includes
BIN_DIR=./bin
SRC_DIR=./src

COBJS=$(BIN_DIR)/ximage_filter.o 
DEPS=$(INC_DIR)/ximage_filter.h 

OPENCV_ARGS=$(shell pkg-config --cflags --libs opencv)

$(BIN_DIR)/%.o: $(INC_DIR)/%.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

$(BIN_DIR)/%.o: $(INC_DIR)/%.cpp $(DEPS)
	$(CPP) -c -o $@ $< $(CFLAGS)

all: sw_access_test clear_output sw_only 

sw_access_test: $(COBJS)
	$(CPP) $(SRC_DIR)/$@.cpp -o $(BIN_DIR)/$@ $^ $(CFLAGS) $(OPENCV_ARGS)

clear_output: $(COBJS)
	 $(CPP) $(SRC_DIR)/$@.cpp -o $(BIN_DIR)/$@ $^ $(CFLAGS) $(OPENCV_ARGS)	

sw_only: $(COBJS)
	 $(CPP) $(SRC_DIR)/$@.cpp -o $(BIN_DIR)/$@ $^ $(CFLAGS) $(OPENCV_ARGS)	

clean:
	rm $(BIN_DIR)/*
