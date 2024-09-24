#
# Copyright (c) 2024 Hibiki Serizawa
#
# Description: Makefile to build AdvancedStringTokenizer library
#
# Create Date: September 24, 2024
# Author: Hibiki Serizawa
#
CXX = g++
CXXFLAGS += -I /opt/vertica/sdk/include -Wall -shared -Wno-unused-value -std=c++11 -g -Og
LDFLAGS += -fPIC
LBLIBS +=
VSQL = /opt/vertica/bin/vsql

.PHONEY: AdvancedStringTokenizer.so install uninstall clean
all: AdvancedStringTokenizer.so

AdvancedStringTokenizer.so: AdvancedStringTokenizer.cpp DFSUtil.cpp SetAdvancedStringTokenizerParameter.cpp ReadAdvancedStringTokenizerConfigurationFile.cpp DeleteAdvancedStringTokenizerConfigurationFile.cpp /opt/vertica/sdk/include/Vertica.cpp /opt/vertica/sdk/include/BuildInfo.h
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -o $@ $^ $(LBLIBS)

install: AdvancedStringTokenizer.so
	$(VSQL) -f ./install.sql

uninstall:
	$(VSQL) -f ./uninstall.sql

clean:
	rm -f AdvancedStringTokenizer.so
