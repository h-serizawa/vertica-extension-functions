# Vertica Extension Functions

## Long2Wide function

Transforms Long-form data to Wide-form data.

Long-form data has one row per observation and one column per variable. This is sometimes called Tidy Data.

|Team|Medal|Count|
|--|--|--|
|United States of America|Gold|39|
|United States of America|Silver|41|
|United States of America|Bronze|33|
|People's Republic of China|Gold|38|
|People's Republic of China|Silver|32|
|People's Republic of China|Bronze|18|
|Japan|Gold|27|
|Japan|Silver|14|
|Japan|Bronze|17|

Wide-form data has one row per value of one of the first variable and one column per value of the second variable. This is sometimes called Messy Data.

|Team|Gold|Silver|Bronze|
|--|--|--|--|
|United States of America|39|41|33|
|People's Republic of China|38|32|18|
|Japan|27|14|17|

### Syntax

```
LONG2WIDE (
    data_item, data_value
    USING PARAMETERS { item_list='comma-separated-items' | item_range_max=max-value [, item_range_min=min-value] } [, zero_if_null=bool_flag] )
OVER ( PARTITION BY expression[,…] )
```

### Arguments
|Argument name|Set to...|
|--|--|
|_data_item_|A table column stores observation or column expression.<br/>LONG2WIDE supports VARCHAR,CHAR,INTEGER,NUMERIC data type for observation column. For the other data types, convert the data to supported data type.|
|_data_value_|A table column stores variable or column expression.<br/>LONG2WIDE supports VARCHAR,CHAR,INTEGER,FLOAT,NUMERIC data type for variable column. For the other data types, convert the data to supported data type.<br/>If observation has multiple variables, only last appeared variable is chosen.|

### Parameters
|Parameter name|Set to...|
|--|--|
|item_list|Comma-separated observations to be displayed as columns. Maximum length is 32,000,000.|
|item_range_max, item_range_min|Maximum/minimum value of the range for observations. List of observations is generated using the range of item_range_min <= observation < item_range_max. Default for item_range_min is 0.|
|zero_if_null|In case that observations specified in item_list don't have the value, it shows NULL value as default. If it sets _true_, it shows ZERO value.|
|PARTITION BY _expression_|Expression on which to divides the rows of the function input. Expression has to be the same as the expression specified before Long2Wide function in SELECT clause.|

### Examples

```
=> CREATE TABLE public.olympics_medals (team VARCHAR(30), medal VARCHAR(6), count INTEGER);

=> INSERT INTO public.olympics_medals VALUES ('United States of America', 'Gold', 39);
=> INSERT INTO public.olympics_medals VALUES ('United States of America', 'Silver', 41);
=> INSERT INTO public.olympics_medals VALUES ('United States of America', 'Bronze', 33);
=> INSERT INTO public.olympics_medals VALUES ('People''s Republic of China', 'Gold', 38);
=> INSERT INTO public.olympics_medals VALUES ('People''s Republic of China', 'Silver', 32);
=> INSERT INTO public.olympics_medals VALUES ('People''s Republic of China', 'Bronze', 18);
=> INSERT INTO public.olympics_medals VALUES ('Japan', 'Gold', 27);
=> INSERT INTO public.olympics_medals VALUES ('Japan', 'Silver', 14);
=> INSERT INTO public.olympics_medals VALUES ('Japan', 'Bronze', 17);
=> COMMIT;

=> SELECT team, medal, count FROM public.olympics_medals;

            team            | medal  | count
----------------------------+--------+-------
 United States of America   | Gold   |    39
 United States of America   | Silver |    41
 United States of America   | Bronze |    33
 People's Republic of China | Gold   |    38
 People's Republic of China | Silver |    32
 People's Republic of China | Bronze |    18
 Japan                      | Gold   |    27
 Japan                      | Silver |    14
 Japan                      | Bronze |    17
(9 rows)

=> SELECT team, long2wide(medal, count USING PARAMETERS item_list='Gold,Silver,Bronze') OVER (PARTITION BY team) FROM public.olympics_medals;

            team            | Gold | Silver | Bronze
----------------------------+------+--------+--------
 United States of America   |   39 |     41 |     33
 People's Republic of China |   38 |     32 |     18
 Japan                      |   27 |     14 |     17
(3 rows)
```

### Installation

Set up your environment to meet C++ Requirements described on the following page.
https://www.vertica.com/docs/latest/HTML/Content/Authoring/ExtendingVertica/UDx/DevEnvironment.htm

To compile the source codes, run the following command:

```
$ make
```

To install Long2Wide function, run the following statement in vsql:

```
=> CREATE LIBRARY long2widelib AS '<your path>/long2wide.so' LANGUAGE 'C++';
=> CREATE TRANSFORM FUNCTION long2wide AS LANGUAGE 'C++' NAME 'Long2WideFactory' LIBRARY long2widelib NOT FENCED;
```

### Notes

Long2Wide function has been tested in Vertica 11.0.1.
