@echo off
set PATH=C:\PROGRA~1\JETBRA~1\CLION2~1.3\bin\mingw\bin;%PATH%
g++ --version > C:\ECS_Cafe\cafe-ecs\ver_out.txt 2>&1
echo ExitCode=%ERRORLEVEL% >> C:\ECS_Cafe\cafe-ecs\ver_out.txt
g++ -std=c++20 -fsyntax-only C:\ECS_Cafe\cafe-ecs\tiny.cpp >> C:\ECS_Cafe\cafe-ecs\ver_out.txt 2>&1
echo ExitCode2=%ERRORLEVEL% >> C:\ECS_Cafe\cafe-ecs\ver_out.txt
