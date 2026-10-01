VERSION 5.00
Object = "{831FDD16-0C5C-11D2-A9FC-0000F8754DA1}#2.1#0"; "MSCOMCTL.OCX"
Object = "{F9043C88-F6F2-101A-A3C9-08002B2F49FB}#1.2#0"; "ComDlg32.OCX"
Begin VB.MDIForm parentMDI
   BackColor       =   &H8000000C&
   Caption         =   "IDF Editor"
   ClientHeight    =   8460
   ClientLeft      =   225
   ClientTop       =   870
   ClientWidth     =   13065
   Icon            =   "MDI-Parent.frx":0000
   LinkTopic       =   "MDIForm1"
   StartUpPosition =   3  'Windows Default
   Begin VB.Timer fileUpdateTimer
      Interval        =   200
      Left            =   480
      Top             =   2280
   End
   Begin MSComctlLib.StatusBar sbStatusBar
      Align           =   2  'Align Bottom
      Height          =   360
      Left            =   0
      TabIndex        =   0
      Top             =   8100
      Width           =   13065
      _ExtentX        =   23045
      _ExtentY        =   635
      _Version        =   393216
      BeginProperty Panels {8E3867A5-8586-11D1-B16A-00C0F0283628}
         NumPanels       =   5
         BeginProperty Panel1 {8E3867AB-8586-11D1-B16A-00C0F0283628}
            AutoSize        =   2
            Object.Width           =   1773
            MinWidth        =   1764
         EndProperty
         BeginProperty Panel2 {8E3867AB-8586-11D1-B16A-00C0F0283628}
            AutoSize        =   2
            Object.Width           =   1773
            MinWidth        =   1764
         EndProperty
         BeginProperty Panel3 {8E3867AB-8586-11D1-B16A-00C0F0283628}
            AutoSize        =   2
         EndProperty
         BeginProperty Panel4 {8E3867AB-8586-11D1-B16A-00C0F0283628}
            AutoSize        =   1
            Object.Width           =   13785
         EndProperty
         BeginProperty Panel5 {8E3867AB-8586-11D1-B16A-00C0F0283628}
            AutoSize        =   2
         EndProperty
      EndProperty
      BeginProperty Font {0BE35203-8F91-11CE-9DE3-00AA004BB851}
         Name            =   "MS Sans Serif"
         Size            =   9.75
         Charset         =   0
         Weight          =   400
         Underline       =   0   'False
         Italic          =   0   'False
         Strikethrough   =   0   'False
      EndProperty
   End
   Begin MSComDlg.CommonDialog openDialog
      Left            =   3840
      Top             =   2880
      _ExtentX        =   847
      _ExtentY        =   847
      _Version        =   393216
   End
   Begin VB.Menu mnuFileTop
      Caption         =   "&File"
      Begin VB.Menu mnuFileNew
         Caption         =   "&New"
         Shortcut        =   ^N
      End
      Begin VB.Menu mnuFileOpen
         Caption         =   "&Open..."
         Shortcut        =   ^O
      End
      Begin VB.Menu mnuFileSpace3
         Caption         =   "-"
      End
      Begin VB.Menu mnuFileRecent
         Caption         =   "mru1"
         Index           =   1
      End
      Begin VB.Menu mnuFileRecent
         Caption         =   "mru2"
         Index           =   2
      End
      Begin VB.Menu mnuFileRecent
         Caption         =   "mru3"
         Index           =   3
      End
      Begin VB.Menu mnuFileRecent
         Caption         =   "mru4"
         Index           =   4
      End
      Begin VB.Menu mnuFileRecent
         Caption         =   "mru5"
         Index           =   5
      End
      Begin VB.Menu mnuFileRecent
         Caption         =   "mru6"
         Index           =   6
      End
      Begin VB.Menu mnuFileSpacer2
         Caption         =   "-"
      End
      Begin VB.Menu mnuFileExit
         Caption         =   "E&xit"
      End
   End
   Begin VB.Menu mnuHelp
      Caption         =   "&Help"
      Begin VB.Menu mnuHelpWhatsNew
         Caption         =   "&Whats New"
      End
      Begin VB.Menu mnuHelpDiv0
         Caption         =   "-"
      End
      Begin VB.Menu mnuHelpContents
         Caption         =   "&Contents"
         Enabled         =   0   'False
      End
      Begin VB.Menu mnuHelpIndex
         Caption         =   "&Index"
         Enabled         =   0   'False
      End
      Begin VB.Menu mnuHelpDocs
         Caption         =   "&Documentation"
         Enabled         =   0   'False
      End
      Begin VB.Menu mnuHelpDiv1
         Caption         =   "-"
      End
      Begin VB.Menu mnuHelpEPDocs
         Caption         =   "EnergyPlus Documentation Menu"
      End
      Begin VB.Menu mnuHelpDiv2
         Caption         =   "-"
      End
      Begin VB.Menu mnuHelpQuickStart
         Caption         =   "EnergyPlus QuickStart"
      End
      Begin VB.Menu mnuHelpEssentials
         Caption         =   "EnergyPlus Essentials"
      End
      Begin VB.Menu mnuHelpGettingStarted
         Caption         =   "EnergyPlus Getting Started"
      End
      Begin VB.Menu mnuHelpIORef
         Caption         =   "EnergyPlus I/O Reference"
      End
      Begin VB.Menu mnuHelpOutDetails
         Caption         =   "EnergyPlus Output Details and Examples"
      End
      Begin VB.Menu mnuHelpEngRef
         Caption         =   "EnergyPlus Engineering Reference"
      End
      Begin VB.Menu mnuHelpAuxProgs
         Caption         =   "EnergyPlus Auxiliary Programs"
      End
      Begin VB.Menu mnuHelpEMSguide
         Caption         =   "EnergyPlus EMS Application Guide"
      End
      Begin VB.Menu mnuHelpCompliance
         Caption         =   "Using EnergyPlus for Compliance"
      End
      Begin VB.Menu mnuHelpExtInterface
         Caption         =   "External Interface Application Guide "
      End
      Begin VB.Menu mnuHelpTips
         Caption         =   "Tips and Tricks Using EnergyPlus"
      End
      Begin VB.Menu mnuHelpAcknowledge
         Caption         =   "EnergyPlus Acknowledgments"
      End
      Begin VB.Menu mnuHelpDiv3
         Caption         =   "-"
      End
      Begin VB.Menu mnuHelpAbout
         Caption         =   "About IDF Editor"
      End
      Begin VB.Menu mnuCreateRangeTestFiles
         Caption         =   "Run Range Tests"
         Visible         =   0   'False
      End
   End
