# Microsoft Developer Studio Project File - Name="xblastc" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Console Application" 0x0103

CFG=xblastc - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "xblastc.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "xblastc.mak" CFG="xblastc - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "xblastc - Win32 Release" (based on "Win32 (x86) Console Application")
!MESSAGE "xblastc - Win32 Debug" (based on "Win32 (x86) Console Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "xblastc - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /c
# ADD BASE RSC /l 0x407 /d "NDEBUG"
# ADD RSC /l 0x407 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386

!ELSEIF  "$(CFG)" == "xblastc - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /GZ /c
# ADD CPP /nologo /w /W0 /Gm /GX /ZI /Od /D "WIN32" /D "_MBCS" /D "WMS" /D "W32" /D "WINMS32" /D "DEBUG_NAT" /D "_WINDOWS" /FR /YX /FD /D /D /GZ /c
# ADD BASE RSC /l 0x407 /d "_DEBUG"
# ADD RSC /l 0x407 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib winmm.lib Ws2_32.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept
# SUBTRACT LINK32 /pdb:none

!ENDIF 

# Begin Target

# Name "xblastc - Win32 Release"
# Name "xblastc - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=D:\games\xbtntexp\xblast\action.c
# End Source File
# Begin Source File

SOURCE=D:\games\xbtntexp\xblast\atom.c
# End Source File
# Begin Source File

SOURCE=D:\games\xbtntexp\xblast\bomb.c
# End Source File
# Begin Source File

SOURCE=D:\games\xbtntexp\xblast\bot.c
# End Source File
# Begin Source File

SOURCE=D:\games\xbtntexp\xblast\browse.c
# End Source File
# Begin Source File

SOURCE=D:\games\xbtntexp\xblast\central.c
# End Source File
# Begin Source File

SOURCE=D:\games\xbtntexp\xblast\cfg_control.c
# End Source File
# Begin Source File

SOURCE=D:\games\xbtntexp\xblast\cfg_demo.c
# End Source File
# Begin Source File

SOURCE=D:\games\xbtntexp\xblast\cfg_game.c
# End Source File
# Begin Source File

SOURCE=D:\games\xbtntexp\xblast\cfg_level.c
# End Source File
# Begin Source File

SOURCE=D:\games\xbtntexp\xblast\cfg_main.c
# End Source File
# Begin Source File

SOURCE=D:\games\xbtntexp\xblast\cfg_player.c
# End Source File
# Begin Source File

SOURCE=D:\games\xbtntexp\xblast\cfg_stat.c
# End Source File
# Begin Source File

SOURCE=D:\games\xbtntexp\xblast\cfg_xblast.c
# End Source File
# Begin Source File

SOURCE=.\chat.c
# End Source File
# Begin Source File

SOURCE=D:\games\xbtntexp\xblast\client.c
# End Source File
# Begin Source File

SOURCE=D:\games\xbtntexp\xblast\color.c
# End Source File
# Begin Source File

SOURCE=D:\games\xbtntexp\xblast\com.c
# End Source File
# Begin Source File

SOURCE=D:\games\xbtntexp\xblast\com_base.c
# End Source File
# Begin Source File

SOURCE=D:\games\xbtntexp\xblast\com_browse.c
# End Source File
# Begin Source File

SOURCE=D:\games\xbtntexp\xblast\com_central.c
# End Source File
# Begin Source File

SOURCE=D:\games\xbtntexp\xblast\com_dg_client.c
# End Source File
# Begin Source File

SOURCE=D:\games\xbtntexp\xblast\com_dg_server.c
# End Source File
# Begin Source File

SOURCE=D:\games\xbtntexp\xblast\com_dgram.c
# End Source File
# Begin Source File

SOURCE=D:\games\xbtntexp\xblast\com_from_central.c
# End Source File
# Begin Source File

SOURCE=D:\games\xbtntexp\xblast\com_listen.c
# End Source File
# Begin Source File

SOURCE=D:\games\xbtntexp\xblast\com_newgame.c
# End Source File
# Begin Source File

SOURCE=D:\games\xbtntexp\xblast\com_query.c
# End Source File
# Begin Source File

SOURCE=D:\games\xbtntexp\xblast\com_reply.c
# End Source File
# Begin Source File

SOURCE=D:\games\xbtntexp\xblast\com_stream.c
# End Source File
# Begin Source File

SOURCE=D:\games\xbtntexp\xblast\com_to_central.c
# End Source File
# Begin Source File

SOURCE=D:\games\xbtntexp\xblast\com_to_client.c
# End Source File
# Begin Source File

SOURCE=D:\games\xbtntexp\xblast\com_to_server.c
# End Source File
# Begin Source File

SOURCE=D:\games\xbtntexp\xblast\dat_rating.c
# End Source File
# Begin Source File

SOURCE=D:\games\xbtntexp\xblast\debug.c
# End Source File
# Begin Source File

SOURCE=D:\games\xbtntexp\xblast\demo.c
# End Source File
# Begin Source File

SOURCE=D:\games\xbtntexp\xblast\event.c
# End Source File
# Begin Source File

SOURCE=D:\games\xbtntexp\xblast\func.c
# End Source File
# Begin Source File

SOURCE=D:\games\xbtntexp\xblast\game.c
# End Source File
# Begin Source File

SOURCE=D:\games\xbtntexp\xblast\game_client.c
# End Source File
# Begin Source File

SOURCE=D:\games\xbtntexp\xblast\game_demo.c
# End Source File
# Begin Source File

SOURCE=D:\games\xbtntexp\xblast\game_local.c
# End Source File
# Begin Source File

SOURCE=D:\games\xbtntexp\xblast\game_server.c
# End Source File
# Begin Source File

SOURCE=D:\games\xbtntexp\xblast\image.c
# End Source File
# Begin Source File

SOURCE=D:\games\xbtntexp\xblast\info.c
# End Source File
# Begin Source File

SOURCE=D:\games\xbtntexp\xblast\ini_file.c
# End Source File
# Begin Source File

SOURCE=D:\games\xbtntexp\xblast\intro.c
# End Source File
# Begin Source File

SOURCE=D:\games\xbtntexp\xblast\introdat.c
# End Source File
# Begin Source File

SOURCE=D:\games\xbtntexp\xblast\level.c
# End Source File
# Begin Source File

SOURCE=D:\games\xbtntexp\xblast\map.c
# End Source File
# Begin Source File

SOURCE=D:\games\xbtntexp\xblast\menu.c
# End Source File
# Begin Source File

SOURCE=D:\games\xbtntexp\xblast\menu_control.c
# End Source File
# Begin Source File

SOURCE=.\menu_edit.c
# End Source File
# Begin Source File

SOURCE=D:\games\xbtntexp\xblast\menu_extras.c
# End Source File
# Begin Source File

SOURCE=D:\games\xbtntexp\xblast\menu_game.c
# End Source File
# Begin Source File

SOURCE=D:\games\xbtntexp\xblast\menu_level.c
# End Source File
# Begin Source File

SOURCE=D:\games\xbtntexp\xblast\menu_network.c
# End Source File
# Begin Source File

SOURCE=D:\games\xbtntexp\xblast\menu_player.c
# End Source File
# Begin Source File

SOURCE=D:\games\xbtntexp\xblast\mi_base.c
# End Source File
# Begin Source File

SOURCE=D:\games\xbtntexp\xblast\mi_button.c
# End Source File
# Begin Source File

SOURCE=D:\games\xbtntexp\xblast\mi_color.c
# End Source File
# Begin Source File

SOURCE=D:\games\xbtntexp\xblast\mi_combo.c
# End Source File
# Begin Source File

SOURCE=D:\games\xbtntexp\xblast\mi_cyclic.c
# End Source File
# Begin Source File

SOURCE=D:\games\xbtntexp\xblast\mi_host.c
# End Source File
# Begin Source File

SOURCE=D:\games\xbtntexp\xblast\mi_int.c
# End Source File
# Begin Source File

SOURCE=D:\games\xbtntexp\xblast\mi_keysym.c
# End Source File
# Begin Source File

SOURCE=D:\games\xbtntexp\xblast\mi_label.c
# End Source File
# Begin Source File

SOURCE=D:\games\xbtntexp\xblast\mi_map.c
# End Source File
# Begin Source File

SOURCE=D:\games\xbtntexp\xblast\mi_player.c
# End Source File
# Begin Source File

SOURCE=D:\games\xbtntexp\xblast\mi_stat.c
# End Source File
# Begin Source File

SOURCE=D:\games\xbtntexp\xblast\mi_string.c
# End Source File
# Begin Source File

SOURCE=D:\games\xbtntexp\xblast\mi_tag.c
# End Source File
# Begin Source File

SOURCE=D:\games\xbtntexp\xblast\mi_toggle.c
# End Source File
# Begin Source File

SOURCE=D:\games\xbtntexp\xblast\mi_tool.c
# End Source File
# Begin Source File

SOURCE=D:\games\xbtntexp\xblast\net_dgram.c
# End Source File
# Begin Source File

SOURCE=D:\games\xbtntexp\xblast\net_socket.c
# End Source File
# Begin Source File

SOURCE=D:\games\xbtntexp\xblast\net_tele.c
# End Source File
# Begin Source File

SOURCE=D:\games\xbtntexp\xblast\network.c
# End Source File
# Begin Source File

SOURCE=D:\games\xbtntexp\xblast\player.c
# End Source File
# Begin Source File

SOURCE=D:\games\xbtntexp\xblast\random.c
# End Source File
# Begin Source File

SOURCE=D:\games\xbtntexp\xblast\scramble.c
# End Source File
# Begin Source File

SOURCE=D:\games\xbtntexp\xblast\server.c
# End Source File
# Begin Source File

SOURCE=D:\games\xbtntexp\xblast\shrink.c
# End Source File
# Begin Source File

SOURCE=D:\games\xbtntexp\xblast\sprite.c
# End Source File
# Begin Source File

SOURCE=D:\games\xbtntexp\xblast\status.c
# End Source File
# Begin Source File

SOURCE=D:\games\xbtntexp\xblast\str_util.c
# End Source File
# Begin Source File

SOURCE=D:\games\xbtntexp\xblast\user.c
# End Source File
# Begin Source File

SOURCE=D:\games\xbtntexp\xblast\util.c
# End Source File
# Begin Source File

SOURCE=.\version.c
# End Source File
# Begin Source File

SOURCE=D:\games\xbtntexp\xblast\w32_atom.c
# End Source File
# Begin Source File

SOURCE=D:\games\xbtntexp\xblast\w32_common.c
# End Source File
# Begin Source File

SOURCE=D:\games\xbtntexp\xblast\w32_config.c
# End Source File
# Begin Source File

SOURCE=D:\games\xbtntexp\xblast\w32_event.c
# End Source File
# Begin Source File

SOURCE=D:\games\xbtntexp\xblast\w32_image.c
# End Source File
# Begin Source File

SOURCE=D:\games\xbtntexp\xblast\w32_init.c
# End Source File
# Begin Source File

SOURCE=D:\games\xbtntexp\xblast\w32_joystick.c
# End Source File
# Begin Source File

SOURCE=D:\games\xbtntexp\xblast\w32_keysym.c
# End Source File
# Begin Source File

SOURCE=D:\games\xbtntexp\xblast\w32_msgbox.c
# End Source File
# Begin Source File

SOURCE=D:\games\xbtntexp\xblast\w32_pixmap.c
# End Source File
# Begin Source File

SOURCE=D:\games\xbtntexp\xblast\w32_sndsrv.c
# End Source File
# Begin Source File

SOURCE=D:\games\xbtntexp\xblast\w32_socket.c
# End Source File
# Begin Source File

SOURCE=D:\games\xbtntexp\xblast\w32_sound.c
# End Source File
# Begin Source File

SOURCE=D:\games\xbtntexp\xblast\w32_sprite.c
# End Source File
# Begin Source File

SOURCE=D:\games\xbtntexp\xblast\w32_text.c
# End Source File
# Begin Source File

SOURCE=D:\games\xbtntexp\xblast\w32_tile.c
# End Source File
# Begin Source File

SOURCE=D:\games\xbtntexp\xblast\xblast.c
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=D:\games\xbtntexp\xblast\action.h
# End Source File
# Begin Source File

SOURCE=D:\games\xbtntexp\xblast\atom.h
# End Source File
# Begin Source File

SOURCE=D:\games\xbtntexp\xblast\bomb.h
# End Source File
# Begin Source File

SOURCE=D:\games\xbtntexp\xblast\bot.h
# End Source File
# Begin Source File

SOURCE=D:\games\xbtntexp\xblast\browse.h
# End Source File
# Begin Source File

SOURCE=D:\games\xbtntexp\xblast\central.h
# End Source File
# Begin Source File

SOURCE=D:\games\xbtntexp\xblast\cfg_control.h
# End Source File
# Begin Source File

SOURCE=D:\games\xbtntexp\xblast\cfg_demo.h
# End Source File
# Begin Source File

SOURCE=D:\games\xbtntexp\xblast\cfg_game.h
# End Source File
# Begin Source File

SOURCE=D:\games\xbtntexp\xblast\cfg_level.h
# End Source File
# Begin Source File

SOURCE=D:\games\xbtntexp\xblast\cfg_main.h
# End Source File
# Begin Source File

SOURCE=D:\games\xbtntexp\xblast\cfg_player.h
# End Source File
# Begin Source File

SOURCE=D:\games\xbtntexp\xblast\cfg_stat.h
# End Source File
# Begin Source File

SOURCE=D:\games\xbtntexp\xblast\cfg_xblast.h
# End Source File
# Begin Source File

SOURCE=.\chat.h
# End Source File
# Begin Source File

SOURCE=D:\games\xbtntexp\xblast\client.h
# End Source File
# Begin Source File

SOURCE=D:\games\xbtntexp\xblast\color.h
# End Source File
# Begin Source File

SOURCE=D:\games\xbtntexp\xblast\com.h
# End Source File
# Begin Source File

SOURCE=D:\games\xbtntexp\xblast\com_base.h
# End Source File
# Begin Source File

SOURCE=D:\games\xbtntexp\xblast\com_browse.h
# End Source File
# Begin Source File

SOURCE=D:\games\xbtntexp\xblast\com_central.h
# End Source File
# Begin Source File

SOURCE=D:\games\xbtntexp\xblast\com_dg_client.h
# End Source File
# Begin Source File

SOURCE=D:\games\xbtntexp\xblast\com_dg_server.h
# End Source File
# Begin Source File

SOURCE=D:\games\xbtntexp\xblast\com_dgram.h
# End Source File
# Begin Source File

SOURCE=D:\games\xbtntexp\xblast\com_from_central.h
# End Source File
# Begin Source File

SOURCE=D:\games\xbtntexp\xblast\com_listen.h
# End Source File
# Begin Source File

SOURCE=D:\games\xbtntexp\xblast\com_newgame.h
# End Source File
# Begin Source File

SOURCE=D:\games\xbtntexp\xblast\com_query.h
# End Source File
# Begin Source File

SOURCE=D:\games\xbtntexp\xblast\com_reply.h
# End Source File
# Begin Source File

SOURCE=D:\games\xbtntexp\xblast\com_stream.h
# End Source File
# Begin Source File

SOURCE=D:\games\xbtntexp\xblast\com_to_central.h
# End Source File
# Begin Source File

SOURCE=D:\games\xbtntexp\xblast\com_to_client.h
# End Source File
# Begin Source File

SOURCE=D:\games\xbtntexp\xblast\com_to_server.h
# End Source File
# Begin Source File

SOURCE=D:\games\xbtntexp\xblast\common.h
# End Source File
# Begin Source File

SOURCE=D:\games\xbtntexp\xblast\config.h
# End Source File
# Begin Source File

SOURCE=D:\games\xbtntexp\xblast\dat_rating.h
# End Source File
# Begin Source File

SOURCE=D:\games\xbtntexp\xblast\debug.h
# End Source File
# Begin Source File

SOURCE=D:\games\xbtntexp\xblast\demo.h
# End Source File
# Begin Source File

SOURCE=D:\games\xbtntexp\xblast\event.h
# End Source File
# Begin Source File

SOURCE=D:\games\xbtntexp\xblast\func.h
# End Source File
# Begin Source File

SOURCE=D:\games\xbtntexp\xblast\game.h
# End Source File
# Begin Source File

SOURCE=D:\games\xbtntexp\xblast\game_client.h
# End Source File
# Begin Source File

SOURCE=D:\games\xbtntexp\xblast\game_demo.h
# End Source File
# Begin Source File

SOURCE=D:\games\xbtntexp\xblast\game_local.h
# End Source File
# Begin Source File

SOURCE=D:\games\xbtntexp\xblast\game_server.h
# End Source File
# Begin Source File

SOURCE=D:\games\xbtntexp\xblast\geom.h
# End Source File
# Begin Source File

SOURCE=D:\games\xbtntexp\xblast\gui.h
# End Source File
# Begin Source File

SOURCE=D:\games\xbtntexp\xblast\image.h
# End Source File
# Begin Source File

SOURCE=D:\games\xbtntexp\xblast\info.h
# End Source File
# Begin Source File

SOURCE=D:\games\xbtntexp\xblast\ini_file.h
# End Source File
# Begin Source File

SOURCE=D:\games\xbtntexp\xblast\intro.h
# End Source File
# Begin Source File

SOURCE=D:\games\xbtntexp\xblast\introdat.h
# End Source File
# Begin Source File

SOURCE=D:\games\xbtntexp\xblast\level.h
# End Source File
# Begin Source File

SOURCE=D:\games\xbtntexp\xblast\map.h
# End Source File
# Begin Source File

SOURCE=D:\games\xbtntexp\xblast\menu.h
# End Source File
# Begin Source File

SOURCE=D:\games\xbtntexp\xblast\menu_control.h
# End Source File
# Begin Source File

SOURCE=.\menu_edit.h
# End Source File
# Begin Source File

SOURCE=D:\games\xbtntexp\xblast\menu_extras.h
# End Source File
# Begin Source File

SOURCE=D:\games\xbtntexp\xblast\menu_game.h
# End Source File
# Begin Source File

SOURCE=D:\games\xbtntexp\xblast\menu_layout.h
# End Source File
# Begin Source File

SOURCE=D:\games\xbtntexp\xblast\menu_level.h
# End Source File
# Begin Source File

SOURCE=D:\games\xbtntexp\xblast\menu_network.h
# End Source File
# Begin Source File

SOURCE=D:\games\xbtntexp\xblast\menu_player.h
# End Source File
# Begin Source File

SOURCE=D:\games\xbtntexp\xblast\mi_base.h
# End Source File
# Begin Source File

SOURCE=D:\games\xbtntexp\xblast\mi_button.h
# End Source File
# Begin Source File

SOURCE=D:\games\xbtntexp\xblast\mi_color.h
# End Source File
# Begin Source File

SOURCE=D:\games\xbtntexp\xblast\mi_combo.h
# End Source File
# Begin Source File

SOURCE=D:\games\xbtntexp\xblast\mi_cyclic.h
# End Source File
# Begin Source File

SOURCE=D:\games\xbtntexp\xblast\mi_host.h
# End Source File
# Begin Source File

SOURCE=D:\games\xbtntexp\xblast\mi_int.h
# End Source File
# Begin Source File

SOURCE=D:\games\xbtntexp\xblast\mi_keysym.h
# End Source File
# Begin Source File

SOURCE=D:\games\xbtntexp\xblast\mi_label.h
# End Source File
# Begin Source File

SOURCE=D:\games\xbtntexp\xblast\mi_map.h
# End Source File
# Begin Source File

SOURCE=D:\games\xbtntexp\xblast\mi_player.h
# End Source File
# Begin Source File

SOURCE=D:\games\xbtntexp\xblast\mi_stat.h
# End Source File
# Begin Source File

SOURCE=D:\games\xbtntexp\xblast\mi_string.h
# End Source File
# Begin Source File

SOURCE=D:\games\xbtntexp\xblast\mi_tag.h
# End Source File
# Begin Source File

SOURCE=D:\games\xbtntexp\xblast\mi_toggle.h
# End Source File
# Begin Source File

SOURCE=D:\games\xbtntexp\xblast\mi_tool.h
# End Source File
# Begin Source File

SOURCE=D:\games\xbtntexp\xblast\net_dgram.h
# End Source File
# Begin Source File

SOURCE=D:\games\xbtntexp\xblast\net_socket.h
# End Source File
# Begin Source File

SOURCE=D:\games\xbtntexp\xblast\net_tele.h
# End Source File
# Begin Source File

SOURCE=D:\games\xbtntexp\xblast\network.h
# End Source File
# Begin Source File

SOURCE=D:\games\xbtntexp\xblast\player.h
# End Source File
# Begin Source File

SOURCE=D:\games\xbtntexp\xblast\random.h
# End Source File
# Begin Source File

SOURCE=D:\games\xbtntexp\xblast\randomlib.h
# End Source File
# Begin Source File

SOURCE=D:\games\xbtntexp\xblast\rating.h
# End Source File
# Begin Source File

SOURCE=D:\games\xbtntexp\xblast\scramble.h
# End Source File
# Begin Source File

SOURCE=D:\games\xbtntexp\xblast\server.h
# End Source File
# Begin Source File

SOURCE=D:\games\xbtntexp\xblast\shrink.h
# End Source File
# Begin Source File

SOURCE=D:\games\xbtntexp\xblast\shrinkdat.h
# End Source File
# Begin Source File

SOURCE=D:\games\xbtntexp\xblast\snd.h
# End Source File
# Begin Source File

SOURCE=D:\games\xbtntexp\xblast\socket.h
# End Source File
# Begin Source File

SOURCE=D:\games\xbtntexp\xblast\sprite.h
# End Source File
# Begin Source File

SOURCE=D:\games\xbtntexp\xblast\status.h
# End Source File
# Begin Source File

SOURCE=D:\games\xbtntexp\xblast\str_util.h
# End Source File
# Begin Source File

SOURCE=D:\games\xbtntexp\xblast\timeval.h
# End Source File
# Begin Source File

SOURCE=D:\games\xbtntexp\xblast\user.h
# End Source File
# Begin Source File

SOURCE=D:\games\xbtntexp\xblast\util.h
# End Source File
# Begin Source File

SOURCE=D:\games\xbtntexp\xblast\version.h
# End Source File
# Begin Source File

SOURCE=D:\games\xbtntexp\xblast\w32_color.h
# End Source File
# Begin Source File

SOURCE=D:\games\xbtntexp\xblast\w32_common.h
# End Source File
# Begin Source File

SOURCE=D:\games\xbtntexp\xblast\w32_config.h
# End Source File
# Begin Source File

SOURCE=D:\games\xbtntexp\xblast\w32_event.h
# End Source File
# Begin Source File

SOURCE=D:\games\xbtntexp\xblast\w32_image.h
# End Source File
# Begin Source File

SOURCE=D:\games\xbtntexp\xblast\w32_joystick.h
# End Source File
# Begin Source File

SOURCE=D:\games\xbtntexp\xblast\w32_keysym.h
# End Source File
# Begin Source File

SOURCE=D:\games\xbtntexp\xblast\w32_mm.h
# End Source File
# Begin Source File

SOURCE=D:\games\xbtntexp\xblast\w32_pixmap.h
# End Source File
# Begin Source File

SOURCE=D:\games\xbtntexp\xblast\w32_sndsrv.h
# End Source File
# Begin Source File

SOURCE=D:\games\xbtntexp\xblast\w32_socket.h
# End Source File
# Begin Source File

SOURCE=D:\games\xbtntexp\xblast\w32_sprite.h
# End Source File
# Begin Source File

SOURCE=D:\games\xbtntexp\xblast\w32_text.h
# End Source File
# Begin Source File

SOURCE=D:\games\xbtntexp\xblast\w32_tile.h
# End Source File
# Begin Source File

SOURCE=D:\games\xbtntexp\xblast\xblast.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# End Group
# End Target
# End Project
