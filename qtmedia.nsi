; qtmedia.nsi
;
;--------------------------------

; The name of the installer
Name "QtMedia"

; The file to write
OutFile "QtMediaInstaller.exe"

; The default installation directory
InstallDir $PROGRAMFILES\QtMedia

; The text to prompt the user to enter a directory
DirText "This will install QtMedia on your computer. Choose a directory"

;--------------------------------

; The stuff to install
Section "" ;No components page, name is not important

; Set output path to the installation directory.
SetOutPath $INSTDIR

; Put file there
File QtMedia.exe
File licensing.txt
File *.dll
File /r shapes
File /r icons

; Tell the compiler to write an uninstaller and to look for a "Uninstall" section
WriteUninstaller $INSTDIR\Uninstall.exe

CreateDirectory "$SMPROGRAMS\QtMedia"
CreateShortCut "$SMPROGRAMS\QtMedia\QtMedia.lnk" "$INSTDIR\QtMedia.exe"
CreateShortCut "$SMPROGRAMS\QtMedia\Uninstall QtMedia.lnk" "$INSTDIR\Uninstall.exe"
SectionEnd ; end the section

;----------------------------------

; The uninstall section
Section "Uninstall"

Delete $INSTDIR\Uninstall.exe
Delete $INSTDIR\*.*
Delete $INSTDIR\icons\*.*
Delete $INSTDIR\shapes\*.*
RMDir $INSTDIR\icons
RMDir $INSTDIR\shapes
RMDir $INSTDIR

Delete "$SMPROGRAMS\QtMedia\QtMedia.lnk"
RMDIR "$SMPROGRAMS\QtMedia."

SectionEnd 
