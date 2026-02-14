VERSION 5.00
Begin VB.Form frmOptions
   BorderStyle     =   3  'Fixed Dialog
   Caption         =   "Options"
   ClientHeight    =   17520
   ClientLeft      =   45
   ClientTop       =   435
   ClientWidth     =   6375
   ControlBox      =   0   'False
   LinkTopic       =   "Form1"
   MaxButton       =   0   'False
   MinButton       =   0   'False
   ScaleHeight     =   17520
   ScaleWidth      =   6375
   ShowInTaskbar   =   0   'False
   StartUpPosition =   1  'CenterOwner
   Begin VB.Frame frmAssoc
      Caption         =   "File Association"
      Height          =   1335
      Left            =   1800
      TabIndex        =   27
      Top             =   14280
      Width           =   4215
      Begin VB.CommandButton cmdAssoc
         Caption         =   "Associate IDF, IMF and EPG files with EP-Launch..."
         Height          =   735
         Left            =   960
         TabIndex        =   28
         Top             =   360
         Width           =   2415
      End
   End
   Begin VB.Frame frmReset
      Caption         =   "Reset"
      Height          =   1455
      Left            =   1800
      TabIndex        =   21
      Top             =   15720
      Width           =   4215
      Begin VB.CommandButton cmdAutoFindAll
         Caption         =   "Auto Find All File Viewers"
         Height          =   375
         Left            =   720
         TabIndex        =   23
         Top             =   360
         Width           =   2895
      End
      Begin VB.CommandButton cmdResetAllExit
         Caption         =   "Reset All Options and Exit"
         Height          =   375
         Left            =   720
         TabIndex        =   22
         Top             =   840
         Width           =   2895
      End
   End
   Begin VB.Frame frmProg
      Caption         =   "Text Editor"
      Height          =   2655
      Left            =   1800
      TabIndex        =   17
      Top             =   11520
      Width           =   4215
      Begin VB.CommandButton cmdClearApp
         Caption         =   "Clear"
         Height          =   375
         Left            =   3000
         TabIndex        =   26
         Top             =   1440
         Width           =   1095
      End
      Begin VB.CommandButton cmdUseOpera
         Caption         =   "Use Opera"
         Height          =   375
         Left            =   2280
         TabIndex        =   25
         Top             =   2040
         Width           =   1335
      End
      Begin VB.CommandButton cmdUseFirefox
         Caption         =   "Use Firefox"
         Height          =   375
         Left            =   720
         TabIndex        =   24
         Top             =   2040
         Width           =   1335
      End
      Begin VB.CommandButton cmdAutoFindApp
         Caption         =   "Auto Find"
         Height          =   375
         Left            =   120
         TabIndex        =   20
         Top             =   1440
         Width           =   1095
      End
      Begin VB.CommandButton cmdSelectApp
         Caption         =   "Select..."
         Height          =   375
         Left            =   1560
         TabIndex        =   19
         Top             =   1440
         Width           =   1095
      End
      Begin VB.TextBox txtProgramPath
         BackColor       =   &H80000004&
         Height          =   1095
         Left            =   120
         Locked          =   -1  'True
         MultiLine       =   -1  'True
         ScrollBars      =   2  'Vertical
         TabIndex        =   18
         Text            =   "frmOptions.frx":0000
         Top             =   240
         Width           =   3975
      End
   End
   Begin VB.ListBox lstCategory
      Height          =   2985
      Left            =   120
      TabIndex        =   16
      Top             =   120
      Width           =   1575
   End
   Begin VB.Frame frmMisc
      Caption         =   "Miscellaneous"
      Height          =   3255
      Left            =   1800
      TabIndex        =   8
      Top             =   8160
      Width           =   4215
      Begin VB.CheckBox chkAutoUpdates
         Caption         =   "Check for Updates to EnergyPlus"
         Height          =   375
         Left            =   240
         TabIndex        =   30
         Top             =   2760
         Width           =   3135
      End
      Begin VB.CheckBox chkRunParamPre
         Caption         =   "Run ParametricPreprocessor"
         Height          =   375
         Left            =   240
         TabIndex        =   29
         Top             =   2400
         Width           =   3735
      End
      Begin VB.CheckBox chkCreateBatch
         Caption         =   "Create Batch File to Run EnergyPlus"
         Height          =   375
         Left            =   240
         TabIndex        =   15
         Top             =   2040
         Width           =   3135
      End
      Begin VB.CheckBox chkCSVProc
         Caption         =   "Create Statistics File"
         Height          =   375
         Left            =   240
         TabIndex        =   14
         Top             =   1680
         Width           =   3735
      End
      Begin VB.CheckBox chkConvIP
         Caption         =   "Convert ESO/MTR to IP Units"
         Height          =   375
         Left            =   240
         TabIndex        =   12
         Top             =   1320
         Width           =   3855
      End
      Begin VB.CheckBox chkVersion
         Caption         =   "Check VERSION Prior to Simulation"
         Height          =   375
         Left            =   240
         TabIndex        =   11
         Top             =   960
         Width           =   3855
      End
      Begin VB.CheckBox chkMore250
         Caption         =   "Allow More Than 250 Columns"
         Height          =   375
         Left            =   240
         TabIndex        =   10
         Top             =   600
         Width           =   3855
      End
      Begin VB.CheckBox chkTabOpen
         Caption         =   "Tab Delimited Open with Spreadsheet"
         Height          =   375
         Left            =   240
         TabIndex        =   9
         Top             =   240
         Width           =   3855
      End
   End
   Begin VB.Frame frmInterface
      Caption         =   "Interface Control"
      Height          =   1455
      Left            =   1800
      TabIndex        =   5
      Top             =   6600
      Width           =   4215
      Begin VB.CheckBox chkAltLayout
         Caption         =   "Alternative Layout"
         Height          =   375
         Left            =   240
         TabIndex        =   7
         Top             =   600
         Width           =   3855
      End
      Begin VB.CheckBox chkWide
         Caption         =   "Extra Wide Window"
         Height          =   375
         Left            =   240
         TabIndex        =   6
         Top             =   240
         Width           =   3855
      End
   End
   Begin VB.Frame frmCommand
      Caption         =   "Command Window Control"
      Height          =   2295
      Left            =   1800
      TabIndex        =   2
      Top             =   3840
      Width           =   4215
      Begin VB.ComboBox cmbNumberProcesses
         Height          =   315
         Left            =   2880
         Style           =   2  'Dropdown List
         TabIndex        =   32
         Top             =   1320
         Width           =   735
      End
      Begin VB.CheckBox chkMinSingleCmd
         Caption         =   "Minimize Single Simulation Command Window"
         Height          =   375
         Left            =   240
         TabIndex        =   13
         Top             =   600
         Width           =   3735
      End
      Begin VB.CheckBox chkMinGroupCmd
         Caption         =   "Minimize Group Simulation Command Window"
         Height          =   375
         Left            =   240
         TabIndex        =   4
         Top             =   960
         Width           =   3855
      End
      Begin VB.CheckBox chkPauseSim
         Caption         =   "Pause During Simulation (Unless Minimized)"
         Height          =   375
         Left            =   240
         TabIndex        =   3
         Top             =   240
         Width           =   3855
      End
      Begin VB.Label Label2
         Caption         =   "Usually 1 to 4 for most PCs."
         Height          =   255
         Left            =   630
         TabIndex        =   33
         Top             =   1575
         Width           =   3255
      End
      Begin VB.Label Label1
         Caption         =   "Number of Simultaneous Processes"
         Height          =   255
         Left            =   240
         TabIndex        =   31
         Top             =   1380
         Width           =   2535
      End
   End
   Begin VB.CommandButton cmdCancel
      Cancel          =   -1  'True
      Caption         =   "Cancel"
      Height          =   375
      Left            =   4560
      TabIndex        =   1
      Top             =   3480
      Width           =   1575
   End
   Begin VB.CommandButton cmdOK
      Caption         =   "OK"
      Default         =   -1  'True
      Height          =   375
      Left            =   2880
      TabIndex        =   0
      Top             =   3480
      Width           =   1575
   End
