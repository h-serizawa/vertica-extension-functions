# Vertica Extension Functions

## AdvancedStringTokenizer function

In 24.1, Vertica removed some preconfigured tokenizers, AdvancedLogTokenizer, BasicLogTokenizer, and WhitespaceLogTokenizer. AdvancedStringTokenizer can be an alternative tokenizer.

AdvancedStringTokenizer separates the text into the tokens using the major and minor separators, and the tokens can include the minor separator characters.

### Syntax

```
AdvancedStringTokenizer (
    unique_id, string_value
)
OVER ()
```

### Arguments
|Argument name|Set to...|
|--|--|
|_unique_id_|The name of the column in the source table that contains a unique identifier.<br/>The column must be the primary key in the source table.|
|_string_value_|The name of the column in the source table that contains the text field. Valid data type is VARCHAR or LONG VARCHAR.|

### Configuration Parameters
|Parameter name|Default value|Set to...|
|--|--|--|
|stopwordscaseinsensitive|''|Comma-separated list of stop words. Words are case insensitive.|
|minorseparators|E'/:=@.-$#%\\\\_'|List of separators used to separate a token into sub-tokens.|
|majorseparators|E' []<>(){}\|!;,''"*&?+\r\n\t'|List of separators used to separate a text into tokens that can include the minor separators.|
|minlength|'2'|Minimum length of a token.|
|maxlength|'128'|Maximum length of a token. The token is truncated if its size exceeds the maximum length.|

Use SetAdvancedStringTokenizerParameter function to set configuration parameters.

```
=> SELECT SetAdvancedStringTokenizerParameter('stopwordscaseinsensitive', 'for,the');
```

Use ReadAdvancedStringTokenizerConfigurationFile function to show all configuration parameters.

```
=> SELECT ReadAdvancedStringTokenizerConfigurationFile() OVER();
        parameter         |         value
--------------------------+------------------------
 majorseparators          |  []<>(){}|!;,'"*&?+

 maxlength                | 128
 minlength                | 2
 minorseparators          | /:=@.-$#%\_
 stopwordscaseinsensitive | for,the
```

### Examples

```
=> CREATE TABLE log (id INT PRIMARY KEY NOT NULL, text VARCHAR(250));
=> CREATE PROJECTION log_super AS SELECT * FROM log ORDER BY id SEGMENTED BY HASH(id) ALL NODES KSAFE;
=> INSERT INTO log VALUES (1, '2014-05-10 00:00:05.700433 %ASA-6-302013: Built outbound TCP connection 9986454 for outside:101.123.123.111/443 (101.123.123.111/443)');
=> COMMIT;
=> CREATE TEXT INDEX idx_log ON log (id, text) STEMMER NONE TOKENIZER AdvancedStringTokenizer(VARCHAR);
=> SELECT * FROM idx_log;
            token            | doc_id
-----------------------------+--------
 %ASA-6-302013:              |      1
 00                          |      1
 00:00:05.700433             |      1
 05                          |      1
 10                          |      1
 101                         |      1
 101.123.123.111/443         |      1
 111                         |      1
 123                         |      1
 2014                        |      1
 2014-05-10                  |      1
 302013                      |      1
 443                         |      1
 700433                      |      1
 9986454                     |      1
 ASA                         |      1
 Built                       |      1
 TCP                         |      1
 connection                  |      1
 for                         |      1
 outbound                    |      1
 outside                     |      1
 outside:101.123.123.111/443 |      1
(23 rows)
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

To install AdvancedStringTokenizer function, run the following command:

```
$ make install
```

To uninstall AdvancedStringTokenizer function, run the following command:

```
$ make uninstall
```

### Notes

AdvancedStringTokenizer function has been tested in Vertica 23.4 to compare the outputs with v_txtindex.AdvancedLogTokenizer.
