/**
 * Copyright (c) 2024 Hibiki Serizawa
 *
 * Description: SQL script to install StringTokenizerWithDelimiter
 *
 * Create Date: September 12, 2024
 * Author: Hibiki Serizawa
 */

\set libfile '\''`pwd`'/StringTokenizerWithDelimiter.so\''
CREATE OR REPLACE LIBRARY StringTokenizerWithDelimiterLib AS :libfile LANGUAGE 'C++';
CREATE OR REPLACE TRANSFORM FUNCTION StringTokenizerWithDelimiter AS LANGUAGE 'C++' NAME 'StringTokenizerWithDelimiterFactory' LIBRARY StringTokenizerWithDelimiterLib NOT FENCED;