End
Attribute VB_Name = "frmOptions"
Attribute VB_GlobalNameSpace = False
Attribute VB_Creatable = False
Attribute VB_PredeclaredId = True
Attribute VB_Exposed = False
'make sure that the order of this list matches the order of the items
'added to the lstCategory list.
Const lstCatCommand = 0
Const lstCatInterface = 1
Const lstCatMisc = 2
Const lstCatText = 3
Const lstCatDrawing = 4
Const lstCatVRML = 5
Const lstCatSpread = 6
Const lstCatDiagram = 7
Const lstCatHTML = 8
Const lstCatESO = 9
Const lstCatPDF = 10
Const lstCatXML = 11
Const lstCatAssoc = 12
Const lstCatReset = 13

'local copies of paths that are used if the OK button is pressed
Dim pathForTextEditor As String
Dim pathForDrawView As String
Dim pathForVRMLview As String
Dim pathForSpreadsheet As String
Dim pathForDiagram As String
Dim pathForHTMLbrowser As String
Dim pathForESOview As String
Dim pathForPDFview As String
Dim pathForXMLview As String






Private Sub cmdAssoc_Click()
Call eplUI.CreateAssociation
End Sub

Private Sub Form_Activate()
Dim found As Integer
Dim i As Integer
'
'========== COMMAND WINDOW CONTROL
'
'Pause During Simulation
If eplUI.pauseDuringRun Then
  chkPauseSim.Value = vbChecked
