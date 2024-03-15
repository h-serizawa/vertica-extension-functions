CXX = g++
CXXFLAGS += -I /opt/vertica/sdk/include -Wall -shared -Wno-unused-value -std=c++11 -g -Og
LDFLAGS += -fPIC
LBLIBS +=
VSQL = /opt/vertica/bin/vsql

.PHONEY: KafkaRemoveMagicByte.so install uninstall clean
all: KafkaRemoveMagicByte.so

KafkaRemoveMagicByte.so: KafkaRemoveMagicByte.cpp /opt/vertica/sdk/include/Vertica.cpp /opt/vertica/sdk/include/BuildInfo.h
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -o $@ $^ $(LBLIBS)

install: KafkaRemoveMagicByte.so
	$(VSQL) -f ./install.sql

uninstall:
	$(VSQL) -f ./uninstall.sql

clean:
	rm -f KafkaRemoveMagicByte.so
	