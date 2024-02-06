/**
 * Copyright (c) 2024 Hibiki Serizawa
 *
 * Description: SQL script to test long2wide
 *
 * Create Date: February 2, 2024
 * Author: Hibiki Serizawa
 */

-- Create Test table
CREATE TABLE public.long2wide_temp_test (key VARCHAR, item1 VARCHAR DEFAULT item2, item2 INTEGER, item3 NUMERIC(10, 2) DEFAULT item2, value1 VARCHAR DEFAULT CHR(64 + item2) || value2, value2 INTEGER, value3 FLOAT DEFAULT value2, value4 NUMERIC DEFAULT value2);

-- Load Test data
\! for i in {1..5}; do for j in {1..10}; do echo "key${i}|${j}|$((${i} * 100 + ${j}))"; done; done | vsql -c 'COPY public.long2wide_temp_test (key, item2, value2) FROM LOCAL STDIN;'

-- Test 1: VARCHAR item and VARCHAR value
\! itemlist=$(vsql -At -c "SELECT LISTAGG(item1 USING PARAMETERS max_length=5120) WITHIN GROUP (ORDER BY item1) FROM (SELECT DISTINCT item1 FROM public.long2wide_temp_test) s;") && vsql -c "SELECT key, long2wide(item1, value1 USING PARAMETERS item_list='${itemlist}', debug=true) OVER (PARTITION BY key) FROM public.long2wide_temp_test ORDER BY key;"

-- Test 2: INTEGER item and INTEGER value
\! itemlist=$(vsql -At -c "SELECT LISTAGG(item2 USING PARAMETERS max_length=5120) WITHIN GROUP (ORDER BY item2) FROM (SELECT DISTINCT item2 FROM public.long2wide_temp_test) s;") && vsql -c "SELECT key, long2wide(item2, value2 USING PARAMETERS item_list='${itemlist}', debug=true) OVER (PARTITION BY key) FROM public.long2wide_temp_test ORDER BY key;"

-- Test 3: NUMERIC item and FLOAT value
\! itemlist=$(vsql -At -c "SELECT LISTAGG(item3 USING PARAMETERS max_length=5120) WITHIN GROUP (ORDER BY item3) FROM (SELECT DISTINCT item3 FROM public.long2wide_temp_test) s;") && vsql -c "SELECT key, long2wide(item3, value3 USING PARAMETERS item_list='${itemlist}', debug=true) OVER (PARTITION BY key) FROM public.long2wide_temp_test ORDER BY key;"

-- Test 4: NUMERIC item and NUMERIC value
\! itemlist=$(vsql -At -c "SELECT LISTAGG(item3 USING PARAMETERS max_length=5120) WITHIN GROUP (ORDER BY item3) FROM (SELECT DISTINCT item3 FROM public.long2wide_temp_test) s;") && vsql -c "SELECT key, long2wide(item3, value4 USING PARAMETERS item_list='${itemlist}', debug=true) OVER (PARTITION BY key) FROM public.long2wide_temp_test ORDER BY key;"

-- Drop Test table
DROP TABLE public.long2wide_temp_test CASCADE;
