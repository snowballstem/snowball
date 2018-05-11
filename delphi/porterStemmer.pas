{ This file was generated automatically by the Snowball to Delphi compiler. }

Unit porterStemmer;

{$HINTS OFF}

Interface

Uses SnowballProgram;

Type
    TporterStemmer = Class(TSnowballProgram)
    private
        B_Y_found : Boolean;
        I_p2 : Integer;
        I_p1 : Integer;
    private
        a_0 : Array Of TAmong;
        a_1 : Array Of TAmong;
        a_2 : Array Of TAmong;
        a_3 : Array Of TAmong;
        a_4 : Array Of TAmong;
        a_5 : Array Of TAmong;
    public
        Constructor Create;
        Function stem : Boolean;
    private
        Function r_shortv : Boolean;
        Function r_R1 : Boolean;
        Function r_R2 : Boolean;
        Function r_Step_1a : Boolean;
        Function r_Step_1b : Boolean;
        Function r_Step_1c : Boolean;
        Function r_Step_2 : Boolean;
        Function r_Step_3 : Boolean;
        Function r_Step_4 : Boolean;
        Function r_Step_5a : Boolean;
        Function r_Step_5b : Boolean;
End;

Implementation

Const
    g_v : Array [0..3] Of Char = (
        Chr(17),
        Chr(65),
        Chr(16),
        Chr(1)
    );

Const
    g_v_WXY : Array [0..4] Of Char = (
        Chr(1),
        Chr(17),
        Chr(65),
        Chr(208),
        Chr(1)
    );

