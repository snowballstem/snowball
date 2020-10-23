-----------------------------------------------------------------------
--  stemmer-factory -- Multi-language stemmer with Snowball generator
--  Copyright (C) 2020 Stephane Carrez
--  Written by Stephane Carrez (Stephane.Carrez@gmail.com)
--
--  Licensed under the Apache License, Version 2.0 (the "License");
--  you may not use this file except in compliance with the License.
--  You may obtain a copy of the License at
--
--      http://www.apache.org/licenses/LICENSE-2.0
--
--  Unless required by applicable law or agreed to in writing, software
--  distributed under the License is distributed on an "AS IS" BASIS,
--  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
--  See the License for the specific language governing permissions and
--  limitations under the License.
-----------------------------------------------------------------------
package Stemmer.Factory with SPARK_Mode is

   type Language_Type is (L_ARABIC, L_BASQUE, L_CATALAN, L_DANISH, L_DUTCH, L_ENGLISH,
                          L_FINNISH, L_FRENCH, L_GERMAN,
                          L_GREEK, L_HINDI, L_HUNGARIAN, L_INDONESIAN, L_IRISH, L_ITALIAN,
                          L_LITHUANIAN, L_NEPALI, L_NORWEGIAN, L_PORTER,
                          L_PORTUGUESE, L_ROMANIAN, L_RUSSIAN, L_SERBIAN, L_SPANISH,
                          L_SWEDISH, L_TAMIL, L_TURKISH);

   function Stem (Language : in Language_Type;
                  Word     : in String) return String;

end Stemmer.Factory;