End
Attribute VB_Name = "parentMDI"
Attribute VB_GlobalNameSpace = False
Attribute VB_Creatable = False
Attribute VB_PredeclaredId = True
Attribute VB_Exposed = False
Option Explicit
Private Declare Function GetForegroundWindow Lib "user32" () As Long
Dim mainWindowHandle As Long

Dim lDocumentCount As Long

Private Sub fileUpdateTimer_Timer()
Dim curWindowHandle As Long
Static prevWindowHandle As Long
On Error Resume Next
'sbStatusBar.Panels(2).Text = Str(GetForegroundWindow())
curWindowHandle = GetForegroundWindow()
If curWindowHandle <> prevWindowHandle Then
  If curWindowHandle = mainWindowHandle Then
    ActiveForm.checkForExternalFileChange
  End If
  prevWindowHandle = curWindowHandle
End If
End Sub

'-----------------------------------------------------------------------------
' Form Load routine
'-----------------------------------------------------------------------------
Private Sub MDIForm_Load()
executableDirectory = CurDir    'store a copy of the current directory
Call setUnits
If useSpecialIDD Then
  Call ReadINI
Else
  IDDFileName = "energy+.idd"
End If
Call ReadIDD
Call findAcrobat
Call findWebBrowser
Call retrieveSettings
sbStatusBar.Panels(1).Text = IDDFileName
If useSpecialIDD Then
  sbStatusBar.Panels(2).Text = specialIDDName & " " & IDDVersion
Else
  sbStatusBar.Panels(2).Text = "EnergyPlus " & IDDVersion
