; sensescanner.nsi
;
;--------------------------------

; The name of the installer
Name "Kinatomic Sense Scanner"

; The file to write
OutFile "SenseScannerInstaller.exe"

; The default installation directory
InstallDir $PROGRAMFILES\SenseScanner

; The text to prompt the user to enter a directory
DirText "This will install Sense Scanner on your computer. Choose a directory"

RequestExecutionLevel admin #NOTE: You still need to check user rights with UserInfo!

;--------------------------------

; The stuff to install
Section "" ;No components page, name is not important

; Set output path to the installation directory.
SetOutPath $INSTDIR

SetShellVarContext all

; Put file there
File sensescanner.exe
File about.html
File lgpl-info.txt
File Kinatomic-Logo.jpg
File *.*
File *.py
File *.exe
File *.dll
File *.manifest
File *.txt
File python.exe
File /r DLLs
File /r Doc
File /r icons
File /r include
File /r Lib
File /r libs
File /r reltracker
File /r Scripts
File /r shapes
File /r tcl
File /r Tools

; Tell the compiler to write an uninstaller and to look for a "Uninstall" section
WriteUninstaller $INSTDIR\Uninstall.exe

CreateDirectory "$SMPROGRAMS\Kinatomic"
CreateShortCut "$SMPROGRAMS\Kinatomic\Sense Scanner.lnk" "$INSTDIR\sensescanner.exe"
CreateShortCut "$SMPROGRAMS\Kinatomic\Uninstall Sense Scanner.lnk" "$INSTDIR\Uninstall.exe"
SectionEnd ; end the section

;----------------------------------

; The uninstall section
Section "Uninstall"

SetShellVarContext all

Delete $INSTDIR\Uninstall.exe
Delete $INSTDIR\*.*
RMDir /r $INSTDIR\DLLs
RMDir /r $INSTDIR\Doc
RMDir /r $INSTDIR\icons
RMDir /r $INSTDIR\include
RMDir /r $INSTDIR\Lib
RMDir /r $INSTDIR\libs
RMDir /r $INSTDIR\reltracker
RMDir /r $INSTDIR\Scripts
RMDir /r $INSTDIR\shapes
RMDir /r $INSTDIR\tcl
RMDir /r $INSTDIR\Tools
RMDir $INSTDIR

Delete "$SMPROGRAMS\Kinatomic\Sense Scanner.lnk"
Delete "$SMPROGRAMS\Kinatomic\Uninstall Sense Scanner.lnk"
RMDIR "$SMPROGRAMS\Kinatomic."

SectionEnd 
