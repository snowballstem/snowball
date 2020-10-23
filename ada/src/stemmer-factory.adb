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
with Stemmer.Arabic;
with Stemmer.Basque;
with Stemmer.Catalan;
with Stemmer.Danish;
with Stemmer.Dutch;
with Stemmer.English;
with Stemmer.Finnish;
with Stemmer.French;
with Stemmer.German;
with Stemmer.Greek;
with Stemmer.Hindi;
with Stemmer.Hungarian;
with Stemmer.Indonesian;
with Stemmer.Irish;
with Stemmer.Italian;
with Stemmer.Lithuanian;
with Stemmer.Nepali;
with Stemmer.Norwegian;
with Stemmer.Porter;
with Stemmer.Portuguese;
with Stemmer.Romanian;
with Stemmer.Russian;
with Stemmer.Serbian;
with Stemmer.Spanish;
with Stemmer.Swedish;
with Stemmer.Tamil;
with Stemmer.Turkish;
package body Stemmer.Factory with SPARK_Mode is

   function Stem (Language : in Language_Type;
                  Word     : in String) return String is
      Result : Boolean := False;
   begin
      case Language is
         when L_ARABIC =>
            declare
               C : Stemmer.Arabic.Context_Type;
            begin
               C.Stem_Word (Word, Result);
               return Get_Result (C);
            end;

         when L_BASQUE =>
            declare
               C : Stemmer.Basque.Context_Type;
            begin
               C.Stem_Word (Word, Result);
               return Get_Result (C);
            end;

         when L_CATALAN =>
            declare
               C : Stemmer.Catalan.Context_Type;
            begin
               C.Stem_Word (Word, Result);
               return Get_Result (C);
            end;

         when L_DANISH =>
            declare
               C : Stemmer.Danish.Context_Type;
            begin
               C.Stem_Word (Word, Result);
               return Get_Result (C);
            end;

         when L_DUTCH =>
            declare
               C : Stemmer.Dutch.Context_Type;
            begin
               C.Stem_Word (Word, Result);
               return Get_Result (C);
            end;

         when L_ENGLISH =>
            declare
               C : Stemmer.English.Context_Type;
            begin
               C.Stem_Word (Word, Result);
               return Get_Result (C);
            end;

         when L_FINNISH =>
            declare
               C : Stemmer.Finnish.Context_Type;
            begin
               C.Stem_Word (Word, Result);
               return Get_Result (C);
            end;

         when L_FRENCH =>
            declare
               C : Stemmer.French.Context_Type;
            begin
               C.Stem_Word (Word, Result);
               return Get_Result (C);
            end;

         when L_GERMAN =>
            declare
               C : Stemmer.German.Context_Type;
            begin
               C.Stem_Word (Word, Result);
               return Get_Result (C);
            end;

         when L_GREEK =>
            declare
               C : Stemmer.Greek.Context_Type;
            begin
               C.Stem_Word (Word, Result);
               return Get_Result (C);
            end;

         when L_HINDI =>
            declare
               C : Stemmer.Hindi.Context_Type;
            begin
               C.Stem_Word (Word, Result);
               return Get_Result (C);
            end;

         when L_HUNGARIAN =>
            declare
               C : Stemmer.Hungarian.Context_Type;
            begin
               C.Stem_Word (Word, Result);
               return Get_Result (C);
            end;

         when L_INDONESIAN =>
            declare
               C : Stemmer.Indonesian.Context_Type;
            begin
               C.Stem_Word (Word, Result);
               return Get_Result (C);
            end;

         when L_IRISH =>
            declare
               C : Stemmer.Irish.Context_Type;
            begin
               C.Stem_Word (Word, Result);
               return Get_Result (C);
            end;

         when L_ITALIAN =>
            declare
               C : Stemmer.Italian.Context_Type;
            begin
               C.Stem_Word (Word, Result);
               return Get_Result (C);
            end;

         when L_LITHUANIAN =>
            declare
               C : Stemmer.Lithuanian.Context_Type;
            begin
               C.Stem_Word (Word, Result);
               return Get_Result (C);
            end;

         when L_NEPALI =>
            declare
               C : Stemmer.Nepali.Context_Type;
            begin
               C.Stem_Word (Word, Result);
               return Get_Result (C);
            end;

         when L_NORWEGIAN =>
            declare
               C : Stemmer.Norwegian.Context_Type;
            begin
               C.Stem_Word (Word, Result);
               return Get_Result (C);
            end;

         when L_PORTER =>
            declare
               C : Stemmer.Porter.Context_Type;
            begin
               C.Stem_Word (Word, Result);
               return Get_Result (C);
            end;

         when L_PORTUGUESE =>
            declare
               C : Stemmer.Portuguese.Context_Type;
            begin
               C.Stem_Word (Word, Result);
               return Get_Result (C);
            end;

         when L_ROMANIAN =>
            declare
               C : Stemmer.Romanian.Context_Type;
            begin
               C.Stem_Word (Word, Result);
               return Get_Result (C);
            end;

         when L_RUSSIAN =>
            declare
               C : Stemmer.Russian.Context_Type;
            begin
               C.Stem_Word (Word, Result);
               return Get_Result (C);
            end;

         when L_SERBIAN =>
            declare
               C : Stemmer.Serbian.Context_Type;
            begin
               C.Stem_Word (Word, Result);
               return Get_Result (C);
            end;

         when L_SPANISH =>
            declare
               C : Stemmer.Spanish.Context_Type;
            begin
               C.Stem_Word (Word, Result);
               return Get_Result (C);
            end;

         when L_SWEDISH =>
            declare
               C : Stemmer.Swedish.Context_Type;
            begin
               C.Stem_Word (Word, Result);
               return Get_Result (C);
            end;

         when L_TAMIL =>
            declare
               C : Stemmer.Tamil.Context_Type;
            begin
               C.Stem_Word (Word, Result);
               return Get_Result (C);
            end;

         when L_TURKISH =>
            declare
               C : Stemmer.Turkish.Context_Type;
            begin
               C.Stem_Word (Word, Result);
               return Get_Result (C);
            end;

      end case;
   end Stem;

end Stemmer.Factory;
