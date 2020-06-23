#
# Makefile for O1
#
# Compiler: g++/gcc
# Linker: g++/gcc
#
#
CXX=g++
CXXFLAGS:=-g -std=c++11 -O3
RM=rm -f
COMMONDIR=../common
NAME=QPP_QPA_iSLIP_sim
OBJ=$(NAME).o
BIN=$(NAME)
SRC=$(NAME).cpp

all: $(BIN)

$(BIN): $(OBJ) $(COMMONDIR)/hdr_histogram.o
	$(CXX) -o $@ $^

$(OBJ): $(SRC)
	$(CXX) $(CXXFLAGS) -I $(COMMONDIR) -c $<

$(COMMONDIR)/hdr_histogram.o: 
	make -f $(COMMONDIR)/Makefile

clean:
	$(RM)  $(OBJ) $(BIN)