Else
  chkPauseSim.Value = vbUnchecked
End If
'Minimize Single Simulation Command Window
If eplUI.minimizeSingleCmd Then
  chkMinSingleCmd.Value = vbChecked
Else
  chkMinSingleCmd.Value = vbUnchecked
End If
'Minimize Group Simulation Command Window
If eplUI.minimizeGroupCmd Then
  chkMinGroupCmd.Value = vbChecked
Else
  chkMinGroupCmd.Value = vbUnchecked
End If
found = -1
For i = 0 To cmbNumberProcesses.ListCount - 1
  If Val(cmbNumberProcesses.List(i)) = eplUI.numberOfSimProcessesAllowed Then
    found = i
    Exit For
  End If
Next i
If found > -1 Then
  cmbNumberProcesses.ListIndex = found
Else
  cmbNumberProcesses.ListIndex = 0
End If
'
'========== INTERFACE CONTROL
'
'Extra Wide Window
If eplUI.useWideView Then
  chkWide.Value = vbChecked
Else
  chkWide.Value = vbUnchecked
End If
'Alternative Layout
If eplUI.useSimAboveView Then
  chkAltLayout.Value = vbChecked
Else
  chkAltLayout.Value = vbUnchecked
End If
'
'========== MISCELLANEOUS
'
'Tab Delimited Open with Spreadsheet
If eplUI.tabWithSpreadsheet Then
  chkTabOpen.Value = vbChecked
Else
  chkTabOpen.Value = vbUnchecked
End If
'Allow More Than 250 Columns
If eplUI.allowGT250Col Then
  chkMore250.Value = vbChecked
Else
  chkMore250.Value = vbUnchecked
End If
'Check VERSION Prior to Simulation
If eplUI.testViewConvertOld Then
  chkVersion.Value = vbChecked
Else
  chkVersion.Value = vbUnchecked
End If
'Convert ESO/MTR to IP Units
If eplUI.convertESOMTRIP Then
  chkConvIP.Value = vbChecked
Else
  chkConvIP.Value = vbUnchecked
End If
'Create Statistics File
If eplUI.createCSVprocFile Then
  chkCSVProc.Value = vbChecked
Else
  chkCSVProc.Value = vbUnchecked
End If
'Create Batch File (normally yes but workaround
'if security doesn't allow batch files to be created
'by software)
If eplUI.CreateRunEPBatch Then
  chkCreateBatch.Value = vbChecked
Else
  chkCreateBatch.Value = vbUnchecked
End If
'parametric preprocessor
If eplUI.enableParametricPreprocessor Then
  chkRunParamPre.Value = vbChecked
Else
  chkRunParamPre.Value = vbUnchecked
End If
'energyplus internet update checks
If eplUI.updateAutoCheck Then
  chkAutoUpdates.Value = vbChecked
Else
  chkAutoUpdates.Value = vbUnchecked
