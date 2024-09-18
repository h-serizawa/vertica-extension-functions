#
# Copyright (c) 2024 Hibiki Serizawa
#
# Description: Makefile to build StringTokenizerWithDelimiter function
#
# Create Date: September 12, 2024
# Author: Hibiki Serizawa
#
CXX = g++
CXXFLAGS += -I /opt/vertica/sdk/include -Wall -shared -Wno-unused-value -std=c++11 -g -Og
LDFLAGS += -fPIC
LBLIBS +=
VSQL = /opt/vertica/bin/vsql

.PHONEY: StringTokenizerWithDelimiter.so install uninstall clean
all: StringTokenizerWithDelimiter.so

StringTokenizerWithDelimiter.so: StringTokenizerWithDelimiter.cpp /opt/vertica/sdk/include/Vertica.cpp /opt/vertica/sdk/include/BuildInfo.h
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -o $@ $^ $(LBLIBS)

install: StringTokenizerWithDelimiter.so
	$(VSQL) -f ./install.sql

uninstall:
	$(VSQL) -f ./uninstall.sql

clean:
	rm -f StringTokenizerWithDelimiter.so