Constructor TporterStemmer.Create;
Begin
    SetLength(a_0, 4);
        a_0[0].Str  := 's';
        a_0[0].Size := Length(a_0[0].Str);
        a_0[0].Index := -1;
        a_0[0].Result := 3;
        a_0[0].Method := nil;

        a_0[1].Str  := 'ies';
        a_0[1].Size := Length(a_0[1].Str);
        a_0[1].Index := 0;
        a_0[1].Result := 2;
        a_0[1].Method := nil;

        a_0[2].Str  := 'sses';
        a_0[2].Size := Length(a_0[2].Str);
        a_0[2].Index := 0;
        a_0[2].Result := 1;
        a_0[2].Method := nil;

        a_0[3].Str  := 'ss';
        a_0[3].Size := Length(a_0[3].Str);
        a_0[3].Index := 0;
        a_0[3].Result := -1;
        a_0[3].Method := nil;

    SetLength(a_1, 13);
        a_1[0].Str  := '';
        a_1[0].Size := Length(a_1[0].Str);
        a_1[0].Index := -1;
        a_1[0].Result := 3;
        a_1[0].Method := nil;

        a_1[1].Str  := 'bb';
        a_1[1].Size := Length(a_1[1].Str);
        a_1[1].Index := 0;
        a_1[1].Result := 2;
        a_1[1].Method := nil;

        a_1[2].Str  := 'dd';
        a_1[2].Size := Length(a_1[2].Str);
        a_1[2].Index := 0;
        a_1[2].Result := 2;
        a_1[2].Method := nil;

        a_1[3].Str  := 'ff';
        a_1[3].Size := Length(a_1[3].Str);
        a_1[3].Index := 0;
        a_1[3].Result := 2;
        a_1[3].Method := nil;

        a_1[4].Str  := 'gg';
        a_1[4].Size := Length(a_1[4].Str);
        a_1[4].Index := 0;
        a_1[4].Result := 2;
        a_1[4].Method := nil;

        a_1[5].Str  := 'bl';
        a_1[5].Size := Length(a_1[5].Str);
        a_1[5].Index := 0;
        a_1[5].Result := 1;
        a_1[5].Method := nil;

        a_1[6].Str  := 'mm';
        a_1[6].Size := Length(a_1[6].Str);
        a_1[6].Index := 0;
        a_1[6].Result := 2;
        a_1[6].Method := nil;

        a_1[7].Str  := 'nn';
        a_1[7].Size := Length(a_1[7].Str);
        a_1[7].Index := 0;
        a_1[7].Result := 2;
        a_1[7].Method := nil;

        a_1[8].Str  := 'pp';
        a_1[8].Size := Length(a_1[8].Str);
        a_1[8].Index := 0;
        a_1[8].Result := 2;
        a_1[8].Method := nil;

        a_1[9].Str  := 'rr';
        a_1[9].Size := Length(a_1[9].Str);
        a_1[9].Index := 0;
        a_1[9].Result := 2;
        a_1[9].Method := nil;

        a_1[10].Str  := 'at';
        a_1[10].Size := Length(a_1[10].Str);
        a_1[10].Index := 0;
        a_1[10].Result := 1;
        a_1[10].Method := nil;

        a_1[11].Str  := 'tt';
        a_1[11].Size := Length(a_1[11].Str);
        a_1[11].Index := 0;
        a_1[11].Result := 2;
        a_1[11].Method := nil;

        a_1[12].Str  := 'iz';
        a_1[12].Size := Length(a_1[12].Str);
        a_1[12].Index := 0;
        a_1[12].Result := 1;
        a_1[12].Method := nil;

    SetLength(a_2, 3);
        a_2[0].Str  := 'ed';
        a_2[0].Size := Length(a_2[0].Str);
        a_2[0].Index := -1;
        a_2[0].Result := 2;
        a_2[0].Method := nil;

        a_2[1].Str  := 'eed';
        a_2[1].Size := Length(a_2[1].Str);
        a_2[1].Index := 0;
        a_2[1].Result := 1;
        a_2[1].Method := nil;

        a_2[2].Str  := 'ing';
        a_2[2].Size := Length(a_2[2].Str);
        a_2[2].Index := -1;
        a_2[2].Result := 2;
        a_2[2].Method := nil;

    SetLength(a_3, 20);
        a_3[0].Str  := 'anci';
        a_3[0].Size := Length(a_3[0].Str);
        a_3[0].Index := -1;
        a_3[0].Result := 3;
        a_3[0].Method := nil;

        a_3[1].Str  := 'enci';
        a_3[1].Size := Length(a_3[1].Str);
        a_3[1].Index := -1;
        a_3[1].Result := 2;
        a_3[1].Method := nil;

        a_3[2].Str  := 'abli';
        a_3[2].Size := Length(a_3[2].Str);
        a_3[2].Index := -1;
        a_3[2].Result := 4;
        a_3[2].Method := nil;

        a_3[3].Str  := 'eli';
        a_3[3].Size := Length(a_3[3].Str);
        a_3[3].Index := -1;
        a_3[3].Result := 6;
        a_3[3].Method := nil;

        a_3[4].Str  := 'alli';
        a_3[4].Size := Length(a_3[4].Str);
        a_3[4].Index := -1;
        a_3[4].Result := 9;
        a_3[4].Method := nil;

        a_3[5].Str  := 'ousli';
        a_3[5].Size := Length(a_3[5].Str);
        a_3[5].Index := -1;
        a_3[5].Result := 12;
        a_3[5].Method := nil;

        a_3[6].Str  := 'entli';
        a_3[6].Size := Length(a_3[6].Str);
        a_3[6].Index := -1;
        a_3[6].Result := 5;
        a_3[6].Method := nil;

        a_3[7].Str  := 'aliti';
        a_3[7].Size := Length(a_3[7].Str);
        a_3[7].Index := -1;
        a_3[7].Result := 10;
        a_3[7].Method := nil;

        a_3[8].Str  := 'biliti';
        a_3[8].Size := Length(a_3[8].Str);
        a_3[8].Index := -1;
        a_3[8].Result := 14;
        a_3[8].Method := nil;

        a_3[9].Str  := 'iviti';
        a_3[9].Size := Length(a_3[9].Str);
        a_3[9].Index := -1;
        a_3[9].Result := 13;
        a_3[9].Method := nil;

        a_3[10].Str  := 'tional';
        a_3[10].Size := Length(a_3[10].Str);
        a_3[10].Index := -1;
        a_3[10].Result := 1;
        a_3[10].Method := nil;

        a_3[11].Str  := 'ational';
        a_3[11].Size := Length(a_3[11].Str);
        a_3[11].Index := 10;
        a_3[11].Result := 8;
        a_3[11].Method := nil;

        a_3[12].Str  := 'alism';
        a_3[12].Size := Length(a_3[12].Str);
        a_3[12].Index := -1;
        a_3[12].Result := 10;
        a_3[12].Method := nil;

        a_3[13].Str  := 'ation';
        a_3[13].Size := Length(a_3[13].Str);
        a_3[13].Index := -1;
        a_3[13].Result := 8;
        a_3[13].Method := nil;

        a_3[14].Str  := 'ization';
        a_3[14].Size := Length(a_3[14].Str);
        a_3[14].Index := 13;
        a_3[14].Result := 7;
        a_3[14].Method := nil;

        a_3[15].Str  := 'izer';
        a_3[15].Size := Length(a_3[15].Str);
        a_3[15].Index := -1;
        a_3[15].Result := 7;
        a_3[15].Method := nil;

        a_3[16].Str  := 'ator';
        a_3[16].Size := Length(a_3[16].Str);
        a_3[16].Index := -1;
        a_3[16].Result := 8;
        a_3[16].Method := nil;

        a_3[17].Str  := 'iveness';
        a_3[17].Size := Length(a_3[17].Str);
        a_3[17].Index := -1;
        a_3[17].Result := 13;
        a_3[17].Method := nil;

        a_3[18].Str  := 'fulness';
        a_3[18].Size := Length(a_3[18].Str);
        a_3[18].Index := -1;
        a_3[18].Result := 11;
        a_3[18].Method := nil;

        a_3[19].Str  := 'ousness';
        a_3[19].Size := Length(a_3[19].Str);
        a_3[19].Index := -1;
        a_3[19].Result := 12;
        a_3[19].Method := nil;

    SetLength(a_4, 7);
        a_4[0].Str  := 'icate';
        a_4[0].Size := Length(a_4[0].Str);
        a_4[0].Index := -1;
        a_4[0].Result := 2;
        a_4[0].Method := nil;

        a_4[1].Str  := 'ative';
        a_4[1].Size := Length(a_4[1].Str);
        a_4[1].Index := -1;
        a_4[1].Result := 3;
        a_4[1].Method := nil;

        a_4[2].Str  := 'alize';
        a_4[2].Size := Length(a_4[2].Str);
        a_4[2].Index := -1;
        a_4[2].Result := 1;
        a_4[2].Method := nil;

        a_4[3].Str  := 'iciti';
        a_4[3].Size := Length(a_4[3].Str);
        a_4[3].Index := -1;
        a_4[3].Result := 2;
        a_4[3].Method := nil;

        a_4[4].Str  := 'ical';
        a_4[4].Size := Length(a_4[4].Str);
        a_4[4].Index := -1;
        a_4[4].Result := 2;
        a_4[4].Method := nil;

        a_4[5].Str  := 'ful';
        a_4[5].Size := Length(a_4[5].Str);
        a_4[5].Index := -1;
        a_4[5].Result := 3;
        a_4[5].Method := nil;

        a_4[6].Str  := 'ness';
        a_4[6].Size := Length(a_4[6].Str);
        a_4[6].Index := -1;
        a_4[6].Result := 3;
        a_4[6].Method := nil;

    SetLength(a_5, 19);
        a_5[0].Str  := 'ic';
        a_5[0].Size := Length(a_5[0].Str);
        a_5[0].Index := -1;
        a_5[0].Result := 1;
        a_5[0].Method := nil;

        a_5[1].Str  := 'ance';
        a_5[1].Size := Length(a_5[1].Str);
        a_5[1].Index := -1;
        a_5[1].Result := 1;
        a_5[1].Method := nil;

        a_5[2].Str  := 'ence';
        a_5[2].Size := Length(a_5[2].Str);
        a_5[2].Index := -1;
        a_5[2].Result := 1;
        a_5[2].Method := nil;

        a_5[3].Str  := 'able';
        a_5[3].Size := Length(a_5[3].Str);
        a_5[3].Index := -1;
        a_5[3].Result := 1;
        a_5[3].Method := nil;

        a_5[4].Str  := 'ible';
        a_5[4].Size := Length(a_5[4].Str);
        a_5[4].Index := -1;
        a_5[4].Result := 1;
        a_5[4].Method := nil;

        a_5[5].Str  := 'ate';
        a_5[5].Size := Length(a_5[5].Str);
        a_5[5].Index := -1;
        a_5[5].Result := 1;
        a_5[5].Method := nil;

        a_5[6].Str  := 'ive';
        a_5[6].Size := Length(a_5[6].Str);
        a_5[6].Index := -1;
        a_5[6].Result := 1;
        a_5[6].Method := nil;

        a_5[7].Str  := 'ize';
        a_5[7].Size := Length(a_5[7].Str);
        a_5[7].Index := -1;
        a_5[7].Result := 1;
        a_5[7].Method := nil;

        a_5[8].Str  := 'iti';
        a_5[8].Size := Length(a_5[8].Str);
        a_5[8].Index := -1;
        a_5[8].Result := 1;
        a_5[8].Method := nil;

        a_5[9].Str  := 'al';
        a_5[9].Size := Length(a_5[9].Str);
        a_5[9].Index := -1;
        a_5[9].Result := 1;
        a_5[9].Method := nil;

        a_5[10].Str  := 'ism';
        a_5[10].Size := Length(a_5[10].Str);
        a_5[10].Index := -1;
        a_5[10].Result := 1;
        a_5[10].Method := nil;

        a_5[11].Str  := 'ion';
        a_5[11].Size := Length(a_5[11].Str);
        a_5[11].Index := -1;
        a_5[11].Result := 2;
        a_5[11].Method := nil;

        a_5[12].Str  := 'er';
        a_5[12].Size := Length(a_5[12].Str);
        a_5[12].Index := -1;
        a_5[12].Result := 1;
        a_5[12].Method := nil;

        a_5[13].Str  := 'ous';
        a_5[13].Size := Length(a_5[13].Str);
        a_5[13].Index := -1;
        a_5[13].Result := 1;
        a_5[13].Method := nil;

        a_5[14].Str  := 'ant';
        a_5[14].Size := Length(a_5[14].Str);
        a_5[14].Index := -1;
        a_5[14].Result := 1;
        a_5[14].Method := nil;

        a_5[15].Str  := 'ent';
        a_5[15].Size := Length(a_5[15].Str);
        a_5[15].Index := -1;
        a_5[15].Result := 1;
        a_5[15].Method := nil;

        a_5[16].Str  := 'ment';
        a_5[16].Size := Length(a_5[16].Str);
        a_5[16].Index := 15;
        a_5[16].Result := 1;
        a_5[16].Method := nil;

        a_5[17].Str  := 'ement';
        a_5[17].Size := Length(a_5[17].Str);
        a_5[17].Index := 16;
        a_5[17].Result := 1;
        a_5[17].Method := nil;

        a_5[18].Str  := 'ou';
        a_5[18].Size := Length(a_5[18].Str);
        a_5[18].Index := -1;
        a_5[18].Result := 1;
        a_5[18].Method := nil;

