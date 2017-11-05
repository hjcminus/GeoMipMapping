@echo off
rmdir /s /q intermediate


cd build
rmdir /s /q vs2013\ipch
rmdir /s /q vs2015\ipch
rmdir /s /q vs2017\ipch
for /r /d %%i in (*) do del "%%i\*.sdf"
for /r /d %%i in (*) do del /A:h "%%i\*.suo"
for /r /d %%i in (*) do del "%%i\*.VC.db"
for /r /d %%i in (*) do del "%%i\*.user"
for /r /d %%i in (*) do del "%%i\*.aps"
cd ..

cd bin
for /r /d %%i in (*) do del "%%i\*.ilk"
for /r /d %%i in (*) do del "%%i\*.pdb"
for /r /d %%i in (*) do del "%%i\*.exp"
for /r /d %%i in (*) do del "%%i\*.iobj"
for /r /d %%i in (*) do del "%%i\*.ipdb"
for /r /d %%i in (*) do del "%%i\*.log"
cd ..

pause
