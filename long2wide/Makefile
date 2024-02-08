#
# Copyright (c) 2024 Hibiki Serizawa
#
# Description: Makefile to build long2wide function
#
# Create Date: February 2, 2024
# Author: Hibiki Serizawa
#
CXX = g++
CXXFLAGS += -I /opt/vertica/sdk/include -Wall -shared -Wno-unused-value -std=c++11 -g -Og
LDFLAGS += -fPIC
LBLIBS +=
VSQL = /opt/vertica/bin/vsql

.PHONEY: long2wide.so install uninstall test clean
all: long2wide.so

long2wide.so: long2wide.cpp /opt/vertica/sdk/include/Vertica.cpp /opt/vertica/sdk/include/BuildInfo.h
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -o $@ $^ $(LBLIBS)

install: long2wide.so
	$(VSQL) -f ./install.sql

uninstall:
	$(VSQL) -f ./uninstall.sql

test: install
	$(VSQL) -f ./sqltest/long2wide_test.sql > ./sqltest/long2wide_test.out

clean:
	rm -f long2wide.so ./sqltest/long2wide_test.out
