;SteatlthGame installer script 
;Written by Grebnev Pavel
;Welcome/Finish Page Example Script by Joost Verburg

!define GAME_NAME 'StealthGame'

!define AUTHOR 'Grebnev Pavel'

!define PUBLISHER 'Grebnev Pavel'

!define ORIGPATH '..\build'

; �������� ������ ��� ����� ������ �� ����� VersionInfo
!define /file MAIN_VERSION '..\VersionInfo'
!define /file BUILD_NUM '..\BuildNum'
; ��� �����
!define /file TAG '..\Tag'
!define VERSION "${MAIN_VERSION}.${BUILD_NUM}"

;--------------------------------
;Include Modern UI

  !include "MUI2.nsh"

;--------------------------------
;General

	;Name and file
	Name "${GAME_NAME}"
	Caption "��������� ${GAME_NAME} ${VERSION}"
	OutFile ".\out\${GAME_NAME} ${VERSION} ${TAG}Setup.exe"
	!system 'mkdir out'

	;������� ��������
	!define MUI_WELCOMEFINISHPAGE_BITMAP ".\Welcome.bmp"
	!define MUI_UNWELCOMEFINISHPAGE_BITMAP ".\UnWelcome.bmp"

	VIProductVersion ${VERSION}
	VIAddVersionKey "ProductName" ${GAME_NAME}
	VIAddVersionKey "Comments" "alpha-release"
	VIAddVersionKey "CompanyName" "${AUTHOR}"
	VIAddVersionKey "FileDescription" "${GAME_NAME} Install"
	VIAddVersionKey "FileVersion" ${VERSION}
	VIAddVersionKey "LegalCopyright" "Copyright ${AUTHOR}"
	VIAddVersionKey "ProductVersion" ${VERSION}
	
	BrandingText "${PUBLISHER} | Powered by NSIS ${NSIS_VERSION}"
	
	;Default installation folder
	InstallDir "$PROGRAMFILES\${GAME_NAME}"

	;Get installation folder from registry if available
	InstallDirRegKey HKCU "Software\${GAME_NAME}" ""
	
	Icon ".\Install.ico"
	!insertmacro MUI_DEFAULT MUI_ICON ".\Install.ico"
	!insertmacro MUI_DEFAULT MUI_UNICON ".\Uninstall.ico"

	;!define MUI_FINISHPAGE_LINK "�������� ���� ����, ����� ������ ������!"
	;!define MUI_FINISHPAGE_LINK_LOCATION "http://ruwhynot.com/"

	!define MUI_FINISHPAGE_RUN "$INSTDIR\Game.exe"
	!define MUI_FINISHPAGE_NOREBOOTSUPPORT

	!define MUI_FINISHPAGE_SHOWREADME
	!define MUI_FINISHPAGE_SHOWREADME_TEXT "Show release notes"
	!define MUI_FINISHPAGE_SHOWREADME_FUNCTION ShowReleaseNotes

	;Request application privileges for Windows Vista
	RequestExecutionLevel user

;--------------------------------
;Interface Settings

	; �������� �������������� ��� ������� ������� "������"
	!define MUI_ABORTWARNING

;--------------------------------
;�������

;����������� ��������� � ����� ������
Function ShowReleaseNotes
	${If} ${FileExists} $INSTDIR\ReleaseNotes.htm
		ExecShell "open" "$INSTDIR\ReleaseNotes.htm"
	${Else}
		ExecShell "open" "http://code.ruwhynot.com/stealthgame"
	${EndIf}
FunctionEnd

;--------------------------------
;Pages

	!insertmacro MUI_PAGE_WELCOME
	!insertmacro MUI_PAGE_LICENSE ".\License.txt"
	!insertmacro MUI_PAGE_COMPONENTS
	!insertmacro MUI_PAGE_DIRECTORY
	!insertmacro MUI_PAGE_INSTFILES
	!insertmacro MUI_PAGE_FINISH

	!insertmacro MUI_UNPAGE_WELCOME
	!insertmacro MUI_UNPAGE_CONFIRM
	!insertmacro MUI_UNPAGE_INSTFILES
	!insertmacro MUI_UNPAGE_FINISH

