# Vertica Extension Functions

## MallocInfo function

Log the output of malloc_info() system call.

This function is used as the debug purpose. Vertica triggers malloc_trim() system call according to the output of malloc_info(). However, it doesn't have the way to show the raw data of the output of malloc_info().

### Syntax

```
MALLOCINFO ( { ['output_filename'] | null } )
```

### Examples

1. Store the output of malloc_info() into UDX_EVENTS system table

```
=> SELECT mallocinfo(null);
=> SELECT __RAW__['mallocinfo'] FROM udx_events WHERE session_id = current_session();
```

2. Store the output of malloc_info() into a file

```
=> SELECT mallocinfo('/tmp/mallocinfo.txt');
```

### Installation

Set up your environment to meet C++ Requirements described on the following page.
https://www.vertica.com/docs/latest/HTML/Content/Authoring/ExtendingVertica/UDx/DevEnvironment.htm

To compile the source codes in 24.1 or later version, run the following command:

```
$ make
```

In 23.4 or previous version, run the following command:

```
$ CXXFLAGS=-D_GLIBCXX_USE_CXX11_ABI=0 make
```

To install MallocInfo function, run the following statement in vsql:

```
=> CREATE LIBRARY mallocinfolib AS '<your path>/mallocinfo.so' LANGUAGE 'C++';
=> CREATE FUNCTION mallocinfo AS LANGUAGE 'C++' NAME 'MallocInfoFactory' LIBRARY mallocinfolib NOT FENCED;
```

### Notes

This function needs to be created as Unfenced UDx to trigger malloc_info() system call in Vertica process.