End If
mainWindowHandle = Me.hwnd
Call displayRecentParentFileMenuList
End Sub

'-----------------------------------------------------------------------------
' Display the recent files in the file menu
'-----------------------------------------------------------------------------
Sub displayRecentParentFileMenuList()
Dim i As Integer
If numRecentFiles > 0 Then
  mnuFileSpacer2.Visible = True
Else
  mnuFileSpacer2.Visible = False
End If
For i = 1 To maxRecentFiles
  If recentFiles(i).nameOnly <> "" Then
    mnuFileRecent(i).Caption = recentFiles(i).nameOnly
    mnuFileRecent(i).Visible = True
  Else
    mnuFileRecent(i).Visible = False
  End If
Next i
End Sub

'-----------------------------------------------------------------------------
' Main routine for creating a new document
'-----------------------------------------------------------------------------
Public Sub doNewDocument()
Dim fd As IDFEdit
Call LoadNewDoc(fd)
End Sub

'-----------------------------------------------------------------------------
' Create a new window and blank document
'-----------------------------------------------------------------------------
Private Sub LoadNewDoc(frmD As IDFEdit)
lDocumentCount = lDocumentCount + 1
Set frmD = New IDFEdit
frmD.Caption = "Document " & lDocumentCount
Call frmD.setFileName("Document " & lDocumentCount)
frmD.initializeFormInstance
frmD.Show
End Sub


'-----------------------------------------------------------------------------
' Routine called when exiting using the [x] box on left of title bar
'-----------------------------------------------------------------------------
Private Sub MDIForm_QueryUnload(Cancel As Integer, UnloadMode As Integer)
Unload frmParent
Call exitProgram 'was commented out
End Sub

Private Sub MDIForm_Unload(Cancel As Integer)
End
End Sub

'-----------------------------------------------------------------------------
'  MENU  FILE:NEW
'-----------------------------------------------------------------------------
Private Sub mnuFileNew_Click()
Dim fd1 As IDFEdit
Call LoadNewDoc(fd1)
End Sub

'-----------------------------------------------------------------------------
'  MENU  FILE:OPEN
'-----------------------------------------------------------------------------
Private Sub mnuFileOpen_Click()
Call doOpenDocument
End Sub

'-----------------------------------------------------------------------------
'  MENU  FILE:EXIT
'-----------------------------------------------------------------------------
Private Sub mnuFileExit_Click()
Call exitProgram
End Sub

'-----------------------------------------------------------------------------
' Open the IDF file
'-----------------------------------------------------------------------------
Public Sub doOpenDocument(Optional inFileName As String)
Dim frmD As IDFEdit
Dim openedFileName As String
Dim otherWindowCaption As String
Dim i As Long
Dim isAlreadyOpen As Boolean
On Error Resume Next
If inFileName = "" Then
  openDialog.CancelError = True
  openDialog.Filter = "EnergyPlus (*.idf)|*.idf|Design Day (*.ddy)|*.ddy|Expanded File(*.expidf)|*.expidf|Report Data Dictionary(*.rdd)|*rdd|Meter Data Dictionary(*.mdd)|*.mdd"
  openDialog.FilterIndex = 1
  openDialog.DialogTitle = "Open EnergyPlus IDF File..."
  openDialog.ShowOpen
  If Err.Number <> 0 Then Exit Sub 'if not cancelled
  openedFileName = openDialog.fileName
  If openedFileName = "" Then Exit Sub
Else
  openedFileName = inFileName
End If
'check if the file is already open in another window
isAlreadyOpen = False
For i = 0 To Forms.Count - 1
  otherWindowCaption = Forms(i).Caption
  If Right(otherWindowCaption, 1) = "*" Then
    otherWindowCaption = Trim(Left(otherWindowCaption, Len(otherWindowCaption) - 1))
  End If
  If otherWindowCaption = openedFileName Then
    isAlreadyOpen = True
    Exit For
  End If
'  Debug.Print "forms [" & Forms(i).Caption & "]"
Next i
If isAlreadyOpen Then
  MsgBox "The file: " & vbCrLf & vbCrLf & openedFileName & vbCrLf & vbCrLf & "is already open in another window", vbInformation, "Warning"
  Exit Sub
