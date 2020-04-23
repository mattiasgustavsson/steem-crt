@echo off
cd ArchiveAccess
call clean
cd ..

cd ArchiveAccessTest
call clean
cd ..

cd UnpackAll
call clean
cd ..

cd ArchiveExploder
call clean
cd ..

cd ArchiveForge
call clean
cd ..

del *.lib /q
del *.dll /q 
