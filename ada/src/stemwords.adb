with Ada.Text_IO;
with Ada.Command_Line;
with Stemmer.Factory;
procedure Stemwords is

   use Stemmer.Factory;

   function Get_Language (Name : in String;
                          Error: out Boolean) return Language_Type;
   function Is_Space (C : in Character) return Boolean;
   procedure Show_Usage;

   function Is_Space (C : in Character) return Boolean is
   begin
      return C = ' ' or C = ASCII.HT;
   end Is_Space;

   function Get_Language (Name : in String;
                          Error: out Boolean) return Language_Type is
   begin
      Error := False;
      return Language_Type'Value ("L_" & Name);

   exception
      when Constraint_Error =>
         Ada.Text_IO.Put_Line ("Unsupported language: " & Name);
         Error := True;
         return Language_Type'Val (0);

   end Get_Language;

   procedure Show_Usage is
   begin
      Ada.Text_IO.Put_Line ("Usage: stemwords <language> <input file> <output file>");
      Ada.Command_Line.Set_Exit_Status(Ada.Command_Line.Failure);
   end Show_Usage;

   Count  : constant Natural := Ada.Command_Line.Argument_Count;
begin
   if Count /= 3 then
      Show_Usage;
      return;
   end if;
   declare
      Bad_Usage : Boolean := False;
      Lang      : constant Language_Type := Get_Language (Ada.Command_Line.Argument (1), Bad_Usage);
      Input     : constant String := Ada.Command_Line.Argument (2);
      Output    : constant String := Ada.Command_Line.Argument (3);
      Src_File  : Ada.Text_IO.File_Type;
      Dst_File  : Ada.Text_IO.File_Type;
   begin
      if Bad_Usage then
         Show_Usage;
         return;
      end if;
      Ada.Text_IO.Open (Src_File, Ada.Text_IO.In_File, Input);
      Ada.Text_IO.Create (Dst_File, Ada.Text_IO.Out_File, Output);
      while not Ada.Text_IO.End_Of_File (Src_File) loop
         declare
            Line : constant String := Ada.Text_IO.Get_Line (Src_File);
            Pos  : Positive := Line'First;
            Last_Pos  : Positive;
            Start_Pos : Positive;
         begin
            while Pos <= Line'Last loop
               Last_Pos := Pos;
               while Pos <= Line'Last and then Is_Space (Line (Pos)) loop
                  Pos := Pos + 1;
               end loop;
               if Last_Pos < Pos then
                  Ada.Text_IO.Put (Dst_File, Line (Last_Pos .. Pos - 1));
               end if;
               exit when Pos > Line'Last;
               Start_Pos := Pos;
               while Pos <= Line'Last and then not Is_Space (Line (Pos)) loop
                  Pos := Pos + 1;
               end loop;
               Ada.Text_IO.Put (Dst_File, Stemmer.Factory.Stem (Lang, Line (Start_Pos .. Pos - 1)));
            end loop;
            Ada.Text_IO.New_Line (Dst_File);
         end;
      end loop;
      Ada.Text_IO.Close (Src_File);
      Ada.Text_IO.Close (Dst_File);
   end;
end Stemwords;
