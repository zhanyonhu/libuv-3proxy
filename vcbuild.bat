@echo off

cd %~dp0

if /i "%1"=="help" goto help
if /i "%1"=="--help" goto help
if /i "%1"=="-help" goto help
if /i "%1"=="/help" goto help
if /i "%1"=="?" goto help
if /i "%1"=="-?" goto help
if /i "%1"=="--?" goto help
if /i "%1"=="/?" goto help

@rem Process arguments.
set config=
set target=Build
set noprojgen=
set nobuild=
set run=
set target_arch=x64
set vs_toolset=x86
set platform=WIN32
set library=executable
set PATH=%PATH%;%~dp0tools\win32

:next-arg
if "%1"=="" goto args-done
if /i "%1"=="update"       goto arg-update
:arg-ok
shift
goto next-arg
:args-done

if defined WindowsSDKDir goto select-target
if defined VCINSTALLDIR goto select-target

@rem Look for Visual Studio 2013
if not defined VS120COMNTOOLS goto vc-set-2012
if not exist "%VS120COMNTOOLS%\..\..\vc\vcvarsall.bat" goto vc-set-2012
call "%VS120COMNTOOLS%\..\..\vc\vcvarsall.bat" %vs_toolset%
set GYP_MSVS_VERSION=2013
goto select-target

:vc-set-2012
@rem Look for Visual Studio 2012
if not defined VS110COMNTOOLS goto vc-set-2010
if not exist "%VS110COMNTOOLS%\..\..\vc\vcvarsall.bat" goto vc-set-2010
call "%VS110COMNTOOLS%\..\..\vc\vcvarsall.bat" %vs_toolset%
set GYP_MSVS_VERSION=2012
goto select-target

:vc-set-2010
@rem Look for Visual Studio 2010
if not defined VS100COMNTOOLS goto vc-set-2008
if not exist "%VS100COMNTOOLS%\..\..\vc\vcvarsall.bat" goto vc-set-2008
call "%VS100COMNTOOLS%\..\..\vc\vcvarsall.bat" %vs_toolset%
set GYP_MSVS_VERSION=2010
goto select-target

:vc-set-2008
@rem Look for Visual Studio 2008
if not defined VS90COMNTOOLS goto vc-set-notfound
if not exist "%VS90COMNTOOLS%\..\..\vc\vcvarsall.bat" goto vc-set-notfound
call "%VS90COMNTOOLS%\..\..\vc\vcvarsall.bat" %vs_toolset%
set GYP_MSVS_VERSION=2008
goto select-target

:vc-set-notfound
echo Warning: Visual Studio not found

:select-target
if not "%config%"=="" goto project-gen
set config=Debug

:project-gen
@rem Skip project generation if requested.
if defined noprojgen goto msbuild

@rem download 3proxy.
if exist third\3proxy\.git goto have_3proxy
echo git clone https://github.com/z3APA3A/3proxy.git third/3proxy
git clone https://github.com/z3APA3A/3proxy.git third/3proxy
if errorlevel 1 goto 3proxy_install_failed
goto have_libuv

:3proxy_install_failed
echo Failed to download 3proxy. Make sure you have git installed, or
echo manually install 3proxy into %~dp0third/3proxy.
exit /b 1

:have_3proxy
goto build_project

:build_project
goto msbuild

:msbuild
@rem Check if VS build env is available
if defined VCINSTALLDIR goto msbuild-found
if defined WindowsSDKDir goto msbuild-found

goto exit

@rem Build the sln with msbuild.
:msbuild-found
msbuild stresstest.sln /t:%target% /p:Configuration=%config% /p:Platform="%platform%" /clp:NoSummary;NoItemAndPropertyList;Verbosity=minimal /nologo
if errorlevel 1 exit /b 1

:arg-update
@rem update source by git.
echo 'git pull easystresstest'
git pull origin master
if exist third/libuv (
	cd third/libuv 
	echo 'git pull libuv'
	git pull origin master
	if exist build/gyp	(
		cd build/gyp
		echo 'git pull gyp'
		git pull origin master
		cd ../../
	)
	cd ../../
)
goto exit

:help
echo vcbuild.bat [update]
echo Examples:
echo   vcbuild.bat              : builds debug build
echo   vcbuild.bat update       : git pull source
goto exit

:exit

