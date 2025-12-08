VERSION 5.00
Object = "{F9043C88-F6F2-101A-A3C9-08002B2F49FB}#1.2#0"; "ComDlg32.OCX"
Object = "{C0A63B80-4B21-11D3-BD95-D426EF2C7949}#1.0#0"; "Vsflex7L.ocx"
Begin VB.Form frmNewQueue
   BorderStyle     =   1  'Fixed Single
   Caption         =   "New Group of Simulations"
   ClientHeight    =   5400
   ClientLeft      =   3465
   ClientTop       =   5130
   ClientWidth     =   11865
   LinkTopic       =   "Form1"
   MaxButton       =   0   'False
   MinButton       =   0   'False
   ScaleHeight     =   5400
   ScaleWidth      =   11865
   StartUpPosition =   1  'CenterOwner
   Begin VB.Frame frameStep4
      Caption         =   "Step 4 of 5 - Output File Locations"
      BeginProperty Font
         Name            =   "MS Sans Serif"
         Size            =   12
         Charset         =   0
         Weight          =   700
         Underline       =   0   'False
         Italic          =   0   'False
         Strikethrough   =   0   'False
      EndProperty
      Height          =   4575
      Left            =   120
      TabIndex        =   9
      Top             =   120
      Width           =   11655
      Begin VB.TextBox Text2
         BackColor       =   &H8000000F&
         BorderStyle     =   0  'None
         Height          =   855
         Left            =   6480
         Locked          =   -1  'True
         MultiLine       =   -1  'True
         TabIndex        =   19
         Text            =   "frmNewQueue.frx":0000
         Top             =   2280
         Width           =   2415
      End
      Begin VB.ComboBox comboRepeatIMF
         Height          =   315
         ItemData        =   "frmNewQueue.frx":0069
         Left            =   3000
         List            =   "frmNewQueue.frx":006B
         Style           =   2  'Dropdown List
         TabIndex        =   16
         Top             =   3000
         Width           =   855
      End
      Begin VB.TextBox textUserDefinedSpec
         Height          =   285
         Left            =   840
         TabIndex        =   14
         Text            =   "%H\%W\%I_%N"
         Top             =   2520
         Width           =   4455
      End
      Begin VB.OptionButton optionUserDefLoc
         Caption         =   "User Defined Location"
         Height          =   255
         Left            =   360
         TabIndex        =   11
         Top             =   1320
         Width           =   8535
      End
      Begin VB.OptionButton optionOriginalLoc
         Caption         =   "Original Input File Location"
         Height          =   255
         Left            =   360
         TabIndex        =   10
         Top             =   480
         Value           =   -1  'True
         Width           =   7575
      End
      Begin VB.TextBox Text1
         BackColor       =   &H8000000F&
         BorderStyle     =   0  'None
         Height          =   855
         Left            =   8880
         Locked          =   -1  'True
         MultiLine       =   -1  'True
         TabIndex        =   18
         Text            =   "frmNewQueue.frx":006D
         Top             =   2280
         Width           =   2535
      End
      Begin VB.Label lblPreview
         BorderStyle     =   1  'Fixed Single
         Caption         =   "Label5"
         Height          =   255
         Left            =   120
         TabIndex        =   21
         Top             =   4200
         Width           =   11415
      End
      Begin VB.Label Label5
         Caption         =   "Preview of Output File Location"
         Height          =   255
         Left            =   120
         TabIndex        =   22
         Top             =   3960
         Width           =   4575
      End
      Begin VB.Label Label4
         Caption         =   "Repeat IMF File Simulations "
         Height          =   255
         Left            =   840
         TabIndex        =   17
         Top             =   3000
         Width           =   2055
      End
      Begin VB.Label Label3
         Caption         =   "Location"
         Height          =   255
         Left            =   840
         TabIndex        =   15
         Top             =   2280
         Width           =   975
      End
      Begin VB.Label Label2
         Caption         =   $"frmNewQueue.frx":00B6
         Height          =   615
         Left            =   720
         TabIndex        =   13
         Top             =   1560
         Width           =   10680
      End
      Begin VB.Label Label1
         Caption         =   $"frmNewQueue.frx":0256
         Height          =   615
         Left            =   720
         TabIndex        =   12
         Top             =   720
         Width           =   10680
      End
   End
   Begin VB.Frame frameStep1
      Caption         =   "Step 1and 2 of 5 - Select ----- Files"
      BeginProperty Font
         Name            =   "MS Sans Serif"
         Size            =   12
         Charset         =   0
         Weight          =   700
         Underline       =   0   'False
         Italic          =   0   'False
         Strikethrough   =   0   'False
      EndProperty
      Height          =   4575
      Left            =   120
      TabIndex        =   2
      Top             =   120
      Width           =   11655
      Begin VB.CommandButton cmdToggle
         Caption         =   "Toggle All"
         Height          =   315
         Left            =   240
         TabIndex        =   20
         Top             =   4140
         Width           =   1095
      End
      Begin VB.CommandButton cmdSelectPath
         Caption         =   "Path.."
         Height          =   255
         Left            =   10680
         TabIndex        =   3
         Top             =   480
         Width           =   855
      End
      Begin VB.ListBox lstOfFiles
         Height          =   3210
         Left            =   120
         Sorted          =   -1  'True
         Style           =   1  'Checkbox
         TabIndex        =   4
         Top             =   840
         Width           =   11415
      End
      Begin VB.Label lblPath
         BorderStyle     =   1  'Fixed Single
         Caption         =   "Label1"
         Height          =   255
         Left            =   120
         TabIndex        =   5
         Top             =   480
         Width           =   10455
      End
   End
   Begin VB.Frame frameStep3
      Caption         =   "Step 3 of 5 - Review Simulations"
      BeginProperty Font
         Name            =   "MS Sans Serif"
         Size            =   12
         Charset         =   0
         Weight          =   700
         Underline       =   0   'False
         Italic          =   0   'False
         Strikethrough   =   0   'False
      EndProperty
      Height          =   4575
      Left            =   120
      TabIndex        =   1
      Top             =   120
      Width           =   11655
      Begin VB.CommandButton cmdDelete
         Caption         =   "Delete Simulation"
         Height          =   375
         Left            =   2160
         TabIndex        =   8
         Top             =   4080
         Width           =   1695
      End
      Begin VB.CommandButton cmdMore
         Caption         =   "Add More Simulations"
         Height          =   375
         Left            =   240
         TabIndex        =   7
         Top             =   4080
         Width           =   1815
      End
      Begin VSFlex7LCtl.VSFlexGrid gridReview
         Height          =   3615
         Left            =   120
         TabIndex        =   6
         Top             =   360
         Width           =   11415
         _cx             =   20135
         _cy             =   6376
         _ConvInfo       =   1
         Appearance      =   1
         BorderStyle     =   1
         Enabled         =   -1  'True
         BeginProperty Font {0BE35203-8F91-11CE-9DE3-00AA004BB851}
            Name            =   "MS Sans Serif"
            Size            =   8.25
            Charset         =   0
            Weight          =   400
            Underline       =   0   'False
            Italic          =   0   'False
            Strikethrough   =   0   'False
         EndProperty
         MousePointer    =   0
         BackColor       =   -2147483643
         ForeColor       =   -2147483640
         BackColorFixed  =   -2147483633
         ForeColorFixed  =   -2147483630
         BackColorSel    =   -2147483635
         ForeColorSel    =   -2147483634
         BackColorBkg    =   -2147483636
         BackColorAlternate=   -2147483643
         GridColor       =   -2147483633
         GridColorFixed  =   -2147483632
         TreeColor       =   -2147483632
         FloodColor      =   192
         SheetBorder     =   -2147483642
         FocusRect       =   1
         HighLight       =   1
         AllowSelection  =   0   'False
         AllowBigSelection=   0   'False
         AllowUserResizing=   0
         SelectionMode   =   1
         GridLines       =   1
         GridLinesFixed  =   2
         GridLineWidth   =   1
         Rows            =   1
         Cols            =   3
         FixedRows       =   1
         FixedCols       =   0
         RowHeightMin    =   0
         RowHeightMax    =   0
         ColWidthMin     =   0
         ColWidthMax     =   0
         ExtendLastCol   =   -1  'True
         FormatString    =   $"frmNewQueue.frx":035E
         ScrollTrack     =   0   'False
         ScrollBars      =   2
         ScrollTips      =   0   'False
         MergeCells      =   0
         MergeCompare    =   0
         AutoResize      =   -1  'True
         AutoSizeMode    =   0
         AutoSearch      =   0
         AutoSearchDelay =   2
         MultiTotals     =   -1  'True
         SubtotalPosition=   1
         OutlineBar      =   0
         OutlineCol      =   0
         Ellipsis        =   2
         ExplorerBar     =   0
         PicturesOver    =   0   'False
         FillStyle       =   0
         RightToLeft     =   0   'False
         PictureType     =   0
         TabBehavior     =   0
         OwnerDraw       =   0
         Editable        =   0
         ShowComboButton =   1
         WordWrap        =   0   'False
         TextStyle       =   0
         TextStyleFixed  =   0
         OleDragMode     =   0
         OleDropMode     =   0
         ComboSearch     =   3
         AutoSizeMouse   =   -1  'True
         FrozenRows      =   0
         FrozenCols      =   0
         AllowUserFreezing=   0
         BackColorFrozen =   0
         ForeColorFrozen =   0
         WallPaperAlignment=   9
      End
   End
   Begin MSComDlg.CommonDialog dialogOpenSave
      Left            =   1080
      Top             =   4800
      _ExtentX        =   847
      _ExtentY        =   847
      _Version        =   393216
   End
   Begin VB.CommandButton cmdNext
      Caption         =   "Next >>"
      Height          =   375
      Left            =   10440
      TabIndex        =   0
      Top             =   4920
      Width           =   1335
   End
