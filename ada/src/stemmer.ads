-----------------------------------------------------------------------
--  stemmer -- Multi-language stemmer with Snowball generator
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
package Stemmer with SPARK_Mode is

   WORD_MAX_LENGTH : constant := 1024;

   type Context_Type is abstract tagged private;

   --  Apply the stemming algorithm on the word initialized in the context.
   procedure Stem (Context : in out Context_Type;
                   Result  : out Boolean) is abstract;

   --  Stem the word and return True if it was reduced.
   procedure Stem_Word (Context  : in out Context_Type'Class;
                        Word     : in String;
                        Result   : out Boolean) with
     Global => null,
     Pre => Word'Length < WORD_MAX_LENGTH;

   --  Get the stem or the input word unmodified.
   function Get_Result (Context : in Context_Type'Class) return String with
     Global => null,
     Post => Get_Result'Result'Length < WORD_MAX_LENGTH;

private

   type Mask_Type is mod 2**32;

   --  A 32-bit character value that was read from UTF-8 sequence.
   --  A modular value is used because shift and logical arithmetic is necessary.
   type Utf8_Type is mod 2**32;

   --  Index of the Grouping_Array.  The index comes from the 32-bit character value
   --  minus a starting offset.  We don't expect large tables and we check against
   --  a maximum value.
   subtype Grouping_Index is Utf8_Type range 0 .. 16384;

   type Grouping_Array is array (Grouping_Index range <>) of Boolean with Pack;

   subtype Among_Index is Natural range 0 .. 65535;
   subtype Among_Start_Index is Among_Index range 1 .. Among_Index'Last;
   subtype Operation_Index is Natural range 0 .. 65535;

   type Among_Type is record
      First       : Among_Start_Index;
      Last        : Among_Index;
      Substring_I : Integer;
      Result      : Integer;
      Operation   : Operation_Index;
   end record;

   type Among_Array_Type is array (Natural range <>) of Among_Type;

   function Eq_S (Context : in Context_Type'Class;
                  S       : in String) return Natural with
     Global => null,
     Pre => S'Length > 0,
     Post => Eq_S'Result = 0 or Eq_S'Result = S'Length;

   function Eq_S_Backward (Context : in Context_Type'Class;
                           S       : in String) return Natural with
     Global => null,
     Pre => S'Length > 0,
     Post => Eq_S_Backward'Result = 0 or Eq_S_Backward'Result = S'Length;

   procedure Find_Among (Context : in out Context_Type'Class;
                         Amongs  : in Among_Array_Type;
                         Pattern : in String;
                         Execute : access procedure
                           (Ctx       : in out Context_Type'Class;
                            Operation : in Operation_Index;
                            Status    : out Boolean);
                         Result  : out Integer) with
     Global => null,
     Pre => Pattern'Length > 0 and Amongs'Length > 0;

   procedure Find_Among_Backward (Context : in out Context_Type'Class;
                                  Amongs  : in Among_Array_Type;
                                  Pattern : in String;
                                  Execute : access procedure
                                    (Ctx       : in out Context_Type'Class;
                                     Operation : in Operation_Index;
                                     Status    : out Boolean);
                                  Result  : out Integer) with
     Global => null,
     Pre => Pattern'Length > 0 and Amongs'Length > 0;

   function Skip_Utf8 (Context : in Context_Type'Class;
                       N       : in Positive) return Integer with
     Global => null;

   function Skip_Utf8_Backward (Context : in Context_Type'Class;
                                N       : in Positive) return Integer with
     Global => null;

   procedure Get_Utf8 (Context : in Context_Type'Class;
                       Value   : out Utf8_Type;
                       Count   : out Natural);

   procedure Get_Utf8_Backward (Context : in Context_Type'Class;
                                Value   : out Utf8_Type;
                                Count   : out Natural);

   function Length (Context : in Context_Type'Class) return Natural;

   function Length_Utf8 (Context : in Context_Type'Class) return Natural;

   function Check_Among (Context : in Context_Type'Class;
                         Pos     : in Natural;
                         Shift   : in Natural;
                         Mask    : in Mask_Type) return Boolean;

   procedure Out_Grouping (Context : in out Context_Type'Class;
                           S       : in Grouping_Array;
                           Min     : in Utf8_Type;
                           Max     : in Utf8_Type;
                           Repeat  : in Boolean;
                           Result  : out Integer);

   procedure Out_Grouping_Backward (Context : in out Context_Type'Class;
                                    S       : in Grouping_Array;
                                    Min     : in Utf8_Type;
                                    Max     : in Utf8_Type;
                                    Repeat  : in Boolean;
                                    Result  : out Integer);

   procedure In_Grouping (Context : in out Context_Type'Class;
                          S       : in Grouping_Array;
                          Min     : in Utf8_Type;
                          Max     : in Utf8_Type;
                          Repeat  : in Boolean;
                          Result  : out Integer);

   procedure In_Grouping_Backward (Context : in out Context_Type'Class;
                                   S       : in Grouping_Array;
                                   Min     : in Utf8_Type;
                                   Max     : in Utf8_Type;
                                   Repeat  : in Boolean;
                                   Result  : out Integer);

   procedure Replace (Context    : in out Context_Type'Class;
                      C_Bra      : in Integer;
                      C_Ket      : in Integer;
                      S          : in String;
                      Adjustment : out Integer) with
     Global => null,
     Pre => C_Bra >= Context.Lb and C_Ket >= C_Bra and C_Ket <= Context.L;

   procedure Slice_Del (Context : in out Context_Type'Class) with
     Global => null,
     Pre => Context.Bra >= Context.Lb and Context.Ket >= Context.Bra
     and Context.Ket <= Context.L;

   procedure Slice_From (Context : in out Context_Type'Class;
                         Text    : in String) with
     Global => null,
     Pre => Context.Bra >= Context.Lb and Context.Ket >= Context.Bra
     and Context.Ket <= Context.L
     and Context.L - Context.Lb + Text'Length + Context.Ket - Context.Bra < Context.P'Length;

   function Slice_To (Context : in Context_Type'Class) return String;

   procedure Insert (Context : in out Context_Type'Class;
                     C_Bra   : in Natural;
                     C_Ket   : in Natural;
                     S       : in String) with
     Global => null,
     Pre => C_Bra >= Context.Lb and C_Ket >= C_Bra and C_Ket <= Context.L;

   --  The context indexes follow the C paradigm: they start at 0 for the first character.
   --  This is necessary because several algorithms rely on this when they compare the
   --  cursor position ('C') or setup some markers from the cursor.
   type Context_Type is abstract tagged record
      C   : Natural := 0;
      L   : Integer := 0;
      Lb  : Integer := 0;
      Bra : Integer := 0;
      Ket : Integer := 0;
      P   : String (1 .. WORD_MAX_LENGTH);
   end record;

end Stemmer;
