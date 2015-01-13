@echo off
setlocal enableextensions,enabledelayedexpansion

set LOCAL=%~d0%~p0

set ALGORITHMS=..\algorithms\
set SNOWBALL=%LOCAL%..\snowball
set TARGET=%LOCAL%Snowball\Algorithms\


echo.
echo  Stemmer generation script
echo.
echo   - Source:    %ALGORITHMS%
echo   - Target:    %TARGET%
echo   - Compiler:  %SNOWBALL%
echo.

rem pause

echo.

IF EXIST "%TARGET%" (
    echo  Clearning target directory
    cd %TARGET%
    del *.cs
    cd %LOCAL%
)

echo  Starting code generation
echo.


call :Compile Danish         danish\stem_ISO_8859_1.sbl             
call :Compile Dutch          dutch\stem_ISO_8859_1.sbl
call :Compile English        english\stem_ISO_8859_1.sbl
call :Compile Finnish        finnish\stem_ISO_8859_1.sbl
call :Compile French         french\stem_ISO_8859_1.sbl
call :Compile German         german\stem_ISO_8859_1.sbl
call :Compile German2        german2\stem_ISO_8859_1.sbl
call :Compile Hungarian      hungarian\stem_Unicode.sbl     
call :Compile Italian        italian\stem_ISO_8859_1.sbl
call :Compile KraaijPohlmann kraaij_pohlmann\stem_ISO_8859_1.sbl
call :Compile Lovins         lovins\stem_ISO_8859_1.sbl
call :Compile Norwegian      norwegian\stem_ISO_8859_1.sbl
call :Compile Porter         porter\stem_ISO_8859_1.sbl
call :Compile Portuguese     portuguese\stem_ISO_8859_1.sbl
call :Compile Romanian       romanian\stem_Unicode.sbl
call :Compile Russian        russian\stem_Unicode.sbl
call :Compile Spanish        spanish\stem_ISO_8859_1.sbl
call :Compile Swedish        swedish\stem_ISO_8859_1.sbl
call :Compile Turkish        turkish\stem_Unicode.sbl

echo.
rem pause

goto :EOF


:Compile
setlocal EnableDelayedExpansion

  set Language=%~1
  set RelativePath=%~2
  set CompletePath=%ALGORITHMS%!RelativePath!
  set ClassName=!Language!Stemmer
  set OutputPath=%TARGET%\!ClassName!.generated

  echo   - Processing !Language! (!RelativePath!)
  %SNOWBALL% !CompletePath! -cs -o !OutputPath! -p Stemmer -name !ClassName!

goto :EOF
