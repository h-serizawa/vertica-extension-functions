/**
 * Copyright (c) 2024 Hibiki Serizawa
 *
 * Description: SQL script to install KafkaRemoveMagicByte
 *
 * Create Date: March 15, 2024
 * Author: Hibiki Serizawa
 */

\set libfile '\''`pwd`'/KafkaRemoveMagicByte.so\''
CREATE OR REPLACE LIBRARY KafkaRemoveMagicByteLib AS :libfile LANGUAGE 'C++';
CREATE OR REPLACE FILTER KafkaRemoveMagicByte AS LANGUAGE 'C++' NAME 'KafkaRemoveMagicByteFactory' LIBRARY KafkaRemoveMagicByteLib NOT FENCED;