End If
'
'========== File paths
'
pathForTextEditor = eplUI.textEditFileName
pathForDrawView = eplUI.dxfViewFileName
pathForVRMLview = eplUI.vrmlAppFileName
pathForSpreadsheet = eplUI.spreadsheetFileName
pathForDiagram = eplUI.svgViewFileName
pathForHTMLbrowser = eplUI.htmlViewFileName
pathForESOview = eplUI.esoViewFileName
pathForPDFview = eplUI.pdfViewerFileName
pathForXMLview = eplUI.xmlViewerFileName
'select first item on list
lstCategory.ListIndex = 0
End Sub

Private Sub cmdOK_Click()
'
'========== COMMAND WINDOW CONTROL
'
'Pause During Simulation
If chkPauseSim.Value = vbChecked Then
  eplUI.pauseDuringRun = True
Else
  eplUI.pauseDuringRun = False
End If
'Minimize Single Simulation Command Window
If chkMinSingleCmd.Value = vbChecked Then
  eplUI.minimizeSingleCmd = True
Else
  eplUI.minimizeSingleCmd = False
End If
'Minimize Group Simulation Command Window
If chkMinGroupCmd.Value = vbChecked Then
  eplUI.minimizeGroupCmd = True
Else
  eplUI.minimizeGroupCmd = False
End If
'Number of simultaneous threads for simulation processes
eplUI.numberOfSimProcessesAllowed = Val(cmbNumberProcesses.List(cmbNumberProcesses.ListIndex))
'
'========== INTERFACE CONTROL
'
'Extra Wide Window
If chkWide.Value = vbChecked Then
  eplUI.useWideView = True
Else
  eplUI.useWideView = False
End If
'Alternative Layout
If chkAltLayout.Value = vbChecked Then
  eplUI.useSimAboveView = True
Else
  eplUI.useSimAboveView = False
End If
'
'========== MISCELLANEOUS
'
'Tab Delimited Open with Spreadsheet
If chkTabOpen.Value = vbChecked Then
  eplUI.tabWithSpreadsheet = True
Else
  eplUI.tabWithSpreadsheet = False
End If
'Allow More Than 250 Columns
If chkMore250.Value = vbChecked Then
  eplUI.allowGT250Col = True
Else
  eplUI.allowGT250Col = False
End If
'Check VERSION Prior to Simulation
If chkVersion.Value = vbChecked Then
  eplUI.testViewConvertOld = True
Else
  eplUI.testViewConvertOld = False
End If
'Convert ESO/MTR to IP Units
If chkConvIP.Value = vbChecked Then
  eplUI.convertESOMTRIP = True
Else
  eplUI.convertESOMTRIP = False
End If
'Create statistics file
If chkCSVProc.Value = vbChecked Then
  eplUI.createCSVprocFile = True
Else
  eplUI.createCSVprocFile = False
End If
'Don't Create Batch File (use parameters instead)
If chkCreateBatch.Value = vbChecked Then
  eplUI.CreateRunEPBatch = True
Else
  eplUI.CreateRunEPBatch = False
End If
If chkRunParamPre.Value = vbChecked Then
  eplUI.enableParametricPreprocessor = True
Else
  eplUI.enableParametricPreprocessor = False
End If
'========== File paths
eplUI.textEditFileName = pathForTextEditor
eplUI.dxfViewFileName = pathForDrawView
eplUI.vrmlAppFileName = pathForVRMLview
eplUI.spreadsheetFileName = pathForSpreadsheet
eplUI.svgViewFileName = pathForDiagram
eplUI.htmlViewFileName = pathForHTMLbrowser
eplUI.esoViewFileName = pathForESOview
eplUI.pdfViewerFileName = pathForPDFview
eplUI.xmlViewerFileName = pathForXMLview
'energyplus internet update checks
If chkAutoUpdates.Value = vbChecked Then
  eplUI.updateAutoCheck = True
Else
  eplUI.updateAutoCheck = False
End If
'FINALLY HIDE THE OPTION WINDOW
Me.Hide
End Sub

