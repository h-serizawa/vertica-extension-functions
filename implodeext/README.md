# Vertica Extension Functions

## ImplodeExt function

Assembles data into array grouped by keys.

Original Implode function has been introduced in Vertica 10.1.0. It accepts only scalar data type as of 12.0. ImplodeExt extends original Implode function to support ARRAY and ROW complex type.

### Syntax

```
IMPLODEEXT (
    input_column
    USING PARAMETERS { max_elements=max-value | allow_truncate=bool_flag] )
OVER ( PARTITION BY expression[,…] )
```

### Arguments
|Argument name|Set to...|
|--|--|
|_input_column_|Column from which to create the array.|

### Parameters
|Parameter name|Set to...|
|--|--|
|max_elements|Maximum number of output elements. Default is 256.|
|allow_truncate|Boolean, if it is true, it truncates results when output elements exceeds maximum number of elements. If it is false, the function returns an error if the output array is too large. Default is false.|
|PARTITION BY _expression_|Expression on which to divides the rows of the function input. Expression has to be the same as the expression specified before ImplodeExt function in SELECT clause.|

### Installation

Set up your environment to meet C++ Requirements described on the following page.
https://www.vertica.com/docs/latest/HTML/Content/Authoring/ExtendingVertica/UDx/DevEnvironment.htm

To compile the source codes, run the following command:

```
$ make
```

To install ImplodeExt function, run the following statement in vsql:

```
=> CREATE LIBRARY implodeextlib AS '<your path>/implodeext.so' LANGUAGE 'C++';
=> CREATE TRANSFORM FUNCTION implodeext AS LANGUAGE 'C++' NAME 'ImplodeExtFactory' LIBRARY implodeextlib [NOT] FENCED;
```

### Notes

ImplodeExt function has been tested in Vertica 11.1.1.