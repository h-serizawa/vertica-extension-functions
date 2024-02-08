#
# Copyright (c) 2024 Hibiki Serizawa
#
# Description: Makefile to build mallocinfo function
#
# Create Date: February 8, 2024
# Author: Hibiki Serizawa
#
CXX = g++
CXXFLAGS += -I /opt/vertica/sdk/include -Wall -shared -Wno-unused-value -std=c++11 -g -Og
LDFLAGS += -fPIC
LBLIBS +=

.PHONEY: mallocinfo.so clean
all: mallocinfo.so

mallocinfo.so: mallocinfo.cpp /opt/vertica/sdk/include/Vertica.cpp /opt/vertica/sdk/include/BuildInfo.h
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -o $@ $^ $(LBLIBS)

clean:
	rm -f mallocinfo.so