End If
'Debug.Print CommonDialog1.filename
lDocumentCount = lDocumentCount + 1
Call AddRecentFileItem(openedFileName)
Set frmD = New IDFEdit
Call frmD.initializeFormInstance
Call frmD.setFileName(openedFileName)
Call frmD.ReadIDF
If Not frmD.isLastError Then
  Call displayRecentParentFileMenuList
  Call frmD.getIDFVersion
  Call frmD.FillList
  frmD.Show
  frmD.Caption = openedFileName 'if successful then show file name on title bar
  frmD.lstObjectTypes.ListIndex = 1
  frmD.mnuFileSave.Enabled = False
  'if from the data set directory open it up using
  'the "Show classes with objects only" view
  If pathOnly(openedFileName) = dataSetPath Then
    frmD.mnuViewClassesWithObjs_Click
  End If
Else
  Unload frmD
End If
End Sub

'-----------------------------------------------------------------------------
' When a file has not been double clicked then just load a blank document
'-----------------------------------------------------------------------------
Public Sub loadBlankDocument()
Dim fd1 As IDFEdit
Call LoadNewDoc(fd1)
End Sub

'-----------------------------------------------------------------------------
' Open an IDF file specified on the command line (double clicked)
'-----------------------------------------------------------------------------
Public Sub doDoubleClickDocument(lineCommand As String)
Dim frmD As IDFEdit
Dim openedFileName As String
Dim otherWindowCaption As String
Dim i As Long
Dim isAlreadyOpen As Boolean
openedFileName = lineCommand
'get rid of front and trailing double quotes
If Left(openedFileName, 1) = Chr(34) Then openedFileName = Mid(openedFileName, 2)
If Right(openedFileName, 1) = Chr(34) Then openedFileName = Left(openedFileName, Len(openedFileName) - 1)
If openedFileName = "" Then Exit Sub
'check if the file is already open in another window
isAlreadyOpen = False
For i = 0 To Forms.Count - 1
  otherWindowCaption = Forms(i).Caption
  If Right(otherWindowCaption, 1) = "*" Then
    otherWindowCaption = Trim(Left(otherWindowCaption, Len(otherWindowCaption) - 1))
  End If
  If otherWindowCaption = openedFileName Then
    isAlreadyOpen = True
    Exit For
  End If
'  Debug.Print "forms [" & Forms(i).Caption & "]"
Next i
If isAlreadyOpen Then
  MsgBox "The file: " & vbCrLf & vbCrLf & openedFileName & vbCrLf & vbCrLf & "is already open in another window", vbInformation, "Warning"
  Exit Sub
End If
'Debug.Print CommonDialog1.filename
lDocumentCount = lDocumentCount + 1
Set frmD = New IDFEdit
Call frmD.initializeFormInstance
Call frmD.setFileName(openedFileName)
Call frmD.ReadIDF
If Not frmD.isLastError Then
  Call AddRecentFileItem(openedFileName)
  Call displayRecentParentFileMenuList
  Call frmD.getIDFVersion
  Call frmD.FillList
  frmD.Show
  frmD.Caption = openedFileName 'if successful then show file name on title bar
  frmD.lstObjectTypes.ListIndex = 1
  frmD.mnuFileSave.Enabled = False
Else
  Unload frmD
End If
End Sub