Private Sub Form_Load()
'create categories for the options window
frmOptions.Height = 4400
'make sure that this list is in same order as constants at top of this file
lstCategory.AddItem "Command Window"
lstCategory.AddItem "Interface"
lstCategory.AddItem "Miscellaneous"
lstCategory.AddItem "Text Editor"
lstCategory.AddItem "Drawing Viewer"
lstCategory.AddItem "VRML Viewer"
lstCategory.AddItem "Spreadsheet"
lstCategory.AddItem "Diagramming"
lstCategory.AddItem "HTML Browser"
lstCategory.AddItem "ESO Viewer"
lstCategory.AddItem "PDF Viewer"
lstCategory.AddItem "XML Viewer"
lstCategory.AddItem "File Association"
lstCategory.AddItem "Reset"
lstCategory.ListIndex = 0
For i = 1 To 8
  cmbNumberProcesses.AddItem Str(i)
Next i
cmbNumberProcesses.AddItem "10"
cmbNumberProcesses.AddItem "12"
cmbNumberProcesses.AddItem "14"
cmbNumberProcesses.AddItem "16"
'
cmbNumberProcesses.AddItem "20"
cmbNumberProcesses.AddItem "24"
cmbNumberProcesses.AddItem "28"
cmbNumberProcesses.AddItem "32"
'
cmbNumberProcesses.AddItem "40"
cmbNumberProcesses.AddItem "48"
cmbNumberProcesses.AddItem "56"
cmbNumberProcesses.AddItem "64"
'
cmbNumberProcesses.AddItem "80"
cmbNumberProcesses.AddItem "96"
cmbNumberProcesses.AddItem "112"
cmbNumberProcesses.AddItem "128"
'
cmbNumberProcesses.AddItem "160"
cmbNumberProcesses.AddItem "192"
cmbNumberProcesses.AddItem "224"
cmbNumberProcesses.AddItem "256"
'
cmbNumberProcesses.AddItem "320"
cmbNumberProcesses.AddItem "384"
cmbNumberProcesses.AddItem "448"
cmbNumberProcesses.AddItem "512"
'
cmbNumberProcesses.AddItem "320"
cmbNumberProcesses.AddItem "384"
cmbNumberProcesses.AddItem "448"
cmbNumberProcesses.AddItem "512"
'
cmbNumberProcesses.AddItem "640"
cmbNumberProcesses.AddItem "768"
cmbNumberProcesses.AddItem "896"
cmbNumberProcesses.AddItem "1024"
End Sub

Private Sub lstCategory_Click()
frmCommand.Top = 4000
frmInterface.Top = 4000
frmMisc.Top = 4000
frmProg.Top = 4000
frmReset.Top = 4000
frmAssoc.Top = 4000
frmProg.Height = 1935
Select Case lstCategory.ListIndex
  Case lstCatCommand 'command window
    frmCommand.Top = 120
  Case lstCatInterface 'interface
    frmInterface.Top = 120
  Case lstCatMisc 'Miscellaneous
    frmMisc.Top = 120
  Case lstCatText 'Text Editor
    frmProg.Top = 120
    frmProg.Caption = "Text Editor"
    txtProgramPath.Text = pathForTextEditor
  Case lstCatDrawing 'Drawing Viewer
    frmProg.Top = 120
    frmProg.Caption = "Drawing Viewer for DXF files"
    txtProgramPath.Text = pathForDrawView
  Case lstCatVRML 'VRML Viewer
    frmProg.Top = 120
    frmProg.Caption = "VRML Viewer for WRL files"
    txtProgramPath.Text = pathForVRMLview
  Case lstCatSpread 'Spreadsheet
    frmProg.Top = 120
    frmProg.Caption = "Spreadsheet for CSV and TAB files"
    txtProgramPath.Text = pathForSpreadsheet
  Case lstCatDiagram 'Diagramming
    frmProg.Top = 120
    frmProg.Caption = "Diagramming Program for SVG files"
    txtProgramPath.Text = pathForDiagram
    frmProg.Height = 2655
    If eplUI.checkIfFileExists("C:\Program Files\Mozilla Firefox\firefox.exe") Then
      cmdUseFirefox.Enabled = True
    Else
      cmdUseFirefox.Enabled = False
    End If
    If eplUI.checkIfFileExists("C:\Program Files\Opera\Opera.exe") Then
      cmdUseOpera.Enabled = True
    Else
      cmdUseOpera.Enabled = False
    End If
  Case lstCatHTML 'HTML Program
    frmProg.Top = 120
    frmProg.Caption = "HTML Browser"
    txtProgramPath.Text = pathForHTMLbrowser
  Case lstCatESO 'ESO Viewer
    frmProg.Top = 120
    frmProg.Caption = "ESO Viewer"
    txtProgramPath.Text = pathForESOview
  Case lstCatPDF 'PDF Viewer
    frmProg.Top = 120
    frmProg.Caption = "PDF Viewer"
    txtProgramPath.Text = pathForPDFview
  Case lstCatXML 'XML Viewer
    frmProg.Top = 120
    frmProg.Caption = "XML Viewer"
    txtProgramPath.Text = pathForXMLview
  Case lstCatAssoc 'Reset
    frmAssoc.Top = 120
  Case lstCatReset 'Reset
    frmReset.Top = 120
