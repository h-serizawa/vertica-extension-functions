#
# Copyright (c) 2024 Hibiki Serizawa
#
# Description: Makefile to build implodeext function
#
# Create Date: February 8, 2024
# Author: Hibiki Serizawa
#
CXX = g++
CXXFLAGS += -I /opt/vertica/sdk/include -Wall -shared -Wno-unused-value -std=c++11 -g -Og
LDFLAGS += -fPIC
LBLIBS +=
VSQL = /opt/vertica/bin/vsql

.PHONEY: implodeext.so install uninstall test clean
all: implodeext.so

implodeext.so: implodeext.cpp /opt/vertica/sdk/include/Vertica.cpp /opt/vertica/sdk/include/BuildInfo.h
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -o $@ $^ $(LBLIBS)

install: implodeext.so
	$(VSQL) -f ./install.sql

uninstall:
	$(VSQL) -f ./uninstall.sql

test: install
	$(VSQL) -f ./sqltest/implodeext_test.sql > ./sqltest/implodeext_test.out

clean:
	rm -f implodeext.so ./sqltest/implodeext_test.out
