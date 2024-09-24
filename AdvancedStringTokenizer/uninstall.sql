/**
 * Copyright (c) 2024 Hibiki Serizawa
 *
 * Description: SQL script to uninstall AdvancedStringTokenizer
 *
 * Create Date: September 24, 2024
 * Author: Hibiki Serizawa
 */

SELECT DeleteAdvancedStringTokenizerConfigurationFile();
DROP LIBRARY AdvancedStringTokenizerLib CASCADE;
