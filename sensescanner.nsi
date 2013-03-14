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

;--------------------------------

; The stuff to install
Section "" ;No components page, name is not important

; Set output path to the installation directory.
SetOutPath $INSTDIR

; Put file there
File sensescanner.exe
File about.html
File lgpl-info.txt
File Kinatomic-Logo.jpg
File *.dll
File *.py
File python.exe
File /r shapes
File /r icons
File /r python-lib
File /r reltracker
File /r site-packages

; Tell the compiler to write an uninstaller and to look for a "Uninstall" section
WriteUninstaller $INSTDIR\Uninstall.exe

CreateDirectory "$SMPROGRAMS\Kinatomic"
CreateShortCut "$SMPROGRAMS\Kinatomic\Sense Scanner.lnk" "$INSTDIR\sensescanner.exe"
CreateShortCut "$SMPROGRAMS\Kinatomic\Uninstall Sense Scanner.lnk" "$INSTDIR\Uninstall.exe"
SectionEnd ; end the section

;----------------------------------

; The uninstall section
Section "Uninstall"

Delete $INSTDIR\Uninstall.exe
Delete $INSTDIR\*.*
Delete $INSTDIR\icons\*.*
Delete $INSTDIR\shapes\*.*
Delete $INSTDIR\python-lib\*.*
Delete $INSTDIR\reltracker\*.*
Delete $INSTDIR\site-packages\*.*
RMDir $INSTDIR\icons
RMDir $INSTDIR\shapes
RMDir /r $INSTDIR\python-lib
RMDir /r $INSTDIR\reltracker
RMDir /r $INSTDIR\site-packages
RMDir $INSTDIR

Delete "$SMPROGRAMS\Kinatomic\Sense Scanner.lnk"
Delete "$SMPROGRAMS\Kinatomic\Uninstall Sense Scanner.lnk"
RMDIR "$SMPROGRAMS\Kinatomic."

SectionEnd 
