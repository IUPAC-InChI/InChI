# Microsoft Developer Studio Project File - Name="mol2inchi" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Console Application" 0x0103

CFG=mol2inchi - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "mol2inchi.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "mol2inchi.mak" CFG="mol2inchi - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "mol2inchi - Win32 Debug" (based on "Win32 (x86) Console Application")
!MESSAGE "mol2inchi - x64 Debug" (based on "Win32 (x86) Console Application")
!MESSAGE "mol2inchi - Win32 Release" (based on "Win32 (x86) Console Application")
!MESSAGE "mol2inchi - x64 Release" (based on "Win32 (x86) Console Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "mol2inchi - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "../../../bin/Windows/$(PlatformName)/$(ConfigurationName)"
# PROP BASE Intermediate_Dir "../../../bin/Windows/$(PlatformName)/$(ConfigurationName)/$(TargetName).tmp"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "../../../bin/Windows/Win32/Debug/"
# PROP Intermediate_Dir "../../../bin/Windows/Win32/Debug/mol2inchi.tmp"
# PROP Target_Dir ""
MTL=midl.exe
# ADD BASE MTL /nologo /win32
# ADD MTL /nologo /win32
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "BUILD_LINK_AS_DLL" /D TARGET_EXE_USING_API=1 /GZ PRECOMP_VC7_TOBEREMOVED /c
# ADD CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /D "_DEBUG" /D "BUILD_LINK_AS_DLL" /D TARGET_EXE_USING_API=1 /D "WIN32" /D "_CONSOLE" /GZ /c
# ADD BASE RSC /l 0x409
# ADD RSC /l 0x409
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /stack:0xf42400 /subsystem:console /debug /machine:IX86 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /stack:0xf42400 /subsystem:console /debug /machine:IX86 /pdbtype:sept

!ELSEIF  "$(CFG)" == "mol2inchi - x64 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "../../../bin/Windows/$(PlatformName)/$(ConfigurationName)"
# PROP BASE Intermediate_Dir "../../../bin/Windows/$(PlatformName)/$(ConfigurationName)/$(TargetName).tmp"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "../../../bin/Windows/x64/Debug/"
# PROP Intermediate_Dir "../../../bin/Windows/x64/Debug/mol2inchi.tmp"
# PROP Target_Dir ""
MTL=midl.exe
# ADD BASE MTL /nologo /win32
# ADD MTL /nologo /win32
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "TARGET_EXE_USING_API" /GZ PRECOMP_VC7_TOBEREMOVED /c
# ADD CPP /nologo /MTd /W3 /Gm /GX /Zi /Od /D "_DEBUG" /D "TARGET_EXE_USING_API" /D "WIN32" /D "_CONSOLE" /D TARGET_EXE_USING_API=1 /GZ /c
# ADD BASE RSC /l 0x409
# ADD RSC /l 0x409
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /stack:0xf42400 /subsystem:console /debug /machine:IX86 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /stack:0xf42400 /subsystem:console /debug /machine:IX86 /pdbtype:sept

!ELSEIF  "$(CFG)" == "mol2inchi - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "../../../bin/Windows/$(PlatformName)/$(ConfigurationName)"
# PROP BASE Intermediate_Dir "../../../bin/Windows/$(PlatformName)/$(ConfigurationName)/$(TargetName).tmp"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "../../../bin/Windows/Win32/Release"
# PROP Intermediate_Dir "../../../bin/Windows/Win32/Release/mol2inchi.tmp"
# PROP Target_Dir ""
MTL=midl.exe
# ADD BASE MTL /nologo /win32
# ADD MTL /nologo /win32
# ADD BASE CPP /nologo /MT /W3 /GX /Zi /Oi /Gy /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /TC PRECOMP_VC7_TOBEREMOVED /c
# ADD CPP /nologo /MT /W3 /GX /Zi /Oi /Gy /D "NDEBUG" /D "WIN32" /D "_CONSOLE" /D TARGET_EXE_USING_API=1 /FR /TC /c
# ADD BASE RSC /l 0x409
# ADD RSC /l 0x409
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /stack:0xf42400 /subsystem:console /debug /machine:IX86 /pdbtype:sept /opt:ref /opt:icf
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /stack:0xf42400 /subsystem:console /debug /machine:IX86 /pdbtype:sept /opt:ref /opt:icf

!ELSEIF  "$(CFG)" == "mol2inchi - x64 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "../../../bin/Windows/$(PlatformName)/$(ConfigurationName)"
# PROP BASE Intermediate_Dir "../../../bin/Windows/$(PlatformName)/$(ConfigurationName)/$(TargetName).tmp"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "../../../bin/Windows/x64/Release/"
# PROP Intermediate_Dir "../../../bin/Windows/x64/Release/mol2inchi.tmp"
# PROP Target_Dir ""
MTL=midl.exe
# ADD BASE MTL /nologo /win32
# ADD MTL /nologo /win32
# ADD BASE CPP /nologo /MT /W3 /GX /Zi /Oi /Gy /D "WIN32" /D "NDEBUG" /D "_CONSOLE" PRECOMP_VC7_TOBEREMOVED /c
# ADD CPP /nologo /MT /W3 /GX /Zi /Oi /Gy /D "NDEBUG" /D "WIN32" /D "_CONSOLE" /D TARGET_EXE_USING_API=1 /c
# ADD BASE RSC /l 0x409
# ADD RSC /l 0x409
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /stack:0xf42400 /subsystem:console /debug /machine:IX86 /pdbtype:sept /opt:ref /opt:icf
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /stack:0xf42400 /subsystem:console /debug /machine:IX86 /pdbtype:sept /opt:ref /opt:icf

!ENDIF 

# Begin Target

# Name "mol2inchi - Win32 Debug"
# Name "mol2inchi - x64 Debug"
# Name "mol2inchi - Win32 Release"
# Name "mol2inchi - x64 Release"
# Begin Group "Source Files"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\src\mol2inchi.c
DEP_CPP_MOL2I=\
	"..\..\..\..\INCHI_BASE\src\ichidrp.h"\
	"..\..\..\..\INCHI_BASE\src\ichisize.h"\
	"..\..\..\..\INCHI_BASE\src\inchi_api.h"\
	"..\..\..\..\INCHI_BASE\src\incomdef.h"\
	"..\..\..\..\INCHI_BASE\src\inpdef.h"\
	"..\..\..\..\INCHI_BASE\src\ixa.h"\
	"..\..\..\..\INCHI_BASE\src\mode.h"\
	"..\..\..\..\INCHI_BASE\src\mol_fmt.h"\
	"..\..\..\..\INCHI_BASE\src\tbb\tbbmalloc_proxy.h"\
	"..\..\..\..\INCHI_BASE\src\util.h"\
	"..\src\mol2inchi.h"\
	"..\src\moreutil.h"\
	
NODEP_CPP_MOL2I=\
	"..\..\..\..\INCHI_BASE\src\tbb\tbb\tbbmalloc_proxy.h"\
	
# End Source File
# Begin Source File

SOURCE=..\src\moreutil.c
DEP_CPP_MOREU=\
	"..\src\moreutil.h"\
	
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl;inc;xsd"
# Begin Source File

SOURCE=..\src\mol2inchi.h
# End Source File
# Begin Source File

SOURCE=..\src\moreutil.h
# End Source File
# Begin Source File

SOURCE=.\resource.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "rc;ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe;resx;tiff;tif;png;wav"
# Begin Source File

SOURCE=.\mol2inchi.rc
# End Source File
# End Group
# End Target
# End Project
