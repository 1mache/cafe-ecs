@echo off
set PATH=C:\PROGRA~1\JETBRA~1\CLION2~1.3\bin\mingw\bin;%PATH%
set INC=-IC:\ECS_Cafe\cafe-ecs\src -IC:\ECS_Cafe\cafe-ecs\src\component -IC:\ECS_Cafe\cafe-ecs\src\system -IC:\ECS_Cafe\cafe-ecs\src\entity -isystem C:\ECS_Cafe\cafe-ecs\dependencies\bagel -isystem C:\ECS_Cafe\cafe-ecs\cmake-build-debug\dependencies\SDL\include-revision -isystem C:\ECS_Cafe\cafe-ecs\dependencies\SDL\include -isystem C:\ECS_Cafe\cafe-ecs\dependencies\SDL_image\include -isystem C:\ECS_Cafe\cafe-ecs\dependencies\box2d\src\..\include -isystem C:\ECS_Cafe\cafe-ecs\cmake-build-debug\dependencies\box2d\src -isystem C:\ECS_Cafe\cafe-ecs\cmake-build-debug\_deps\json-src\include
set FLAGS=-std=c++20 -Wall -Wextra -Wshadow -Wpedantic -Wformat=2 -Wcast-align -Wconversion -Wsign-conversion -Wnull-dereference -Wdouble-promotion -Wno-old-style-cast -Wno-unused -Wno-unused-parameter -Werror -fsyntax-only

echo === CheckPastrySystem.cpp ===
g++ %FLAGS% %INC% C:\ECS_Cafe\cafe-ecs\src\system\CheckPastrySystem.cpp 2>&1
echo ExitCode=%ERRORLEVEL%
echo === CustomerSystem.cpp ===
g++ %FLAGS% %INC% C:\ECS_Cafe\cafe-ecs\src\system\CustomerSystem.cpp 2>&1
echo ExitCode=%ERRORLEVEL%
echo === Entities.cpp ===
g++ %FLAGS% %INC% C:\ECS_Cafe\cafe-ecs\src\entity\Entities.cpp 2>&1
echo ExitCode=%ERRORLEVEL%
echo === MainGameScene.cpp ===
g++ %FLAGS% %INC% C:\ECS_Cafe\cafe-ecs\src\MainGameScene.cpp 2>&1
echo ExitCode=%ERRORLEVEL%
echo === OrderGrade.cpp ===
g++ %FLAGS% %INC% C:\ECS_Cafe\cafe-ecs\src\component\OrderGrade.cpp 2>&1
echo ExitCode=%ERRORLEVEL%
