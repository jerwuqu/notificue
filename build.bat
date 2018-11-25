@rem Verify existence of build tools 
@if defined INCLUDE goto :BUILD
@echo You must open a "Visual Studio .NET Command Prompt" to run this script
@exit 1

:BUILD
@rem Vars
@setlocal
@set BT_CL=cl /nologo /c /O2
@set BT_LINK=link /NOLOGO
@set LIB_TOML=ext\tomlc99\toml.c

@rem Output dirs
@if not exist bin mkdir bin
@if not exist obj\main mkdir obj\main
@if not exist obj\dll mkdir obj\dll

@rem Build exts
%BT_CL% /Foobj\main\ %LIB_TOML%
@if errorlevel 1 goto :BUILDERR

@rem Build notificue
%BT_CL% /Wall /Foobj\main\ main\*.c
@if errorlevel 1 goto :BUILDERR
%BT_LINK% /OUT:bin\notificue.exe user32.lib gdi32.lib obj\main\*.obj
@if errorlevel 1 goto :BUILDERR

@rem Build notificue-dll
%BT_CL% /Wall /Foobj\dll\ dll\*.c
@if errorlevel 1 goto :BUILDERR
%BT_LINK% /DLL /IMPLIB:%TEMP%\dll.lib /OUT:bin\notificue.dll user32.lib obj\dll\*.obj
@if errorlevel 1 goto :BUILDERR

@rem Finished
@exit 0

@rem Error handling
:BUILDERR
@echo ------------------
@echo -- Build failed --
@echo ------------------
@exit 1
