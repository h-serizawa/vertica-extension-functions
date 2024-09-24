/**
 * Copyright (c) 2024 Hibiki Serizawa
 *
 * Description: SQL script to install AdvancedStringTokenizer
 *
 * Create Date: September 24, 2024
 * Author: Hibiki Serizawa
 */

\set libfile '\''`pwd`'/AdvancedStringTokenizer.so\''
CREATE OR REPLACE LIBRARY AdvancedStringTokenizerLib AS :libfile LANGUAGE 'C++';
CREATE OR REPLACE TRANSFORM FUNCTION AdvancedStringTokenizer AS LANGUAGE 'C++' NAME 'AdvancedStringTokenizerFactory' LIBRARY AdvancedStringTokenizerLib NOT FENCED;
CREATE OR REPLACE FUNCTION SetAdvancedStringTokenizerParameter AS LANGUAGE 'C++' NAME 'SetAdvancedStringTokenizerParameterFactory' LIBRARY AdvancedStringTokenizerLib NOT FENCED;
CREATE OR REPLACE TRANSFORM FUNCTION ReadAdvancedStringTokenizerConfigurationFile AS LANGUAGE 'C++' NAME 'ReadAdvancedStringTokenizerConfigurationFileFactory' LIBRARY AdvancedStringTokenizerLib NOT FENCED;
CREATE OR REPLACE FUNCTION DeleteAdvancedStringTokenizerConfigurationFile AS LANGUAGE 'C++' NAME 'DeleteAdvancedStringTokenizerConfigurationFileFactory' LIBRARY AdvancedStringTokenizerLib NOT FENCED;

SELECT SetAdvancedStringTokenizerParameter('stopwordscaseinsensitive', '');
SELECT SetAdvancedStringTokenizerParameter('minorseparators', E'/:=@.-$#%\\_');
SELECT SetAdvancedStringTokenizerParameter('majorseparators', E' []<>(){}|!;,''"*&?+\r\n\t');
SELECT SetAdvancedStringTokenizerParameter('minlength', '2');
SELECT SetAdvancedStringTokenizerParameter('maxlength', '128');