End Select
End Sub


Private Sub cmdAutoFindApp_Click()
Select Case lstCategory.ListIndex
  Case lstCatText 'Text Editor
    pathForTextEditor = eplUI.findProgramUsingExtension("txt")
    txtProgramPath.Text = pathForTextEditor
  Case lstCatDrawing 'Drawing Viewer
    pathForDrawView = eplUI.findProgramUsingExtension("dxf")
    txtProgramPath.Text = pathForDrawView
  Case lstCatVRML 'VRML Viewer
    pathForVRMLview = eplUI.findProgramUsingExtension("wrl")
    txtProgramPath.Text = pathForVRMLview
  Case lstCatSpread 'Spreadsheet
    pathForSpreadsheet = eplUI.findProgramUsingExtension("xls")
    txtProgramPath.Text = pathForSpreadsheet
  Case lstCatDiagram 'Diagramming
    pathForDiagram = eplUI.findProgramUsingExtension("svg")
    txtProgramPath.Text = pathForDiagram
  Case lstCatHTML 'HTML Program
    pathForHTMLbrowser = eplUI.findProgramUsingExtension("html")
    txtProgramPath.Text = pathForHTMLbrowser
  Case lstCatESO 'ESO Viewer
    pathForESOview = eplUI.findProgramUsingExtension("eso")
    txtProgramPath.Text = pathForESOview
  Case lstCatPDF 'PDF Viewer
    pathForPDFview = eplUI.findProgramUsingExtension("pdf")
    txtProgramPath.Text = pathForPDFview
  Case lstCatXML 'XML Viewer
    pathForXMLview = eplUI.findProgramUsingExtension("xml")
    txtProgramPath.Text = pathForXMLview
End Select
End Sub

Private Sub cmdSelectApp_Click()
Select Case lstCategory.ListIndex
  Case lstCatText 'Text Editor
    pathForTextEditor = eplUI.SelectApplication("Find Text Editor Program", pathForTextEditor)
    txtProgramPath.Text = pathForTextEditor
  Case lstCatDrawing 'Drawing Viewer
    pathForDrawView = eplUI.SelectApplication("Find DXF Drawing Viewer Program", pathForDrawView)
    txtProgramPath.Text = pathForDrawView
  Case lstCatVRML 'VRML Viewer
    pathForVRMLview = eplUI.SelectApplication("Find VRML Drawing Viewer Program", pathForVRMLview)
    txtProgramPath.Text = pathForVRMLview
  Case lstCatSpread 'Spreadsheet
    pathForSpreadsheet = eplUI.SelectApplication("Find Spreadsheet Program", pathForSpreadsheet)
    txtProgramPath.Text = pathForSpreadsheet
  Case lstCatDiagram 'Diagramming
    pathForDiagram = eplUI.SelectApplication("Find SVG Diagraming Program", pathForDiagram)
    txtProgramPath.Text = pathForDiagram
  Case lstCatHTML 'HTML Program
    pathForHTMLbrowser = eplUI.SelectApplication("Find HTML Browser Program", pathForHTMLbrowser)
    txtProgramPath.Text = pathForHTMLbrowser
  Case lstCatESO 'ESO Viewer
    pathForESOview = eplUI.SelectApplication("Find ESO File Viewer Program", pathForESOview)
    txtProgramPath.Text = pathForESOview
  Case lstCatPDF 'PDF Viewer
    pathForPDFview = eplUI.SelectApplication("Find PDF Viewer Program", pathForPDFview)
    txtProgramPath.Text = pathForPDFview
  Case lstCatXML 'XML Viewer
    pathForXMLview = eplUI.SelectApplication("Find XML Viewer Program", pathForXMLview)
    txtProgramPath.Text = pathForXMLview