'-----------------------------------------------------------------------------
' Get the location from the registry
'-----------------------------------------------------------------------------
Private Sub retrieveSettings()
Dim fileNameFromRegistry As String
Dim i As Integer
Me.Left = GetSetting("IDF Editor", "Location", "MainLeft", 90)
Me.Top = GetSetting("IDF Editor", "Location", "MainTop", 90)
Me.Width = GetSetting("IDF Editor", "Location", "MainWidth", 11910)
Me.Height = GetSetting("IDF Editor", "Location", "MainHeight", 8805)
formLayoutOption = GetSetting("IDF Editor", "Location", "Layout", 1)
saveOrderOptDefault = GetSetting("IDF Editor", "Option", "SaveOrderDef", saveOrderSorted)
specialFormatOptDefault = GetSetting("IDF Editor", "Option", "SpecialFormatDef", specFormNo)
checkRangeOnSave = GetSetting("IDF Editor", "Option", "CheckRangeOnSave", checkRangeNo)
useWordWrap = GetSetting("IDF Editor", "Option", "WordWrap", False)
previousVersion = GetSetting("IDF Editor", "Option", "Version", "")
ignoreValidatingMacros = GetSetting("IDF EDitor", "Option", "IgnoreValidatingMacros", False)
'get recently used files
For i = maxRecentFiles To 1 Step -1
  fileNameFromRegistry = GetSetting("IDF Editor", "RecentFiles", Trim$(CStr(i)), "")
  If fileNameFromRegistry <> "" Then Call AddRecentFileItem(fileNameFromRegistry)
Next i
'--------------------------------------------------------------------------
'CEPTChange - To get setting for showing or not showing Quick Select Combos
'--------------------------------------------------------------------------
ShowQuickSelectCombos = GetSetting("IDF Editor", "Location", "QuickSelect", False)
End Sub

'-----------------------------------------------------------------------------
' Save location to the registry
'-----------------------------------------------------------------------------
Private Sub saveAllSettings()
Dim i As Integer
If Me.WindowState <> vbMinimized Then
  SaveSetting "IDF Editor", "Location", "MainLeft", Me.Left
  SaveSetting "IDF Editor", "Location", "MainTop", Me.Top
  SaveSetting "IDF Editor", "Location", "MainWidth", Me.Width
  SaveSetting "IDF Editor", "Location", "MainHeight", Me.Height
End If
SaveSetting "IDF Editor", "Location", "Layout", formLayoutOption
SaveSetting "IDF Editor", "Option", "SaveOrderDef", saveOrderOptDefault
SaveSetting "IDF Editor", "Option", "SpecialFormatDef", specialFormatOptDefault
SaveSetting "IDF Editor", "Option", "CheckRangeOnSave", checkRangeOnSave
SaveSetting "IDF Editor", "Option", "WordWrap", useWordWrap
SaveSetting "IDF Editor", "Option", "Version", ver
SaveSetting "IDF Editor", "Option", "IgnoreValidatingMacros", ignoreValidatingMacros
'save recently used file list
If numRecentFiles > 0 Then
  For i = 1 To numRecentFiles
    SaveSetting "IDF Editor", "RecentFiles", Trim$(CStr(i)), recentFiles(i).nameWithPath
  Next i
End If
'--------------------------------------------------------------------------
'CEPTChange - To save setting for showing or not showing Quick Select Combos
'--------------------------------------------------------------------------
SaveSetting "IDF Editor", "Location", "QuickSelect", ShowQuickSelectCombos
End Sub


'-----------------------------------------------------------------------------
' Add a file to the recently used file list
' Since the list is in the order that it is displayed, a new file
' gets put in the first position and the other files get moved
' down the list.
'-----------------------------------------------------------------------------
Public Sub AddRecentFileItem(newFileWithPath As String)
Dim i As Integer
Dim found As Boolean
found = False
For i = 1 To numRecentFiles
  If LCase(recentFiles(i).nameWithPath) = LCase(newFileWithPath) Then
    found = True
  End If
Next i
If Not found Then
  If numRecentFiles > 0 Then
    'move the existing items further down the list
    For i = numRecentFiles To 1 Step -1
      If i < maxRecentFiles Then
        recentFiles(i + 1).nameWithPath = recentFiles(i).nameWithPath
        recentFiles(i + 1).nameOnly = recentFiles(i).nameOnly
      End If
    Next i
  End If
  'add the new item to the top
  recentFiles(1).nameWithPath = newFileWithPath
  recentFiles(1).nameOnly = fileWithExt(newFileWithPath)
  'increment the count
  If numRecentFiles < maxRecentFiles Then
    numRecentFiles = numRecentFiles + 1
  End If
End If
Debug.Print "--- recent files"
For i = 1 To numRecentFiles
  Debug.Print i, recentFiles(i).nameOnly