End
Attribute VB_Name = "frmNewQueue"
Attribute VB_GlobalNameSpace = False
Attribute VB_Creatable = False
Attribute VB_PredeclaredId = True
Attribute VB_Exposed = False
Option Explicit
Dim queueStep As Integer
Public queueFileName As String

Const maxNumInputFiles = 10000
Dim listOfInputFiles(maxNumInputFiles) As String
Dim numInputFiles As Integer
Dim inputFilePath As String
Const getInputFiles = 1

Const maxNumWeatherFiles = 10000
Dim listOfWeatherFiles(maxNumWeatherFiles) As String
Dim numWeatherFiles As Integer
Dim weatherFilePath As String
Const getWeatherFiles = 2


'=======================================================
' To display the folder selector
'=======================================================
Private Type BROWSEINFO
   hOwner           As Long
   pidlRoot         As Long
   pszDisplayName   As String
   lpszTitle        As String
   ulFlags          As Long
   lpfn             As Long
   lParam           As Long
   iImage           As Long
End Type
Const BIF_RETURNONLYFSDIRS = &H1
Private Declare Function SHGetPathFromIDList Lib "shell32" Alias "SHGetPathFromIDListA" (ByVal pidl As Long, ByVal pszPath As String) As Long
Private Declare Function SHBrowseForFolder Lib "shell32" Alias "SHBrowseForFolderA" (lpBrowseInfo As BROWSEINFO) As Long
Private Declare Sub CoTaskMemFree Lib "ole32" (ByVal pv As Long)


