# NSIS (Nullsoft Scriptable Install System) script to build the MS-Windows
# installer of OSMO (http://clayo.org/osmo/)
#
# 2009, Nacho Alonso Gonzalez <nacho.alonso.gonzalez@gmail.com>
#
# Script based on an example written by Joost Verburg

!include "MUI2.nsh"

;General
  ;Name and file
  Name "OSMO"
  OutFile "osmo-0.2.8.exe"

  ;Default installation folder
  InstallDir "$PROGRAMFILES\Osmo"
  
  ;Get installation folder from registry if available
  InstallDirRegKey HKCU "Software\Osmo" ""

  ;Request application privileges for Windows Vista
  RequestExecutionLevel user

  ;Application Icon
  !define MUI_ICON "osmo.ico"
  
  !define MUI_ABORTWARNING

;Pages
  !insertmacro MUI_PAGE_WELCOME
  !insertmacro MUI_PAGE_LICENSE "COPYING"
  !insertmacro MUI_PAGE_DIRECTORY
  !insertmacro MUI_PAGE_INSTFILES

  !insertmacro MUI_UNPAGE_CONFIRM
  !insertmacro MUI_UNPAGE_INSTFILES
  !define MUI_FINISHPAGE_RUN "$INSTDIR\osmo.exe"
  !insertmacro MUI_PAGE_FINISH
  
;Languages
 
  !insertmacro MUI_LANGUAGE "English" ;Default language
  !insertmacro MUI_LANGUAGE "French"
  !insertmacro MUI_LANGUAGE "German"
  !insertmacro MUI_LANGUAGE "Spanish"
  !insertmacro MUI_LANGUAGE "SpanishInternational"
  !insertmacro MUI_LANGUAGE "SimpChinese"
  !insertmacro MUI_LANGUAGE "TradChinese"
  !insertmacro MUI_LANGUAGE "Japanese"
  !insertmacro MUI_LANGUAGE "Korean"
  !insertmacro MUI_LANGUAGE "Italian"
  !insertmacro MUI_LANGUAGE "Dutch"
  !insertmacro MUI_LANGUAGE "Danish"
  !insertmacro MUI_LANGUAGE "Swedish"
  !insertmacro MUI_LANGUAGE "Norwegian"
  !insertmacro MUI_LANGUAGE "NorwegianNynorsk"
  !insertmacro MUI_LANGUAGE "Finnish"
  !insertmacro MUI_LANGUAGE "Greek"
  !insertmacro MUI_LANGUAGE "Russian"
  !insertmacro MUI_LANGUAGE "Portuguese"
  !insertmacro MUI_LANGUAGE "PortugueseBR"
  !insertmacro MUI_LANGUAGE "Polish"
  !insertmacro MUI_LANGUAGE "Ukrainian"
  !insertmacro MUI_LANGUAGE "Czech"
  !insertmacro MUI_LANGUAGE "Slovak"
  !insertmacro MUI_LANGUAGE "Croatian"
  !insertmacro MUI_LANGUAGE "Bulgarian"
  !insertmacro MUI_LANGUAGE "Hungarian"
  !insertmacro MUI_LANGUAGE "Thai"
  !insertmacro MUI_LANGUAGE "Romanian"
  !insertmacro MUI_LANGUAGE "Latvian"
  !insertmacro MUI_LANGUAGE "Macedonian"
  !insertmacro MUI_LANGUAGE "Estonian"
  !insertmacro MUI_LANGUAGE "Turkish"
  !insertmacro MUI_LANGUAGE "Lithuanian"
  !insertmacro MUI_LANGUAGE "Slovenian"
  !insertmacro MUI_LANGUAGE "Serbian"
  !insertmacro MUI_LANGUAGE "SerbianLatin"
  !insertmacro MUI_LANGUAGE "Arabic"
  !insertmacro MUI_LANGUAGE "Farsi"
  !insertmacro MUI_LANGUAGE "Hebrew"
  !insertmacro MUI_LANGUAGE "Indonesian"
  !insertmacro MUI_LANGUAGE "Mongolian"
  !insertmacro MUI_LANGUAGE "Luxembourgish"
  !insertmacro MUI_LANGUAGE "Albanian"
  !insertmacro MUI_LANGUAGE "Breton"
  !insertmacro MUI_LANGUAGE "Belarusian"
  !insertmacro MUI_LANGUAGE "Icelandic"
  !insertmacro MUI_LANGUAGE "Malay"
  !insertmacro MUI_LANGUAGE "Bosnian"
  !insertmacro MUI_LANGUAGE "Kurdish"
  !insertmacro MUI_LANGUAGE "Irish"
  !insertmacro MUI_LANGUAGE "Uzbek"
  !insertmacro MUI_LANGUAGE "Galician"
  !insertmacro MUI_LANGUAGE "Afrikaans"
  !insertmacro MUI_LANGUAGE "Catalan"
  !insertmacro MUI_LANGUAGE "Esperanto"


Section "Osmo" SecOsmo
  SetOutPath "$INSTDIR"
  File /a "osmo.ico"
  File /r "src\*"
  
  ;Program shortcut
  ; Group
  CreateDirectory "$SMPROGRAMS\Osmo"
  ; Application
  CreateShortCut "$SMPROGRAMS\Osmo\Osmo.lnk" "$INSTDIR\osmo.exe" "" "$INSTDIR\osmo.ico"
  ; Uninstaller
  CreateShortCut "$SMPROGRAMS\Osmo\Uninstall.lnk" "$INSTDIR\Uninstall.exe"
  ; Quick launch shortcut
  CreateShortCut "$QUICKLAUNCH\osmo.lnk" "$INSTDIR\osmo.exe" "" "$INSTDIR\osmo.ico"

  ;Store installation folder
  WriteRegStr HKCU "Software\Osmo" "" "$INSTDIR"
  
  ;Create uninstaller
  WriteUninstaller "$INSTDIR\Uninstall.exe"

SectionEnd

;Uninstaller Section

Section "Uninstall"
  
  ; Delete shortcuts
  Delete "$SMPROGRAMS\Osmo\Osmo.lnk"
  Delete "$SMPROGRAMS\Osmo\Uninstall.lnk"
  RMDir "$SMPROGRAMS\Osmo"
  
  ;Quick launch bar shortcut
  Delete "$QUICKLAUNCH\osmo.lnk"
  
  ; Delete application itself
  RMDir /r "$INSTDIR"

  DeleteRegKey /ifempty HKCU "Software\Osmo"

SectionEnd