;--------------------------------
;Languages

	!insertmacro MUI_LANGUAGE "English"

;--------------------------------
;Installer Sections

;������������ ������ � ������������ ����
Section "1. Install the game" Sction_IWGame

	;����, ��������� � ���, ��� ��� ������������ ���������
	SectionIn RO

	SetOutPath $INSTDIR
	File /r ${ORIGPATH}\*.*
	
	CreateDirectory "$SMPROGRAMS\${GAME_NAME}"
	CreateShortCut "$SMPROGRAMS\${GAME_NAME}\${GAME_NAME}.lnk" "$INSTDIR\Game.exe" "" "$INSTDIR\Game.ico"
	CreateShortCut "$SMPROGRAMS\${GAME_NAME}\Editor.lnk" "$INSTDIR\Editor.exe" "" "$INSTDIR\Editor.ico"
	CreateShortCut "$SMPROGRAMS\${GAME_NAME}\Uninstall.lnk" "$INSTDIR\Uninstall.exe"

	;Store installation folder
	WriteRegStr HKCU "Software\${PUBLISHER}" "" $INSTDIR

	;������ � ������� ������ �� �����������
	WriteRegStr HKLM "SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\${GAME_NAME}" "DisplayIcon" "$INSTDIR\Uninstall.ico"
	WriteRegStr HKLM "SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\${GAME_NAME}" "DisplayName" "${GAME_NAME}"
	WriteRegStr HKLM "SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\${GAME_NAME}" "DisplayVersion" "${VERSION}"
	WriteRegStr HKLM "SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\${GAME_NAME}" "Publisher" "${PUBLISHER}"
	WriteRegStr HKLM "SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\${GAME_NAME}" "UninstallString" "$INSTDIR\Uninstall.exe"
	WriteRegStr HKLM "SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\${GAME_NAME}" "InstallLocation" $INSTDIR
	WriteRegStr HKLM "SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\${GAME_NAME}" "URLInfoAbout" "http://code.ruwhynot.com/stealthgame"
	WriteRegDWORD HKLM "SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\${GAME_NAME}" "NoModify" 0x00000001
	WriteRegDWORD HKLM "SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\${GAME_NAME}" "NoRepair" 0x00000001
	
	;Create uninstaller
	WriteUninstaller "$INSTDIR\Uninstall.exe"

SectionEnd

;������ ���������� ������ �� ������� �����
Section "2. Desktop shortcut" Sction_Shortcut

	CreateShortCut "$DESKTOP\${GAME_NAME}.lnk" "$INSTDIR\Game.exe" "" "$INSTDIR\Game.ico"

SectionEnd

;--------------------------------
;Descriptions

	;Assign language strings to sections
	!insertmacro MUI_FUNCTION_DESCRIPTION_BEGIN
		!insertmacro MUI_DESCRIPTION_TEXT ${Sction_IWGame} "Install ${GAME_NAME} to the computer."
		!insertmacro MUI_DESCRIPTION_TEXT ${Sction_Shortcut} "Place shortcut on the desktop."
	!insertmacro MUI_FUNCTION_DESCRIPTION_END

;--------------------------------
;Uninstaller Section

Section "Uninstall"

	; ������� ���� ������� (���� ������� ���������� �������� �� �����������)
	RMDir /r $INSTDIR

	; ������� ������ �� ���� ����
	Delete "$SMPROGRAMS\${GAME_NAME}\${GAME_NAME}.lnk"
	Delete "$SMPROGRAMS\${GAME_NAME}\Editor.lnk"
	Delete "$SMPROGRAMS\${GAME_NAME}\Uninstall.lnk"
	RMDir "$SMPROGRAMS\${GAME_NAME}"
	
	; ������� ����� � �������� �����
	Delete "$DESKTOP\${GAME_NAME}.lnk"

	; ������� ���������� ��������������
	DeleteRegKey /ifempty HKLM "SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\${GAME_NAME}"

	; ������� �������������
	Delete "$INSTDIR\Uninstall.exe"

	; ������� �������
	RMDir "$INSTDIR"

	; ������� ���������� �� ������������� ���������
	DeleteRegKey /ifempty HKCU "Software\${PUBLISHER}"

SectionEnd