'=======================================================
' Initialize this form
'=======================================================
Private Sub Form_Load()
Dim i As Integer
queueStep = 1
If eplUI.lastGrpInputDirectory <> "" Then
  inputFilePath = eplUI.lastGrpInputDirectory
Else
  inputFilePath = eplUI.lastInputDirectory
End If
If Right(inputFilePath, 1) <> "\" Then inputFilePath = inputFilePath & "\"
'inputFilePath = inputFilePath & "ExampleFiles\"
If eplUI.lastGrpWeatherDirectory <> "" Then
  weatherFilePath = eplUI.lastGrpWeatherDirectory
Else
  weatherFilePath = eplUI.lastWeatherDirectory
End If
If Right(weatherFilePath, 1) <> "\" Then weatherFilePath = weatherFilePath & "\"
'weatherFilePath = weatherFilePath & "WeatherData\"
For i = 1 To 999
  comboRepeatIMF.AddItem Trim(Str(i))
Next i
comboRepeatIMF.ListIndex = 0
comboRepeatIMF.Enabled = False
textUserDefinedSpec.Enabled = False
Call displayStep
End Sub

'=======================================================
' NEXT >> button pressed
'=======================================================
Private Sub cmdNext_Click()
If queueStep <= 2 Then Call getFileList
queueStep = queueStep + 1
If queueStep > 5 Then queueStep = 5
Call displayStep
End Sub

