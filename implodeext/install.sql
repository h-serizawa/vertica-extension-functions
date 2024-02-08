/**
 * Copyright (c) 2024 Hibiki Serizawa
 *
 * Description: SQL script to install implodeext
 *
 * Create Date: February 8, 2024
 * Author: Hibiki Serizawa
 */

\set libfile '\''`pwd`'/implodeext.so\''
CREATE OR REPLACE LIBRARY implodeextlib AS :libfile LANGUAGE 'C++';
CREATE OR REPLACE TRANSFORM FUNCTION implodeext AS LANGUAGE 'C++' NAME 'ImplodeExtFactory' LIBRARY implodeextlib NOT FENCED;
