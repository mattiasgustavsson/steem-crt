@echo off
del *.ncb
del *.ncb
attrib -h *.suo
del *.suo
rmdir /s /q Debug
rmdir /s /q Release