'=======================================================
' Add more files to the list
'=======================================================
Private Sub cmdMore_Click()
queueStep = 1
Call displayStep
End Sub

'=======================================================
' Delete the current row from the review grid
'=======================================================
Private Sub cmdDelete_Click()
Dim i As Integer
If gridReview.Row > 0 Then
  gridReview.RemoveItem gridReview.Row
  gridReview.Row = 0
  'renumber rows
  For i = 1 To (gridReview.Rows - 1)
    gridReview.TextMatrix(i, 0) = Trim(Str(i))
  Next i
End If
End Sub

'=======================================================
' Reverse what is checked for all items in input
' file or weather file lists
'=======================================================
Private Sub cmdToggle_Click()
Dim theTopRow As Integer
Dim i As Integer
theTopRow = lstOfFiles.TopIndex
lstOfFiles.Visible = False
For i = 0 To (lstOfFiles.ListCount - 1)
  If lstOfFiles.Selected(i) Then
    lstOfFiles.Selected(i) = False
  Else
    lstOfFiles.Selected(i) = True
  End If
Next i
lstOfFiles.TopIndex = theTopRow
lstOfFiles.Visible = True
End Sub


'=======================================================
' Display the current step and hide the other steps
'=======================================================
Sub displayStep()
'hide frames
frameStep1.Left = -20000
frameStep3.Left = -20000
frameStep4.Left = -20000
Select Case queueStep
  Case 1  'input files
    frameStep1.Left = 120
    frameStep1.Caption = "Step 1 of 5 - Select Input Files"
    lblPath.Caption = inputFilePath
    Call showFileList(inputFilePath)
  Case 2  'weather files
    frameStep1.Left = 120
    frameStep1.Caption = "Step 2 of 5 - Select Weather Files"
    lblPath.Caption = weatherFilePath
    Call showFileList(weatherFilePath)
  Case 3  'review
    Call addItemsToReviewList
    frameStep3.Left = 120
  Case 4  'output
    frameStep4.Left = 120
    Call updatePreview
  Case 5  'save files
    queueFileName = getQueueFileName
    If queueFileName <> "" Then
      Call saveQueueFile
      eplUI.lastGrpInputDirectory = inputFilePath
     eplUI.lastGrpWeatherDirectory = weatherFilePath
      Unload Me
    Else
      queueStep = 4
      frameStep4.Left = 120
    End If
End Select
End Sub

'=======================================================
' PATH button pressed to get new path
'=======================================================
Private Sub cmdSelectPath_Click()
Dim newPath As String
Dim oldPath As String
Select Case queueStep
  Case 1  'input files
    oldPath = inputFilePath
    newPath = getPath()
    If newPath <> "" Then
      inputFilePath = newPath
      lblPath.Caption = newPath
      Call showFileList(newPath)
    End If
  Case 2  'weather files
    oldPath = weatherFilePath
    newPath = getPath()
    If newPath <> "" Then
      weatherFilePath = newPath
      lblPath.Caption = newPath
      Call showFileList(newPath)
    End If
End Select
End Sub

