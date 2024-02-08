/**
 * Copyright (c) 2024 Hibiki Serizawa
 *
 * Description: SQL script to test implodeext
 *
 * Create Date: February 8, 2024
 * Author: Hibiki Serizawa
 */

-- Create Test table
CREATE TABLE public.implodeext_temp_test (key INTEGER, values ROW(field1 VARCHAR, field2 INTEGER, field3 NUMERIC(10, 2)));

-- Load Test data
\! for i in {1..5}; do for j in {1..10}; do vsql -c "INSERT INTO public.implodeext_temp_test VALUES (${i}, ROW('value${i}_${j}', $((${j} * 1000 + ${i})), ${j}.${i}));COMMIT;"; done; done

-- Test
SELECT key, implodeext(values) OVER (PARTITION BY key) AS values FROM public.implodeext_temp_test ORDER BY key;

-- Drop Test table
DROP TABLE public.implodeext_temp_test CASCADE;