End;

Function TporterStemmer.r_shortv : Boolean;
Var
    C : Integer;
Begin
    { (, line 19 }
    If (Not (OutGroupingBk(g_v_WXY, 89, 121))) Then
    Begin
        Begin Result := False; Exit; End;
    End;
    If (Not (InGroupingBk(g_v, 97, 121))) Then
    Begin
        Begin Result := False; Exit; End;
    End;
    If (Not (OutGroupingBk(g_v, 97, 121))) Then
    Begin
        Begin Result := False; Exit; End;
    End;
    Result := True;
End;

Function TporterStemmer.r_R1 : Boolean;
Var
    C : Integer;
Begin
    If Not (I_p1 <= FCursor) Then
    Begin
        Begin Result := False; Exit; End;
    End;
    Result := True;
End;

Function TporterStemmer.r_R2 : Boolean;
Var
    C : Integer;
Begin
    If Not (I_p2 <= FCursor) Then
    Begin
        Begin Result := False; Exit; End;
    End;
    Result := True;
End;

Function TporterStemmer.r_Step_1a : Boolean;
Var
    C : Integer;
    AmongVar : Integer;
Begin
    { (, line 24 }
    { [, line 25 }
    FKet := FCursor;
    { substring, line 25 }
    AmongVar := FindAmongBk(a_0, 4);
    If (AmongVar = 0) Then
    Begin
        Begin Result := False; Exit; End;
    End;
    { ], line 25 }
    FBra := FCursor;
    Case AmongVar Of
        0:
        Begin
            Begin Result := False; Exit; End;
        End;
        1:
        Begin
            { (, line 26 }
            { <-, line 26 }
            SliceFrom('ss');
        End;
        2:
        Begin
            { (, line 27 }
            { <-, line 27 }
            SliceFrom('i');
        End;
        3:
        Begin
            { (, line 29 }
            { delete, line 29 }
            SliceDel;
        End;
    End;
    Result := True;
End;

Function TporterStemmer.r_Step_1b : Boolean;
Var
    C : Integer;
    AmongVar : Integer;
    v_1 : Integer;
    v_3 : Integer;
    v_4 : Integer;
Label
    lab0,
    lab1;
Begin
    { (, line 33 }
    { [, line 34 }
    FKet := FCursor;
    { substring, line 34 }
    AmongVar := FindAmongBk(a_2, 3);
    If (AmongVar = 0) Then
    Begin
        Begin Result := False; Exit; End;
    End;
    { ], line 34 }
    FBra := FCursor;
    Case AmongVar Of
        0:
        Begin
            Begin Result := False; Exit; End;
        End;
        1:
        Begin
            { (, line 35 }
            { call R1, line 35 }
            If (Not r_R1) Then
            Begin
                Begin Result := False; Exit; End;
            End;
            { <-, line 35 }
            SliceFrom('ee');
        End;
        2:
        Begin
            { (, line 37 }
            { test, line 38 }
            v_1 := FLimit - FCursor;
            { gopast, line 38 }
            While True Do
            Begin
                Repeat
                    If (Not (InGroupingBk(g_v, 97, 121))) Then
                    Begin
                        goto lab1;
                    End;
                    goto lab0;
                Until True;
lab1:
                If (FCursor <= FBkLimit) Then
                Begin
                    Begin Result := False; Exit; End;
                End;
                Dec(FCursor);
            End;
lab0:
            FCursor := FLimit - v_1;
            { delete, line 38 }
            SliceDel;
            { test, line 39 }
            v_3 := FLimit - FCursor;
            { substring, line 39 }
            AmongVar := FindAmongBk(a_1, 13);
            If (AmongVar = 0) Then
            Begin
                Begin Result := False; Exit; End;
            End;
            FCursor := FLimit - v_3;
            Case AmongVar Of
                0:
                Begin
                    Begin Result := False; Exit; End;
                End;
                1:
                Begin
                    { (, line 41 }
                    { <+, line 41 }
                    Begin
                        C := FCursor;
                        insert(FCursor, FCursor, 'e');
                        FCursor := C;
                    End;
                End;
                2:
                Begin
                    { (, line 44 }
                    { [, line 44 }
                    FKet := FCursor;
                    { next, line 44 }
                    If (FCursor <= FBkLimit) Then
                    Begin
                        Begin Result := False; Exit; End;
                    End;
                    Dec(FCursor);
                    { ], line 44 }
                    FBra := FCursor;
                    { delete, line 44 }
                    SliceDel;
                End;
                3:
                Begin
                    { (, line 45 }
                    { atmark, line 45 }
                    If (FCursor <> I_p1) Then
                    Begin
                        Begin Result := False; Exit; End;
                    End;
                    { test, line 45 }
                    v_4 := FLimit - FCursor;
                    { call shortv, line 45 }
                    If (Not r_shortv) Then
                    Begin
                        Begin Result := False; Exit; End;
                    End;
                    FCursor := FLimit - v_4;
                    { <+, line 45 }
                    Begin
                        C := FCursor;
                        insert(FCursor, FCursor, 'e');
                        FCursor := C;
                    End;
                End;
            End;
        End;
    End;
    Result := True;
End;

Function TporterStemmer.r_Step_1c : Boolean;
Var
    C : Integer;
    v_1 : Integer;
Label
    lab0,
    lab1,
    lab2,
    lab3;
Begin
    { (, line 51 }
    { [, line 52 }
    FKet := FCursor;
    { or, line 52 }
    Repeat
        v_1 := FLimit - FCursor;
        Repeat
            { literal, line 52 }
            If (Not (EqSBk(1, 'y'))) Then
            Begin
                goto lab1;
            End;
            goto lab0;
        Until True;
lab1:
        FCursor := FLimit - v_1;
        { literal, line 52 }
        If (Not (EqSBk(1, 'Y'))) Then
        Begin
            Begin Result := False; Exit; End;
        End;
    Until True;
lab0:
    { ], line 52 }
    FBra := FCursor;
    { gopast, line 53 }
    While True Do
    Begin
        Repeat
            If (Not (InGroupingBk(g_v, 97, 121))) Then
            Begin
                goto lab3;
            End;
            goto lab2;
        Until True;
lab3:
        If (FCursor <= FBkLimit) Then
        Begin
            Begin Result := False; Exit; End;
        End;
        Dec(FCursor);
    End;
lab2:
    { <-, line 54 }
    SliceFrom('i');
    Result := True;
End;

Function TporterStemmer.r_Step_2 : Boolean;
Var
    C : Integer;
    AmongVar : Integer;
Begin
    { (, line 57 }
    { [, line 58 }
    FKet := FCursor;
    { substring, line 58 }
    AmongVar := FindAmongBk(a_3, 20);
    If (AmongVar = 0) Then
    Begin
        Begin Result := False; Exit; End;
    End;
    { ], line 58 }
    FBra := FCursor;
    { call R1, line 58 }
    If (Not r_R1) Then
    Begin
        Begin Result := False; Exit; End;
    End;
    Case AmongVar Of
        0:
        Begin
            Begin Result := False; Exit; End;
        End;
        1:
        Begin
            { (, line 59 }
            { <-, line 59 }
            SliceFrom('tion');
        End;
        2:
        Begin
            { (, line 60 }
            { <-, line 60 }
            SliceFrom('ence');
        End;
        3:
        Begin
            { (, line 61 }
            { <-, line 61 }
            SliceFrom('ance');
        End;
        4:
        Begin
            { (, line 62 }
            { <-, line 62 }
            SliceFrom('able');
        End;
        5:
        Begin
            { (, line 63 }
            { <-, line 63 }
            SliceFrom('ent');
        End;
        6:
        Begin
            { (, line 64 }
            { <-, line 64 }
            SliceFrom('e');
        End;
        7:
        Begin
            { (, line 66 }
            { <-, line 66 }
            SliceFrom('ize');
        End;
        8:
        Begin
            { (, line 68 }
            { <-, line 68 }
            SliceFrom('ate');
        End;
        9:
        Begin
            { (, line 69 }
            { <-, line 69 }
            SliceFrom('al');
        End;
        10:
        Begin
            { (, line 71 }
            { <-, line 71 }
            SliceFrom('al');
        End;
        11:
        Begin
            { (, line 72 }
            { <-, line 72 }
            SliceFrom('ful');
        End;
        12:
        Begin
            { (, line 74 }
            { <-, line 74 }
            SliceFrom('ous');
        End;
        13:
        Begin
            { (, line 76 }
            { <-, line 76 }
            SliceFrom('ive');
        End;
        14:
        Begin
            { (, line 77 }
            { <-, line 77 }
            SliceFrom('ble');
        End;
    End;
    Result := True;
End;

Function TporterStemmer.r_Step_3 : Boolean;
Var
    C : Integer;
    AmongVar : Integer;
Begin
    { (, line 81 }
    { [, line 82 }
    FKet := FCursor;
    { substring, line 82 }
    AmongVar := FindAmongBk(a_4, 7);
    If (AmongVar = 0) Then
    Begin
        Begin Result := False; Exit; End;
    End;
    { ], line 82 }
    FBra := FCursor;
    { call R1, line 82 }
    If (Not r_R1) Then
    Begin
        Begin Result := False; Exit; End;
    End;
    Case AmongVar Of
        0:
        Begin
            Begin Result := False; Exit; End;
        End;
        1:
        Begin
            { (, line 83 }
            { <-, line 83 }
            SliceFrom('al');
        End;
        2:
        Begin
            { (, line 85 }
            { <-, line 85 }
            SliceFrom('ic');
        End;
        3:
        Begin
            { (, line 87 }
            { delete, line 87 }
            SliceDel;
        End;
    End;
    Result := True;
End;

Function TporterStemmer.r_Step_4 : Boolean;
Var
    C : Integer;
    AmongVar : Integer;
    v_1 : Integer;
Label
    lab0,
    lab1;
Begin
    { (, line 91 }
    { [, line 92 }
    FKet := FCursor;
    { substring, line 92 }
    AmongVar := FindAmongBk(a_5, 19);
    If (AmongVar = 0) Then
    Begin
        Begin Result := False; Exit; End;
    End;
    { ], line 92 }
    FBra := FCursor;
    { call R2, line 92 }
    If (Not r_R2) Then
    Begin
        Begin Result := False; Exit; End;
    End;
    Case AmongVar Of
        0:
        Begin
            Begin Result := False; Exit; End;
        End;
        1:
        Begin
            { (, line 95 }
            { delete, line 95 }
            SliceDel;
        End;
        2:
        Begin
            { (, line 96 }
            { or, line 96 }
            Repeat
                v_1 := FLimit - FCursor;
                Repeat
                    { literal, line 96 }
                    If (Not (EqSBk(1, 's'))) Then
                    Begin
                        goto lab1;
                    End;
                    goto lab0;
                Until True;
lab1:
                FCursor := FLimit - v_1;
                { literal, line 96 }
                If (Not (EqSBk(1, 't'))) Then
                Begin
                    Begin Result := False; Exit; End;
                End;
            Until True;
lab0:
            { delete, line 96 }
            SliceDel;
        End;
    End;
    Result := True;
End;

Function TporterStemmer.r_Step_5a : Boolean;
Var
    C : Integer;
    v_1 : Integer;
    v_2 : Integer;
Label
    lab0,
    lab1,
    lab2;
Begin
    { (, line 100 }
    { [, line 101 }
    FKet := FCursor;
    { literal, line 101 }
    If (Not (EqSBk(1, 'e'))) Then
    Begin
        Begin Result := False; Exit; End;
    End;
    { ], line 101 }
    FBra := FCursor;
    { or, line 102 }
    Repeat
        v_1 := FLimit - FCursor;
        Repeat
            { call R2, line 102 }
            If (Not r_R2) Then
            Begin
                goto lab1;
            End;
            goto lab0;
        Until True;
lab1:
        FCursor := FLimit - v_1;
        { (, line 102 }
        { call R1, line 102 }
        If (Not r_R1) Then
        Begin
            Begin Result := False; Exit; End;
        End;
        { not, line 102 }
        Begin
            v_2 := FLimit - FCursor;
            Repeat
                { call shortv, line 102 }
                If (Not r_shortv) Then
                Begin
                    goto lab2;
                End;
                Begin Result := False; Exit; End;
            Until True;
lab2:
            FCursor := FLimit - v_2;
        End;
    Until True;
lab0:
    { delete, line 103 }
    SliceDel;
    Result := True;
End;

Function TporterStemmer.r_Step_5b : Boolean;
Var
    C : Integer;
Begin
    { (, line 106 }
    { [, line 107 }
    FKet := FCursor;
    { literal, line 107 }
    If (Not (EqSBk(1, 'l'))) Then
    Begin
        Begin Result := False; Exit; End;
    End;
    { ], line 107 }
    FBra := FCursor;
    { call R2, line 108 }
    If (Not r_R2) Then
    Begin
        Begin Result := False; Exit; End;
    End;
    { literal, line 108 }
    If (Not (EqSBk(1, 'l'))) Then
    Begin
        Begin Result := False; Exit; End;
    End;
    { delete, line 109 }
    SliceDel;
    Result := True;
End;

Function TporterStemmer.stem : Boolean;
Var
    C : Integer;
    v_1 : Integer;
    v_2 : Integer;
    v_3 : Integer;
    v_4 : Integer;
    v_5 : Integer;
    v_10 : Integer;
    v_11 : Integer;
    v_12 : Integer;
    v_13 : Integer;
    v_14 : Integer;
    v_15 : Integer;
    v_16 : Integer;
    v_17 : Integer;
    v_18 : Integer;
    v_19 : Integer;
    v_20 : Integer;
Label
    lab0,
    lab1,
    lab2,
    lab3,
    lab4,
    lab5,
    lab6,
    lab7,
    lab8,
    lab9,
    lab10,
    lab11,
    lab12,
    lab13,
    lab14,
    lab15,
    lab16,
    lab17,
    lab18,
    lab19,
    lab20,
    lab21,
    lab22,
    lab23,
    lab24,
    lab25,
    lab26,
    lab27;
Begin
    { (, line 113 }
    { unset Y_found, line 115 }
    B_Y_found := False;
    { do, line 116 }
    v_1 := FCursor;
    Repeat
        { (, line 116 }
        { [, line 116 }
        FBra := FCursor;
        { literal, line 116 }
        If (Not (EqS(1, 'y'))) Then
        Begin
            goto lab0;
        End;
        { ], line 116 }
        FKet := FCursor;
        { <-, line 116 }
        SliceFrom('Y');
        { set Y_found, line 116 }
        B_Y_found := True;
    Until True;
lab0:
    FCursor := v_1;
    { do, line 117 }
    v_2 := FCursor;
    Repeat
        { repeat, line 117 }
lab2:
        While True Do
        Begin
            v_3 := FCursor;
            Repeat
                { (, line 117 }
                { goto, line 117 }
                While True Do
                Begin
                    v_4 := FCursor;
                    Repeat
                        { (, line 117 }
                        If (Not (InGrouping(g_v, 97, 121))) Then
                        Begin
                            goto lab5;
                        End;
                        { [, line 117 }
                        FBra := FCursor;
                        { literal, line 117 }
                        If (Not (EqS(1, 'y'))) Then
                        Begin
                            goto lab5;
                        End;
                        { ], line 117 }
                        FKet := FCursor;
                        FCursor := v_4;
                        goto lab4;
                    Until True;
lab5:
                    FCursor := v_4;
                    If (FCursor >= FLimit) Then
                    Begin
                        goto lab3;
                    End;
                    Inc(FCursor);
                End;
lab4:
                { <-, line 117 }
                SliceFrom('Y');
                { set Y_found, line 117 }
                B_Y_found := True;
                goto lab2;
            Until True;
lab3:
            FCursor := v_3;
            Break;
        End;
    Until True;
lab1:
    FCursor := v_2;
    I_p1 := FLimit;
    I_p2 := FLimit;
    { do, line 121 }
    v_5 := FCursor;
    Repeat
        { (, line 121 }
        { gopast, line 122 }
        While True Do
        Begin
            Repeat
                If (Not (InGrouping(g_v, 97, 121))) Then
                Begin
                    goto lab8;
                End;
                goto lab7;
            Until True;
lab8:
            If (FCursor >= FLimit) Then
            Begin
                goto lab6;
            End;
            Inc(FCursor);
        End;
lab7:
        { gopast, line 122 }
        While True Do
        Begin
            Repeat
                If (Not (OutGrouping(g_v, 97, 121))) Then
                Begin
                    goto lab10;
                End;
                goto lab9;
            Until True;
lab10:
            If (FCursor >= FLimit) Then
            Begin
                goto lab6;
            End;
            Inc(FCursor);
        End;
lab9:
        { setmark p1, line 122 }
        I_p1 := FCursor;
        { gopast, line 123 }
        While True Do
        Begin
            Repeat
                If (Not (InGrouping(g_v, 97, 121))) Then
                Begin
                    goto lab12;
                End;
                goto lab11;
            Until True;
lab12:
            If (FCursor >= FLimit) Then
            Begin
                goto lab6;
            End;
            Inc(FCursor);
        End;
lab11:
        { gopast, line 123 }
        While True Do
        Begin
            Repeat
                If (Not (OutGrouping(g_v, 97, 121))) Then
                Begin
                    goto lab14;
                End;
                goto lab13;
            Until True;
lab14:
            If (FCursor >= FLimit) Then
            Begin
                goto lab6;
            End;
            Inc(FCursor);
        End;
lab13:
        { setmark p2, line 123 }
        I_p2 := FCursor;
    Until True;
lab6:
    FCursor := v_5;
    { backwards, line 126 }
    FBkLimit := FCursor; FCursor := FLimit;
    { (, line 126 }
    { do, line 127 }
    v_10 := FLimit - FCursor;
    Repeat
        { call Step_1a, line 127 }
        If (Not r_Step_1a) Then
        Begin
            goto lab15;
        End;
    Until True;
lab15:
    FCursor := FLimit - v_10;
    { do, line 128 }
    v_11 := FLimit - FCursor;
    Repeat
        { call Step_1b, line 128 }
        If (Not r_Step_1b) Then
        Begin
            goto lab16;
        End;
    Until True;
lab16:
    FCursor := FLimit - v_11;
    { do, line 129 }
    v_12 := FLimit - FCursor;
    Repeat
        { call Step_1c, line 129 }
        If (Not r_Step_1c) Then
        Begin
            goto lab17;
        End;
    Until True;
lab17:
    FCursor := FLimit - v_12;
    { do, line 130 }
    v_13 := FLimit - FCursor;
    Repeat
        { call Step_2, line 130 }
        If (Not r_Step_2) Then
        Begin
            goto lab18;
        End;
    Until True;
lab18:
    FCursor := FLimit - v_13;
    { do, line 131 }
    v_14 := FLimit - FCursor;
    Repeat
        { call Step_3, line 131 }
        If (Not r_Step_3) Then
        Begin
            goto lab19;
        End;
    Until True;
lab19:
    FCursor := FLimit - v_14;
    { do, line 132 }
    v_15 := FLimit - FCursor;
    Repeat
        { call Step_4, line 132 }
        If (Not r_Step_4) Then
        Begin
            goto lab20;
        End;
    Until True;
lab20:
    FCursor := FLimit - v_15;
    { do, line 133 }
    v_16 := FLimit - FCursor;
    Repeat
        { call Step_5a, line 133 }
        If (Not r_Step_5a) Then
        Begin
            goto lab21;
        End;
    Until True;
lab21:
    FCursor := FLimit - v_16;
    { do, line 134 }
    v_17 := FLimit - FCursor;
    Repeat
        { call Step_5b, line 134 }
        If (Not r_Step_5b) Then
        Begin
            goto lab22;
        End;
    Until True;
lab22:
    FCursor := FLimit - v_17;
    FCursor := FBkLimit;    { do, line 137 }
    v_18 := FCursor;
    Repeat
        { (, line 137 }
        { Boolean test Y_found, line 137 }
        If (Not (B_Y_found)) Then
        Begin
            goto lab23;
        End;
        { repeat, line 137 }
lab24:
        While True Do
        Begin
            v_19 := FCursor;
            Repeat
                { (, line 137 }
                { goto, line 137 }
                While True Do
                Begin
                    v_20 := FCursor;
                    Repeat
                        { (, line 137 }
                        { [, line 137 }
                        FBra := FCursor;
                        { literal, line 137 }
                        If (Not (EqS(1, 'Y'))) Then
                        Begin
                            goto lab27;
                        End;
                        { ], line 137 }
                        FKet := FCursor;
                        FCursor := v_20;
                        goto lab26;
                    Until True;
lab27:
                    FCursor := v_20;
                    If (FCursor >= FLimit) Then
                    Begin
                        goto lab25;
                    End;
                    Inc(FCursor);
                End;
lab26:
                { <-, line 137 }
                SliceFrom('y');
                goto lab24;
            Until True;
lab25:
            FCursor := v_19;
            Break;
        End;
    Until True;
lab23:
    FCursor := v_18;
    Result := True;
End;

End.
