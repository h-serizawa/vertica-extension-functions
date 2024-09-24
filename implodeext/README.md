# Vertica Extension Functions

## ImplodeExt function

Assembles data into array grouped by keys.

Original Implode function has been introduced in Vertica 10.1.0. It accepts only scalar data type as of 24.1. ImplodeExt extends original Implode function to support ARRAY and ROW complex type.

### Syntax

```
IMPLODEEXT (
    input_column
    [ USING PARAMETERS { max_elements=max-value | allow_truncate=bool_flag } ] )
OVER ( PARTITION BY expression[,…] )
```

### Arguments
|Argument name|Set to...|
|--|--|
|_input_column_|Column from which to create the array.|

### Parameters
|Parameter name|Set to...|
|--|--|
|max_elements|Maximum number of output elements. Default is 256.<br/>Even if this parameter is set to huge value, the function returns an error if the output array exceeds 32MB that is the system limit of length of variable-length column.|
|allow_truncate|Boolean, if it is true, it truncates results when output elements exceeds maximum number of elements. If it is false, the function returns an error if the output array is too large. Default is false.|
|PARTITION BY _expression_|Expression on which to divides the rows of the function input. Expression has to be the same as the expression specified before ImplodeExt function in SELECT clause.|

### Examples

```
=> SELECT * FROM public.statuses ORDER BY equipment_id;

 equipment_id |                  status
--------------+------------------------------------------
     18498209 | {"part1":1265,"part2":1716,"part3":1403}
     18498209 | {"part1":1279,"part2":1820,"part3":1426}
     18498209 | {"part1":1271,"part2":1802,"part3":1412}
     21848523 | {"part1":1329,"part2":1808,"part3":1487}
     21848523 | {"part1":1303,"part2":1811,"part3":1500}
     21848523 | {"part1":1314,"part2":1841,"part3":1490}
(6 rows)

=> SELECT equipment_id, implodeext(status) OVER (PARTITION BY equipment_id) AS status FROM public.statuses ORDER BY equipment_id;

 equipment_id |                                                            status
--------------+------------------------------------------------------------------------------------------------------------------------------
     18498209 | [{"part1":1265,"part2":1716,"part3":1403},{"part1":1279,"part2":1820,"part3":1426},{"part1":1271,"part2":1802,"part3":1412}]
     21848523 | [{"part1":1329,"part2":1808,"part3":1487},{"part1":1303,"part2":1811,"part3":1500},{"part1":1314,"part2":1841,"part3":1490}]
(2 rows)
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

To install ImplodeExt function, run the following command:

```
$ make install
```

To uninstall ImplodeExt function, run the following command:

```
$ make uninstall
```

To test ImplodeExt function, run the following command. Output file is ./sqltest/implodeext_test.out:

```
$ make test
```

### Notes

ImplodeExt function has been tested in Vertica 23.4 and 24.1.
