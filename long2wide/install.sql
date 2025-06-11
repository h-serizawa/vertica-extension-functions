/**
 * Copyright (c) 2024-2025 Hibiki Serizawa
 *
 * Description: SQL script to install long2wide
 *
 * Create Date: February 2, 2024
 * Author: Hibiki Serizawa
 */

\set libfile '\''`pwd`'/long2wide.so\''
CREATE OR REPLACE LIBRARY long2widelib AS :libfile LANGUAGE 'C++';
CREATE OR REPLACE TRANSFORM FUNCTION long2wide AS LANGUAGE 'C++' NAME 'Long2WideFactory' LIBRARY long2widelib NOT FENCED;
