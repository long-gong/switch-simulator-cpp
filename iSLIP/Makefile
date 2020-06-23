#
# Makefile for iSLIP
#
# Compiler: g++/gcc
# Linker: g++/gcc
#
#
CXX=g++
CXXFLAGS:=-g -std=c++11 -O3
RM=rm -f
COMMONDIR=../common
OBJ=iSLIP_sim.o
BIN=iSLIP_sim

all: $(BIN)

$(BIN): $(OBJ) $(COMMONDIR)/hdr_histogram.o
	$(CXX) -o $@ $^

$(OBJ): iSLIP_sim.cpp
	$(CXX) $(CXXFLAGS) -I $(COMMONDIR) -c $<

$(COMMONDIR)/hdr_histogram.o: 
	make -f $(COMMONDIR)/Makefile

clean:
	$(RM)  $(OBJ) $(BIN)

