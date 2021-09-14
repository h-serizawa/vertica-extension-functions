#
# Copyright (c) 2021 Hibiki Serizawa
#
# Description: Makefile to build long2wide function
#
# Create Date: September 14, 2021
# Author: Hibiki Serizawa
#
CXX = g++
CXXFLAGS = -I /opt/vertica/sdk/include -Wall -shared -Wno-unused-value -std=c++11 -D_GLIBCXX_USE_CXX11_ABI=0 -g -Og
LDFLAGS = -fPIC
LBLIBS =

SRC = long2wide.cpp
OBJ = $(SRC:.cpp=.so)

all: $(OBJ)

$(OBJ): $(SRC)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -o $@ $^ /opt/vertica/sdk/include/Vertica.cpp $(LBLIBS)

clean:
	rm -f $(OBJ)
