@echo off
set GCC=C:\PROGRA~1\JETBRA~1\CLION2~1.3\bin\mingw\bin\g++.exe
echo int x = 1; > C:\ECS_Cafe\cafe-ecs\tiny.cpp
%GCC% -std=c++20 -fsyntax-only C:\ECS_Cafe\cafe-ecs\tiny.cpp > C:\ECS_Cafe\cafe-ecs\tiny_out.txt 2>&1
echo ExitCode=%ERRORLEVEL% >> C:\ECS_Cafe\cafe-ecs\tiny_out.txt
