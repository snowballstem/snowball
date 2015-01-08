@echo off
setlocal enableextensions,enabledelayedexpansion

set LOCAL=%~d0%~p0

set ALG_PATH=..\algorithms\
set SNOWBALL=%LOCAL%snowball.exe
set TARGET=%LOCAL%Snowball\Algorithms\


echo.
echo  Stemmer generation script
echo.
echo   - Source:    %ALG_PATH%
echo   - Target:    %TARGET%
echo   - Compiler:  %SNOWBALL%
echo.

pause

echo.

IF EXIST "%TARGET%" (
    echo  Clearning target directory
    cd %TARGET%
    del *.cs
    cd %LOCAL%
)

echo  Starting code generation
echo.

for /f "tokens=*" %%D in ('dir /b /s /a:d "%ALG_PATH%"') do (
    
    echo   - Processing %%~nD
    pushd %%D
    
    call :FirstUp result %%~nD
    set FILE_NAME=!result:_=!Stemmer
    
    for /r %%f in (*8859_1.sbl) do (
        SET FILE_PATH=%%f
        SET TARGET_PATH=%TARGET%\!FILE_NAME!.generated
                
        %SNOWBALL% !FILE_PATH! -cs -o !TARGET_PATH! -name !FILE_NAME! -u        
    )
    
    popd
)


goto :EOF


:FirstUp
setlocal EnableDelayedExpansion
set "temp=%~2"
set "helper=##AABBCCDDEEFFGGHHIIJJKKLLMMNNOOPPQQRRSSTTUUVVWWXXYYZZ"
set "first=!helper:*%temp:~0,1%=!"
set "first=!first:~0,1!"
if "!first!"=="#" set "first=!temp:~0,1!"
set "temp=!first!!temp:~1!"
(
    endlocal
    set "result=%temp%"
    goto :eof
)