'=======================================================
' Get path using built in API for folder browser
'=======================================================
Function getPath() As String
Dim bi As BROWSEINFO
Dim pidl As Long
Dim path As String
Dim pos As Integer
Dim dir As String
bi.hOwner = Me.hWnd
bi.pidlRoot = 0&
bi.lpszTitle = ""
bi.ulFlags = BIF_RETURNONLYFSDIRS
pidl = SHBrowseForFolder(bi)
path = Space$(500)
If SHGetPathFromIDList(ByVal pidl, ByVal path) Then
   pos = InStr(path, Chr$(0))
   dir = Left(path, pos - 1)
   If Right(dir, 1) <> "\" Then dir = dir & "\"
   getPath = dir
End If
Call CoTaskMemFree(pidl)
End Function

'=======================================================
' Show the files for the selected directory
'=======================================================
Sub showFileList(pathForFiles As String)
Dim fileNm As String
lstOfFiles.Clear
Select Case queueStep
  Case 1 'input files
    fileNm = dir(pathForFiles & "*.IDF", vbNormal)
    Do While fileNm <> ""
      lstOfFiles.AddItem fileNm
      fileNm = dir
    Loop
    fileNm = dir(pathForFiles & "*.IMF", vbNormal)
    Do While fileNm <> ""
      lstOfFiles.AddItem fileNm
      fileNm = dir
    Loop
  Case 2 'weather files
    lstOfFiles.AddItem "No Weather File"
    fileNm = dir(pathForFiles & "*.EPW", vbNormal)
    Do While fileNm <> ""
      lstOfFiles.AddItem fileNm
      fileNm = dir
    Loop
End Select
End Sub

'=======================================================
' gather the selected files
'=======================================================
Sub getFileList()
Dim i As Integer
Select Case queueStep
  Case 1 'input files
    numInputFiles = 0
    For i = 0 To (lstOfFiles.ListCount - 1)
      If lstOfFiles.Selected(i) Then
        numInputFiles = numInputFiles + 1
        listOfInputFiles(numInputFiles) = inputFilePath & lstOfFiles.List(i)
      End If
    Next i
    For i = 1 To numInputFiles
      Debug.Print "listofInputFiles: "; listOfInputFiles(i)
    Next i
  Case 2 'weather files
    numWeatherFiles = 0
    For i = 0 To (lstOfFiles.ListCount - 1)
      If lstOfFiles.Selected(i) Then
        numWeatherFiles = numWeatherFiles + 1
        'don't include path
        If lstOfFiles.List(i) <> "No Weather File" Then
          listOfWeatherFiles(numWeatherFiles) = weatherFilePath & lstOfFiles.List(i)
        Else
          listOfWeatherFiles(numWeatherFiles) = lstOfFiles.List(i)
        End If
      End If
    Next i
    For i = 1 To numWeatherFiles
      Debug.Print "listofWeatherFiles: "; listOfWeatherFiles(i)
    Next i
End Select
End Sub

'=======================================================
' Add the weather file and input file combinations to
' the review list
'=======================================================
Sub addItemsToReviewList()
Dim iInput As Integer
Dim jWeather As Integer
Dim nd As Node
Dim curRow As Integer
curRow = gridReview.Rows - 1
gridReview.Rows = gridReview.Rows + numInputFiles * numWeatherFiles
For iInput = 1 To numInputFiles
  For jWeather = 1 To numWeatherFiles
    curRow = curRow + 1
    gridReview.TextMatrix(curRow, 0) = Trim(Str(curRow))
    gridReview.TextMatrix(curRow, 1) = listOfInputFiles(iInput)
    gridReview.TextMatrix(curRow, 2) = listOfWeatherFiles(jWeather)
  Next jWeather
Next iInput
End Sub