Next i
End Sub

'-----------------------------------------------------------------------------
' clean up stuff and exit the program
'-----------------------------------------------------------------------------
Public Sub exitProgram()
Call saveAllSettings
Unload frmParent
'End
End Sub


Private Sub mnuFileRecent_Click(Index As Integer)
Call doOpenDocument(recentFiles(Index).nameWithPath)
End Sub

Private Sub mnuHelpAbout_Click()
About.Show
End Sub

' References to EnergyPlus Documentation
Private Sub mnuHelpAcknowledge_Click()
' Call startAcrobat("Acknowledgments.pdf")
Call viewWebPage("https://energyplus.readthedocs.io/en/latest/acknowledgments/acknowledgments.html")
End Sub
Private Sub mnuHelpAuxProgs_Click()
'Call startAcrobat("AuxiliaryPrograms.pdf")
Call viewWebPage("https://energyplus.readthedocs.io/en/latest/auxiliary-programs/auxiliary-programs.html")
End Sub
Private Sub mnuHelpEngRef_Click()
Call startAcrobat("EngineeringReference.pdf")
End Sub
Private Sub mnuHelpEPDocs_Click()
'Call startAcrobat("EPlusMainMenu.pdf")
Call viewWebPage(documentationPath + "index.html")
End Sub


Private Sub mnuHelpGettingStarted_Click()
Call startAcrobat("GettingStarted.pdf")
End Sub
Private Sub mnuHelpIORef_Click()
Call startAcrobat("InputOutputReference.pdf")
End Sub
Private Sub mnuHelpOutDetails_Click()
Call startAcrobat("OutputDetailsAndExamples.pdf")
End Sub
Private Sub mnuHelpCompliance_Click()
Call startAcrobat("Using_EnergyPlus_for_Compliance.pdf")
End Sub
Private Sub mnuHelpEMSguide_Click()
'Call startAcrobat("EMS_Application_Guide.pdf")
Call viewWebPage("https://energyplus.readthedocs.io/en/latest/ems-application-guide/ems-application-guide.html")
End Sub
Private Sub mnuHelpExtInterface_Click()
Call startAcrobat("ExternalInterfaces_Application_Guide.pdf")
End Sub
Private Sub mnuHelpQuickStart_Click()
Call viewWebPage("https://energyplus.readthedocs.io/en/latest/quick_start/quick_start.html")
End Sub
Private Sub mnuHelpEssentials_Click()
Call viewWebPage("https://energyplus.readthedocs.io/en/latest/essentials/essentials.html")
End Sub

Private Sub mnuHelpTips_Click()
'Call startAcrobat("Tips_and_Tricks_Using_EnergyPlus.pdf")
Call viewWebPage("https://energyplus.readthedocs.io/en/latest/tips_and_tricks/tips_and_tricks.html")
End Sub
Private Sub mnuHelpWhatsNew_Click()
frmWhatsNew.Show
End Sub

'     NOTICE
'
'     The contents of this file are subject to the EnergyPlus Open Source License
'     Version 1.0 (the "License"); you may not use this file except in compliance
'     with the License. You may obtain a copy of the License at
'
'     http://apps1.eere.energy.gov/buildings/energyplus/energyplus_licensing.cfm
'
'     Software distributed under the License is distributed on an "AS IS" basis,
'     WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License for
'     the specific language governing rights and limitations under the License.
'
'     Copyright © 1996-2014 GARD Analytics.  All rights reserved.
'
'     NOTICE: The U.S. Government is granted for itself and others acting on its
'     behalf a paid-up, nonexclusive, irrevocable, worldwide license in this data to
'     reproduce, prepare derivative works, and perform publicly and display publicly.
'     Beginning five (5) years after permission to assert copyright is granted,
'     subject to two possible five year renewals, the U.S. Government is granted for
'     itself and others acting on its behalf a paid-up, non-exclusive, irrevocable
'     worldwide license in this data to reproduce, prepare derivative works,
'     distribute copies to the public, perform publicly and display publicly, and to
'     permit others to do so.
'
'     TRADEMARKS: EnergyPlus is a trademark of the US Department of Energy.
'