End Select
End Sub

Private Sub cmdClearApp_Click()
Select Case lstCategory.ListIndex
  Case lstCatText 'Text Editor
    pathForTextEditor = ""
    txtProgramPath.Text = pathForTextEditor
  Case lstCatDrawing 'Drawing Viewer
    pathForDrawView = ""
    txtProgramPath.Text = pathForDrawView
  Case lstCatVRML 'VRML Viewer
    pathForVRMLview = ""
    txtProgramPath.Text = pathForVRMLview
  Case lstCatSpread 'Spreadsheet
    pathForSpreadsheet = ""
    txtProgramPath.Text = pathForSpreadsheet
  Case lstCatDiagram 'Diagramming
    pathForDiagram = ""
    txtProgramPath.Text = pathForDiagram
  Case lstCatHTML 'HTML Program
    pathForHTMLbrowser = ""
    txtProgramPath.Text = pathForHTMLbrowser
  Case lstCatESO 'ESO Viewer
    pathForESOview = ""
    txtProgramPath.Text = pathForESOview
  Case lstCatPDF 'PDF Viewer
    pathForPDFview = ""
    txtProgramPath.Text = pathForPDFview
  Case lstCatXML 'XML Viewer
    pathForXMLview = ""
    txtProgramPath.Text = pathForXMLview
End Select
End Sub


Private Sub cmdUseFirefox_Click()
pathForDiagram = "C:\Program Files\Mozilla Firefox\firefox.exe"
txtProgramPath.Text = pathForDiagram
End Sub

Private Sub cmdUseOpera_Click()
pathForDiagram = "C:\Program Files\Opera\Opera.exe"
txtProgramPath.Text = pathForDiagram
End Sub


'=========== from RESET options
Private Sub cmdAutoFindAll_Click()
If MsgBox("Are you certain that you want to automatically find the programs that view the different file formats? This will overwrite any programs that you have manually selected.", vbYesNo) = vbYes Then
  pathForTextEditor = eplUI.findProgramUsingExtension("txt")
  pathForDrawView = eplUI.findProgramUsingExtension("dxf")
  pathForVRMLview = eplUI.findProgramUsingExtension("wrl")
  pathForSpreadsheet = eplUI.findProgramUsingExtension("xls")
  pathForDiagram = eplUI.findProgramUsingExtension("svg")
  pathForHTMLbrowser = eplUI.findProgramUsingExtension("html")
  pathForESOview = eplUI.findProgramUsingExtension("eso")
  pathForPDFview = eplUI.findProgramUsingExtension("pdf")
  pathForXMLview = eplUI.findProgramUsingExtension("xml")
End If
End Sub

Private Sub cmdResetAllExit_Click()
If MsgBox("Are you sure you want to reset all settings to the default value? This will overwrite any settings that have manually changed. This will also exit the program.", vbYesNo) = vbYes Then
  DeleteSetting "EP-Launch", "Pointers"
  DeleteSetting "EP-Launch", "RecentInputs"
  DeleteSetting "EP-Launch", "RecentWeather"
  DeleteSetting "EP-Launch", "Location"
  DeleteSetting "EP-Launch", "RecentGroup"
  DeleteSetting "EP-Launch", "RecentUtilIn"
  DeleteSetting "EP-Launch", "RecentUtilWthr"
  DeleteSetting "EP-Launch", "UpdateCheck"
  DeleteSetting "EP-Launch", "ViewResultsSet"
  End
End If
End Sub

Private Sub cmdCancel_Click()
Me.Hide
End Sub




'     NOTICE
'
'     The contents of this file are subject to the EnergyPlus Open Source License
'     Version 1.0 (the "License"); you may not use this file except in compliance
'     with the License. You may obtain a copy of the License at
'
'     https://energyplus.net/licensing
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
