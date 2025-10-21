-----------------------------------------------------------------------
--  stemmer -- Multi-language stemmer with Snowball generator
--  Written by Stephane Carrez (Stephane.Carrez@gmail.com)
--  All rights reserved.
--
--  Redistribution and use in source and binary forms, with or without
--  modification, are permitted provided that the following conditions
--  are met:
--
--    1. Redistributions of source code must retain the above copyright notice,
--       this list of conditions and the following disclaimer.
--    2. Redistributions in binary form must reproduce the above copyright notice,
--       this list of conditions and the following disclaimer in the documentation
--       and/or other materials provided with the distribution.
--    3. Neither the name of the Snowball project nor the names of its contributors
--       may be used to endorse or promote products derived from this software
--       without specific prior written permission.
--
--  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
--  ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
--  WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
--  DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
--  ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
--  (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
--  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
--  ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
--  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
--  SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
-----------------------------------------------------------------------
with Interfaces;
package body Stemmer with SPARK_Mode is

   subtype Byte is Interfaces.Unsigned_8;
   use type Interfaces.Unsigned_8;

   procedure Stem_Word (Z      : in out Context_Type'Class;
                        Word   : in String;
                        Result : out Boolean) is
   begin
      Z.P (1 .. Word'Length) := Word;
      Z.Len := Word'Length;
      Z.C := 0;
      Z.L := Word'Length;
      Z.Lb := 0;
      Stemmer.Stem (Z, Result);
   end Stem_Word;

   function Get_Result (Z : in Context_Type'Class) return String is
   begin
      return Z.P (1 .. Z.Len);
   end Get_Result;

   procedure Eq_S (Z      : in out Context_Type'Class;
                   S      : in String;
                   Len    : in Char_Index;
                   Result : out Boolean) is
   begin
      if Z.L - Z.C < Len then
         Result := False;
         return;
      end if;
      if Z.P (Z.C + 1 .. Z.C + Len) /= S (S'First .. Len) then
         Result := False;
         return;
      end if;
      Z.C := Z.C + Len;
      Result := True;
   end Eq_S;

   procedure Eq_S_Backward (Z      : in out Context_Type'Class;
                            S      : in String;
                            Len    : in Char_Index;
                            Result : out Boolean) is
   begin
      if Z.C - Z.Lb < Len then
         Result := False;
         return;
      end if;
      if Z.P (Z.C + 1 - Len .. Z.C) /= S (S'First .. Len) then
         Result := False;
         return;
      end if;
      Z.C := Z.C - Len;
      Result := True;
   end Eq_S_Backward;

   function Length_Utf8 (S   : in String;
                         Len : in Char_Index) return Natural is
      Count : Natural := 0;
      Pos   : Positive := 1;
      Val   : Byte;
   begin
      while Pos <= Len loop
         Val := Character'Pos (S (Pos));
         Pos := Pos + 1;
         if Val >= 16#C0# or Val < 16#80# then
            Count := Count + 1;
         end if;
      end loop;
      return Count;
   end Length_Utf8;

   function Check_Among (Z     : in Context_Type'Class;
                         Pos   : in Char_Index;
                         Shift : in Natural;
                         Mask  : in Mask_Type) return Boolean is
      use Interfaces;
      Val : constant Byte := Character'Pos (Z.P (Pos + 1));
   begin
      if Natural (Shift_Right (Val, 5)) /= Shift then
         return True;
      end if;
      return (Shift_Right (Unsigned_64 (Mask), Natural (Val and 16#1f#)) and 1) = 0;
   end Check_Among;

   procedure Find_Among (Z       : in out Context_Type'Class;
                         Amongs  : in Among_Array_Type;
                         Pattern : in String;
                         Execute : access procedure
                           (Ctx       : in out Context_Type'Class;
                            Operation : in Operation_Index;
                            Status    : out Boolean);
                         Result  : out Integer) is
      I   : Natural := Amongs'First;
      J   : Natural := Amongs'Last + 1;
      Common_I : Natural := 0;
      Common_J : Natural := 0;
      First_Key_Inspected : Boolean := False;
      C   : constant Natural := Z.C;
      L   : constant Integer := Z.L;
   begin
      loop
         declare
            K      : constant Natural := I + (J - I) / 2;
            W      : constant Among_Type := Amongs (K);
            Common : Natural := (if Common_I < Common_J then Common_I else Common_J);
            Diff   : Integer := 0;
         begin
            for I2 in W.First + Common .. W.Last loop
               if C + Common = L then
                  Diff := -1;
                  exit;
               end if;
               Diff := Character'Pos (Z.P (C + Common + 1))
                 - Character'Pos (Pattern (I2));
               exit when Diff /= 0;
               Common := Common + 1;
            end loop;
            if Diff < 0 then
               J := K;
               Common_J := Common;
            else
               I := K;
               Common_I := Common;
            end if;
         end;
         if J - I <= 1 then
            exit when I > 0 or J = I or First_Key_Inspected;
            First_Key_Inspected := True;
         end if;
      end loop;

      loop
         declare
            W   : constant Among_Type := Amongs (I);
            Len : constant Natural := W.Last - W.First + 1;
            Status : Boolean;
         begin
            if Common_I >= Len then
               Z.C := C + Len;
               if W.Operation = 0 then
                  Result := W.Result;
                  return;
               end if;
               Execute (Z, W.Operation, Status);
               if Status then
                  Z.C := C + Len;
                  Result := W.Result;
                  return;
               end if;
            end if;
            exit when W.Substring_I < 0;
            I := W.Substring_I;
         end;
      end loop;
      Result := 0;
   end Find_Among;

   procedure Find_Among_Backward (Z       : in out Context_Type'Class;
                                  Amongs  : in Among_Array_Type;
                                  Pattern : in String;
                                  Execute : access procedure
                                    (Ctx       : in out Context_Type'Class;
                                     Operation : in Operation_Index;
                                     Status    : out Boolean);
                                  Result  : out Integer) is
      I   : Natural := Amongs'First;
      J   : Natural := Amongs'Last + 1;
      Common_I : Natural := 0;
      Common_J : Natural := 0;
      First_Key_Inspected : Boolean := False;
      C   : constant Integer := Z.C;
      Lb  : constant Integer := Z.Lb;
   begin
      loop
         declare
            K      : constant Natural := I + (J - I) / 2;
            W      : constant Among_Type := Amongs (K);
            Common : Natural := (if Common_I < Common_J then Common_I else Common_J);
            Diff   : Integer := 0;
         begin
            for I2 in reverse W.First .. W.Last - Common loop
               if C - Common = Lb then
                  Diff := -1;
                  exit;
               end if;
               Diff := Character'Pos (Z.P (C - Common))
                 - Character'Pos (Pattern (I2));
               exit when Diff /= 0;
               Common := Common + 1;
            end loop;
            if Diff < 0 then
               J := K;
               Common_J := Common;
            else
               I := K;
               Common_I := Common;
            end if;
         end;
         if J - I <= 1 then
            exit when I > 0 or J = I or First_Key_Inspected;
            First_Key_Inspected := True;
         end if;
      end loop;

      loop
         declare
            W   : constant Among_Type := Amongs (I);
            Len : constant Natural := W.Last - W.First + 1;
            Status : Boolean;
         begin
            if Common_I >= Len then
               Z.C := C - Len;
               if W.Operation = 0 then
                  Result := W.Result;
                  return;
               end if;
               Execute (Z, W.Operation, Status);
               if Status then
                  Z.C := C - Len;
                  Result := W.Result;
                  return;
               end if;
            end if;
            exit when W.Substring_I < 0;
            I := W.Substring_I;
         end;
      end loop;
      Result := 0;
   end Find_Among_Backward;

   function Skip_Utf8 (Z : in Context_Type'Class) return Result_Index is
      Pos : Char_Index := Z.C;
      Val : Byte;
   begin
      if Pos >= Z.L then
         return -1;
      end if;
      Pos := Pos + 1;
      Val := Character'Pos (Z.P (Pos));
      if Val >= 16#C0# then
         while Pos < Z.L loop
            Val := Character'Pos (Z.P (Pos + 1));
            exit when Val >= 16#C0# or Val < 16#80#;
            Pos := Pos + 1;
         end loop;
      end if;
      return Pos;
   end Skip_Utf8;

   function Skip_Utf8 (Z : in Context_Type'Class;
                       N       : in Integer) return Result_Index is
      Pos : Char_Index := Z.C;
      Val : Byte;
   begin
      if N < 0 then
          return -1;
      end if;
      for I in 1 .. N loop
         if Pos >= Z.L then
            return -1;
         end if;
         Pos := Pos + 1;
         Val := Character'Pos (Z.P (Pos));
         if Val >= 16#C0# then
            while Pos < Z.L loop
               Val := Character'Pos (Z.P (Pos + 1));
               exit when Val >= 16#C0# or Val < 16#80#;
               Pos := Pos + 1;
            end loop;
         end if;
      end loop;
      return Pos;
   end Skip_Utf8;

   function Skip_Utf8_Backward (Z : in Context_Type'Class) return Result_Index is
      Pos : Char_Index := Z.C;
      Val : Byte;
   begin
      if Pos <= Z.Lb then
         return -1;
      end if;
      Val := Character'Pos (Z.P (Pos));
      Pos := Pos - 1;
      if Val >= 16#80# then
         while Pos > Z.Lb loop
            Val := Character'Pos (Z.P (Pos + 1));
            exit when Val >= 16#C0#;
            Pos := Pos - 1;
         end loop;
      end if;
      return Pos;
   end Skip_Utf8_Backward;

   function Skip_Utf8_Backward (Z : in Context_Type'Class;
                                N : in Integer) return Result_Index is
      Pos : Char_Index := Z.C;
      Val : Byte;
   begin
      if N < 0 then
          return -1;
      end if;
      for I in 1 .. N loop
         if Pos <= Z.Lb then
            return -1;
         end if;
         Val := Character'Pos (Z.P (Pos));
         Pos := Pos - 1;
         if Val >= 16#80# then
            while Pos > Z.Lb loop
               Val := Character'Pos (Z.P (Pos + 1));
               exit when Val >= 16#C0#;
               Pos := Pos - 1;
            end loop;
         end if;
      end loop;
      return Pos;
   end Skip_Utf8_Backward;

   function Shift_Left (Value : in Utf8_Type;
                        Shift : in Natural) return Utf8_Type
     is (Utf8_Type (Interfaces.Shift_Left (Interfaces.Unsigned_32 (Value), Shift)));

   procedure Get_Utf8 (Z     : in Context_Type'Class;
                       Value : out Utf8_Type;
                       Count : out Natural) is
      B0, B1, B2, B3 : Byte;
   begin
      if Z.C >= Z.L then
         Value := 0;
         Count := 0;
         return;
      end if;
      B0 := Character'Pos (Z.P (Z.C + 1));
      if B0 < 16#C0# or Z.C + 1 >= Z.L then
         Value := Utf8_Type (B0);
         Count := 1;
         return;
      end if;
      B1 := Character'Pos (Z.P (Z.C + 2)) and 16#3F#;
      if B0 < 16#E0# or Z.C + 2 >= Z.L then
         Value := Shift_Left (Utf8_Type (B0 and 16#1F#), 6) or Utf8_Type (B1);
         Count := 2;
         return;
      end if;
      B2 := Character'Pos (Z.P (Z.C + 3)) and 16#3F#;
      if B0 < 16#F0# or Z.C + 3 >= Z.L then
         Value := Shift_Left (Utf8_Type (B0 and 16#0F#), 12)
           or Shift_Left (Utf8_Type (B1), 6) or Utf8_Type (B2);
         Count := 3;
         return;
      end if;
      B3 := Character'Pos (Z.P (Z.C + 4)) and 16#3F#;
      Value := Shift_Left (Utf8_Type (B0 and 16#07#), 18)
        or Shift_Left (Utf8_Type (B1), 12)
        or Shift_Left (Utf8_Type (B2), 6) or Utf8_Type (B3);
      Count := 4;
   end Get_Utf8;

   procedure Get_Utf8_Backward (Z     : in Context_Type'Class;
                                Value : out Utf8_Type;
                                Count : out Natural) is
      B0, B1, B2, B3 : Byte;
   begin
      if Z.C <= Z.Lb then
         Value := 0;
         Count := 0;
         return;
      end if;
      B3 := Character'Pos (Z.P (Z.C));
      if B3 < 16#80# or Z.C - 1 <= Z.Lb then
         Value := Utf8_Type (B3);
         Count := 1;
         return;
      end if;
      B2 := Character'Pos (Z.P (Z.C - 1));
      if B2 >= 16#C0# or Z.C - 2 <= Z.Lb then
         B3 := B3 and 16#3F#;
         Value := Shift_Left (Utf8_Type (B2 and 16#1F#), 6) or Utf8_Type (B3);
         Count := 2;
         return;
      end if;
      B1 := Character'Pos (Z.P (Z.C - 2));
      if B1 >= 16#E0# or Z.C - 3 <= Z.Lb then
         B3 := B3 and 16#3F#;
         B2 := B2 and 16#3F#;
         Value := Shift_Left (Utf8_Type (B1 and 16#0F#), 12)
           or Shift_Left (Utf8_Type (B2), 6) or Utf8_Type (B3);
         Count := 3;
         return;
      end if;
      B0 := Character'Pos (Z.P (Z.C - 3));
      B1 := B1 and 16#1F#;
      B2 := B2 and 16#3F#;
      B3 := B3 and 16#3F#;
      Value := Shift_Left (Utf8_Type (B0 and 16#07#), 18)
        or Shift_Left (Utf8_Type (B1), 12)
        or Shift_Left (Utf8_Type (B2), 6) or Utf8_Type (B3);
      Count := 4;
   end Get_Utf8_Backward;

   procedure Out_Grouping (Z      : in out Context_Type'Class;
                           S      : in Grouping_Array;
                           Min    : in Utf8_Type;
                           Max    : in Utf8_Type;
                           Repeat : in Boolean;
                           Result : out Result_Index) is
      Ch    : Utf8_Type;
      Count : Natural;
   begin
      if Z.C >= Z.L then
         Result := -1;
         return;
      end if;

      loop
         Get_Utf8 (Z, Ch, Count);
         if Count = 0 then
            Result := -1;
            return;
         end if;
         if Ch <= Max and Ch >= Min then
            Ch := Ch - Min;
            if S (Ch) then
               Result := Count;
               return;
            end if;
         end if;
         Z.C := Z.C + Count;
         exit when not Repeat;
      end loop;
      Result := 0;
   end Out_Grouping;

   procedure Out_Grouping_Backward (Z      : in out Context_Type'Class;
                                    S      : in Grouping_Array;
                                    Min    : in Utf8_Type;
                                    Max    : in Utf8_Type;
                                    Repeat : in Boolean;
                                    Result : out Result_Index) is
      Ch    : Utf8_Type;
      Count : Natural;
   begin
      if Z.C <= Z.Lb then
         Result := -1;
         return;
      end if;

      loop
         Get_Utf8_Backward (Z, Ch, Count);
         if Count = 0 then
            Result := -1;
            return;
         end if;
         if Ch <= Max and Ch >= Min then
            Ch := Ch - Min;
            if S (Ch) then
               Result := Count;
               return;
            end if;
         end if;
         Z.C := Z.C - Count;
         exit when not Repeat;
      end loop;
      Result := 0;
   end Out_Grouping_Backward;

   procedure In_Grouping (Z      : in out Context_Type'Class;
                          S      : in Grouping_Array;
                          Min    : in Utf8_Type;
                          Max    : in Utf8_Type;
                          Repeat : in Boolean;
                          Result : out Result_Index) is
      Ch    : Utf8_Type;
      Count : Natural;
   begin
      if Z.C >= Z.L then
         Result := -1;
         return;
      end if;

      loop
         Get_Utf8 (Z, Ch, Count);
         if Count = 0 then
            Result := -1;
            return;
         end if;
         if Ch > Max or Ch < Min then
            Result := Count;
            return;
         end if;
         Ch := Ch - Min;
         if not S (Ch) then
            Result := Count;
            return;
         end if;
         Z.C := Z.C + Count;
         exit when not Repeat;
      end loop;
      Result := 0;
   end In_Grouping;

   procedure In_Grouping_Backward (Z      : in out Context_Type'Class;
                                   S      : in Grouping_Array;
                                   Min    : in Utf8_Type;
                                   Max    : in Utf8_Type;
                                   Repeat : in Boolean;
                                   Result : out Result_Index) is
      Ch    : Utf8_Type;
      Count : Natural;
   begin
      if Z.C <= Z.Lb then
         Result := -1;
         return;
      end if;

      loop
         Get_Utf8_Backward (Z, Ch, Count);
         if Count = 0 then
            Result := -1;
            return;
         end if;
         if Ch > Max or Ch < Min then
            Result := Count;
            return;
         end if;
         Ch := Ch - Min;
         if not S (Ch) then
            Result := Count;
            return;
         end if;
         Z.C := Z.C - Count;
         exit when not Repeat;
      end loop;
      Result := 0;
   end In_Grouping_Backward;

   procedure Replace (Z     : in out Context_Type'Class;
                      C_Bra : in Char_Index;
                      C_Ket : in Char_Index;
                      S     : in String;
                      Len   : in Char_Index) is
      Adjustment : Integer;
   begin
      Adjustment := Len - (C_Ket - C_Bra);
      if Adjustment /= 0 then
         Z.P (C_Ket + Adjustment + 1 .. Z.Len + Adjustment + 1)
           := Z.P (C_Ket + 1 .. Z.Len + 1);
         Z.Len := Z.Len + Adjustment;
         Z.L := Z.L + Adjustment;
         if Z.C >= C_Ket then
            Z.C := Z.C + Adjustment;
         elsif Z.C > C_Bra then
            Z.C := C_Bra;
         end if;
      end if;
      if Len > 0 then
         Z.P (C_Bra + 1 .. C_Bra + Len) := S (S'First .. Len);
      end if;
   end Replace;

   procedure Slice_Del (Z : in out Context_Type'Class) is
   begin
      Replace (Z, Z.Bra, Z.Ket, "", 0);
      Z.Ket := Z.Bra;
   end Slice_Del;

   procedure Slice_From (Z    : in out Context_Type'Class;
                         Text : in String;
                         Len  : in Char_Index) is
   begin
      Replace (Z, Z.Bra, Z.Ket, Text, Len);
      Z.Ket := Z.Bra + Len;
   end Slice_From;

   procedure Insert (Z   : in out Context_Type'Class;
                     S   : in String;
                     Len : in Char_Index) is
      C : Char_Index;
   begin
      C := Z.C;
      Replace (Z, Z.C, Z.C, S, Len);
      if C <= Z.Bra then
         Z.Bra := Z.Bra + Len;
      end if;
      if C <= Z.Ket then
         Z.Ket := Z.Ket + Len;
      end if;
   end Insert;

end Stemmer;