'=======================================================
' Get the file name for the Queue file aka GROUP file
' using the file extension EPG
'=======================================================
Function getQueueFileName()
On Error Resume Next
dialogOpenSave.fileName = ""
dialogOpenSave.DialogTitle = "Step 5 - Save Group File"
dialogOpenSave.Filter = "EnergyPlus Group (*.epg)|*.epg"
dialogOpenSave.Flags = cdlOFNOverwritePrompt + cdlOFNNoReadOnlyReturn
dialogOpenSave.CancelError = True
dialogOpenSave.ShowSave
If Err.Number = 0 Then
  getQueueFileName = dialogOpenSave.fileName
Else
  getQueueFileName = ""
End If
End Function

'=======================================================
' Save the EPG file
'=======================================================
Sub saveQueueFile()
Dim fNum As Integer
Dim maxFileNameLength As Integer
Dim maxWeatherNameLength As Integer
Dim numExtraInSpaces As Integer
Dim numExtraWthrSpaces As Integer
Dim numExtraOutSpaces As Integer
Dim curInput As String
Dim curWthr As String
Dim outFile As String
Dim repeatCount As Integer
Dim i As Integer, j As Integer
'find the longest input file name
For i = 1 To (gridReview.Rows - 1)
  If Len(gridReview.TextMatrix(i, 1)) > maxFileNameLength Then
    maxFileNameLength = Len(gridReview.TextMatrix(i, 1))
  End If
  If Len(gridReview.TextMatrix(i, 2)) > maxWeatherNameLength Then
    maxWeatherNameLength = Len(gridReview.TextMatrix(i, 2))
  End If
Next i
' now write the file
fNum = FreeFile
On Error Resume Next
Open queueFileName For Output As fNum
  Print #fNum, "! EnergyPlus Group File"
  Print #fNum, "! ------------------------------------------------------------------------------------------------"
  Print #fNum, "! Each line represents a specific simulation. If you don't want a simulation to run, add a comment"
  Print #fNum, "! character (an exclamation point) to the beginning of that line. Commas are used to separate the "
  Print #fNum, "! fields. Each line consists of the following fields: "
  Print #fNum, "!"
  Print #fNum, "!    input file name, weather file name, output file name (no extension), counter"
  Print #fNum, "!"
  Print #fNum, "! ------------------------------------------------------------------------------------------------"
  For i = 1 To (gridReview.Rows - 1)
    'create the output file name
    curInput = gridReview.TextMatrix(i, 1)
    curWthr = gridReview.TextMatrix(i, 2)
    'if it is an IMF file then apply repeat to the rows
    If UCase(Right(curInput, 3)) = "IMF" Then
      If optionOriginalLoc.Value Then 'original location
        repeatCount = 1
      Else 'user defined
        repeatCount = Val(comboRepeatIMF.Text)
      End If
    Else
      repeatCount = 1
    End If
    For j = 1 To repeatCount
      'create the output file name without extension
      outFile = makeOutFileFromTemplate(curInput, curWthr, queueFileName, j)
      'write the line with extra spaces
      numExtraInSpaces = maxFileNameLength - Len(curInput)
      numExtraWthrSpaces = maxWeatherNameLength - Len(curWthr)
      numExtraOutSpaces = 80 - Len(outFile) 'used guessed length since writing a line at a time
      If numExtraOutSpaces < 5 Then numExtraOutSpaces = 5
      Print #fNum, curInput; ","; Space(numExtraInSpaces); curWthr; ","; Space(numExtraWthrSpaces); outFile; ","; Space(numExtraOutSpaces); j
    Next j
  Next i
Close fNum
End Sub

'=======================================================
' Do the string substitutions from the template
' using the input file name, weather file name
' and group file name.
'=======================================================
Function makeOutFileFromTemplate(inName As String, wthrName As String, queueName As String, counter As Integer) As String
Dim inFile As String
Dim inPath As String
Dim wthrFile As String
Dim wthrPath As String
Dim grpFile As String
Dim grpPath As String
Dim counterString As String
Dim templateString As String
Dim work As String
counterString = Trim(Str(counter))
inFile = fileNameOnly(inName)
inPath = pathOnly2(inName)
wthrFile = fileNameOnly(wthrName)
wthrPath = pathOnly2(wthrName)
grpFile = fileNameOnly(queueName)
grpPath = pathOnly2(queueName)
' if use original location or user defined location
If optionOriginalLoc.Value Then
  templateString = "%j\%i"
