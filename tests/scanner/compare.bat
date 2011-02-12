@echo off
FOR %%f IN (*.in) DO (
FC %%f.out %%f.ans > NUL
    if ERRORLEVEL 1  (
        echo %%~nf : Bad
     )
)
pause
