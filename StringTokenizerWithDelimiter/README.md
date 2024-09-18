# Vertica Extension Functions

## StrimgTokenizerWithDelimiter function

As of 24.3, Vertica has a preconfigured tokenizer named v_txtindex.StringTokenizer. It splits the string into words by splitting on white space. Another preconfigured tokenizer named v_txtindex.StringTokenizerDelim can split the string into tokens using the specified delimiter character. However, using it as a tokenizer for Text Index is not easy.

StrimgTokenizerWithDelimiter splits the string into words by splitting on user-specified delimiter, and can be used as a tokenizer for Text Index.

### Syntax

```
StrimgTokenizerWithDelimiter (
    unique_id, string_value
)
OVER ()
```

### Arguments
|Argument name|Set to...|
|--|--|
|_unique_id_|The name of the column in the source table that contains a unique identifier.<br/>The column must be the primary key in the source table.|
|_string_value_|The name of the column in the source table that contains the text field. Valid data type is VARCHAR.|

### Session Parameters
|Library name|Parameter name|Set to...|
|--|--|--|
|StringTokenizerWithDelimiterLib|delimiter|A single character to be used as a delimiter. Default value is ';'.|

If StrimgTokenizerWithDelimiter is used as a tokenizer for Text Index with a non-default delimiter, you must set this session parameter every time you insert data into the table.

### Examples

#### Using as Tokenizer for Text Index

```
=> CREATE TABLE musical_scale (id INT NOT NULL, phrase VARCHAR(200), PRIMARY KEY (id) ENABLED) PARTITION BY hash(id);
=> INSERT INTO musical_scale VALUES (1, 'do;re;mi'), (2, 'fa;sol;la;si;do');
=> COMMIT;

=> SELECT * FROM musical_scale;
 id |     phrase
----+-----------------
  1 | do;re;mi
  2 | fa;sol;la;si;do
(2 rows)

=> CREATE TEXT INDEX idx_musical_scale ON musical_scale (id, phrase) STEMMER NONE TOKENIZER StringTokenizerWithDelimiter(VARCHAR);
CREATE INDEX

=> SELECT * FROM idx_musical_scale;
 token | doc_id |      partition
-------+--------+---------------------
 do    |      1 | 5783548743464686114
 mi    |      1 | 5783548743464686114
 re    |      1 | 5783548743464686114
 do    |      2 | 1618211815126016456
 fa    |      2 | 1618211815126016456
 la    |      2 | 1618211815126016456
 si    |      2 | 1618211815126016456
 sol   |      2 | 1618211815126016456
(8 rows)
```

#### Using as Tokenizer for Text Index with non-default delimiter

```
=> CREATE TABLE week (id INT NOT NULL, day VARCHAR(200), PRIMARY KEY (id) ENABLED) PARTITION BY hash(id);
=> INSERT INTO week VALUES (1, 'Mon-Tue-Wed'), (2, 'Thu-Fri-Sat-Sun');
=> COMMIT;

=> SELECT * FROM week;
 id |       day
----+-----------------
  1 | Mon-Tue-Wed
  2 | Thu-Fri-Sat-Sun
(2 rows)

=> ALTER SESSION SET UDPARAMETER FOR StringTokenizerWithDelimiterLib delimiter = '-';

=> CREATE TEXT INDEX idx_week ON week (id, day) STEMMER NONE TOKENIZER StringTokenizerWithDelimiter(VARCHAR);
CREATE INDEX

=> SELECT * FROM idx_week;
 token | doc_id |      partition
-------+--------+---------------------
 Mon   |      1 | 5783548743464686114
 Tue   |      1 | 5783548743464686114
 Wed   |      1 | 5783548743464686114
 Fri   |      2 | 1618211815126016456
 Sat   |      2 | 1618211815126016456
 Sun   |      2 | 1618211815126016456
 Thu   |      2 | 1618211815126016456
(7 rows)
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

To install StrimgTokenizerWithDelimiter function, run the following command:

```
$ make install
```

To uninstall StrimgTokenizerWithDelimiter function, run the following command:

```
$ make uninstall
```

### Notes

StrimgTokenizerWithDelimiter function has been tested in Vertica 23.4 and 24.1.
