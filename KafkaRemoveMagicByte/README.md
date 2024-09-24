# Vertica Extension Functions

## KafkaRemoveMagicByte filter

Vertica database can consume data from Apache Kafka. If the data type is JSON, some middleware based on Kafka adds a magic byte at the beginning of JSON data to show the associated Schema ID. As of Vertica version 24.1, the built-in Kafka integration features can handle JSON data without any additional information like this magic byte.

KafkaRemoveMagicByte is a custom Filter to remove the unexpected data in front of the JSON data.

### Syntax

```
COPY table-table SOURCE KafkaSource([param=value [,...]]) FILTER KafkaRemoveMagicByte() PARSER KafkaJsonParser([param=value [,...]]);
```

### Installation

Set up your environment to meet C++ Requirements described on the following page.
https://docs.vertica.com/latest/en/extending/developing-udxs/developing-with-sdk/setting-up-development-environment/

To compile the source codes in 24.1 or later version, run the following command:

```
$ make
```

In 23.4 or previous version, run the following command:

```
$ CXXFLAGS=-D_GLIBCXX_USE_CXX11_ABI=0 make
```

To install Long2Wide function, run the following command:

```
$ make install
```

To uninstall Long2Wide function, run the following command:

```
$ make uninstall
```

### Notes

KafkaRemoveMagicByte filter has been tested in Vertica 24.1.