Else 'user defined
  templateString = Trim(textUserDefinedSpec.Text)
End If
' input file
work = stringSubstitute(templateString, "%I", inFile)
work = stringSubstitute(work, "%i", inFile)
work = stringSubstitute(work, "%J", inPath)
work = stringSubstitute(work, "%j", inPath)
' weather file
work = stringSubstitute(work, "%W", wthrFile)
work = stringSubstitute(work, "%w", wthrFile)
work = stringSubstitute(work, "%X", wthrPath)
work = stringSubstitute(work, "%x", wthrPath)
' group file
work = stringSubstitute(work, "%G", grpFile)
work = stringSubstitute(work, "%g", grpFile)
work = stringSubstitute(work, "%H", grpPath)
work = stringSubstitute(work, "%h", grpPath)
'counter
work = stringSubstitute(work, "%N", Trim(Str(counter)))
work = stringSubstitute(work, "%n", Trim(Str(counter)))
'assign to function
makeOutFileFromTemplate = work
End Function


'=======================================================
' Location option processing
'=======================================================
Private Sub optionOriginalLoc_Click()
comboRepeatIMF.Enabled = False
textUserDefinedSpec.Enabled = False
Call updatePreview
End Sub
Private Sub optionUserDefLoc_Click()
comboRepeatIMF.Enabled = True
textUserDefinedSpec.Enabled = True
Call updatePreview
End Sub

'=======================================================
' Substitute a string for another string
'=======================================================
Function stringSubstitute(mainString As String, searchString As String, substituteString As String) As String
Dim foundLoc As Integer
Dim lenMain As Integer
Dim lenSrch As Integer
Dim frontString As String
Dim endString As String
'check to make sure that the substitute string doesn't include the search string
If InStr(substituteString, searchString) > 0 Then Exit Function
foundLoc = InStr(mainString, searchString)
Do While foundLoc > 0
  lenMain = Len(mainString)
  lenSrch = Len(searchString)
  If foundLoc > 1 Then
    frontString = Left(mainString, foundLoc - 1)
  Else
    frontString = ""
  End If
  If lenMain >= (foundLoc + lenSrch) Then
    endString = Mid(mainString, foundLoc + lenSrch)
  Else
    endString = ""
  End If
  mainString = frontString & substituteString & endString
  foundLoc = InStr(mainString, searchString)
Loop
stringSubstitute = mainString
End Function

'=======================================================
' Returns only the path for the file and path provided no trailing slash
'=======================================================
Function pathOnly2(pathWithFile As String) As String
Dim slpt As Integer
slpt = InStrRev(pathWithFile, "\") 'finds last slash
If slpt > 1 Then
  pathOnly2 = Left(pathWithFile, slpt - 1)
Else
  pathOnly2 = "c:"
End If
End Function

'=======================================================
' Returns only the file name without extension
'=======================================================
Function fileNameOnly(pathWithFile As String) As String
Dim slshPt As Integer
Dim dotPt As Integer
slshPt = InStrRev(pathWithFile, "\") 'finds last slash
dotPt = InStrRev(pathWithFile, ".") 'finds last slash
If slshPt > 1 And dotPt > slshPt Then
  fileNameOnly = Mid(pathWithFile, slshPt + 1, dotPt - (slshPt + 1))
Else
  fileNameOnly = "nofile"
End If
End Function

Sub updatePreview()
Dim curInput As String
Dim curWthr As String
If gridReview.Rows > 1 And gridReview.Cols > 1 Then
  curInput = gridReview.TextMatrix(1, 1)
  curWthr = gridReview.TextMatrix(1, 2)
  lblPreview.Caption = makeOutFileFromTemplate(curInput, curWthr, "<group path>\<group file>.grp", 1)
End If
End Sub

Private Sub textUserDefinedSpec_Change()
Call updatePreview
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
