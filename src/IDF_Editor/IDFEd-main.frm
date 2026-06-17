VERSION 5.00
Object = "{F9043C88-F6F2-101A-A3C9-08002B2F49FB}#1.2#0"; "ComDlg32.OCX"
Object = "{831FDD16-0C5C-11D2-A9FC-0000F8754DA1}#2.1#0"; "MSCOMCTL.OCX"
Object = "{C0A63B80-4B21-11D3-BD95-D426EF2C7949}#1.0#0"; "Vsflex7L.ocx"
Begin VB.Form IDFEdit
   Caption         =   "IDF Edit"
   ClientHeight    =   7275
   ClientLeft      =   150
   ClientTop       =   720
   ClientWidth     =   10035
   Icon            =   "IDFEd-main.frx":0000
   LinkTopic       =   "Form1"
   MDIChild        =   -1  'True
   ScaleHeight     =   7275
   ScaleWidth      =   10035
   Begin VB.CommandButton cmdDuplicateObjectAndChange
      Caption         =   "Dup Obj + Chg"
      Height          =   330
      Left            =   3120
      TabIndex        =   21
      Top             =   60
      Width           =   1260
   End
   Begin VB.Timer timerJumpList
      Interval        =   100
      Left            =   1680
      Top             =   480
   End
   Begin VB.ComboBox cboClassCategories
      BackColor       =   &H00E1FFFF&
      Height          =   315
      Left            =   135
      Sorted          =   -1  'True
      TabIndex        =   2
      Text            =   "Select Class Category..."
      Top             =   870
      Width           =   4980
   End
   Begin VB.ComboBox cboClasses
      BackColor       =   &H00E1FFFF&
      Height          =   315
      Left            =   135
      Sorted          =   -1  'True
      TabIndex        =   3
      Text            =   "Select Class..."
      Top             =   1305
      Width           =   4950
   End
   Begin VB.Timer mainTimer
      Interval        =   500
      Left            =   8520
      Top             =   480
   End
   Begin VB.PictureBox picRightSplitter
      Appearance      =   0  'Flat
      BackColor       =   &H00000000&
      BorderStyle     =   0  'None
      FillColor       =   &H8000000F&
      ForeColor       =   &H80000008&
      Height          =   4215
      Left            =   9840
      MousePointer    =   7  'Size N S
      ScaleHeight     =   4215
      ScaleWidth      =   135
      TabIndex        =   20
      Top             =   3000
      Visible         =   0   'False
      Width           =   135
   End
   Begin VB.PictureBox picLeftSplitter
      Appearance      =   0  'Flat
      BackColor       =   &H00000000&
      BorderStyle     =   0  'None
      FillColor       =   &H8000000F&
      ForeColor       =   &H80000008&
      Height          =   1455
      Left            =   5160
      MousePointer    =   7  'Size N S
      ScaleHeight     =   1455
      ScaleWidth      =   135
      TabIndex        =   19
      Top             =   2880
      Visible         =   0   'False
      Width           =   135
   End
   Begin VB.PictureBox picLowerSplitter
      Appearance      =   0  'Flat
      BackColor       =   &H00000000&
      BorderStyle     =   0  'None
      FillColor       =   &H8000000F&
      ForeColor       =   &H80000008&
      Height          =   135
      Left            =   7320
      MousePointer    =   7  'Size N S
      ScaleHeight     =   135
      ScaleWidth      =   2535
      TabIndex        =   18
      Top             =   4320
      Visible         =   0   'False
      Width           =   2535
   End
   Begin VB.PictureBox picUpperSplitter
      Appearance      =   0  'Flat
      BackColor       =   &H00000000&
      BorderStyle     =   0  'None
      FillColor       =   &H8000000F&
      ForeColor       =   &H80000008&
      Height          =   135
      Left            =   7320
      MousePointer    =   7  'Size N S
      ScaleHeight     =   135
      ScaleWidth      =   2535
      TabIndex        =   17
      Top             =   2400
      Visible         =   0   'False
      Width           =   2535
   End
   Begin VB.CommandButton cmdCopyObject
      Caption         =   "Copy Obj"
      Height          =   330
      Left            =   5400
      TabIndex        =   7
      Top             =   60
      Width           =   900
   End
   Begin VB.CommandButton cmdPasteObject
      Caption         =   "Paste Obj"
      Enabled         =   0   'False
      Height          =   330
      Left            =   6360
      TabIndex        =   8
      Top             =   60
      Width           =   900
   End
   Begin VB.CommandButton cmdNewObject
      Caption         =   "New Obj"
      Height          =   330
      Left            =   1200
      TabIndex        =   4
      Top             =   60
      Width           =   900
   End
   Begin VB.CommandButton cmdDuplicateObject
      Caption         =   "Dup Obj"
      Height          =   330
      Left            =   2160
      TabIndex        =   5
      Top             =   60
      Width           =   900
   End
   Begin VB.CommandButton cmdDeleteObject
      Caption         =   "Del Obj"
      Height          =   330
      Left            =   4440
      TabIndex        =   6
      Top             =   60
      Width           =   900
   End
   Begin MSComctlLib.ImageList ilToolBar
      Left            =   7680
      Top             =   480
      _ExtentX        =   1005
      _ExtentY        =   1005
      BackColor       =   -2147483643
      ImageWidth      =   16
      ImageHeight     =   16
      MaskColor       =   12632256
      _Version        =   393216
      BeginProperty Images {2C247F25-8591-11D1-B16A-00C0F0283628}
         NumListImages   =   12
         BeginProperty ListImage1 {2C247F27-8591-11D1-B16A-00C0F0283628}
            Picture         =   "IDFEd-main.frx":08CA
            Key             =   ""
         EndProperty
         BeginProperty ListImage2 {2C247F27-8591-11D1-B16A-00C0F0283628}
            Picture         =   "IDFEd-main.frx":09DC
            Key             =   ""
         EndProperty
         BeginProperty ListImage3 {2C247F27-8591-11D1-B16A-00C0F0283628}
            Picture         =   "IDFEd-main.frx":0AEE
            Key             =   ""
         EndProperty
         BeginProperty ListImage4 {2C247F27-8591-11D1-B16A-00C0F0283628}
            Picture         =   "IDFEd-main.frx":0C00
            Key             =   ""
         EndProperty
         BeginProperty ListImage5 {2C247F27-8591-11D1-B16A-00C0F0283628}
            Picture         =   "IDFEd-main.frx":0D12
            Key             =   ""
         EndProperty
         BeginProperty ListImage6 {2C247F27-8591-11D1-B16A-00C0F0283628}
            Picture         =   "IDFEd-main.frx":0E24
            Key             =   ""
         EndProperty
         BeginProperty ListImage7 {2C247F27-8591-11D1-B16A-00C0F0283628}
            Picture         =   "IDFEd-main.frx":0F36
            Key             =   ""
         EndProperty
         BeginProperty ListImage8 {2C247F27-8591-11D1-B16A-00C0F0283628}
            Picture         =   "IDFEd-main.frx":1048
            Key             =   ""
         EndProperty
         BeginProperty ListImage9 {2C247F27-8591-11D1-B16A-00C0F0283628}
            Picture         =   "IDFEd-main.frx":115A
            Key             =   ""
         EndProperty
         BeginProperty ListImage10 {2C247F27-8591-11D1-B16A-00C0F0283628}
            Picture         =   "IDFEd-main.frx":12B4
            Key             =   ""
         EndProperty
         BeginProperty ListImage11 {2C247F27-8591-11D1-B16A-00C0F0283628}
            Picture         =   "IDFEd-main.frx":1B8E
            Key             =   ""
         EndProperty
         BeginProperty ListImage12 {2C247F27-8591-11D1-B16A-00C0F0283628}
            Picture         =   "IDFEd-main.frx":2468
            Key             =   ""
         EndProperty
      EndProperty
   End
   Begin MSComctlLib.Toolbar Toolbar1
      Align           =   1  'Align Top
      Height          =   336
      Left            =   0
      TabIndex        =   0
      Top             =   0
      Width           =   10032
      _ExtentX        =   17701
      _ExtentY        =   582
      ButtonWidth     =   609
      ButtonHeight    =   582
      AllowCustomize  =   0   'False
      Appearance      =   1
      ImageList       =   "ilToolBar"
      _Version        =   393216
      BeginProperty Buttons {66833FE8-8583-11D1-B16A-00C0F0283628}
         NumButtons      =   5
         BeginProperty Button1 {66833FEA-8583-11D1-B16A-00C0F0283628}
            Style           =   3
         EndProperty
         BeginProperty Button2 {66833FEA-8583-11D1-B16A-00C0F0283628}
            Key             =   "newIDF"
            Description     =   "New IDF File"
            Object.ToolTipText     =   "Create New IDF File"
            ImageIndex      =   1
         EndProperty
         BeginProperty Button3 {66833FEA-8583-11D1-B16A-00C0F0283628}
            Key             =   "openIDF"
            Object.ToolTipText     =   "Open IDF File"
            ImageIndex      =   2
         EndProperty
         BeginProperty Button4 {66833FEA-8583-11D1-B16A-00C0F0283628}
            Key             =   "saveIDF"
            Object.ToolTipText     =   "Save Current IDF File"
            ImageIndex      =   3
         EndProperty
         BeginProperty Button5 {66833FEA-8583-11D1-B16A-00C0F0283628}
            Style           =   3
         EndProperty
      EndProperty
      Begin VB.Label Label5
         Caption         =   "Object"
         Height          =   255
         Left            =   3960
         TabIndex        =   16
         Top             =   2160
         Width           =   1215
      End
      Begin VB.Label Label4
         Caption         =   "Label4"
         Height          =   255
         Left            =   3360
         TabIndex        =   1
         Top             =   480
         Width           =   1575
      End
   End
   Begin VSFlex7LCtl.VSFlexGrid grdNew
      Height          =   2415
      Left            =   120
      TabIndex        =   14
      Top             =   4440
      Width           =   9615
      _cx             =   16960
      _cy             =   4260
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
      AllowSelection  =   -1  'True
      AllowBigSelection=   -1  'True
      AllowUserResizing=   1
      SelectionMode   =   0
      GridLines       =   1
      GridLinesFixed  =   2
      GridLineWidth   =   1
      Rows            =   50
      Cols            =   10
      FixedRows       =   1
      FixedCols       =   2
      RowHeightMin    =   0
      RowHeightMax    =   0
      ColWidthMin     =   0
      ColWidthMax     =   0
      ExtendLastCol   =   0   'False
      FormatString    =   ""
      ScrollTrack     =   0   'False
      ScrollBars      =   3
      ScrollTips      =   0   'False
      MergeCells      =   0
      MergeCompare    =   0
      AutoResize      =   -1  'True
      AutoSizeMode    =   1
      AutoSearch      =   0
      AutoSearchDelay =   2
      MultiTotals     =   -1  'True
      SubtotalPosition=   1
      OutlineBar      =   0
      OutlineCol      =   0
      Ellipsis        =   0
      ExplorerBar     =   0
      PicturesOver    =   0   'False
      FillStyle       =   0
      RightToLeft     =   0   'False
      PictureType     =   0
      TabBehavior     =   1
      OwnerDraw       =   0
      Editable        =   2
      ShowComboButton =   1
      WordWrap        =   -1  'True
      TextStyle       =   0
      TextStyleFixed  =   0
      OleDragMode     =   0
      OleDropMode     =   0
      ComboSearch     =   0
      AutoSizeMouse   =   -1  'True
      FrozenRows      =   0
      FrozenCols      =   0
      AllowUserFreezing=   0
      BackColorFrozen =   0
      ForeColorFrozen =   0
      WallPaperAlignment=   9
   End
   Begin MSComDlg.CommonDialog CommonDialog1
      Left            =   6960
      Top             =   480
      _ExtentX        =   847
      _ExtentY        =   847
      _Version        =   393216
   End
   Begin VB.TextBox txtExplain
      BackColor       =   &H8000000F&
      Height          =   1575
      Left            =   5280
      Locked          =   -1  'True
      MultiLine       =   -1  'True
      ScrollBars      =   2  'Vertical
      TabIndex        =   10
      Top             =   2760
      Width           =   4575
   End
   Begin VB.TextBox txtComment
      BackColor       =   &H00C0C0C0&
      Height          =   1695
      Left            =   5280
      Locked          =   -1  'True
      MultiLine       =   -1  'True
      ScrollBars      =   2  'Vertical
      TabIndex        =   15
      Top             =   720
      Width           =   4575
   End
   Begin VB.ListBox lstObjectTypes
      Height          =   2205
      Left            =   120
      TabIndex        =   9
      Top             =   1725
      Width           =   5055
   End
   Begin VB.Image imgRightSplitter
      Height          =   2295
      Left            =   9840
      MousePointer    =   9  'Size W E
      Top             =   720
      Width           =   135
   End
   Begin VB.Image imgLeftSplitter
      Height          =   2295
      Left            =   5160
      MousePointer    =   9  'Size W E
      Top             =   720
      Width           =   135
   End
   Begin VB.Image imgLowerSplitter
      Height          =   135
      Left            =   120
      MousePointer    =   7  'Size N S
      Top             =   4320
      Width           =   6975
   End
   Begin VB.Image imgUpperSplitter
      Height          =   135
      Left            =   5280
      MousePointer    =   7  'Size N S
      Top             =   2400
      Width           =   1935
   End
   Begin VB.Label lblExplain
      Caption         =   "Explanation of Object and Current Field"
      Height          =   255
      Left            =   5280
      TabIndex        =   12
      Top             =   2520
      Width           =   3500
   End
   Begin VB.Label lblObjectList
      Caption         =   "Class List"
      Height          =   255
      Left            =   120
      TabIndex        =   13
      Top             =   480
      Width           =   975
   End
   Begin VB.Label lblComment
      Caption         =   "Comments from IDF"
      Height          =   255
      Left            =   5280
      TabIndex        =   11
      Top             =   480
      Width           =   1695
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
      Begin VB.Menu mnuFileOpenDataSet
         Caption         =   "Open &DataSet"
         Begin VB.Menu mnuFileOpenDataSetSub
            Caption         =   "datasetfile"
            Index           =   1
         End
      End
      Begin VB.Menu mnuFileClose
         Caption         =   "&Close"
      End
      Begin VB.Menu mnuFileSpace1
         Caption         =   "-"
      End
      Begin VB.Menu mnuFileSave
         Caption         =   "&Save"
         Enabled         =   0   'False
         Shortcut        =   ^S
      End
      Begin VB.Menu mnuFileSaveAs
         Caption         =   "Save &As..."
      End
      Begin VB.Menu mnuFileSaveOption
         Caption         =   "Sa&ve Options..."
      End
      Begin VB.Menu mnuFileSpace2
         Caption         =   "-"
      End
      Begin VB.Menu mnuFilePageSetup
         Caption         =   "Page Set&up"
         Enabled         =   0   'False
      End
      Begin VB.Menu mnuFilePrint
         Caption         =   "&Print"
         Enabled         =   0   'False
         Shortcut        =   ^P
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
      Begin VB.Menu mnuFileSpace4
         Caption         =   "-"
      End
      Begin VB.Menu mnuFileExit
         Caption         =   "E&xit"
      End
   End
   Begin VB.Menu mnuEditTop
      Caption         =   "&Edit"
      Begin VB.Menu mnuEditUndo
         Caption         =   "&Undo"
         Enabled         =   0   'False
         Shortcut        =   ^Z
      End
      Begin VB.Menu mnuEditRedo
         Caption         =   "Red&o"
         Enabled         =   0   'False
         Shortcut        =   ^Y
      End
      Begin VB.Menu mnuEditDiv1
         Caption         =   "-"
      End
      Begin VB.Menu mnuEditCut
         Caption         =   "Cu&t Object"
         Enabled         =   0   'False
         Shortcut        =   ^X
      End
      Begin VB.Menu mnuEditCopy
         Caption         =   "&Copy Object"
         Shortcut        =   ^C
      End
      Begin VB.Menu mnuEditPaste
         Caption         =   "&Paste Object"
         Shortcut        =   ^V
      End
      Begin VB.Menu mnuEditCopySpread
         Caption         =   "Copy for &Spreadsheet"
      End
      Begin VB.Menu mnuEditDiv4
         Caption         =   "-"
      End
      Begin VB.Menu mnuEditFillRight
         Caption         =   "Fill Right"
         Shortcut        =   ^D
      End
      Begin VB.Menu mnuEditDiv2
         Caption         =   "-"
      End
      Begin VB.Menu mnuEditNew
         Caption         =   "&New Object"
      End
      Begin VB.Menu mnuEditDuplicate
         Caption         =   "Dup&licate Object"
      End
      Begin VB.Menu mnuEditDuplicateAndChange
         Caption         =   "Duplicate and Change Object"
      End
      Begin VB.Menu mnuEditDelete
         Caption         =   "&Delete Object"
      End
      Begin VB.Menu mnuEditDiv3
         Caption         =   "-"
      End
      Begin VB.Menu mnuEditFind
         Caption         =   "Find Class.."
         Shortcut        =   ^F
      End
      Begin VB.Menu mnuEditFindPrevious
         Caption         =   "Find Previous Class"
         Shortcut        =   ^T
      End
      Begin VB.Menu mnuEditFindNext
         Caption         =   "Find Next Class"
         Shortcut        =   ^G
      End
      Begin VB.Menu mnuEditSearch
         Caption         =   "Search and Replace.."
         Shortcut        =   ^H
      End
      Begin VB.Menu mnuEditDiv5
         Caption         =   "-"
      End
      Begin VB.Menu mnuEditNextRowAfterEnter
         Caption         =   "Next Row after Enter"
         Checked         =   -1  'True
      End
   End
   Begin VB.Menu mnuViewTop
      Caption         =   "&View"
      Begin VB.Menu mnuViewIP
         Caption         =   "&Inch-Pound"
      End
      Begin VB.Menu mnuViewSi
         Caption         =   "&SI Units"
         Checked         =   -1  'True
      End
      Begin VB.Menu mnuViewDiv1
         Caption         =   "-"
      End
      Begin VB.Menu mnuViewNarrowColumn
         Caption         =   "&Narrow Column"
      End
      Begin VB.Menu mnuViewMediumColumn
         Caption         =   "&Medium Column"
      End
      Begin VB.Menu mnuViewWideColumn
         Caption         =   "&Wide Column"
      End
      Begin VB.Menu mnuViewWider
         Caption         =   "W&ider"
         Begin VB.Menu mnuViewWidth30
            Caption         =   "30"
         End
         Begin VB.Menu mnuViewWidth40
            Caption         =   "40"
         End
         Begin VB.Menu mnuViewWidth50
            Caption         =   "50"
         End
         Begin VB.Menu mnuViewWidth60
            Caption         =   "60"
         End
         Begin VB.Menu mnuViewWidth70
            Caption         =   "70"
         End
         Begin VB.Menu mnuViewWidth80
            Caption         =   "80"
         End
         Begin VB.Menu mnuViewWidth90
            Caption         =   "90"
         End
      End
      Begin VB.Menu mnuViewDiv2
         Caption         =   "-"
      End
      Begin VB.Menu mnuViewWordWrap
         Caption         =   "Word Wrap"
      End
      Begin VB.Menu mnuDiv5
         Caption         =   "-"
      End
      Begin VB.Menu mnuViewClassesWithObjs
         Caption         =   "Show Classes with Objects Only"
         Shortcut        =   ^L
      End
      Begin VB.Menu mnuQuickSelect
         Caption         =   "Show Quick Select Dropdowns"
         Checked         =   -1  'True
         Shortcut        =   ^Q
      End
      Begin VB.Menu mnuDiv7
         Caption         =   "-"
      End
      Begin VB.Menu mnuViewUseNodeEditor
         Caption         =   "Use Node Name Editor"
         Checked         =   -1  'True
      End
      Begin VB.Menu mnuDiv3
         Caption         =   "-"
      End
      Begin VB.Menu mnuViewLayoutOptions
         Caption         =   "Layout Options.."
      End
      Begin VB.Menu mnuViewDiv4
         Caption         =   "-"
      End
      Begin VB.Menu mnuViewValidityCheck
         Caption         =   "Validity Check.."
         Shortcut        =   ^R
      End
   End
   Begin VB.Menu mnuJumpTop
      Caption         =   "&Jump"
      Begin VB.Menu mnuJumpItem
         Caption         =   "No items"
         Index           =   0
      End
   End
   Begin VB.Menu mnuWindow
      Caption         =   "&Window"
      WindowList      =   -1  'True
      Begin VB.Menu mnuWindowCascade
         Caption         =   "&Cascade"
      End
      Begin VB.Menu mnuWindowTileHoriz
         Caption         =   "Tile &Horizontal"
      End
      Begin VB.Menu mnuWindowTileVert
         Caption         =   "Tile &Vertical"
      End
      Begin VB.Menu mnuWindowArrange
         Caption         =   "&Arrange Icons"
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
      Begin VB.Menu mnuHelpQuickStart
         Caption         =   "EnergyPlus QuickStart Webpage"
      End
      Begin VB.Menu mnuHelpEssentials
         Caption         =   "EnergyPlus Essentials"
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
      Begin VB.Menu mnuHelpDiv6
         Caption         =   "-"
      End
      Begin VB.Menu mnuHelpPlant
         Caption         =   "EnergyPlus Plant Applications Guide"
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
      Begin VB.Menu mnuHelpDiv5
         Caption         =   "-"
      End
      Begin VB.Menu mnuHelpGettingStarted
         Caption         =   "EnergyPlus Getting Started"
      End
      Begin VB.Menu mnuHelpAuxProgs
         Caption         =   "EnergyPlus Auxiliary Programs"
      End
      Begin VB.Menu mnuHelpTips
         Caption         =   "EnergyPlus Tips and Tricks"
      End
      Begin VB.Menu mnuHelpAcknowledge
         Caption         =   "EnergyPlus Acknowledgments"
      End
      Begin VB.Menu mnuHelpDiv3
         Caption         =   "-"
      End
      Begin VB.Menu mnuCreateObjectList
         Caption         =   "Create objectList.txt"
      End
      Begin VB.Menu mnuCreateAllObjectIDF
         Caption         =   "Create allObject.idf"
      End
      Begin VB.Menu mnuCreateFieldsMissingUnits
         Caption         =   "Create fieldsMissingUnits.txt"
      End
      Begin VB.Menu mnuCreateRefObjListTxt
         Caption         =   "Create RefObjList.txt"
      End
      Begin VB.Menu mnuHelpDiv4
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
Attribute VB_Name = "IDFEdit"
Attribute VB_GlobalNameSpace = False
Attribute VB_Creatable = False
Attribute VB_PredeclaredId = True
Attribute VB_Exposed = False
Option Explicit
Private Declare Function GetForegroundWindow Lib "user32" () As Long

Dim lstObjectLast As Long
Dim defaultColumnWidth As Long
Dim displayUnits As Integer 'flag to display units in IP or SI
Dim lastKeyEnter As Boolean
Dim downWhenEnter As Boolean
Dim lstObjectHeading As Long
Dim IDFVersion As String
Dim isVersionMismatch As Boolean
Dim fileDateTimeStamp As String
Dim mainWindowHandle As Long

'variables related to the layout of the main form
'and position of the splitters
Dim currentFormLayoutOption As Integer
Dim upperMoving As Boolean
Dim lowerMoving As Boolean
Dim leftMoving As Boolean
Dim rightMoving As Boolean
Const bottomMargin = 120
Const topMargin = 720
Const leftMargin = 120
Const rightMargin = 120
Const minCommentHeight = 1000
Const minCommentWidth = 1000
Const minExplainWidth = 1000
Const minExplainHeight = 1000
Const minGridHeight = 2000
Const minGridWidth = 2000
Const minListWidth = 2000
Const minListHeight = 1000
Const upperGapHeight = 400
Const lowerGapHeight = 80
Const rightGapWidth = 80
Const leftGapWidth = 80
Const explainLabelHeight = 240
Const explainLabelWidth = 3500

Const dispUnitIP = 1
Const dispUnitSI = 2

Const findNext = 1
Const findPrev = 2
'-----------------------------------------------------------------------------
' Variables used for this particular file
'-----------------------------------------------------------------------------
Dim IDFObject() As objectRecord
Dim maxUsedObject As Long
Dim lastUsedObjectInOrigFile As Long
Dim sizeObject As Long

Dim IDFValue() As IDFValueRecord
Dim maxUsedValue As Long
Dim sizeValue As Long

Dim IDFComment() As String 'holds the comments on the parameters
Dim maxUsedComment As Long
Dim sizeComment As Long

Dim IDDClassObjPt() As IDDClassObjPtType

' ==================== Variables for File Names ==========================
'Dim IDFFileName As String  'the name and path for the active IDF file
Dim IDFFileName As String  'the name and path for the active IDF file
Dim weatherFileName As String
Dim IDFAltered As Boolean  'if true than changes have been made to file that haven't been saved
Dim IDFEdPrevEd As Boolean 'if true than the previous editor was the IDF editor and don't need to save .ORG file
Dim IDFUntitled As Boolean 'when true no file name has been given to the active IDF file

' ==================== Variables for Interface ==========================
Dim actClass As Long    ' pointer to the active class
Dim actObject As Long   ' pointer to the active object (just selected by user)
Dim actField As Long    ' pointer to the active field
Dim actValue As Long    ' pointer to the active value
Dim actRow As Long      ' the grid row that is currently active
Dim actCol As Long      ' the grid column that is currently active
Dim prevClass As Long   ' pointer to the previously active class
Dim prevQuickSelectStatus As Boolean    ' CEPTChange: status of ShowQuickSelectCombos before Showing only classes with objects

' ==================== Variables for Range Testing ==========================
Const numSubTests = 8
Const numReadVariables = 1000
Dim subTestNames(numSubTests) As String
Const STminMinus = 1
Const STminimum = 2
Const STzero = 3
Const STmaximum = 4
Const STmaxPlus = 5
Const STnominal = 6
Const STnomMinus = 7
Const STnomPlus = 8
Dim logItemState(numSubTests) As Long 'state of log item 0=no entry, 1=successful, 2=error, 3=crashed,4=not run,5=unknown
Dim logItemResults(numSubTests, numReadVariables) As Double
Dim logItemPointers(numReadVariables) As Long
Dim logItemVariables(numReadVariables) As String
Dim logItemInputs(numSubTests) As Double
Dim numUsedReadVariables As Long
Dim firstRunInSuite As Boolean

Dim numDigitsShown As Long

Dim showAllClasses As Boolean
Dim useNodeEditor As Boolean

' arrays related to the Jump menu
Dim newCellSelected As Boolean 'used for creating jump list with timer
Const maxJumpMenuSize = 40
Dim jmpClass(0 To maxJumpMenuSize) As Long
Dim jmpObj(0 To maxJumpMenuSize) As Long
Dim jmpFld(0 To maxJumpMenuSize) As Long

Dim recentlyUsedNodeNames(20) As String 'fixed size since just recently used node names
Dim mostRecentUsedNodeName As Integer 'points to the most recently used item in array
' the next two variables are used to keep track of the last values of a non-numeric field
' if they have changed they are made available to the search and replace dialog box
Dim lastValueBeforeEdit As String
Dim lastValueAfterEdit As String

' ==================== variables for names from the RDD file ==========================
Dim autoObjListVar As String   'holds list in format appropriate for pull down list
Dim autoObjListMeter As String 'holds list in format appropriate for pull down list
Dim autoObjListTimeStamp As Date

' Save Options for active file
Public saveOrderOption As Integer
Public specialFormatOption As Integer
Public isLastError As Boolean

'array that holds the special unit pointers for fields that could have multiple units
'positive values point to the convUnit array
'Dim specialUnit() As Integer

'-----------------------------------------------------------------------------
' CEPTChange: Added for autocomplete functionality on combobox
'-----------------------------------------------------------------------------
Private Sub cboClassCategories_KeyPress(KeyAscii As Integer)
    KeyAscii = AutoCompleteCombo(cboClassCategories, KeyAscii)
End Sub

'-----------------------------------------------------------------------------
' CEPTChange: Added cboClassCategories_LostFocus()
'-----------------------------------------------------------------------------
Private Sub cboClassCategories_LostFocus()
  ' fill cboClasses with classes under selected category
  FillClassCombo Trim(cboClassCategories.Text)
  Debug.Print "Combo filled : " & Trim(cboClassCategories.Text)
End Sub

'-----------------------------------------------------------------------------
' CEPTChange: Added to support autocomplete feature -
' change list selection immediately on changes in dropdown not only at validation
'-----------------------------------------------------------------------------
Private Sub cboClasses_Change()
    Call cboClasses_Validate(False)
End Sub

'-----------------------------------------------------------------------------
' CEPTChange: Added to support autocomplete feature -
' change list selection immediately on changes in dropdown not only at validation
'-----------------------------------------------------------------------------
Private Sub cboClasses_Click()
    Call cboClasses_Validate(False)
End Sub

'-----------------------------------------------------------------------------
' CEPTChange: Added for autodropdown functionality on combobox on GotFocus()
'-----------------------------------------------------------------------------
Private Sub cboClasses_GotFocus()
    If cboClasses.Visible Then AutoDropDownComboBox cboClasses
End Sub

'-----------------------------------------------------------------------------
' CEPTChange: Added for autocomplete functionality on combobox
'-----------------------------------------------------------------------------
Private Sub cboClasses_KeyPress(KeyAscii As Integer)
    KeyAscii = AutoCompleteCombo(cboClasses, KeyAscii)
End Sub

'-----------------------------------------------------------------------------
' CEPTChange: Added cboClasses_Validate
'-----------------------------------------------------------------------------
Private Sub cboClasses_Validate(Cancel As Boolean)
' pass selection action to listbox to position it accordingly
Dim lIndex As Long

If cboClasses.ListIndex >= 0 Then ' only if there is a proper selection
  lIndex = ItemDataToIndex(lstObjectTypes, Trim(cboClasses.ItemData(cboClasses.ListIndex)))
  If lIndex <> -1 Then ' extra trap for wrong selection
    lstObjectTypes.ListIndex = lIndex
  End If
End If
End Sub




'-----------------------------------------------------------------------------
' When the form is activated resize it.
'-----------------------------------------------------------------------------
Private Sub Form_Activate()
Debug.Print "Form Activate"
Form_Resize
If doesClipContainObject Then
  mnuEditPaste.Enabled = True
  cmdPasteObject.Enabled = True
Else
  mnuEditPaste.Enabled = False
  cmdPasteObject.Enabled = False
End If
If isVersionMismatch Then
  frmParent.sbStatusBar.Panels(5) = "Version Mismatch"
Else
  frmParent.sbStatusBar.Panels(5) = ""
End If
' CEPTChange: Mark menu item -------
If ShowQuickSelectCombos Then
    mnuQuickSelect.Checked = True
Else
    mnuQuickSelect.Checked = False
End If
' CEPTChange over ------------------
mnuViewWordWrap.Checked = useWordWrap
grdNew.WordWrap = useWordWrap
Call displayRecentChildFileMenuList
Call checkForExternalFileChange
Call addDataSetFilesToMenu


End Sub

'-----------------------------------------------------------------------------
' Add the data set files to the open data set sub menu
'-----------------------------------------------------------------------------
Sub addDataSetFilesToMenu()
Dim nextFile As String
Dim counter As Integer
Dim lastMenu As Integer
nextFile = Dir(dataSetPath & "*.idf")
counter = 0
lastMenu = mnuFileOpenDataSetSub.Count
Do While nextFile <> ""
  counter = counter + 1
  If counter > lastMenu Then
    Load mnuFileOpenDataSetSub(counter)
  End If
  mnuFileOpenDataSetSub(counter).Caption = nextFile
  nextFile = Dir
Loop
End Sub

'-----------------------------------------------------------------------------
' Check if a different program has changed the active file
'-----------------------------------------------------------------------------
Sub checkForExternalFileChange()
Dim curFileDateTime As String
If Not IDFUntitled Then
  curFileDateTime = FileDateTime(IDFFileName)
  If curFileDateTime <> fileDateTimeStamp Then
    MsgBox "The file has changed by a different program. Close the file and open it again.", vbCritical, "File Changed"
    fileDateTimeStamp = curFileDateTime
  End If
End If
End Sub


'-----------------------------------------------------------------------------
' Check if clipboard contains valid data when the form get repainted
' (switch from an overlapping application)
'-----------------------------------------------------------------------------
Private Sub Form_Paint()
If doesClipContainObject Then
  mnuEditPaste.Enabled = True
  cmdPasteObject.Enabled = True
Else
  mnuEditPaste.Enabled = False
  cmdPasteObject.Enabled = False
End If
End Sub

'-----------------------------------------------------------------------------
'  Routine that gets initialized when an instance of the form loads
'-----------------------------------------------------------------------------
Private Sub Form_Load()
Dim i As Long, j As Long
Dim a As String
Me.Width = 15000
Me.Height = 8000
'currentFormLayoutOption = floShortShort
'lblObjectList = "Class List for EnergyPlus " & IDDVersion
End Sub

'-----------------------------------------------------------------------------
' Initialize the form (for open or blank)
'-----------------------------------------------------------------------------
Sub initializeFormInstance()
Dim i As Integer
Debug.Print "=========================================="
Debug.Print "IDF Editor"
Debug.Print Date, Time
isVersionMismatch = False
downWhenEnter = True
IDFUntitled = True
Call SetDisplayUnits(dispUnitSI)   'default is displaying SI units
autoObjListTimeStamp = DateSerial(1950, 1, 1) 'set to old date and time
'The following caused problems with spanish environments.
'autoObjListTimeStamp = CDate("Jan 1, 1950 1:00") 'set to old date and time
'IDFFileName = "UNTITLED.IDF"
grdNew.ColAlignment(-1) = flexAlignLeftCenter 'left justify everything in grid
prevClass = 1
Call InitializeArrays
showAllClasses = True
For i = 1 To maxJumpMenuSize
  Load mnuJumpItem(i)
Next i
Call FillList
Call MakeEmptyIDF 'when program starts an empty file is displayed
Call FillGrid
'lblVersion.Caption = "EnergyPlus " & IDDVersion
'Call retrieveSettings
defaultColumnWidth = 1500
numDigitsShown = 14
mnuViewMediumColumn.Checked = True
grdNew.ColWidth(-1) = defaultColumnWidth
grdNew.ColWidth(0) = 4000
grdNew.ColWidth(1) = 1000
cmdDuplicateObject.Enabled = False
cmdDuplicateObjectAndChange.Enabled = False
cmdDeleteObject.Enabled = False
cmdPasteObject.Enabled = doesClipContainObject()
cmdCopyObject.Enabled = False
mnuEditDelete.Enabled = False
mnuEditDuplicate.Enabled = False
mnuEditDuplicateAndChange.Enabled = False
mnuEditPaste.Enabled = doesClipContainObject()
currentFormLayoutOption = formLayoutOption
saveOrderOption = saveOrderOptDefault
mnuViewWordWrap.Checked = useWordWrap
grdNew.WordWrap = useWordWrap
specialFormatOption = specialFormatOptDefault
useNodeEditor = True
mnuViewUseNodeEditor.Checked = True
Call displayRecentChildFileMenuList
End Sub

'-----------------------------------------------------------------------------
' Display the recent files in the file menu
'-----------------------------------------------------------------------------
Sub displayRecentChildFileMenuList()
Dim i As Integer
Debug.Print "displayRecentChildFileMenuList"
If numRecentFiles > 0 Then
  mnuFileSpace4.Visible = True
Else
  mnuFileSpace4.Visible = False
End If
For i = 1 To maxRecentFiles
  If recentFiles(i).nameOnly <> "" Then
    mnuFileRecent(i).Caption = recentFiles(i).nameOnly
    mnuFileRecent(i).Visible = True
  Else
    mnuFileRecent(i).Visible = False
  End If
  Debug.Print "  ["; recentFiles(i).nameWithPath; "]"
Next i
End Sub

'-----------------------------------------------------------------------------
' The exit routine when someone uses the [x] in the corner of the window
'-----------------------------------------------------------------------------
Private Sub Form_QueryUnload(Cancel As Integer, UnloadMode As Integer)
Dim needToSave As Boolean
needToSave = IsFileChanged()
If needToSave Then
  Cancel = True
  Exit Sub
End If
End Sub

'-----------------------------------------------------------------------------
' When the form is resized the routine is called so that objects on the
' form can be resized also
'-----------------------------------------------------------------------------
Private Sub Form_Resize()
Call resizeControls(picUpperSplitter.Top, picLowerSplitter.Top, picRightSplitter.Left, picLeftSplitter.Left)
End Sub

'-----------------------------------------------------------------------------
' Routine called just before editing can occur
' Sets up the drop down lists
'-----------------------------------------------------------------------------
Private Sub grdNew_BeforeEdit(ByVal Row As Long, ByVal Col As Long, Cancel As Boolean)
Dim comboListString As String
Dim curObjListName As Long, curObjListItem As Long
Dim iClass As Long, iField As Long, iObject As Long
Dim deltaField As Long, StartVal As Long
Dim defaultValConv As Double, minimumValConv As Double, maximumValConv As Double
Dim useUnits As Long
Dim defaultChoiceString As String
Dim iListOfObjList As Long
Dim i As Long
actRow = Row - 1
actCol = Col - 2
If actCol < 0 Then Exit Sub 'no objects are defined and cell is clicked
actField = IDDClassDat(actClass).fieldStart + actRow
actObject = grdNew.ColData(Col)
comboListString = ""
'Debug.Print IDDField(actField).name, IDDField(actField).type
Select Case IDDField(actField).type
  Case 1, 2 'real number or long
    useUnits = getUnitsForCell(Row, Col, actClass)
    If displayUnits = dispUnitSI Or useUnits = noUnitsSpecified Then
      defaultValConv = IDDField(actField).defaultValue
      minimumValConv = IDDField(actField).minimum
      maximumValConv = IDDField(actField).maximum
    Else
      defaultValConv = IDDField(actField).defaultValue * convUnits(useUnits).mult + convUnits(useUnits).offset
      minimumValConv = IDDField(actField).minimum * convUnits(useUnits).mult + convUnits(useUnits).offset
      maximumValConv = IDDField(actField).maximum * convUnits(useUnits).mult + convUnits(useUnits).offset
    End If
    comboListString = "|" & grdNew.TextMatrix(Row, Col) & vbTab & "current"
    If IDDField(actField).defSpecified Then
      If IDDField(actField).defaultAutosize Then
        comboListString = comboListString & "|autosize" & vbTab & "default"
      ElseIf IDDField(actField).defaultAutoCalc Then
        comboListString = comboListString & "|autocalculate" & vbTab & "default"
      Else
        comboListString = comboListString & "|" & toIntl(Str(defaultValConv), numDigitsShown) & vbTab & "default"
      End If
    End If
    If IDDField(actField).minSpecified And Not IDDField(actField).exclusiveMin Then
      comboListString = comboListString & "|" & toIntl(Str(minimumValConv), numDigitsShown) & vbTab & "minimum"
    End If
    If IDDField(actField).maxSpecified And Not IDDField(actField).exclusiveMax Then
      comboListString = comboListString & "|" & toIntl(Str(maximumValConv), numDigitsShown) & vbTab & "maximum"
    End If
    If IDDField(actField).autosizable Then
      comboListString = comboListString & "|" & "autosize" & vbTab & "determined by software"
    End If
    If IDDField(actField).autocalculatable Then
      comboListString = comboListString & "|" & "autocalculate" & vbTab & "determined by software"
    End If
    'following used to fix CR6640
    If comboListString = "|" & vbTab & "current" Then comboListString = "| " & vbTab & "current"
  Case 3 'alpha
    'do nothing - already editable
  Case 4 'choice
    comboListString = "|" & grdNew.TextMatrix(Row, Col) & vbTab & "current" & "|"
    If IDDField(actField).defaultChoice > 0 Then
      comboListString = comboListString & IDDChoice(IDDField(actField).defaultChoice) & vbTab & "default" & "|"
      defaultChoiceString = IDDChoice(IDDField(actField).defaultChoice)
    Else
      defaultChoiceString = ""
    End If
    If IDDField(actField).choiceStart > 0 Then 'if multiple choice
      For i = IDDField(actField).choiceStart To IDDField(actField).choiceEnd
        If IDDChoice(i) <> defaultChoiceString Then
          comboListString = comboListString & IDDChoice(i) & vbTab & "choice" & "|"
        End If
      Next i
    End If
    comboListString = comboListString & "<BLANK>"
  Case 5 'object list
    comboListString = "|" & grdNew.TextMatrix(Row, Col) & vbTab & "current" & "|"
    ' New code (3/23/2000) to dynamically generate the list of choices.
    ' Must scan through the current objects that are linked to the
    ' particular reference list.
    ' Added ability to handle multiple object-lists in Nov 2010
    For iListOfObjList = IDDField(actField).listOfObjListStart To IDDField(actField).listOfObjListEnd
      If iListOfObjList > 0 Then
        curObjListName = ListOfObjList(iListOfObjList)
        If curObjListName > 0 Then
          Debug.Print "Using Object List: "; IDDObjListName(curObjListName).name
          curObjListItem = IDDObjListName(curObjListName).objListItemStart
          ' first loop through all of the object list items
          Select Case IDDField(actField).autoObjList
            Case autoObjListKindVar
              Call updateAutoObjLists
              comboListString = comboListString & autoObjListVar
            Case autoObjListKindMeter
              Call updateAutoObjLists
              comboListString = comboListString & autoObjListMeter
            Case autoObjListKindVarMeter
              Call updateAutoObjLists
              comboListString = comboListString & autoObjListVar & autoObjListMeter
            Case Else 'all lists but automatic ones
              Do While curObjListItem > 0
                iClass = IDDObjListItem(curObjListItem).classWithRef
                If IDDObjListItem(curObjListItem).fieldWithRef <> fwrClassName Then 'for normal \reference
                  iField = IDDObjListItem(curObjListItem).fieldWithRef
                  Debug.Print "  Scanning through "; IDDClassDat(iClass).name, IDDField(iField).name
                  iObject = IDDClassObjPt(iClass).objectStart
                  deltaField = iField - IDDClassDat(iClass).fieldStart
                  Do While iObject > 0
                    StartVal = IDFObject(iObject).valueStart
                    'the following line actually adds items to the pull down list
                    comboListString = comboListString & IDFValue(StartVal + deltaField).entry & vbTab & "object" & "|"
                    Debug.Print "  Add to listbox: "; IDFValue(StartVal + deltaField).entry
                    iObject = IDFObject(iObject).nextObjectInClass
                  Loop
                Else 'for \reference-class-name
                  comboListString = comboListString & IDDClassDat(iClass).name & vbTab & "object" & "|"
                End If
                curObjListItem = IDDObjListItem(curObjListItem).nextObjListItem
              Loop
          End Select
        End If
      End If
    Next iListOfObjList
    comboListString = comboListString & "<BLANK>"
  Case 6 'node
    If useNodeEditor Then
      comboListString = "..."
    Else
      comboListString = ""
    End If
End Select
grdNew.ComboList = comboListString
End Sub


'-----------------------------------------------------------------------------
' For cells with buttons (\type node only) displays the node select dialog
'-----------------------------------------------------------------------------
Private Sub grdNew_CellButtonClick(ByVal Row As Long, ByVal Col As Long)
Dim cellContents As String
Dim iObject As Long
Dim classIndex As Long
Dim firstValue As Long
Dim firstField As Long
Dim numfields  As Long
Dim valueIndx As Long
Dim fieldIndx As Long
Dim jFld As Long
Dim i As Long
Dim j As Long
'set the current value of the cell
frmNodeSelect.currentNodeName = grdNew.TextMatrix(Row, Col)
'set related information about current cell
frmNodeSelect.currentClassName = IDDClassDat(actClass).name
frmNodeSelect.currentFieldName = IDDField(actField).name
frmNodeSelect.currentObjectName = IDFValue(IDFObject(actObject).valueStart).entry
' populate the public array of node names used by the dialog box
maxNodeNameDialog = 0
For iObject = 1 To maxUsedObject
  If IDFObject(iObject).nextObjectInClass <> isDeleted Then
    classIndex = IDFObject(iObject).classType
    firstValue = IDFObject(iObject).valueStart
    firstField = IDDClassDat(classIndex).fieldStart
    numfields = IDDClassDat(classIndex).fieldEnd - firstField + 1
    For jFld = 1 To numfields
      valueIndx = firstValue + jFld - 1
      fieldIndx = firstField + jFld - 1
      If IDDField(fieldIndx).type = 6 Then  'node
        If IDFValue(valueIndx).entry <> "" Then
          maxNodeNameDialog = maxNodeNameDialog + 1
          Call resizeNodeNameDialogArray(1)
          nodeNameDialog(maxNodeNameDialog).name = IDFValue(valueIndx).entry
          nodeNameDialog(maxNodeNameDialog).objName = IDFValue(firstValue).entry
          nodeNameDialog(maxNodeNameDialog).clsIndx = classIndex
          nodeNameDialog(maxNodeNameDialog).fldIndx = fieldIndx
          nodeNameDialog(maxNodeNameDialog).isUnique = True 'just set all to true for now
          nodeNameDialog(maxNodeNameDialog).isRecent = False 'just set to false for now
        End If
      End If
    Next jFld
  End If
Next iObject
'check for uniqueness
For i = 2 To maxNodeNameDialog
  For j = 1 To i - 1
    If nodeNameDialog(j).name = nodeNameDialog(i).name Then
      nodeNameDialog(i).isUnique = False
      Exit For
    End If
  Next j
Next i
' manage the recently used node name list
Call addToRecentlyUsedNodeNames(grdNew.TextMatrix(Row, Col))
For i = 1 To maxNodeNameDialog
  For j = 1 To UBound(recentlyUsedNodeNames)
    If recentlyUsedNodeNames(j) <> "" Then
      If nodeNameDialog(i).name = recentlyUsedNodeNames(j) Then
        nodeNameDialog(i).isRecent = True
      End If
    End If
  Next j
Next i
' call the dialog box as "modal" so it must be in front
frmNodeSelect.Show vbModal
'perform the comparison with old value that is usually in AfterEdito because it doesn't get called for buttons
cellContents = frmNodeSelect.currentNodeName
If IDFValue(actValue).entry <> Trim(cellContents) Then
  Call ShowFileAltered
  lastValueBeforeEdit = IDFValue(actValue).entry
  lastValueAfterEdit = LTrim(cellContents)
End If
IDFValue(actValue).leadingSpaces = numberOfLeadingSpaces(cellContents)
IDFValue(actValue).entry = LTrim(cellContents)
' set the return value to the value shown
grdNew.TextMatrix(Row, Col) = cellContents
'add new value to recently used
Call addToRecentlyUsedNodeNames(cellContents)
End Sub

Private Sub grdNew_KeyPressEdit(ByVal Row As Long, ByVal Col As Long, KeyAscii As Integer)
If KeyAscii = 13 Then
  'Debug.Print "enter hit"
  lastKeyEnter = True
Else
  lastKeyEnter = False
End If
End Sub

'-----------------------------------------------------------------------------
' When curser is moved over the main grid this routine shows tooltips and
' information bar.
'-----------------------------------------------------------------------------
Private Sub grdNew_MouseMove(Button As Integer, Shift As Integer, x As Single, y As Single)
Static r As Long, c As Long
Dim nr As Long, nc As Long
Dim useUnits As Long
' get coordinates
nr = grdNew.MouseRow
nc = grdNew.MouseCol
' update tooltip text
If c <> nc Or r <> nr Then
  r = nr
  c = nc
  grdNew.ToolTipText = ""
  If nr > 1 And nc > 1 Then
    useUnits = getUnitsForCell(nr, nc, actClass)
    If useUnits <> noUnitsSpecified Then
      If displayUnits = dispUnitSI Then
        grdNew.ToolTipText = "Units: {" & convUnits(useUnits).siName & "} for Obj" & (nc - 1)
      Else
        grdNew.ToolTipText = "Units: {" & convUnits(useUnits).ipName & "} for Obj" & (nc - 1)
      End If
    End If
  End If
End If
End Sub

'-----------------------------------------------------------------------------
' Called when a new cell is selected
'-----------------------------------------------------------------------------
Private Sub grdNew_SelChange()
Call selectCell
End Sub

'-----------------------------------------------------------------------------
' Called when a cell is clicked on
'-----------------------------------------------------------------------------
Private Sub grdNew_Click()
Call selectCell
End Sub

'-----------------------------------------------------------------------------
' After the edit is complete including the validation this is used to
' store the edited value into the array
'-----------------------------------------------------------------------------
Private Sub grdNew_AfterEdit(ByVal Row As Long, ByVal Col As Long)
'check if changed value - and make file altered
Dim cellContents As String, origContents As String
Dim useUnits As Long
Dim curValue As Double
Dim outOfRange As Boolean
Dim curFieldType As Integer
cellContents = grdNew.TextMatrix(Row, Col)
' OLD FIX FOR CR5772 but now handling blank original contents better
'If Trim(cellContents) = "0" Then
'  cellContents = "0.0"
'  grdNew.TextMatrix(Row, Col) = cellContents
'End If
If actCol >= 0 Then
  curFieldType = IDDField(actField).type
  'for compact schedules - treat some fields as numeric if they contain just numbers
  If actClass = classScheduleCompact Then
    If IsNumeric(cellContents) Then
      curFieldType = 1 'if it is a number, treat it like a number (only for ScheduleCompact)
    End If
  End If
  Select Case curFieldType
    Case 1, 2  'this is a real or long
      If Left(LTrim(cellContents), 1) = "=" Then
        IDFValue(actValue).entry = cellContents
        IDFValue(actValue).isExpression = True
      ElseIf LCase(Trim(cellContents)) = "autosize" Then
        If IDFValue(actValue).entry <> Trim(cellContents) Then Call ShowFileAltered
        IDFValue(actValue).entry = cellContents
      ElseIf LCase(Trim(cellContents)) = "autocalculate" Then
        If IDFValue(actValue).entry <> Trim(cellContents) Then Call ShowFileAltered
        IDFValue(actValue).entry = cellContents
      Else
        useUnits = getUnitsForCell(Row, Col, actClass)
        ' determine the original contents of the cell before the edit
        ' first check if it is blank because blank should not run through toIntl since that creates a zero
        ' this fixes CR6915 which was a fix to CR5772 (see lines near top of this routine)
        If Trim(IDFValue(actValue).entry) <> "" Then
          If IsNumeric(IDFValue(actValue).entry) Then
            If displayUnits = dispUnitSI Or useUnits = noUnitsSpecified Then
              origContents = toIntl(IDFValue(actValue).entry, numDigitsShown)
            Else
              origContents = toIntl(Val(IDFValue(actValue).entry) * convUnits(useUnits).mult + convUnits(useUnits).offset, numDigitsShown)
            End If
          Else
            origContents = IDFValue(actValue).entry
          End If
        Else 'keep original as blank so comparison works and gets written back into cell
          origContents = ""
        End If
        'if different then show the file being altered
        Debug.Print " current["; cellContents; "]  orig["; origContents; "]"
        If cellContents <> origContents Then
          Call ShowFileAltered
          If displayUnits = dispUnitSI Or useUnits = noUnitsSpecified Then
            IDFValue(actValue).entry = fromIntl(cellContents)
          Else
            If Trim(cellContents) <> "" Then
                IDFValue(actValue).entry = (Val(fromIntl(cellContents)) - convUnits(useUnits).offset) / convUnits(useUnits).mult
            Else
                IDFValue(actValue).entry = ""
            End If
          End If
        End If
        Debug.Print " final entry ["; IDFValue(actValue).entry; "] after unit conversion"
      End If
      'check if the value entered is within the range allowed
      curValue = Val(IDFValue(actValue).entry)
      outOfRange = False
      If IDFValue(actValue).entry <> "autocalculate" And IDFValue(actValue).entry <> "autosize" Then
        If IDDField(actField).maxSpecified Then
          If IDDField(actField).exclusiveMax Then
            If curValue >= IDDField(actField).maximum Then outOfRange = True
          Else
            If curValue > IDDField(actField).maximum Then outOfRange = True
          End If
        End If
        If IDDField(actField).minSpecified Then
          If IDDField(actField).exclusiveMin Then
            If curValue <= IDDField(actField).minimum Then outOfRange = True
          Else
            If curValue < IDDField(actField).minimum Then outOfRange = True
          End If
        End If
      End If
      If outOfRange Then
        grdNew.Cell(flexcpBackColor, Row, Col) = outOfRangeColor
      Else
        grdNew.Cell(flexcpBackColor, Row, Col) = 0
      End If
    Case 0, 3, 6
      If IDFValue(actValue).entry <> Trim(cellContents) Then
        Call ShowFileAltered
        lastValueBeforeEdit = IDFValue(actValue).entry
        lastValueAfterEdit = LTrim(cellContents)
      End If
      IDFValue(actValue).leadingSpaces = numberOfLeadingSpaces(cellContents)
      IDFValue(actValue).entry = LTrim(cellContents)
    Case 4
      If IDFValue(actValue).entry <> Trim(cellContents) Then
        Call ShowFileAltered
        lastValueBeforeEdit = IDFValue(actValue).entry
        lastValueAfterEdit = LTrim(cellContents)
      End If
      IDFValue(actValue).leadingSpaces = numberOfLeadingSpaces(cellContents)
      IDFValue(actValue).entry = LTrim(cellContents)
      'display error color if choice is not valid
      If Not isChoiceValid(IDFValue(actValue).entry, actField) Then 'call function that determines if the choice is valid
        grdNew.Cell(flexcpBackColor, Row, Col) = outOfRangeColor
      Else
        grdNew.Cell(flexcpBackColor, Row, Col) = 0
      End If
    Case 5
      If IDFValue(actValue).entry <> Trim(cellContents) Then
        Call ShowFileAltered
        lastValueBeforeEdit = IDFValue(actValue).entry
        lastValueAfterEdit = LTrim(cellContents)
      End If
      IDFValue(actValue).leadingSpaces = numberOfLeadingSpaces(cellContents)
      IDFValue(actValue).entry = LTrim(cellContents)
      'display error color if reference is not valid
      If Not isReferenceValid(IDFValue(actValue).entry, actField) Then 'call function that determines if the reference is valid
        grdNew.Cell(flexcpBackColor, Row, Col) = outOfRangeColor
      Else
        grdNew.Cell(flexcpBackColor, Row, Col) = 0
      End If
  End Select
  ' for any type of field, if it is required and blank show that it is out of range
  If IDDField(actField).required And Trim(IDFValue(actValue).entry) = "" And Not IDDField(actField).defSpecified Then
    grdNew.Cell(flexcpBackColor, Row, Col) = outOfRangeColor
  End If
End If
If lastKeyEnter And downWhenEnter Then
  If grdNew.Row + 1 < grdNew.Rows Then
    grdNew.Row = grdNew.Row + 1
    Call selectCell
  End If
End If
If useWordWrap Then grdNew.AutoSize 0, grdNew.Cols - 1
End Sub

'-----------------------------------------------------------------------------
' After the user is done editing this routine is called to see if the entry
' was valid. If not, cancel is set to true and the cell remains in "edit" mode
'-----------------------------------------------------------------------------
Private Sub grdNew_ValidateEdit(ByVal Row As Long, ByVal Col As Long, Cancel As Boolean)
Dim newContents As String
newContents = grdNew.EditText
If actCol >= 0 Then
  Select Case IDDField(actField).type
    Case 1, 2  'this is a real or long
      If Left(LTrim(newContents), 1) = "=" Then
        Cancel = False
      ElseIf LCase(Trim(newContents)) = "autosize" Then
        Cancel = False
      ElseIf LCase(Trim(newContents)) = "autocalculate" Then
        Cancel = False
      ElseIf IsNumeric(newContents) Then
        Cancel = False
      ElseIf newContents = "" Then
        Cancel = False
      Else
        Cancel = True
      End If
    Case Else
      Cancel = False
  End Select
End If
End Sub

'-----------------------------------------------------------------------------
' Resize all of the objects on the main form
'-----------------------------------------------------------------------------
Sub resizeControls(upperSplitLoc As Single, lowerSplitLoc As Single, rightSplitLoc As Single, leftSplitLoc As Single)
Dim newTxtWidth As Long
Dim newLblLeft As Long
Dim formWd As Single
Dim formHt As Single
Dim GapBetControls As Long ' CEPTChange - for distance between combos

On Error Resume Next
formWd = Me.ScaleWidth
formHt = Me.ScaleHeight

GapBetControls = picLeftSplitter.Width ' CEPTChange - for distance between combos

If Me.WindowState <> vbMinimized Then
  Select Case currentFormLayoutOption
    Case floShortShort
      '-----------------------------------------
      'CEPTChange - to accomodate the combo boxes
      '-----------------------------------------
      If ShowQuickSelectCombos = True Then
        cboClassCategories.Visible = True
        cboClasses.Visible = True
        cboClassCategories.Move leftMargin, topMargin, _
           leftSplitLoc - leftMargin
        cboClasses.Move leftMargin, topMargin + cboClasses.Height + GapBetControls, _
           leftSplitLoc - leftMargin
        lstObjectTypes.Move leftMargin, topMargin + cboClasses.Height + cboClassCategories.Height + (2 * GapBetControls), _
           leftSplitLoc - leftMargin, lowerSplitLoc - topMargin - cboClasses.Height - cboClassCategories.Height - (2 * GapBetControls)
      Else
        cboClassCategories.Visible = False
        cboClasses.Visible = False
        ' CEPTChange over--------
        lstObjectTypes.Move leftMargin, topMargin, _
             leftSplitLoc - leftMargin, lowerSplitLoc - topMargin
      End If
      txtComment.Move leftSplitLoc + leftGapWidth, topMargin, _
         formWd - (leftSplitLoc + leftGapWidth + rightMargin), upperSplitLoc - topMargin
      lblComment.Left = leftSplitLoc + leftGapWidth
      txtExplain.Move leftSplitLoc + leftGapWidth, upperSplitLoc + upperGapHeight, _
         formWd - (leftSplitLoc + leftGapWidth + rightMargin), lowerSplitLoc - (upperSplitLoc + upperGapHeight)
      lblExplain.Move leftSplitLoc + leftGapWidth, upperSplitLoc + upperGapHeight - explainLabelHeight, explainLabelWidth, explainLabelHeight
      grdNew.Move leftMargin, lowerSplitLoc + lowerGapHeight, _
         formWd - (leftMargin + rightMargin), formHt - (lowerSplitLoc + lowerGapHeight + bottomMargin)
    Case floShortTall
      '-----------------------------------------
      'CEPTChange - to accomodate the combo boxes
      '-----------------------------------------
      If ShowQuickSelectCombos = True Then
        cboClassCategories.Visible = True
        cboClasses.Visible = True
        cboClassCategories.Move leftMargin, topMargin, _
           leftSplitLoc - leftMargin
        cboClasses.Move leftMargin, topMargin + cboClasses.Height + GapBetControls, _
           leftSplitLoc - leftMargin
        lstObjectTypes.Move leftMargin, topMargin + cboClasses.Height + cboClassCategories.Height + (2 * GapBetControls), _
           leftSplitLoc - leftMargin, lowerSplitLoc - topMargin - cboClasses.Height - cboClassCategories.Height
      Else
        cboClassCategories.Visible = False
        cboClasses.Visible = False
        ' CEPTChange over-------
        lstObjectTypes.Move leftMargin, topMargin, _
           leftSplitLoc - leftMargin, lowerSplitLoc - topMargin
      End If
      txtComment.Move leftSplitLoc + leftGapWidth, topMargin, _
         rightSplitLoc - (leftSplitLoc + leftGapWidth), lowerSplitLoc - topMargin
      lblComment.Left = leftSplitLoc + leftGapWidth
      txtExplain.Move rightSplitLoc + rightGapWidth, topMargin, _
         formWd - (rightSplitLoc + rightGapWidth + rightMargin), lowerSplitLoc - topMargin
      lblExplain.Move rightSplitLoc + rightGapWidth, topMargin - explainLabelHeight, explainLabelWidth, explainLabelHeight
      grdNew.Move leftMargin, lowerSplitLoc + lowerGapHeight, _
         formWd - (leftMargin + rightMargin), formHt - (lowerSplitLoc + lowerGapHeight + bottomMargin)
    Case floTallShort
      '-----------------------------------------
      'CEPTChange - to accomodate the combo boxes
      '-----------------------------------------
      If ShowQuickSelectCombos = True Then
        cboClassCategories.Visible = True
        cboClasses.Visible = True
        cboClassCategories.Move leftMargin, topMargin, _
           leftSplitLoc - leftMargin
        cboClasses.Move leftMargin, topMargin + cboClasses.Height + GapBetControls, _
           leftSplitLoc - leftMargin
        lstObjectTypes.Move leftMargin, topMargin + cboClasses.Height + cboClassCategories.Height + (2 * GapBetControls), _
           leftSplitLoc - leftMargin, formHt - (topMargin + bottomMargin) - cboClasses.Height - cboClassCategories.Height
      Else
        cboClassCategories.Visible = False
        cboClasses.Visible = False
        ' CEPTChange over-------
        lstObjectTypes.Move leftMargin, topMargin, _
           leftSplitLoc - leftMargin, formHt - (topMargin + bottomMargin)
      End If
      txtComment.Move leftSplitLoc + leftGapWidth, topMargin, _
         formWd - (leftSplitLoc + leftGapWidth + rightMargin), upperSplitLoc - topMargin
      lblComment.Left = leftSplitLoc + leftGapWidth
      txtExplain.Move leftSplitLoc + leftGapWidth, upperSplitLoc + upperGapHeight, _
         formWd - (leftSplitLoc + leftGapWidth + rightMargin), lowerSplitLoc - (upperSplitLoc + upperGapHeight)
      lblExplain.Move leftSplitLoc + leftGapWidth, upperSplitLoc + upperGapHeight - explainLabelHeight, explainLabelWidth, explainLabelHeight
      grdNew.Move leftSplitLoc + leftGapWidth, lowerSplitLoc + lowerGapHeight, _
         formWd - (leftSplitLoc + leftGapWidth), formHt - (lowerSplitLoc + lowerGapHeight + bottomMargin)
    Case floTallTall
      '-----------------------------------------
      'CEPTChange - to accomodate the combo boxes
      '-----------------------------------------
      If ShowQuickSelectCombos = True Then
        cboClassCategories.Visible = True
        cboClasses.Visible = True
        cboClassCategories.Move leftMargin, topMargin, _
           leftSplitLoc - leftMargin
        cboClasses.Move leftMargin, topMargin + cboClasses.Height + GapBetControls, _
           leftSplitLoc - leftMargin
        lstObjectTypes.Move leftMargin, topMargin + cboClasses.Height + cboClassCategories.Height + (2 * GapBetControls), _
           leftSplitLoc - leftMargin, formHt - (topMargin + bottomMargin) - cboClasses.Height - cboClassCategories.Height
      Else
        cboClassCategories.Visible = False
        cboClasses.Visible = False
        ' CEPTChange over-------
        lstObjectTypes.Move leftMargin, topMargin, _
           leftSplitLoc - leftMargin, formHt - (topMargin + bottomMargin)
      End If
      txtComment.Move leftSplitLoc + leftGapWidth, topMargin, _
         rightSplitLoc - (leftSplitLoc + leftGapWidth), lowerSplitLoc - topMargin
      lblComment.Left = leftSplitLoc + leftGapWidth
      txtExplain.Move rightSplitLoc + rightGapWidth, topMargin, _
         formWd - (rightSplitLoc + rightGapWidth + rightMargin), lowerSplitLoc - topMargin
      lblExplain.Move rightSplitLoc + rightGapWidth, topMargin - explainLabelHeight, explainLabelWidth, explainLabelHeight
      grdNew.Move leftSplitLoc + leftGapWidth, lowerSplitLoc + lowerGapHeight, _
         formWd - (leftSplitLoc + leftGapWidth), formHt - (lowerSplitLoc + lowerGapHeight + bottomMargin)
  End Select
  imgLowerSplitter.Move leftMargin, lowerSplitLoc, formWd - (leftMargin + rightMargin), lowerGapHeight
  imgUpperSplitter.Move leftMargin, upperSplitLoc, formWd - (leftMargin + rightMargin), upperGapHeight
  imgRightSplitter.Move rightSplitLoc, topMargin, rightGapWidth, formWd - (topMargin + bottomMargin)
  imgLeftSplitter.Move leftSplitLoc, topMargin, leftGapWidth, formWd - (topMargin + bottomMargin)
End If
End Sub

'-----------------------------------------------------------------------------
'  Management of Lower Splitter
'-----------------------------------------------------------------------------
Private Sub imgLowerSplitter_MouseDown(Button As Integer, Shift As Integer, x As Single, y As Single)
With imgLowerSplitter
  picLowerSplitter.Move .Left, .Top, .Width, 80
End With
picLowerSplitter.Visible = True
lowerMoving = True
End Sub
Private Sub imgLowerSplitter_MouseUp(Button As Integer, Shift As Integer, x As Single, y As Single)
Call resizeControls(picUpperSplitter.Top, picLowerSplitter.Top, picRightSplitter.Left, picLeftSplitter.Left)
picLowerSplitter.Visible = False
lowerMoving = False
End Sub
Private Sub imgLowerSplitter_MouseMove(Button As Integer, Shift As Integer, x As Single, y As Single)
Dim splitPos As Single
Dim minLimit As Single
Dim maxLimit As Single
If lowerMoving Then
  splitPos = y + imgLowerSplitter.Top
  Select Case currentFormLayoutOption
    Case floShortShort, floTallShort
      minLimit = picUpperSplitter.Top + upperGapHeight + minExplainHeight + lowerGapHeight
      maxLimit = Me.Height - (bottomMargin + minGridHeight)
    Case floShortTall, floTallTall
      minLimit = topMargin + minCommentHeight + lowerGapHeight
      maxLimit = Me.Height - (bottomMargin + minGridHeight)
  End Select
  If splitPos < minLimit Then splitPos = minLimit
  If splitPos > maxLimit Then splitPos = maxLimit
  picLowerSplitter.Top = splitPos
End If
End Sub

'-----------------------------------------------------------------------------
'  Management of Upper Splitter
'-----------------------------------------------------------------------------
Private Sub imgUpperSplitter_MouseDown(Button As Integer, Shift As Integer, x As Single, y As Single)
With imgUpperSplitter
  picUpperSplitter.Move .Left, .Top, .Width, 80
End With
picUpperSplitter.Visible = True
upperMoving = True
End Sub
Private Sub imgUpperSplitter_MouseUp(Button As Integer, Shift As Integer, x As Single, y As Single)
Call resizeControls(picUpperSplitter.Top, picLowerSplitter.Top, picRightSplitter.Left, picLeftSplitter.Left)
picUpperSplitter.Visible = False
upperMoving = False
End Sub
Private Sub imgUpperSplitter_MouseMove(Button As Integer, Shift As Integer, x As Single, y As Single)
Dim splitPos As Single
Dim minLimit As Single
Dim maxLimit As Single
If upperMoving Then
  splitPos = y + imgUpperSplitter.Top
  Select Case currentFormLayoutOption
    Case floShortShort, floTallShort
      minLimit = topMargin + minCommentHeight
      maxLimit = picLowerSplitter.Top - (minExplainHeight + upperGapHeight)
    Case floShortTall, floTallTall
      'in this mode the upper splitter is not used.
  End Select
  If splitPos < minLimit Then splitPos = minLimit
  If splitPos > maxLimit Then splitPos = maxLimit
  picUpperSplitter.Top = splitPos
End If
End Sub

'-----------------------------------------------------------------------------
'  Management of Left Splitter
'-----------------------------------------------------------------------------
Private Sub imgLeftSplitter_MouseDown(Button As Integer, Shift As Integer, x As Single, y As Single)
With imgLeftSplitter
  picLeftSplitter.Move .Left, .Top, 80, .Height
End With
picLeftSplitter.Visible = True
leftMoving = True
End Sub
Private Sub imgLeftSplitter_MouseUp(Button As Integer, Shift As Integer, x As Single, y As Single)
Call resizeControls(picUpperSplitter.Top, picLowerSplitter.Top, picRightSplitter.Left, picLeftSplitter.Left)
picLeftSplitter.Visible = False
leftMoving = False
End Sub
Private Sub imgLeftSplitter_MouseMove(Button As Integer, Shift As Integer, x As Single, y As Single)
Dim splitPos As Single
Dim minLimit As Single
Dim maxLimit As Single
If leftMoving Then
  splitPos = x + imgLeftSplitter.Left
  Select Case currentFormLayoutOption
    Case floShortShort, floTallShort
      minLimit = leftMargin + minListWidth
      maxLimit = Me.Width - (minCommentWidth + rightMargin)
    Case floShortTall, floTallTall
      minLimit = leftMargin + minListWidth
      maxLimit = picRightSplitter.Left - (minCommentWidth + rightGapWidth)
  End Select
  If splitPos < minLimit Then splitPos = minLimit
  If splitPos > maxLimit Then splitPos = maxLimit
  picLeftSplitter.Left = splitPos
End If
End Sub

'-----------------------------------------------------------------------------
'  Management of Right Splitter
'-----------------------------------------------------------------------------
Private Sub imgRightSplitter_MouseDown(Button As Integer, Shift As Integer, x As Single, y As Single)
With imgRightSplitter
  picRightSplitter.Move .Left, .Top, 80, .Height
End With
picRightSplitter.Visible = True
rightMoving = True
End Sub
Private Sub imgRightSplitter_MouseUp(Button As Integer, Shift As Integer, x As Single, y As Single)
Call resizeControls(picUpperSplitter.Top, picLowerSplitter.Top, picRightSplitter.Left, picLeftSplitter.Left)
picRightSplitter.Visible = False
rightMoving = False
End Sub
Private Sub imgRightSplitter_MouseMove(Button As Integer, Shift As Integer, x As Single, y As Single)
Dim splitPos As Single
Dim minLimit As Single
Dim maxLimit As Single
If rightMoving Then
  splitPos = x + imgRightSplitter.Left
  Select Case currentFormLayoutOption
    Case floShortShort, floTallShort
      'in this mode the right splitter is not used.
    Case floShortTall, floTallTall
      minLimit = picLeftSplitter.Left + leftGapWidth + minCommentWidth
      maxLimit = Me.Width - (minExplainWidth + rightMargin)
  End Select
  If splitPos < minLimit Then splitPos = minLimit
  If splitPos > maxLimit Then splitPos = maxLimit
  picRightSplitter.Left = splitPos
End If
End Sub


'-----------------------------------------------------------------------------
' Clicking on the Oject Types List Selects the Class and Displays the Objects
'-----------------------------------------------------------------------------
Private Sub lstObjectTypes_Click()
Dim found As Long
Dim i As Long
curIDDClass = lstObjectTypes.ItemData(lstObjectTypes.ListIndex)
actClass = curIDDClass
If curIDDClass > 0 Then
  txtComment.Text = ""
  Call FillGrid
Else
  Select Case lstObjectHeading
    Case 0  'just clicked on group name - i.e. no heading up or down
      lstObjectTypes.ListIndex = lstObjectLast
    Case vbKeyUp
      found = 0
      For i = lstObjectTypes.ListIndex - 1 To 0 Step -1
        If lstObjectTypes.ItemData(i) > 0 Then
          found = i
          Exit For
        End If
      Next i
      If found > 0 Then
        lstObjectTypes.ListIndex = found
      Else
        lstObjectTypes.ListIndex = lstObjectLast
      End If
    Case vbKeyDown
      found = 0
      For i = lstObjectTypes.ListIndex + 1 To lstObjectTypes.ListCount - 1
        If lstObjectTypes.ItemData(i) > 0 Then
          found = i
          Exit For
        End If
      Next i
      If found > 0 Then
        lstObjectTypes.ListIndex = found
      Else
        lstObjectTypes.ListIndex = lstObjectLast
      End If
    Case vbKeyHome
      found = 0
      For i = 0 To lstObjectTypes.ListCount - 1
        If lstObjectTypes.ItemData(i) > 0 Then
          found = i
          Exit For
        End If
      Next i
      If found > 0 Then
        lstObjectTypes.ListIndex = found
      Else
        lstObjectTypes.ListIndex = lstObjectLast
      End If
  End Select
End If
lstObjectLast = lstObjectTypes.ListIndex
If IDDClassObjPt(actClass).objectCount > 0 Then
  grdNew.Row = 1
  grdNew.Col = 2
  grdNew.TopRow = 1
  grdNew.LeftCol = 2
  Call selectCell
  cmdDuplicateObject.Enabled = True
  cmdDuplicateObjectAndChange.Enabled = True
  cmdDeleteObject.Enabled = True
  cmdCopyObject.Enabled = True
  mnuEditDelete.Enabled = True
  mnuEditDuplicate.Enabled = True
  mnuEditDuplicateAndChange.Enabled = True
  mnuEditCopy.Enabled = True
Else
  If IDDClassDat(actClass).memo <> "" Then
    txtExplain.Text = IDDClassDat(actClass).memo
  Else
    txtExplain.Text = ""
  End If
  cmdDuplicateObject.Enabled = False
  cmdDuplicateObjectAndChange.Enabled = False
  cmdDeleteObject.Enabled = False
  cmdCopyObject.Enabled = False
  mnuEditDelete.Enabled = False
  mnuEditDuplicate.Enabled = False
  mnuEditDuplicateAndChange.Enabled = False
  mnuEditCopy.Enabled = False
End If
End Sub

'-----------------------------------------------------------------------------
' Catch the up and down arrows to move down the list over group titles
'-----------------------------------------------------------------------------
Private Sub lstObjectTypes_KeyDown(KeyCode As Integer, Shift As Integer)
If KeyCode = vbKeyUp Then
  lstObjectHeading = vbKeyUp
ElseIf KeyCode = vbKeyDown Then
  lstObjectHeading = vbKeyDown
ElseIf KeyCode = vbKeyHome Then
  lstObjectHeading = vbKeyHome
Else
  lstObjectHeading = 0
End If
End Sub




Private Sub mainTimer_Timer()
'Dim curWindowHandle As Long
'Dim prevWindowHandle As Long
'b = parentMDI.hWnd
'curWindowHandle = GetForegroundWindow()
'If curWindowHandle <> prevWindowHandle Then
'  If curWindowHandle = mainWindowHandle Then
'    parentmdi.ActiveForm.
'
'if mainWindowHandle =a
'lblObjectList.Caption = a
End Sub

Private Sub mnuCreateAllObjectIDF_Click()
Call dumpAllObjectIDF
MsgBox "AllObject.idf file created in program directory.", vbInformation, "Create AllObject.idf"
End Sub

Private Sub mnuCreateFieldsMissingUnits_Click()
Call dumpFieldsMissingUnits
MsgBox "FieldsMissingUnits.txt file created in program directory.", vbInformation, "Create FieldsMissingUnits.txt"
End Sub

Private Sub mnuCreateObjectList_Click()
Call dumpObjectList
MsgBox "ObjectList.txt file created in program directory.", vbInformation, "Create ObjectList.txt"
End Sub

Private Sub mnuCreateRefObjListTxt_Click()
Call dumpRefObjList
MsgBox "RefObjList.txt file created in program directory.", vbInformation, "Create RefObjList.txt"
End Sub


Private Sub mnuEditFillRight_Click()
Call fillGridToRight
End Sub

Private Sub mnuEditSearch_Click()
Set frmSearch.curFormData = Me
frmSearch.useAsReplaceField = ""
frmSearch.useAsSearchField = ""
If IDFValue(actValue).entry = lastValueAfterEdit Then
  frmSearch.useAsReplaceField = lastValueAfterEdit
  frmSearch.useAsSearchField = lastValueBeforeEdit
ElseIf IDFValue(actValue).entry <> "" Then
  frmSearch.useAsSearchField = IDFValue(actValue).entry
End If
frmSearch.Show vbModal
End Sub



'-----------------------------------------------------------------------------
'  FILE MENU STUBS
'-----------------------------------------------------------------------------
Private Sub mnuFileExit_Click()
Unload frmParent
End Sub

Private Sub mnuFileNew_Click()
Call frmParent.doNewDocument
End Sub
Private Sub mnuFileOpen_Click()
Call frmParent.doOpenDocument
End Sub
Private Sub mnuFileClose_Click()
Dim needToSave As Boolean
needToSave = IsFileChanged()
If needToSave Then
  Exit Sub
Else
  Unload Me
End If
End Sub
Private Sub mnuFilePostProc_Click()
Call SelectPostProcessor
End Sub

Private Sub mnuFileOpenDataSetSub_Click(Index As Integer)
Call frmParent.doOpenDocument(dataSetPath & mnuFileOpenDataSetSub(Index).Caption)
End Sub
Private Sub mnuFileRecent_Click(Index As Integer)
Call frmParent.doOpenDocument(recentFiles(Index).nameWithPath)
End Sub
Private Sub mnuFileSave_Click()
Call doSaveIDFFile(False)
End Sub
Private Sub mnuFileSaveAs_Click()
Call doSaveIDFFile(True)
End Sub
Private Sub mnuFileBatch_Click()
Call SelectRunBatchFile
End Sub
Private Sub mnuFileWeather_Click()
Call SelectWeatherFile
End Sub
Private Sub mnuFileSaveOption_Click()
Dim oldSaveOrderOption As Integer
Dim oldSpecialFormatOption As Integer
oldSaveOrderOption = saveOrderOption
oldSpecialFormatOption = specialFormatOption
saveOption.activeSaveOrderOpt = saveOrderOption
saveOption.activeSpecialFormatOpt = specialFormatOption
saveOption.Show vbModal
saveOrderOption = saveOption.activeSaveOrderOpt
specialFormatOption = saveOption.activeSpecialFormatOpt
If saveOrderOption <> oldSaveOrderOption Or specialFormatOption <> oldSpecialFormatOption Then
  Call ShowFileAltered
End If
End Sub


'-----------------------------------------------------------------------------
' EDIT MENU STUBS
'-----------------------------------------------------------------------------
Private Sub mnuEditDelete_Click()
Call IDFDeleteObject
End Sub
Private Sub mnuEditDuplicate_Click()
Call IDFDuplicateObject
End Sub
Private Sub mnuEditDuplicateAndChange_Click()
Call IDFDuplicateObjectAndChange
End Sub
Private Sub mnuEditNew_Click()
Dim li As Long
li = lstObjectTypes.ListIndex
Call IDFNewObject
Call FillList
Call FillGrid
If li > lstObjectTypes.ListCount - 1 Then
  li = 0
End If
lstObjectTypes.ListIndex = li
grdNew.Col = grdNew.Cols - 1
grdNew.ShowCell grdNew.TopRow, grdNew.Cols - 1
Call selectCell
Call ShowFileAltered
End Sub
Private Sub mnuEditNextRowAfterEnter_Click()
If mnuEditNextRowAfterEnter.Checked Then
  mnuEditNextRowAfterEnter.Checked = False
  downWhenEnter = False
Else
  mnuEditNextRowAfterEnter.Checked = True
  downWhenEnter = True
End If
End Sub
Private Sub mnuEditCopy_Click()
Call doCopyObject
End Sub
Private Sub mnuEditCopySpread_Click()
Call doCopyForSpreadsheet
End Sub
Private Sub mnuEditPaste_Click()
Call doPasteObject
End Sub
'Find menu items
Private Sub mnuEditFind_Click()
frmFind.Show vbModal
If searchTerm <> "" Then
  Call findSearchTerm(findNext)
End If
End Sub
Private Sub mnuEditFindNext_Click()
Call findSearchTerm(findNext)
End Sub
Private Sub mnuEditFindPrevious_Click()
Call findSearchTerm(findPrev)
End Sub


'-----------------------------------------------------------------------------
' HELP MENU STUBS
'-----------------------------------------------------------------------------
Private Sub mnuHelpAbout_Click()
About.Show
End Sub
Private Sub mnuCreateRangeTestFiles_Click()
Call CreateRangeTestFiles
End Sub



' References to EnergyPlus Documentation
Private Sub mnuHelpGettingStarted_Click()
Call startAcrobat("GettingStarted.pdf")
End Sub
Private Sub mnuHelpIORef_Click()
Call startAcrobat("InputOutputReference.pdf")
End Sub
Private Sub mnuHelpEngRef_Click()
Call startAcrobat("EngineeringReference.pdf")
End Sub
Private Sub mnuHelpOutDetails_Click()
Call startAcrobat("OutputDetailsAndExamples.pdf")
End Sub
Private Sub mnuHelpAuxProgs_Click()
'Call startAcrobat("AuxiliaryPrograms.pdf")
Call viewWebPage("https://energyplus.readthedocs.io/en/latest/auxiliary-programs/auxiliary-programs.html")
End Sub
Private Sub mnuHelpAcknowledge_Click()
'Call startAcrobat("Acknowledgments.pdf")
Call viewWebPage("https://energyplus.readthedocs.io/en/latest/acknowledgments/acknowledgments.html")
End Sub
Private Sub mnuHelpCompliance_Click()
Call startAcrobat("UsingEnergyPlusForCompliance.pdf")
End Sub
Private Sub mnuHelpEMSguide_Click()
'Call startAcrobat("EMSApplicationGuide.pdf")
Call viewWebPage("https://energyplus.readthedocs.io/en/latest/ems-application-guide/ems-application-guide.html")
End Sub
Private Sub mnuHelpExtInterface_Click()
Call startAcrobat("ExternalInterfacesApplicationGuide.pdf")
End Sub
Private Sub mnuHelpQuickStart_Click()
Call viewWebPage("https://energyplus.readthedocs.io/en/latest/quick_start/quick_start.html")
End Sub
Private Sub mnuHelpEssentials_Click()
'Call startAcrobat("EnergyPlusEssentials.pdf")
Call viewWebPage("https://energyplus.readthedocs.io/en/latest/essentials/essentials.html")
End Sub
Private Sub mnuHelpPlant_Click()
Call startAcrobat("PlantApplicationGuide.pdf")
End Sub
Private Sub mnuHelpTips_Click()
Call viewWebPage("https://energyplus.readthedocs.io/en/latest/tips_and_tricks/tips_and_tricks.html")
End Sub




Private Sub mnuHelpWhatsNew_Click()
frmWhatsNew.Show
End Sub




Private Sub mnuJumpItem_Click(Index As Integer)
If mnuJumpItem(Index).Caption <> "No destination" Then
  Call jumpToObjectField(jmpClass(Index), jmpObj(Index), jmpFld(Index))
End If
End Sub

Private Sub mnuJumpTop_Click()
'mnuJumpItem(0).Caption = "This is the first item"
'Load mnuJumpItem(1)
'Load mnuJumpItem(2)
'Load mnuJumpItem(3)
'Load mnuJumpItem(4)
'mnuJumpItem(1).Caption = "This is the second item"
'mnuJumpItem(2).Caption = "This is the third item"
'mnuJumpItem(3).Caption = "This is the fourth item"
'mnuJumpItem(4).Caption = "This is the fifth item"
End Sub

'--------------------------------------------------
' CEPTChange - Show/Hide quick select comboboxes
'--------------------------------------------------
Private Sub mnuQuickSelect_Click()
mnuQuickSelect.Checked = Not mnuQuickSelect.Checked
ShowQuickSelectCombos = mnuQuickSelect.Checked
prevQuickSelectStatus = mnuQuickSelect.Checked
cboClassCategories.Text = ""
cboClasses.Text = ""
cboClasses.Clear
Call resizeControls(picUpperSplitter.Top, picLowerSplitter.Top, picRightSplitter.Left, picLeftSplitter.Left)
If ShowQuickSelectCombos Then cboClassCategories.SetFocus
End Sub

'-----------------------------------------------------------------------------
' VIEW MENU STUBS
'-----------------------------------------------------------------------------
Private Sub mnuViewNarrowColumn_Click()
Call clearViewWidthMenus
mnuViewNarrowColumn.Checked = True
defaultColumnWidth = 1000
numDigitsShown = 10
Call FillGrid
End Sub
Private Sub mnuViewMediumColumn_Click()
Call clearViewWidthMenus
mnuViewMediumColumn.Checked = True
defaultColumnWidth = 1500
numDigitsShown = 14
Call FillGrid
End Sub

Private Sub mnuViewWideColumn_Click()
Call clearViewWidthMenus
mnuViewWideColumn.Checked = True
defaultColumnWidth = 2000
numDigitsShown = 18
Call FillGrid
End Sub
Private Sub mnuViewWidth30_Click()
Call clearViewWidthMenus
mnuViewWidth30.Checked = True
defaultColumnWidth = 3000
numDigitsShown = 18
Call FillGrid
End Sub
Private Sub mnuViewWidth40_Click()
Call clearViewWidthMenus
mnuViewWidth40.Checked = True
defaultColumnWidth = 4000
numDigitsShown = 18
Call FillGrid
End Sub
Private Sub mnuViewWidth50_Click()
Call clearViewWidthMenus
mnuViewWidth50.Checked = True
defaultColumnWidth = 5000
numDigitsShown = 18
Call FillGrid
End Sub
Private Sub mnuViewWidth60_Click()
Call clearViewWidthMenus
mnuViewWidth60.Checked = True
defaultColumnWidth = 6000
numDigitsShown = 18
Call FillGrid
End Sub
Private Sub mnuViewWidth70_Click()
Call clearViewWidthMenus
mnuViewWidth70.Checked = True
defaultColumnWidth = 7000
numDigitsShown = 18
Call FillGrid
End Sub
Private Sub mnuViewWidth80_Click()
Call clearViewWidthMenus
mnuViewWidth80.Checked = True
defaultColumnWidth = 8000
numDigitsShown = 18
Call FillGrid
End Sub
Private Sub mnuViewWidth90_Click()
Call clearViewWidthMenus
mnuViewWidth90.Checked = True
defaultColumnWidth = 9000
numDigitsShown = 18
Call FillGrid
End Sub

Sub clearViewWidthMenus()
mnuViewNarrowColumn.Checked = False
mnuViewMediumColumn.Checked = False
mnuViewWideColumn.Checked = False
mnuViewWidth30.Checked = False
mnuViewWidth40.Checked = False
mnuViewWidth50.Checked = False
mnuViewWidth60.Checked = False
mnuViewWidth70.Checked = False
mnuViewWidth80.Checked = False
mnuViewWidth90.Checked = False
End Sub

Private Sub mnuViewIP_Click()
Call SetDisplayUnits(dispUnitIP)
Call FillGrid
End Sub
Private Sub mnuViewSI_Click()
Call SetDisplayUnits(dispUnitSI)
Call FillGrid
End Sub

Sub SetDisplayUnits(unitsIn As Integer)
'only make change if not in that mode already
If unitsIn <> displayUnits Then
  Select Case unitsIn
    Case dispUnitIP
      mnuViewIP.Checked = True
      mnuViewSi.Checked = False
      displayUnits = dispUnitIP
    Case dispUnitSI
      mnuViewIP.Checked = False
      mnuViewSi.Checked = True
      displayUnits = dispUnitSI
    Case Else
      MsgBox "ERROR! " & vbCrLf & vbCrLf & "SetDisplayUnits to invalid option:" & unitsIn, vbInformation, "IP Unit Conversion"
  End Select
End If
End Sub

Public Sub mnuViewClassesWithObjs_Click()
Dim cntObjUsed As Long
Dim prevSelectedItem As Long
Dim found As Long
Dim i As Long
'save the currently selected item based on itemdata pointer to array
prevSelectedItem = lstObjectTypes.ItemData(lstObjectTypes.ListIndex)
'count the number of objects used
For i = 1 To maxUsedIDDClass
  cntObjUsed = cntObjUsed + IDDClassObjPt(i).objectCount
Next i
'if more than zero objects used then flip to other mode of viewing list
If cntObjUsed = 0 Then
  mnuViewClassesWithObjs.Checked = False
  showAllClasses = True
Else
  If mnuViewClassesWithObjs.Checked = True Then
    mnuViewClassesWithObjs.Checked = False
    showAllClasses = True
  Else
    mnuViewClassesWithObjs.Checked = True
    showAllClasses = False
  End If
End If
'call the main display routine
Call FillList
'now search for the previously selected item in the list
found = -1
For i = 0 To lstObjectTypes.ListCount - 1
  If lstObjectTypes.ItemData(i) = prevSelectedItem Then
    found = i
    Exit For
  End If
Next i
If found >= 0 Then
  lstObjectTypes.ListIndex = found
End If
'------------------------------------------------------
' CEPTChange - Hide/Show Autocomplete Combo boxes if visible
'------------------------------------------------------
If mnuViewClassesWithObjs.Checked = True Then
    ' if only classes with objects being shown, hide the QuickSelectCombos
    If ShowQuickSelectCombos Then
      ShowQuickSelectCombos = False
      mnuQuickSelect.Checked = False
      cboClassCategories.Text = ""
      cboClasses.Text = ""
      cboClasses.Clear
      Call resizeControls(picUpperSplitter.Top, picLowerSplitter.Top, picRightSplitter.Left, picLeftSplitter.Left)
      prevQuickSelectStatus = True ' set the flag to restore quickselect combos when all classes are shown
    End If
Else
    If prevQuickSelectStatus = True Then ' restore comboboxes if previously visible
      ShowQuickSelectCombos = True
      mnuQuickSelect.Checked = True
      Call resizeControls(picUpperSplitter.Top, picLowerSplitter.Top, picRightSplitter.Left, picLeftSplitter.Left)
    End If
End If
'-----------------------------------------
' CEPTChange over
'-----------------------------------------
End Sub

Private Sub mnuViewLayoutOptions_Click()
formLayoutOption = currentFormLayoutOption
LayoutOption.Show vbModal
currentFormLayoutOption = formLayoutOption
Call resizeControls(picUpperSplitter.Top, picLowerSplitter.Top, picRightSplitter.Left, picLeftSplitter.Left)
End Sub


Private Sub mnuViewUseNodeEditor_Click()
If mnuViewUseNodeEditor.Checked Then
  useNodeEditor = False
  mnuViewUseNodeEditor.Checked = False
Else
  useNodeEditor = True
  mnuViewUseNodeEditor.Checked = True
End If
If IDDField(actField).type = 6 Then 'node
  If useNodeEditor Then
    grdNew.ComboList = "..."
  Else
    grdNew.ComboList = ""
  End If
End If
End Sub

Private Sub mnuViewWordWrap_Click()
If mnuViewWordWrap.Checked Then
  useWordWrap = False
  mnuViewWordWrap.Checked = False
  grdNew.WordWrap = False
Else
  useWordWrap = True
  mnuViewWordWrap.Checked = True
  grdNew.WordWrap = True
End If
grdNew.AutoSize 0, grdNew.Cols - 1
End Sub

Private Sub mnuViewValidityCheck_Click()
Set frmErrorCheck.curFormData = Me
frmErrorCheck.reportWhenNoneFound = True
frmErrorCheck.Show vbModal
End Sub

'-----------------------------------------------------------------------------
' WINDOW MENU STUBS
'-----------------------------------------------------------------------------
Private Sub mnuWindowArrange_Click()
frmParent.Arrange vbArrangeIcons
End Sub
Private Sub mnuWindowCascade_Click()
frmParent.Arrange vbCascade
End Sub
Private Sub mnuWindowTileHoriz_Click()
frmParent.Arrange vbTileHorizontal
End Sub
Private Sub mnuWindowTileVert_Click()
frmParent.Arrange vbTileVertical
End Sub

Private Sub timerJumpList_Timer()
If newCellSelected Then
  Call updateJumpList
  newCellSelected = False
End If
End Sub

'-----------------------------------------------------------------------------
'  TOOLBAR ROUTINE
'-----------------------------------------------------------------------------
Private Sub Toolbar1_ButtonClick(ByVal Button As MSComctlLib.Button)
Dim needToSave As Long
Select Case Button.Key
  Case "newIDF"
    Call frmParent.doNewDocument
  Case "openIDF"
    Call frmParent.doOpenDocument
  Case "saveIDF"
    Call doSaveIDFFile(False)
  Case "printIDF"
  Case "cut"
  Case "copy"
  Case "paste"
End Select
End Sub
Private Sub cmdDeleteObject_Click()
Call IDFDeleteObject
End Sub
Private Sub cmdDuplicateObject_Click()
Call IDFDuplicateObject
End Sub
Private Sub cmdDuplicateObjectAndChange_Click()
Call IDFDuplicateObjectAndChange
End Sub
Private Sub cmdNewObject_Click()
Dim li As Long
li = lstObjectTypes.ListIndex
Call IDFNewObject
Call FillList
Call FillGrid
If li > lstObjectTypes.ListCount - 1 Then
  li = 0
End If
lstObjectTypes.ListIndex = li
Call lstObjectTypes_Click
grdNew.Col = grdNew.Cols - 1
grdNew.ColWidth(grdNew.Col) = defaultColumnWidth
grdNew.ShowCell grdNew.TopRow, grdNew.Cols - 1
Call selectCell
Call ShowFileAltered
End Sub
Private Sub cmdCopyObject_Click()
Call doCopyObject
End Sub
Private Sub cmdPasteObject_Click()
Call doPasteObject
End Sub


' ---------- Interface Oriented Support Routines ----------

'-----------------------------------------------------------------------------
' Fills the Object List
'-----------------------------------------------------------------------------
Sub FillList()
Dim i As Long, j As Long, t As String
Dim topItem As Long
Dim cntObjUsed As Long
Dim curShowAllClasses As Boolean
topItem = lstObjectTypes.TopIndex
lstObjectTypes.Visible = False  'to add speed to filling the list
lstObjectTypes.Clear
cboClassCategories.Clear ' CEPTChange: Clear combo
txtComment.Text = ""
'check if any objects are used and if not do not allow
'the showAllClasses = false to be activated
curShowAllClasses = showAllClasses
If Not showAllClasses Then
  For i = 1 To maxUsedIDDClass
    cntObjUsed = cntObjUsed + IDDClassObjPt(i).objectCount
  Next i
  If cntObjUsed = 0 Then
    showAllClasses = True
    mnuViewClassesWithObjs.Checked = False
    curShowAllClasses = True
  End If
End If
For i = 1 To maxUsedClassGroup
  If curShowAllClasses Then
    If i > 1 Then lstObjectTypes.AddItem ""
    lstObjectTypes.AddItem IDDClassGroup(i).name
    cboClassCategories.AddItem IDDClassGroup(i).name ' CEPTChange: Added to fill categories combo
    ' CEPTChange: Removed "If ShowQuickSelectCombos Then" from above to fill combo on all cases
    ' CEPTChange: only show and hide controlled by menu option
    lstObjectTypes.AddItem "---------------------------"
  End If
  For j = IDDClassGroup(i).classStart To IDDClassGroup(i + 1).classStart - 1
    'Debug.Print "Object Count", IDDClassdat(j).name, iddclassdat(j).objectCount
    If curShowAllClasses Then
      If IDDClassObjPt(j).objectCount > 0 Then
        t = Right("0000" & RTrim(LTrim(Str(IDDClassObjPt(j).objectCount))), 4)
      Else
        t = "------"
      End If
      lstObjectTypes.AddItem "[" & t & "]  " & IDDClassDat(j).name
      lstObjectTypes.ItemData(lstObjectTypes.NewIndex) = j
      IDDClassObjPt(j).lstObjIndx = lstObjectTypes.NewIndex
    Else
      If IDDClassObjPt(j).objectCount > 0 Then
        t = Right("0000" & RTrim(LTrim(Str(IDDClassObjPt(j).objectCount))), 4)
        lstObjectTypes.AddItem "[" & t & "]  " & IDDClassDat(j).name
        lstObjectTypes.ItemData(lstObjectTypes.NewIndex) = j
        IDDClassObjPt(j).lstObjIndx = lstObjectTypes.NewIndex
      End If
    End If
  Next j
Next i
If curShowAllClasses Then
  lstObjectLast = 2
  lstObjectTypes.ListIndex = 2
  lstObjectTypes.TopIndex = topItem 'keep the top item shown the same after the list has been refreshed
Else
  lstObjectLast = 0
  lstObjectTypes.ListIndex = 0
End If
lstObjectTypes.Visible = True 'now show it again complete
End Sub

'-----------------------------------------------------------------------------
'DUMP FIELDS MISSING UNITS
'-----------------------------------------------------------------------------
Sub dumpFieldsMissingUnits()
Dim iClass As Long
Dim jField As Long
Dim fn As Integer
fn = FreeFile
Open App.Path & "\FieldsMissingUnits.txt" For Output As fn
For iClass = 1 To maxUsedIDDClass
  For jField = IDDClassDat(iClass).fieldStart To IDDClassDat(iClass).fieldEnd
    If IDDField(jField).type = 1 Then
      If IDDField(jField).Units = "" Then
        Print #fn, IDDClassDat(iClass).name & ","; Tab(60); IDDField(jField).name
      End If
    End If
  Next jField
  'Print #fn, "" 'leave blank line between objects
Next iClass
Close fn
End Sub

'-----------------------------------------------------------------------------
'DUMP OBJECT TYPES
'-----------------------------------------------------------------------------
Sub dumpObjectList()
Dim i As Integer
Dim fn As Integer
fn = FreeFile
Open App.Path & "\ObjectList.txt" For Output As fn
For i = 0 To lstObjectTypes.ListCount - 1
  Print #fn, lstObjectTypes.List(i)
Next i
Close fn
End Sub

Sub dumpRefObjList()
Dim i As Integer
Dim fn As Integer
fn = FreeFile
Open App.Path & "\RefObjList.txt" For Output As fn
For i = 1 To maxUsedObjListName
  Print #fn, IDDObjListName(i).name

Next i
Close fn
End Sub

'-----------------------------------------------------------------------------
'DUMP OBJECT IDF FILE
' File used for debugging only and has no reasonable values in each field
'-----------------------------------------------------------------------------
Sub dumpAllObjectIDF()
Dim iClass As Long
Dim jField As Long
Dim fn As Integer
fn = FreeFile
Open App.Path & "\AllObject.idf" For Output As fn
For iClass = 1 To maxUsedIDDClass
  Print #fn, IDDClassDat(iClass).name & ","  'print the class name - left justified
  For jField = IDDClassDat(iClass).fieldStart To IDDClassDat(iClass).fieldEnd
    Print #fn, Tab(5); Trim(Str(Rnd + 1));
    If jField < IDDClassDat(iClass).fieldEnd Then   'follow every entry with a comma unless last entry than use semicolon
      Print #fn, ",";
    Else
      Print #fn, ";";
    End If
    Print #fn, Tab(20); "!- "; IDDField(jField).name;
    If IDDField(jField).Units <> "" Then
      Print #fn, " {"; IDDField(jField).Units; "}"  'include units if not blank
    Else
      Print #fn,  'just include the carriage return
    End If
  Next jField
  Print #fn, "" 'leave blank line between objects
Next iClass
Close fn
End Sub

'-----------------------------------------------------------------------------
'  Routine to Fill the Main Grid on Interface
'-----------------------------------------------------------------------------
Sub FillGrid()
Dim i As Long, j As Long
Dim numfields As Long, firstField As Long
Dim curObject As Long 'current object being displayed (each object in different column)
Dim curField As Long ' pointer to the current item in IDDField array
Dim StartVal As Long ' the first value pointed to by the current object being displayed
Dim useUnits As Long 'pointer to the convUnits array from the IDDfield array
Dim curValue As Double
Dim curAN As Integer
Dim outOfRange As Boolean
Dim minimumVal As Double, maximumVal As Double
grdNew.Clear
'headings
grdNew.TextMatrix(0, 0) = "Field"
grdNew.TextMatrix(0, 1) = "Units"
'set some general field level variables
firstField = IDDClassDat(curIDDClass).fieldStart
numfields = IDDClassDat(curIDDClass).fieldEnd - firstField + 1
'define the size of the grid for the particular class
grdNew.Rows = numfields + 1  'set the number of rows on the grid
grdNew.Cols = IDDClassObjPt(curIDDClass).objectCount + 2  'set the number of columns on the grid
'create an array for rows with special unit handling
'ReDim specialUnit(grdNew.Rows, grdNew.Cols)
'fill heading (left) columns
For j = 1 To numfields
  grdNew.TextMatrix(j, 0) = IDDField(firstField + j - 1).name
  useUnits = IDDField(firstField + j - 1).unitsIndex
  grdNew.RowData(j) = useUnits
  If useUnits = unitsVaryByObject Then
    grdNew.TextMatrix(j, 1) = "varies"
  ElseIf IDDClassDat(curIDDClass).specialMultiUnit Then
    Select Case curIDDClass
      Case classScheduleDayHourly
        If j >= 3 Then
          grdNew.TextMatrix(j, 1) = "varies"
        End If
      Case classScheduleDayInterval
        If j >= 4 Then
          If j Mod 2 = 1 Then
            grdNew.TextMatrix(j, 1) = "varies"
          End If
        End If
      Case classScheduleConstant
        If j >= 3 Then
          grdNew.TextMatrix(j, 1) = "varies"
        End If
      Case classScheduleDayList
        If j >= 5 Then
          grdNew.TextMatrix(j, 1) = "varies"
        End If
      Case classScheduleCompact
        If j >= 3 Then
          grdNew.TextMatrix(j, 1) = "varies"
        End If
    End Select
  ElseIf useUnits <> noUnitsSpecified Then
    If displayUnits = dispUnitSI Then
      grdNew.TextMatrix(j, 1) = convUnits(useUnits).siName
    Else
      grdNew.TextMatrix(j, 1) = convUnits(useUnits).ipName
    End If
  End If
  'indicate the REQUIRED by a blue font in field name
  If IDDField(firstField + j - 1).required Then
    grdNew.Cell(flexcpForeColor, j, 0) = vbBlue
  End If
Next j
'fill heading (top) row
For i = 2 To grdNew.Cols - 1
    grdNew.TextMatrix(0, i) = "Obj" & (i - 1)
    grdNew.ColAlignment(i) = 1 'align everything to the left
Next i
'------------ Fill Body of Grid ----------------
curObject = IDDClassObjPt(curIDDClass).objectStart
i = grdNew.FixedCols  'pointer to the first column for data
Do While curObject > 0
  StartVal = IDFObject(curObject).valueStart
  grdNew.ColData(i) = curObject  'store the current object pointer for the column
  For j = 1 To numfields
    curField = firstField + j - 1
    curAN = IDDField(curField).AN
    'special handling of ScheduleCompact fields
    If curIDDClass = classScheduleCompact Then
      If IsNumeric(IDFValue(StartVal + j - 1).entry) Then
        curAN = 2
      End If
    End If
    If curAN = 1 Then 'if an alpha
      If IDDField(curField).preserveIndent Then
        grdNew.TextMatrix(j, i) = Space(IDFValue(StartVal + j - 1).leadingSpaces) & IDFValue(StartVal + j - 1).entry
      Else
        grdNew.TextMatrix(j, i) = IDFValue(StartVal + j - 1).entry
      End If
      If IDDField(curField).type = 5 Then 'object-list
        If Not isReferenceValid(IDFValue(StartVal + j - 1).entry, curField) Then 'call function that determines if the reference is valid
          grdNew.Cell(flexcpBackColor, j, i) = outOfRangeColor
        End If
      ElseIf IDDField(curField).type = 4 Then 'generic choice
        If Not isChoiceValid(IDFValue(StartVal + j - 1).entry, curField) Then 'call function that determines if the choice is valid
          grdNew.Cell(flexcpBackColor, j, i) = outOfRangeColor
        End If
      End If
    Else  'if a numeric
      'grdNew.textmatrix(j, i)) = Format(Val(IDFValue(StartVal + j - 1).entry))
      If IDFValue(StartVal + j - 1).entry = "" Then  'don't display blanks as zeros
        'do nothing
      ElseIf LCase(IDFValue(StartVal + j - 1).entry) = "autosize" Then
        grdNew.TextMatrix(j, i) = "autosize"
      ElseIf LCase(IDFValue(StartVal + j - 1).entry) = "autocalculate" Then
        grdNew.TextMatrix(j, i) = "autocalculate"
      ElseIf IDFValue(StartVal + j - 1).isExpression Then
        grdNew.TextMatrix(j, i) = IDFValue(StartVal + j - 1).entry
      Else
        useUnits = getUnitsForCell(j, i, curIDDClass)
        If displayUnits = dispUnitSI Or useUnits = noUnitsSpecified Then
          grdNew.TextMatrix(j, i) = toIntl(IDFValue(StartVal + j - 1).entry, numDigitsShown)
        Else
          grdNew.TextMatrix(j, i) = toIntl(Val(IDFValue(StartVal + j - 1).entry) * convUnits(useUnits).mult + convUnits(useUnits).offset, numDigitsShown)
        End If
        'check if the value entered is within the range allowed
        curValue = Val(IDFValue(StartVal + j - 1).entry)
        outOfRange = False
        minimumVal = IDDField(curField).minimum
        maximumVal = IDDField(curField).maximum
        If IDDField(curField).maxSpecified Then
          If IDDField(curField).exclusiveMax Then
            If curValue >= maximumVal Then outOfRange = True
          Else
            If curValue > maximumVal Then outOfRange = True
          End If
        End If
        If IDDField(curField).minSpecified Then
          If IDDField(curField).exclusiveMin Then
            If curValue <= minimumVal Then outOfRange = True
          Else
            If curValue < minimumVal Then outOfRange = True
          End If
        End If
        If outOfRange Then
          grdNew.Cell(flexcpBackColor, j, i) = outOfRangeColor
        Else
          grdNew.Cell(flexcpBackColor, j, i) = 0
        End If
      End If
    End If
    ' for any type of field, if it is required and blank show that it is out of range
    If IDDField(curField).required And Trim(grdNew.TextMatrix(j, i)) = "" And Not IDDField(curField).defSpecified Then
      grdNew.Cell(flexcpBackColor, j, i) = outOfRangeColor
    End If

    'indicate the REQUIRED by a grey bar in each cell
    'If IDDField(curField).required Then
      'grdNew.Cell(flexcpFloodPercent, j, i) = -3
      'grdNew.Cell(flexcpFloodColor, j, i) = vbGrayText
    'End If
  Next j
  i = i + 1
  curObject = IDFObject(curObject).nextObjectInClass
Loop
'grdNew.ColData(i) = -1 'for last column set the object to a negative number as a flag
' this is no longer needed since the last column is no longer blank
grdNew.Row = 1
grdNew.Col = 1
grdNew.RowSel = 1
grdNew.ColSel = 1
grdNew.ColWidth(-1) = defaultColumnWidth
grdNew.ColWidth(0) = 4000
grdNew.ColWidth(1) = 1300
If useWordWrap Then grdNew.AutoSize 0, grdNew.Cols - 1
End Sub


'-----------------------------------------------------------------------------
' Get the unit information from the rowdata property of the grid
' if the units vary by object - looks up the unit indirectly
'-----------------------------------------------------------------------------
Function getUnitsForCell(rowNum As Long, colNum As Long, IDDClassCur As Long) As Integer
Dim curUnits As Integer
Dim fieldForUnit As Long
Dim refFieldValIndex As Long
Dim refFieldVal As String
Dim unitFound As Integer
Dim iUnit As Integer
Dim firstField As Long
Dim curObject As Long
Dim curField As Long
Dim StartVal As Long
Dim useScheduleTypeUnit As Boolean
Dim fieldWithScheduleTypeLimitName As Integer
Dim scheduleTypeUnitName As String
Dim curScheduleTypeUnitObj As Long
Dim possSchTypeUnitName As String
Dim found As Long
curUnits = grdNew.RowData(rowNum)
'check if units vary by object
If curUnits = unitsVaryByObject Then
  firstField = IDDClassDat(IDDClassCur).fieldStart
  curObject = grdNew.ColData(colNum)
  StartVal = IDFObject(curObject).valueStart
  curField = firstField + rowNum - 1
  fieldForUnit = IDDField(curField).unitsBasedOnFieldIndex
  refFieldValIndex = (fieldForUnit - firstField) + StartVal
  If refFieldValIndex > 0 And refFieldValIndex <= maxUsedValue Then
    refFieldVal = IDFValue(refFieldValIndex).entry
    If refFieldVal <> "" Then
      unitFound = 0
      For iUnit = 1 To maxUsedConvUnits
        If refFieldVal = convUnits(iUnit).multiUnitName Then
          unitFound = iUnit
          Exit For
        End If
      Next iUnit
      If unitFound > 0 Then
        curUnits = unitFound
      End If
    Else
      curUnits = unitsDimensionless
    End If
  End If
  If curUnits = unitsVaryByObject Then
    'if the units have not been found use dimensionless
    curUnits = unitsDimensionless
  End If
End If
If IDDClassDat(IDDClassCur).specialMultiUnit Then
  firstField = IDDClassDat(IDDClassCur).fieldStart
  curObject = grdNew.ColData(colNum)
  StartVal = IDFObject(curObject).valueStart
  curField = firstField + rowNum - 1
  useScheduleTypeUnit = False
  Select Case IDDClassCur
'    Case classScheduleTypeLimits
    Case classScheduleDayHourly 'convert field that are the third and later
      If rowNum >= 3 Then
        fieldWithScheduleTypeLimitName = 2
        useScheduleTypeUnit = True
      End If
    Case classScheduleDayInterval 'convert every other field
      If rowNum >= 4 Then
        If rowNum Mod 2 = 1 Then
          fieldWithScheduleTypeLimitName = 2
          useScheduleTypeUnit = True
        End If
      End If
    Case classScheduleConstant 'convert one numeric field
      If rowNum >= 3 Then
        fieldWithScheduleTypeLimitName = 2
        useScheduleTypeUnit = True
      End If
    Case classScheduleDayList
      If rowNum >= 5 Then
        fieldWithScheduleTypeLimitName = 2
        useScheduleTypeUnit = True
      End If
    Case classScheduleCompact
      'determine if the value is just a number or if it
      'contains Through or Until of For
      If IsNumeric(IDFValue(StartVal + rowNum - 1).entry) Or (IsNumeric(grdNew.TextMatrix(rowNum, colNum))) Then
        fieldWithScheduleTypeLimitName = 2
        useScheduleTypeUnit = True
      End If
  End Select
  If useScheduleTypeUnit Then
    scheduleTypeUnitName = IDFValue(StartVal + fieldWithScheduleTypeLimitName - 1).entry
    curUnits = unitsDimensionless
    If scheduleTypeUnitName <> "" Then
      found = 0
      curScheduleTypeUnitObj = IDDClassObjPt(classScheduleTypeLimits).objectStart
      Do While curScheduleTypeUnitObj > 0
        possSchTypeUnitName = IDFValue(IDFObject(curScheduleTypeUnitObj).valueStart).entry 'get first field
        If UCase(scheduleTypeUnitName) = UCase(possSchTypeUnitName) Then
          found = curScheduleTypeUnitObj
          Exit Do
        End If
        curScheduleTypeUnitObj = IDFObject(curScheduleTypeUnitObj).nextObjectInClass
      Loop
      If found > 0 Then
        refFieldVal = IDFValue(IDFObject(found).valueStart + 4).entry 'get fifth field "Unit Type"
        If refFieldVal <> "" Then
          unitFound = 0
          For iUnit = 1 To maxUsedConvUnits
            If UCase(refFieldVal) = UCase(convUnits(iUnit).multiUnitName) Then
              unitFound = iUnit
              Exit For
            End If
          Next iUnit
          If unitFound > 0 Then
            curUnits = unitFound
          End If
        End If
      End If
    End If
  End If
End If
getUnitsForCell = curUnits
End Function



'-----------------------------------------------------------------------------
' Modifies the controls to respond to a click in a grid cell
'-----------------------------------------------------------------------------
Static Sub selectCell()
Dim t As String
Dim i As Long
Dim useUnits As Long
Dim defaultValConv As Double, minimumValConv As Double, maximumValConv As Double
actRow = grdNew.Row - 1
actCol = grdNew.Col - 2
actField = IDDClassDat(actClass).fieldStart + actRow
'get the pointer to the actual object selected
actObject = grdNew.ColData(grdNew.Col)
useUnits = getUnitsForCell(grdNew.Row, grdNew.Col, actClass)
If actObject > 0 Then 'point to a defined object if -1 then clicked on blank column
  actValue = IDFObject(actObject).valueStart + actRow
  frmParent.sbStatusBar.Panels(4).Text = grdNew.TextMatrix(grdNew.Row, grdNew.Col)
  'frmParent.sbStatusBar.Panels(3).Text = grdNew.TextMatrix(grdNew.Row, 1)
  If useUnits = noUnitsSpecified Then
    frmParent.sbStatusBar.Panels(3).Text = ""
  Else
    If displayUnits = dispUnitSI Then
      frmParent.sbStatusBar.Panels(3).Text = convUnits(useUnits).siName
    Else
      frmParent.sbStatusBar.Panels(3).Text = convUnits(useUnits).ipName
    End If
  End If
  'show the comments from the IDF file
  If IDFValue(actValue).commentStart > 0 Then
    t = IDFComment(IDFValue(actValue).commentStart)
    For i = IDFValue(actValue).commentStart + 1 To IDFValue(actValue).commentEnd
      t = t & vbCrLf & IDFComment(i)
    Next i
    txtComment.Text = t
  Else
    txtComment.Text = ""
  End If
End If
'Include explanation for selected field in txtExplain
'first pick up the memo from the class
t = "Object Description: "
If IDDClassDat(actClass).memo <> "" Then
  t = t & IDDClassDat(actClass).memo & vbCrLf
Else
  t = t & ""
End If
'next pick up the note for that field
t = t & vbCrLf & "Field Description: "
t = t & IDDField(actField).note
t = t & vbCrLf & "ID: " & IDDField(actField).id

Select Case IDDField(actField).type
  Case 1, 2 'real number or integer
    'see if default, min and max need to be converted before display
    If displayUnits = dispUnitSI Or useUnits = noUnitsSpecified Then
      defaultValConv = IDDField(actField).defaultValue
      minimumValConv = IDDField(actField).minimum
      maximumValConv = IDDField(actField).maximum
    Else
      defaultValConv = IDDField(actField).defaultValue * convUnits(useUnits).mult + convUnits(useUnits).offset
      minimumValConv = IDDField(actField).minimum * convUnits(useUnits).mult + convUnits(useUnits).offset
      maximumValConv = IDDField(actField).maximum * convUnits(useUnits).mult + convUnits(useUnits).offset
    End If
    If IDDField(actField).defSpecified Then
      If IDDField(actField).defaultAutosize Then
        t = t & vbCrLf & "Default is to Autosize"
      ElseIf IDDField(actField).defaultAutoCalc Then
        t = t & vbCrLf & "Default is to Autocalculate"
      Else
        t = t & vbCrLf & "Default: " & toIntl(Str(defaultValConv), numDigitsShown)
      End If
    Else
      t = t & vbCrLf & "No default value available"
    End If
    If IDDField(actField).maxSpecified Or IDDField(actField).minSpecified Then
      t = t & vbCrLf & "Range: "
      If IDDField(actField).minSpecified Then
        If IDDField(actField).exclusiveMin Then
          t = t & toIntl(Str(minimumValConv), numDigitsShown) & " < "
        Else
          t = t & toIntl(Str(minimumValConv), numDigitsShown) & " <= "
        End If
      Else
          t = t & " No minimum but "
      End If
      t = t & "X "
      If IDDField(actField).maxSpecified Then
        If IDDField(actField).exclusiveMax Then
          t = t & "< " & toIntl(Str(maximumValConv), numDigitsShown)
        Else
          t = t & "<= " & toIntl(Str(maximumValConv), numDigitsShown)
        End If
      Else
        t = t & " but no maximum"
      End If
    End If
  Case 3  'alpha
    t = t & vbCrLf & "Enter a alphanumeric value"
  Case 4  'generic choice
    t = t & vbCrLf & "Select from list of choices"
  Case 5  'object-list choice
    t = t & vbCrLf & "Select from list of objects"
  Case 6  'node
    t = t & vbCrLf & "Enter or select node name"
End Select
If IDDField(actField).required Then
    t = t & vbCrLf & "This field is required."
End If
If IDDField(actField).deprecated Then
    t = "DEPRECATED - the value of this field is no longer used by EnergyPlus." & vbCrLf & vbCrLf & t
End If
txtExplain.Text = t
newCellSelected = True
End Sub


' ---------- File Oriented Routines ----

'-----------------------------------------------------------------------------
' A routine to set the name of the active IDF file
'-----------------------------------------------------------------------------
Sub setFileName(nameOfFile As String)
IDFUntitled = False
IDFFileName = nameOfFile
End Sub

'-----------------------------------------------------------------------------
' This routine does the surrounding logic to the save function
' such as whether the "save as" dialog is needed and whether
' to save an original (.org) file copy if the IDF Editor has
' never touched the file.
'-----------------------------------------------------------------------------
Sub doSaveIDFFile(saveAs As Boolean)
Dim needDialog As Boolean
Dim tempFileName As String, nameWOExt As String
'select the current cell - this triggers ValidateEdit and AfterEdit so if value
'is being edited when the save button is pressed the new value is used.
grdNew.Select grdNew.Row, grdNew.Col
' first determine if the save is really needed
' if the save button is pushed but it is not untitled then don't bother saving
If Not (saveAs Or IDFAltered Or IDFUntitled) Then Exit Sub
' if no title has been established need need dialog
' if want a save as then need the dialog
If IDFUntitled Or saveAs Then
  CommonDialog1.Filter = "EnergyPlus (*.idf)|*.idf"
  CommonDialog1.FilterIndex = 1
  CommonDialog1.fileName = IDFFileName
  CommonDialog1.DialogTitle = "Save EnergyPlus IDF File As"
  CommonDialog1.Flags = &H2 'overwrite prompt on - will bring up a confirmation if overwriting
  CommonDialog1.ShowSave  'bring up the dialog box
  tempFileName = CommonDialog1.fileName
  If tempFileName = "" Then
    Exit Sub  'if cancel is pressed exit the save routine
  Else  ' a name was entered
    IDFFileName = tempFileName
  End If
Else  'make a backup copy
  ' !!!!!!!!!!!!!! Implement this later
  'FileCopy IDFFileName, Left(IDFFileName, InStr(IDFFileName, ".") - 1) & ".bak"
End If
Me.Caption = IDFFileName  'use active name and get rid of *
IDFAltered = False
IDFUntitled = False
'call the main routine to save a file in module Main.
Call SaveIDF
If checkRangeOnSave = checkRangeYes Then
  Set frmErrorCheck.curFormData = Me
  frmErrorCheck.reportWhenNoneFound = False
  frmErrorCheck.Show vbModal
End If
End Sub

'-----------------------------------------------------------------------------
' This sub checks if a file has been altered and allows the user to cancel the
' action if it has
'-----------------------------------------------------------------------------
Function IsFileChanged() As Boolean
Dim retButton As Long
If Not IDFAltered Then Exit Function
retButton = MsgBox("Do you want an opportunity to save the changes you made prior to clearing this file?" & vbCrLf & vbCrLf & IDFFileName, vbExclamation + vbYesNo, "Warning")
If retButton = vbYes Then
  IsFileChanged = True
Else
  IsFileChanged = False
End If
End Function

'-----------------------------------------------------------------------------
' unused
'-----------------------------------------------------------------------------
Sub SelectRunBatchFile()
CommonDialog1.Filter = "Batch File (*.bat)|*.bat"
CommonDialog1.FilterIndex = 1
CommonDialog1.ShowOpen
Debug.Print CommonDialog1.fileName
'call readIDF
End Sub

'-----------------------------------------------------------------------------
' unused
'-----------------------------------------------------------------------------
Sub SelectWeatherFile()
CommonDialog1.Filter = "EnergyPlus Weather (*.epw)|*.bat|Blast Weather (*.wea)|(*.wea)"
CommonDialog1.FilterIndex = 1
CommonDialog1.ShowOpen
Debug.Print CommonDialog1.fileName
'call readIDF
End Sub

'-----------------------------------------------------------------------------
' unused
'-----------------------------------------------------------------------------
Sub SelectPostProcessor()
CommonDialog1.Filter = "Batch File (*.bat)|*.bat|Program (*.exe)"
CommonDialog1.FilterIndex = 1
CommonDialog1.ShowOpen
Debug.Print CommonDialog1.fileName
'call readIDF
End Sub

'-----------------------------------------------------------------------------
' Indicator if the file has been altered
'-----------------------------------------------------------------------------
Public Sub ShowFileAltered()
Me.Caption = IDFFileName & " *"
IDFAltered = True
mnuFileSave.Enabled = True
End Sub

'-----------------------------------------------------------------------------
' This routine initializes all arrays except for the ones used to read
' an IDF file.
'-----------------------------------------------------------------------------
Public Sub InitializeArrays()
subTestNames(1) = "minMinus"
subTestNames(2) = "minimum"
subTestNames(3) = "zero"
subTestNames(4) = "maximum"
subTestNames(5) = "maxPlus"
subTestNames(6) = "nominal"
subTestNames(7) = "nomMinus"
subTestNames(8) = "nomPlus"
sizeValue = 200
ReDim IDFValue(sizeValue)
sizeObject = 200
ReDim IDFObject(sizeObject)
sizeComment = 200
ReDim IDFComment(sizeComment)
ReDim IDDClassObjPt(sizeClassDat)
End Sub

'-----------------------------------------------------------------------------
' Creates a empty set of IDF arrays.
' Used when starting a new file and when program first opens.
'-----------------------------------------------------------------------------
Sub MakeEmptyIDF()
Dim i As Long
' clear the object pointers in the class record
For i = 1 To maxUsedIDDClass
  IDDClassObjPt(i).objectCount = 0
  IDDClassObjPt(i).objectStart = 0
Next i
' clear the arrays
maxUsedObject = 0
Erase IDFObject
ReDim IDFObject(sizeObject)
Call resizeObjectArray(1)
' value array
maxUsedValue = 1
Erase IDFValue
ReDim IDFValue(sizeValue)
Call resizeValueArray(1)
' comment array
Erase IDFComment
ReDim IDFComment(sizeComment)
Call resizeCommentArray(1)
maxUsedComment = 0
End Sub

'-----------------------------------------------------------------------------
' Increases the size of the value array if more space is needed
'-----------------------------------------------------------------------------
Sub resizeValueArray(addedSpace As Long)
Dim orgSize As Long
orgSize = sizeValue
Do While maxUsedValue + addedSpace > sizeValue
  sizeValue = sizeValue * 2
Loop
If sizeValue > orgSize Then
  On Error Resume Next
  ReDim Preserve IDFValue(sizeValue)
  If Err.Number <> 0 Then
    MsgBox "Out of memory when resizing the IDFValue array" + _
    vbCrLf + vbCrLf + "If you are using many Schedule:Compacts input objects in your file, one workaround so that you " + _
    "can open the file in IDF Editor, is to reduce " + _
    "the length of the Schedule:Compact that appears in the Energy+.idd file. Make a copy of the Energy+.idd file " + _
    "called ORIGINAL-Energy+.idd so you can always go back to it if you have to. Then open the Energ+.idd file with " + _
    "a text editor such as Notepad and find the section on Schedule:Compact. After A1000 change the comma to a " + _
    "semi-colon and delete the lines that start with A1001 to A9981, the end of that Schedule:Compact input object" + _
    "description. Save the modified Energy+.idd file and try opening the file in IDF Editor again. " + _
    "If that doesn't fix the issue you might want to look at the issue where this was first uncovered:" + _
    vbCrLf + vbCrLf + "https://github.com/NREL/EnergyPlus/issues/9010 ", vbCritical, "Critical Memory Error"
    On Error GoTo 0
    Exit Sub
  End If
  On Error GoTo 0
End If
'Debug.Print "Value array resized to: "; sizeValue
End Sub

'-----------------------------------------------------------------------------
' Increases the size of the object array if more space is needed
'-----------------------------------------------------------------------------
Sub resizeObjectArray(addedSpace As Long)
Dim orgSize As Long
orgSize = sizeObject
Do While maxUsedObject + addedSpace > sizeObject
  sizeObject = sizeObject * 2
Loop
If sizeObject > orgSize Then
  ReDim Preserve IDFObject(sizeObject)
End If
'Debug.Print "Object array resized to: "; sizeObject
End Sub

'-----------------------------------------------------------------------------
' Increases the size of the comment array if more space is needed
'-----------------------------------------------------------------------------
Sub resizeCommentArray(addedSpace As Long)
Dim orgSize As Long
orgSize = sizeComment
Do While maxUsedComment + addedSpace > sizeComment
  sizeComment = sizeComment * 2
Loop
If sizeComment > orgSize Then
  ReDim Preserve IDFComment(sizeComment)
End If
'Debug.Print "Comment array resized to: "; sizeComment
End Sub

'-----------------------------------------------------------------------------
' Reads a IDF file into active file arrays
'
' Their are two types of IDF files:
'  (a) Previously written by the IDF Editor
'  (b) Files written by hand or a different editor
'
' For (a) cases, the part of the file identifies some additional
' information and the parameter name comments are ignored. These
' comments come in the format !-
' These special comments are not displayed to the user since
' they are by and for the IDF Editor.  Included in these IDF !-
' comments are the IP values of any SI unit parameters.  Also
' note that in (a) files all parameter comments are shown on
' the line before the parameter.  One !- types of comments that
' contain parameter name, unit and IP value will appear on the
' same line as the value.  Also in (a) files only one value will be
' shown per line and the order of the objects in file is based on
' the order they appear in the IDD.
'
' For (b) types of files they shouldn't contain !- or any other
' special header information that should be ignored.  All comments
' are displayed for the user including ones that simply name the
' parameter (caution (b) files that contain the parameter names will
' end up with a large number of duplicative comments).
'-----------------------------------------------------------------------------
Sub ReadIDF()
Dim t As String, tLength As Long
Dim curChar As String, curLoc As Long
Dim lastWord As String 'build up of current active word
Dim explptLoc As Long, afterExplPtFull As String
Dim specialLoc As Long, afterSpecial As String, afterSpecialFull As String
Dim specialKey As String, specialVal As String
Dim spaceLoc As Long
Dim inFH As Integer
Dim i As Long, j As Integer
' parseMode indicates the current status of parsing
' = 1 not in Object
' = 2 in Object
Const NotInObject = 1
Const InObject = 2
Dim parseMode As Long
Dim curClass As Long  ' the class pointer for the object being parsed
Dim curField As Long  ' the pointer to the field number in the class
Dim aField As Long    ' the pointer to the IDDField array
Dim curValue As Long  ' the pointer to the current item in the IDFValue array
Dim numfields As Long ' the number of possible fields for that class
Dim commentBlockStart As Long  'points to the beginning of the comments
Dim objPoint As Long  ' used to transverse the object list looking for an empty object pointer
Dim lastObjectWithPt As Long 'used to hold last good pointer in object array
Dim refPt As Long     'pointer to the list of reference lists
Dim newComment As Boolean ' true when a comment has been recently found
Dim flagRangeTest As Boolean 'true when in range test mode
Dim lineNumber As Long       'line number of the file
Dim tempFileName As String
Dim tempLineOut As String
Dim tmpFH As Integer
Const tempBlockSize = 100
Dim tempBlock(tempBlockSize) As String
Dim tempBlockCnt As Integer
Dim fileEnd As Boolean
Dim tWithSpecial As String
Dim startsWithEquals As Boolean
Dim numIndentSpaces As Integer
Dim curObj As Long
isLastError = False
commentBlockStart = 1
maxUsedValue = 1
IDFAltered = False
IDFEdPrevEd = False
parseMode = NotInObject
newComment = False
flagRangeTest = True
fileDateTimeStamp = FileDateTime(IDFFileName)
Debug.Print "IDF File: "; IDFFileName
On Error Resume Next
Call MakeEmptyIDF       'this will clear the existing arrays so a new file can be read over an old one
inFH = FreeFile
Open IDFFileName For Input As inFH
If Err.Number <> 0 Then
  MsgBox "File not found:" & vbCrLf & vbCrLf & IDFFileName, vbExclamation, "Error"
  Exit Sub
End If
On Error GoTo 0
lineNumber = 0
fileEnd = False
numIndentSpaces = 0
Do While Not fileEnd
  Call getNextLine(inFH, t, fileEnd)
  'Debug.Print t
  tempLineOut = ""
  afterSpecialFull = ""
  afterExplPtFull = ""
  lineNumber = lineNumber + 1
  numIndentSpaces = numberOfLeadingSpaces(t)
  t = LTrim(RTrim(t)) 'get rid of preceeding and trailing spaces
  If t <> "" Then
    'first deal with normal and regular comments
    specialLoc = InStr(1, t, "!-") 'this is a special comment
    If specialLoc > 0 Then  'special comment
      afterSpecial = LTrim(RTrim(Mid(t, specialLoc + 2)))
      afterSpecialFull = Mid(t, specialLoc)
      If afterSpecial <> "" Then
        spaceLoc = InStr(afterSpecial, " ")
        If spaceLoc > 0 Then
          specialKey = UCase(RTrim(Left(afterSpecial, spaceLoc)))
          specialVal = LTrim(Mid(afterSpecial, spaceLoc + 1))
          Select Case specialKey
            Case "GENERATOR"
              If Left(specialKey, 9) = "IDFEditor" Then   'was the IDFEditor what created this file?
                IDFEdPrevEd = True
              End If
              Debug.Print "Special Comment ("; specialKey; ")"; specialVal
            Case "IDDDATE"
              'need to check time stamp on the files and compare to current IDD
              'MsgBox "The IDD has been revised since this file was last saved", vbInformation, "IDF Parsing"
            Case "WEATHERFILE"
              weatherFileName = specialKey
            Case "POSTPROCESSOR"
              postprocessorFileName = specialKey
            Case "OPTION"
              If InStr(specialVal, "SortedOrder") > 0 Then saveOrderOption = saveOrderSorted
              If InStr(specialVal, "OriginalOrderTop") > 0 Then saveOrderOption = saveOrderOrigTop
              If InStr(specialVal, "OriginalOrderBottom") > 0 Then saveOrderOption = saveOrderOrigBot
              If InStr(specialVal, "UseSpecialFormat") > 0 Then specialFormatOption = specFormYes
              If InStr(specialVal, "ViewInIPunits") > 0 Then Call SetDisplayUnits(dispUnitIP)
            Case Else
              'ignore since probably just parameter name, units and IP value
          End Select
        End If
      End If
      tWithSpecial = t
      t = Left(t, specialLoc - 1)
    End If
    explptLoc = InStr(1, t, "!")
    If explptLoc = 1 Then 'this is comment on its own line - aka block comment
      Call resizeCommentArray(1)
      maxUsedComment = maxUsedComment + 1
      If specialLoc > 0 Then
        IDFComment(maxUsedComment) = Mid(tWithSpecial, 2)
      Else
        IDFComment(maxUsedComment) = Mid(t, 2)
      End If
      newComment = True
      If InStr(1, t, "RangeTestOn") > 0 Then
        flagRangeTest = True
        Debug.Print "RangeTestOn", t
      End If
      If InStr(1, t, "RangeTestOff") > 0 Then
        flagRangeTest = False
        Debug.Print "RangeTestOff", t
      End If
      t = ""
    ElseIf explptLoc > 1 Then 'this is a comment after a line
      Call resizeCommentArray(1)
      maxUsedComment = maxUsedComment + 1
      If specialLoc > 0 Then
        IDFComment(maxUsedComment) = Mid(tWithSpecial, explptLoc + 1)
      Else
        IDFComment(maxUsedComment) = Mid(t, explptLoc + 1)
      End If
      newComment = True
      t = Left(t, explptLoc - 1)
    End If
  End If
  ' if after processing normal and special comments their is still text
  ' to process...
  If t <> "" Then
    If parseMode = InObject And lastWord <> "" Then
' Fix CR7216 by adding gosub addidfvalue and no longer warn about missing comma.
'      errDisplayChild "Comma may be missing: " & lastWord & vbCrLf & vbCrLf & _
'      "It appears in the object: " & IDDClassDat(IDFObject(maxUsedObject).classType).name & vbCrLf & _
'      "Line number: " & lineNumber & vbCrLf & vbCrLf & _
'      "Each field in an object needs to be separated by commas and a single value cannot be spread over two lines. " & _
'      "Not all objects may have been loaded into the IDF Editor and data could be lost. It is recommended that you save the file using a different name or close the " & _
'      "file without saving.  You may fix the file using a text editor such as Notepad. In a text editor, look for the object listed above and check the comma placement.", _
'      "IDF Parsing Error"
      lastWord = Trim(lastWord)
      GoSub AddIDFValue
      If isLastError Then Exit Do
    End If
    lastWord = ""
    For curLoc = 1 To Len(t)
      curChar = Mid$(t, curLoc, 1)
      Select Case curChar
        Case ","
          Select Case parseMode
            Case NotInObject  'indicates a new object
              'find the index to the class the object belongs to
              lastWord = UCase(RTrim(lastWord)) 'get rid of possible trailing spaces and make upper case
              curClass = 0
              For i = 1 To maxUsedIDDClass
                If UCase(IDDClassDat(i).name) = lastWord Then
                  curClass = i
                  Exit For
                End If
              Next i
              If curClass = 0 Then
                isLastError = errDisplayChild("Object type not recognized: " & lastWord & vbCrLf & vbCrLf & _
                "It appears on line: " & t & vbCrLf & _
                "Line number: " & lineNumber & vbCrLf & vbCrLf & _
               "This error may be caused by a misspelled object name. " & _
               "Not all objects may have been loaded into the IDF Editor and data could be lost. It is recommended that you save the file using a different name or close the " & _
               "file without saving.  You may fix the file using a text editor such as Notepad. In a text editor, look for the object listed above and check the I/O Reference to see if it is a valid object.", _
               "IDF Parsing Error")
                If isLastError Then Exit Do
              Else
                ' define new object
                Call resizeObjectArray(1)
                maxUsedObject = maxUsedObject + 1
                IDFObject(maxUsedObject).valueStart = maxUsedValue
                ' walk through pointers to find next empty class pointer
                objPoint = IDDClassObjPt(curClass).objectStart
                If objPoint = 0 Then 'no objects defined
                  IDDClassObjPt(curClass).objectStart = maxUsedObject 'pointer in class list to first object
                  IDFObject(maxUsedObject).classType = curClass  'pointer in object list to class
                Else
                  Do While objPoint > 0
                    lastObjectWithPt = objPoint 'preserve previous pointer since that is where we need to write
                    objPoint = IDFObject(objPoint).nextObjectInClass 'follow chain of objects (linked list)
                  Loop
                  IDFObject(lastObjectWithPt).nextObjectInClass = maxUsedObject
                  IDFObject(maxUsedObject).classType = curClass  'pointer in object list to class
                End If
                IDFObject(maxUsedObject).nextObjectInClass = 0 'set the next object link to zero to show last defined
                IDDClassObjPt(curClass).objectCount = IDDClassObjPt(curClass).objectCount + 1
                'calculate the number of fields that should be reserved for this object based on the class
                numfields = IDDClassDat(curClass).fieldEnd - IDDClassDat(curClass).fieldStart + 1
                'clear the fields for the new object
                'This makes sure that the fields are clear for the recently read object
                'remember these must be cleared incase not all fields are given values in the
                'IDF file for that object, i.e., location has five values, if only three are
                'given the other two still should be blank and allocated.
                Call resizeValueArray(numfields)
                For i = 1 To numfields
                  IDFValue(maxUsedValue + i).entry = ""
                Next i
                curValue = maxUsedValue  'set the pointer to the IDFValue array to be the next value
                curField = 0
                maxUsedValue = maxUsedValue + numfields
                parseMode = InObject
                lastWord = ""
              End If
            Case InObject 'any other than first comma must end a value
              GoSub AddIDFValue
              If isLastError Then Exit Do
          End Select
        Case ";"
          Select Case parseMode
            Case NotInObject
              'it is a section if it runs into a semi-colon and is not in an object
            Case InObject
              GoSub AddIDFValue
              parseMode = NotInObject
              If isLastError Then Exit Do
          End Select
          parseMode = NotInObject
        Case " ", Chr(9)
          If lastWord <> "" Then lastWord = lastWord & " "  'append space to last word only if not zero length
        Case Else
          lastWord = lastWord & curChar   'append letter to last word
      End Select
    Next curLoc ' loop through the remaining text on the line
  End If
Loop 'through the file
Close inFH
'error handling for a bad file
If isLastError Then
  On Error GoTo 0
  Unload Me
End If
lastUsedObjectInOrigFile = maxUsedObject
' report on number of objects being used.
Debug.Print "-"
Debug.Print "Report on Array Usage"
Debug.Print "IDDClassGroup", , maxUsedClassGroup, sizeClassGroup, maxUsedClassGroup / sizeClassGroup
Debug.Print "iddclassdat", , maxUsedIDDClass, sizeClassDat, maxUsedIDDClass / sizeClassDat
Debug.Print "IDDField", , maxUsedField, sizeField, maxUsedField / sizeField
Debug.Print "IDDChoice", , maxUsedChoice, sizeChoice, maxUsedChoice / sizeChoice
Debug.Print "ConvUnits", maxUsedConvUnits
Debug.Print "IDFObject", , maxUsedObject, sizeObject, maxUsedObject / sizeObject
Debug.Print "IDFValue", , maxUsedValue, sizeValue, maxUsedValue / sizeValue
Debug.Print "IDFComment", , maxUsedComment, sizeComment, maxUsedComment / sizeComment
Debug.Print "Number of lines", lineNumber
Exit Sub

'-----------------------------------------------------------------------------
' The following is a (old fashioned) subroutine
' Add the value to the current object
' remember the space is already allocated in the IDFValue array
' so it just needs to be put their
'-----------------------------------------------------------------------------
AddIDFValue:
IDFValue(curValue).entry = lastWord
IDFValue(curValue).rangeTestStatus = flagRangeTest
IDFValue(curValue).leadingSpaces = numIndentSpaces
'for expressions
If Left(lastWord, 1) = "=" Then
  IDFValue(curValue).isExpression = True
Else
  IDFValue(curValue).isExpression = False
End If
'If flagRangeTest Then Debug.Print lastWord, flagRangeTest
If newComment Then
  IDFValue(curValue).commentStart = commentBlockStart
  IDFValue(curValue).commentEnd = maxUsedComment
  newComment = False
 End If
aField = IDDClassDat(curClass).fieldStart + curField
commentBlockStart = maxUsedComment + 1  'set for next block of comments
curField = curField + 1
curValue = curValue + 1
If curValue > maxUsedValue Then
  isLastError = errDisplayChild("Too many fields for object: " & IDDClassDat(IDFObject(maxUsedObject).classType).name & vbCrLf & _
  "Line number: " & lineNumber & vbCrLf & vbCrLf & _
  "It appears on line: " & t & vbCrLf & vbCrLf & "This error may be caused by a missing semi-colon at the end of an object. " & _
  "Not all objects may have been loaded into the IDF Editor and data could be lost. It is recommended that you save the file using a different name or close the " & _
  "file without saving.  You may fix the file using a text editor such as Notepad. In a text editor, look for the object listed above.  Each object should end with a semi-colon. " & _
  "In addition, the object may be extensible, see Energy+.idd in main EnergyPlus install folder and search for \extensible for instructions.", "IDF Parsing Error")
  Debug.Print lastWord, curField, numfields, maxUsedValue
End If
lastWord = ""
Return
End Sub

'-----------------------------------------------------------------------------
' Abstracts the reading of a line of file to enable reading files with
' mixed carriage return and line feed characters
'-----------------------------------------------------------------------------
Sub getNextLine(fileHandle As Integer, readLine As String, EOFflag As Boolean)
Static readBuffer As String
Dim posLineBreak As Integer
Dim lineIn As String
'if buffer is empty read a line from the file
If Len(readBuffer) = 0 Then
  If Not EOF(fileHandle) Then
    Line Input #fileHandle, lineIn
    'carriage returns or line feeds and if found read line up to that point
    'and buffer the rest
    posLineBreak = InStr(lineIn, vbCr)
    If posLineBreak = 0 Then posLineBreak = InStr(lineIn, vbLf)
    If posLineBreak = 0 Then
      readLine = lineIn
    Else
      readLine = Left(lineIn, posLineBreak - 1)
      readBuffer = Mid(lineIn, posLineBreak + 1)
    End If
  Else
    readLine = ""
    EOFflag = True
  End If
Else 'the buffer still has some text
  posLineBreak = InStr(readBuffer, vbCr)
  If posLineBreak = 0 Then posLineBreak = InStr(readBuffer, vbLf)
  If posLineBreak = 0 Then posLineBreak = InStr(readBuffer, vbTab)
  If posLineBreak = 0 Then
    readLine = readBuffer
    readBuffer = ""
    If EOF(fileHandle) Then EOFflag = True
  Else
    readLine = Left(readBuffer, posLineBreak - 1)
    readBuffer = Mid(readBuffer, posLineBreak + 1)
  End If
End If
End Sub


'-----------------------------------------------------------------------------
' This routine saves the IDF file in "enhanced" format that includes
' special comment characters !- for comments that can be ignored later
'
' Note the file is not saved as it appeared in the original input file
' since all data is parsed out, put in arrays and then written to the file
' this is especially important since !- are ignored and special
' fields are present.
'
' The logic governing the dialog box and such is in the
' IDFEdit Module called SaveIDDFile
'
' March 2007 - Modified routine to write either in sorted order or in the order
' that the objects originally appeared.
'-----------------------------------------------------------------------------
Sub SaveIDF()
Dim iClass As Long, iObject As Long
Dim outFH As Integer
Dim optionString As String
'dump object list in order they appear in array
'For j = 1 To maxUsedObject
'  Debug.Print IDDClassDat(IDFObject(j).classType).name
'Next j
On Error Resume Next
outFH = FreeFile
Open IDFFileName For Output As outFH
If Err.Number = 75 Then
  MsgBox "Read Only File - Cannot Save File", vbCritical, "File Error"
  Exit Sub
ElseIf Err.Number <> 0 Then
  MsgBox "File Not Saved - Try a Different Path - Error#: " & Err.Number, vbCritical, "File Error"
  Exit Sub
End If
On Error GoTo 0
Print #outFH, "!-Generator IDFEditor "; ver
Select Case saveOrderOption
  Case saveOrderSorted
    optionString = "SortedOrder"
  Case saveOrderOrigTop
    optionString = "OriginalOrderTop"
  Case saveOrderOrigBot
    optionString = "OriginalOrderBottom"
End Select
If specialFormatOption = specFormYes Then
  optionString = optionString & " UseSpecialFormat"
End If
If displayUnits = dispUnitIP Then
  optionString = optionString & " ViewInIPunits"
End If
Print #outFH, "!-Option "; optionString
'Print #outFH, "!-IddDate "; "???"
'Print #outFH, "!-WeatherFile "; weatherFileName
'Print #outFH, "!-PostProcessor "; postprocessorFileName
Print #outFH, ""
Print #outFH, "!-NOTE: All comments with '!-' are ignored by the IDFEditor and are generated automatically."
Print #outFH, "!-      Use '!' comments if they need to be retained when using the IDFEditor."
Print #outFH, ""
Select Case saveOrderOption
  Case saveOrderSorted
    For iClass = 1 To maxUsedIDDClass
      If IDDClassObjPt(iClass).objectCount > 0 Then
        Print #outFH, ""
        Print #outFH, "!-   ===========  ALL OBJECTS IN CLASS: "; UCase(IDDClassDat(iClass).name); " ==========="
        Print #outFH, ""
        iObject = IDDClassObjPt(iClass).objectStart
        Do While iObject > 0
          Call saveSingleObject(iObject, outFH)
          iObject = IDFObject(iObject).nextObjectInClass
        Loop
      End If
    ' loop through all classes
    Next iClass
  Case saveOrderOrigTop
    'write the new objects first
    If maxUsedObject >= lastUsedObjectInOrigFile + 1 Then
      For iObject = lastUsedObjectInOrigFile + 1 To maxUsedObject
        If IDFObject(iObject).nextObjectInClass <> isDeleted Then
          Call saveSingleObject(iObject, outFH)
        End If
      Next iObject
    End If
    'write the objects that appeared in the original file
    For iObject = 1 To lastUsedObjectInOrigFile
      If IDFObject(iObject).nextObjectInClass <> isDeleted Then
        Call saveSingleObject(iObject, outFH)
      End If
    Next iObject
  Case saveOrderOrigBot
    For iObject = 1 To maxUsedObject
      If IDFObject(iObject).nextObjectInClass <> isDeleted Then
        Call saveSingleObject(iObject, outFH)
      End If
    Next iObject
End Select
Close outFH
mnuFileSave.Enabled = False
'update time stamp
fileDateTimeStamp = FileDateTime(IDFFileName)
End Sub

'-----------------------------------------------------------------------------
' Routine saves a single object
'-----------------------------------------------------------------------------
Sub saveSingleObject(objIndex As Long, fileHandle As Integer)
Static prevFormat As Integer
Dim classIndex As Long
Dim iValue As Long, iComment As Long, iField As Long
Dim firstField As Long, numfields As Long
Dim StartVal As Long, numFilledfields As Long
Dim j As Long, tabPlace As Long
Dim commaLoc As Long
Dim t As String
Dim lastBlank As Boolean
Dim outputObjectNameYet As Boolean
Dim curFormat As Integer
Dim curName As String
Dim modName As String
Dim vertexCount As Integer
Dim lastChar As String
Dim specialFieldCount As Integer
Dim beginOfLineCount As Integer
Dim noNextCR As Boolean
Dim valueTab As Integer
numFilledfields = 1
classIndex = IDFObject(objIndex).classType
firstField = IDDClassDat(classIndex).fieldStart
numfields = IDDClassDat(classIndex).fieldEnd - firstField + 1
outputObjectNameYet = False
'Print #fileHandle, iddclassdat(classIndex).name & ","   'print the class name - left justified
StartVal = IDFObject(objIndex).valueStart
'first go through fields and find last non-blank
For j = numfields To 1 Step -1
  iValue = StartVal + j - 1
  If IDFValue(iValue).entry <> "" Then
    numFilledfields = j
    Exit For
  End If
Next j
' first scan backwards through the list and remove
' any ending values that are blank
For j = numFilledfields To 1 Step -1
  iValue = StartVal + j - 1
  If UCase(IDFValue(iValue).entry) = "<BLANK>" Then
    numFilledfields = numFilledfields - 1  'remove any trailing blanks
  ElseIf IDFValue(iValue).entry <> "" Then
    Exit For
  End If
Next j
' confirm that the minimum number of fields will be written
If numFilledfields < IDDClassDat(classIndex).minFields Then
  numFilledfields = IDDClassDat(classIndex).minFields
End If
' now scan through the fields
If numFilledfields <= 0 And numfields > 0 Then numFilledfields = 1
'first go through all fields and create the output strings for each
For j = 1 To numFilledfields
  iValue = StartVal + j - 1
  iField = firstField + j - 1
  t = IDFValue(iValue).entry
  If UCase(t) = "<BLANK>" Then
    t = ""
  End If
  If IDDField(iField).AN = 2 Then 'a numeric
    If LTrim(t) <> "" Then  'if blank leave blank - don't convert to a zero
      'remove to make internationalization correct
      't = LTrim(Val(t))
      If t = "0.0000000E+00" Then t = "0"  'get rid of long string of zeros for zero values
      t = Trim(t)
      If Left(t, 1) = "." Then
        t = "0" & t
      End If
    End If
  End If
  IDFValue(iValue).outstr = t
Next j
'if special format option
If specialFormatOption = specFormYes Then
  curFormat = IDDClassDat(classIndex).format
Else
  curFormat = formatStandard
End If
'add space after single line format objects if the next object is not single line format
If prevFormat = formatSingleLine And curFormat <> formatSingleLine Then Print #fileHandle,
'write out comments at top of object
For j = 1 To numFilledfields
  iValue = StartVal + j - 1
  If IDFValue(iValue).commentStart > 0 Then
    For iComment = IDFValue(iValue).commentStart To IDFValue(iValue).commentEnd
      Print #fileHandle, "!"; IDFComment(iComment)
    Next iComment
  End If
Next j
'Depending on the format code used structure the layout of the output object
Select Case curFormat
  Case formatSingleLine
    Print #fileHandle, IDDClassDat(classIndex).name & ","; 'print the class name - left justified
    For j = 1 To numFilledfields
      iField = firstField + j - 1
      iValue = StartVal + j - 1
      Print #fileHandle, IDFValue(iValue).outstr;
      If j < numFilledfields Then   'follow every entry with a comma unless last entry than use semicolon
        Print #fileHandle, ",";
      Else
        Print #fileHandle, ";"  'end of line
      End If
    Next j
'    Print #fileHandle, "" 'leave blank line between objects
  Case formatVertices
    Print #fileHandle, IDDClassDat(classIndex).name & "," 'print the class name - left justified
    vertexCount = 0
    For j = 1 To numFilledfields
      iField = firstField + j - 1
      iValue = StartVal + j - 1
      t = IDFValue(iValue).outstr
      tabPlace = 30
      If Len(t) > 24 Then tabPlace = Len(t) + 8
      If vertexCount = 0 Then Print #fileHandle, Tab(5);
      Print #fileHandle, t;
      If j < numFilledfields Then   'follow every entry with a comma unless last entry than use semicolon
        Print #fileHandle, ",";
      Else
        Print #fileHandle, ";";
      End If
      curName = LCase(IDDField(firstField + j - 1).name)
      If InStr(curName, "vertex") > 0 And InStr(curName, "coordinate") > 0 Or InStr(curName, "origin") > 0 Then
        vertexCount = vertexCount + 1
        'make a modified field name that does not include the search for terms
        modName = Replace(curName, "vertex", " ")
        modName = Replace(modName, "coordinate", " ")
        modName = Replace(modName, "origin", " ")
        'just keep the index number
        If Val(modName) > 0 Then
          modName = Str(Val(modName))
        Else
          modName = ""
        End If
      Else
        vertexCount = 0
      End If
      Select Case vertexCount
        Case 0
          Print #fileHandle, Tab(tabPlace); "!- "; IDDField(firstField + j - 1).name;
          If IDDField(firstField + j - 1).Units <> "" Then
            Print #fileHandle, " {"; IDDField(firstField + j - 1).Units; "}"  'include units if not blank
          Else
            Print #fileHandle,  'just include the carriage return
          End If
        Case 1, 2
          Print #fileHandle, " "; 'add a space - let the coordinates stay on same line
        Case 3
          Print #fileHandle, Tab(40); " !- X,Y,Z "; modName; " {"; IDDField(firstField + j - 1).Units; "}" 'the units and carriage return
          vertexCount = 0 'reset the counter
      End Select
    Next j
    Print #fileHandle, "" 'leave blank line between objects
  Case formatCompactSch
    Print #fileHandle, IDDClassDat(classIndex).name & ","  'print the class name - left justified
    noNextCR = False
    For j = 1 To numFilledfields
      iField = firstField + j - 1
      iValue = StartVal + j - 1
      t = IDFValue(iValue).outstr
      tabPlace = 30
      If Len(t) > 24 Then tabPlace = Len(t) + 8
      If Not noNextCR Then Print #fileHandle, Tab(5);
      Print #fileHandle, t;
      If j < numFilledfields Then   'follow every entry with a comma unless last entry than use semicolon
        Print #fileHandle, ",";
      Else
        Print #fileHandle, ";";
      End If
      If InStr(LCase(t), "until:") > 0 Then
        Print #fileHandle, " "; 'just add a space but stay on same line
        noNextCR = True
      Else
        Print #fileHandle, Tab(tabPlace); "!- "; IDDField(firstField + j - 1).name;
        If IDDField(firstField + j - 1).Units <> "" Then
          Print #fileHandle, " {"; IDDField(firstField + j - 1).Units; "}"  'include units if not blank
        Else
          Print #fileHandle,  'just include the carriage return
        End If
        noNextCR = False
      End If
    Next j
    Print #fileHandle, "" 'leave blank line between objects
  Case formatFluidProperty
    Print #fileHandle, IDDClassDat(classIndex).name & ","  'print the class name - left justified
    specialFieldCount = 0
    beginOfLineCount = 1
    For j = 1 To numFilledfields
      iField = firstField + j - 1
      iValue = StartVal + j - 1
      t = IDFValue(iValue).outstr
      tabPlace = 30
      If Len(t) > 24 Then tabPlace = Len(t) + 8
      If (specialFieldCount Mod 10) = 0 Then Print #fileHandle, Tab(5);
      Print #fileHandle, t;
      If j < numFilledfields Then   'follow every entry with a comma unless last entry than use semicolon
        Print #fileHandle, ",";
      Else
        Print #fileHandle, ";";
      End If
      lastChar = Right(IDDField(firstField + j - 1).name, 1)
      If Asc(lastChar) <= 57 And Asc(lastChar) >= 48 Then
        specialFieldCount = specialFieldCount + 1
      Else
        specialFieldCount = 0
      End If
      If specialFieldCount = 0 Then
        Print #fileHandle, Tab(tabPlace); "!- "; IDDField(firstField + j - 1).name;
        If IDDField(firstField + j - 1).Units <> "" Then
          Print #fileHandle, " {"; IDDField(firstField + j - 1).Units; "}"  'include units if not blank
        Else
          Print #fileHandle,  'just include the carriage return
        End If
      Else
        If (specialFieldCount Mod 10) <> 0 And j < numFilledfields Then
          Print #fileHandle, " "; 'add a space - let the field stay on same line
        Else
          If IDDField(firstField + j - 1).Units <> "" Then
            Print #fileHandle, Tab(140); " !- values "; beginOfLineCount; " to "; specialFieldCount; " {"; IDDField(firstField + j - 1).Units; "}" 'the units and carriage return
          Else
            Print #fileHandle, Tab(140); " !- values "; beginOfLineCount; " to "; specialFieldCount 'the carriage return without units
          End If
          beginOfLineCount = specialFieldCount + 1 'the next line will begin with the next property
        End If
      End If
    Next j
    Print #fileHandle, "" 'leave blank line between objects
  Case formatViewFactor
    Print #fileHandle, IDDClassDat(classIndex).name & ","  'print the class name - left justified
    For j = 1 To numFilledfields
      iField = firstField + j - 1
      iValue = StartVal + j - 1
      t = IDFValue(iValue).outstr
      tabPlace = 30
      If Len(t) > 24 Then tabPlace = Len(t) + 8
      If ((j = 1) Or (((j - 1) Mod 3) = 1)) Then Print #fileHandle, Tab(5);
      Print #fileHandle, t;
      If j < numFilledfields Then   'follow every entry with a comma unless last entry than use semicolon
        Print #fileHandle, ",";
      Else
        Print #fileHandle, ";";
      End If
      If ((j - 1) Mod 3) <> 0 And j < numFilledfields Then
        Print #fileHandle, " "; 'add a space - let the field stay on same line
      Else
        If j > 1 Then
          Print #fileHandle, Tab(tabPlace); "!- i j "; IDDField(firstField + j - 1).name;
        Else
          Print #fileHandle, Tab(tabPlace); "!- "; IDDField(firstField + j - 1).name;
        End If
        If IDDField(firstField + j - 1).Units <> "" Then
          Print #fileHandle, " {"; IDDField(firstField + j - 1).Units; "}"  'include units if not blank
        Else
          Print #fileHandle,  'just include the carriage return
        End If
      End If
    Next j
    Print #fileHandle, "" 'leave blank line between objects
  Case formatSpectral
    Print #fileHandle, IDDClassDat(classIndex).name & ","  'print the class name - left justified
    For j = 1 To numFilledfields
      iField = firstField + j - 1
      iValue = StartVal + j - 1
      t = IDFValue(iValue).outstr
      tabPlace = 30
      If Len(t) > 24 Then tabPlace = Len(t) + 8
      If ((j = 1) Or (((j - 1) Mod 4) = 1)) Then Print #fileHandle, Tab(5);
      Print #fileHandle, t;
      If j < numFilledfields Then   'follow every entry with a comma unless last entry than use semicolon
        Print #fileHandle, ",";
      Else
        Print #fileHandle, ";";
      End If
      If ((j - 1) Mod 4) <> 0 And j < numFilledfields Then
        Print #fileHandle, " "; 'add a space - let the field stay on same line
      Else
        Print #fileHandle, Tab(70); "!- "; IDDField(firstField + j - 1).name;
        If IDDField(firstField + j - 1).Units <> "" Then
          Print #fileHandle, " {"; IDDField(firstField + j - 1).Units; "}"  'include units if not blank
        Else
          Print #fileHandle,  'just include the carriage return
        End If
      End If
    Next j
    Print #fileHandle, "" 'leave blank line between objects
  Case Else 'includes formatStandard (0)
    Print #fileHandle, IDDClassDat(classIndex).name & ","  'print the class name - left justified
    For j = 1 To numFilledfields
      iField = firstField + j - 1
      iValue = StartVal + j - 1
      t = IDFValue(iValue).outstr
      If IDDField(firstField + j - 1).preserveIndent Then
        valueTab = IDFValue(iValue).leadingSpaces + 1
        tabPlace = 50
        If Len(t) > 44 Then tabPlace = Len(t) + valueTab + 3
      Else
        valueTab = 5
        tabPlace = 30
        If Len(t) > 24 Then tabPlace = Len(t) + valueTab + 3
      End If
      Print #fileHandle, Tab(valueTab); t;
      If j < numFilledfields Then   'follow every entry with a comma unless last entry than use semicolon
        Print #fileHandle, ",";
      Else
        Print #fileHandle, ";";
      End If
      Print #fileHandle, Tab(tabPlace); "!- "; IDDField(firstField + j - 1).name;
      If IDDField(firstField + j - 1).Units <> "" Then
        Print #fileHandle, " {"; IDDField(firstField + j - 1).Units; "}"  'include units if not blank
      Else
        Print #fileHandle,  'just include the carriage return
      End If
    Next j
    Print #fileHandle, "" 'leave blank line between objects
End Select
prevFormat = curFormat 'store the format used for the
End Sub

'-----------------------------------------------------------------------------
'Scan through the array looking for the idf version
'-----------------------------------------------------------------------------
Sub getIDFVersion(Optional silent As Boolean = False)
Dim i As Integer
' loop through all classes
For i = 1 To maxUsedIDDClass
  If UCase(IDDClassDat(i).name) = "VERSION" Then
    IDFVersion = Trim(IDFValue(IDFObject(IDDClassObjPt(i).objectStart).valueStart).entry)
    If InStr(IDDVersion, IDFVersion) = 0 Then
      frmParent.sbStatusBar.Panels(5) = "Version Mismatch"
      If Not silent Then
        MsgBox "The file you are opening is a different version than the EnergyPlus program and IDD file you are using. You may want to close the file and use Transition to update the file to the latest version. Editing and saving the file may make it incompatible with an older version of EnergyPlus. ", vbExclamation, "VERSION MISMATCH"
      End If
      isVersionMismatch = True
    Else
      frmParent.sbStatusBar.Panels(5) = ""
      isVersionMismatch = False
    End If
    Exit For
  End If
Next i
End Sub

'-----------------------------------------------------------------------------
' This routine creates a new object and duplicates the values from the currently
' selected object into the new object
'-----------------------------------------------------------------------------
Sub IDFDuplicateObject()
Dim rowSt As Long, rowEnd As Long, colSt As Long, colEnd As Long
Dim iVal As Long, i As Long, numfields As Long
Dim li As Long
Dim iCol As Long, iObj As Long
li = lstObjectTypes.ListIndex
grdNew.GetSelection rowSt, colSt, rowEnd, colEnd
If actObject = 0 Then Exit Sub    'can't duplicate if not on a selected object
For iCol = colSt To colEnd
  iObj = grdNew.ColData(iCol)
  iVal = IDFObject(iObj).valueStart
  numfields = IDDClassDat(actClass).fieldEnd - IDDClassDat(actClass).fieldStart + 1
  Call IDFNewObject 'first make a new object
  ' now copy values into new object
  For i = 0 To numfields - 1
    IDFValue(maxUsedValue + i - numfields).entry = IDFValue(iVal + i).entry
    Debug.Print "dup "; IDFValue(iVal + i).entry
  Next i
Next iCol
Call FillList
Call FillGrid
If li > lstObjectTypes.ListCount - 1 Then
  li = 0
End If
lstObjectTypes.ListIndex = li
grdNew.Col = grdNew.Cols - 1
grdNew.ShowCell grdNew.TopRow, grdNew.Cols - 1
Call selectCell
Call ShowFileAltered
End Sub

'-----------------------------------------------------------------------------
' This routine creates a new object and duplicates the values from the currently
' selected object into the new object and it allows the user to change text
' that appears in one or more fields
'-----------------------------------------------------------------------------
Sub IDFDuplicateObjectAndChange()
Dim rowSt As Long, rowEnd As Long, colSt As Long, colEnd As Long
Dim curVal As Long, jField As Long, numfields As Long, iLength As Long
Dim li As Long
Dim curObj As Long
Dim numChar As Long
Dim found As Boolean
Dim hintText As String
li = lstObjectTypes.ListIndex
grdNew.GetSelection rowSt, colSt, rowEnd, colEnd
If actObject = 0 Then Exit Sub    'can't duplicate if not on a selected object
If colEnd <> colEnd Then Exit Sub 'can't do changes across multiple objects
' find the text suggestion for search box
curObj = grdNew.ColData(colSt)
curVal = IDFObject(curObj).valueStart
numfields = IDDClassDat(actClass).fieldEnd - IDDClassDat(actClass).fieldStart + 1
numChar = 2 'assume that this is the minimum number of characters that must match
For iLength = 1 To Len(IDFValue(curVal).entry)
  found = False
  hintText = Left(IDFValue(curVal).entry, numChar)
  For jField = 1 To numfields - 1
    If InStr(IDFValue(curVal + jField).entry, hintText) > 0 Then
      found = True
      Exit For
    End If
  Next jField
  If found Then
    numChar = numChar + 1
  Else
    Exit For
  End If
Next iLength
numChar = numChar - 1 'shorten since last find did not work
If numChar > 1 Then
  searchTerm = Left(IDFValue(curVal).entry, numChar)
Else
  searchTerm = Left(IDFValue(curVal).entry, 5)
End If
' show the dialog
frmDuplicateAndChange.Show vbModal
' do the replacement text if not changed
If replaceTerm <> "" Then
  Call IDFNewObject 'first make a new object
  ' now copy values into new object
  For jField = 0 To numfields - 1
    IDFValue(maxUsedValue + jField - numfields).entry = Replace(IDFValue(curVal + jField).entry, searchTerm, replaceTerm)
    Debug.Print "dup "; IDFValue(curVal + jField).entry
  Next jField
  Call FillList
  Call FillGrid
  If li > lstObjectTypes.ListCount - 1 Then
    li = 0
  End If
  lstObjectTypes.ListIndex = li
  grdNew.Col = grdNew.Cols - 1
  grdNew.ShowCell grdNew.TopRow, grdNew.Cols - 1
  Call selectCell
  Call ShowFileAltered
End If
End Sub

'-----------------------------------------------------------------------------
' This routine creates a new blank object with all default values set
'-----------------------------------------------------------------------------
Sub IDFNewObject()
Dim objPoint As Long, lastObjectWithPt As Long, numfields As Long
Dim i As Long, iField As Long, refPt As Long
'Debug.Print actClass, actField, iddclassdat(actClass).name, IDDField(actField).name
'find the last object in class
Call resizeObjectArray(1)
maxUsedObject = maxUsedObject + 1
IDFObject(maxUsedObject).valueStart = maxUsedValue
' walk through pointers to find next empty class pointer
objPoint = IDDClassObjPt(actClass).objectStart
If objPoint = 0 Then 'no objects defined
  IDDClassObjPt(actClass).objectStart = maxUsedObject 'pointer in class list to first object
  IDFObject(maxUsedObject).classType = actClass  'pointer in object list to class
Else
  Do While objPoint > 0
    lastObjectWithPt = objPoint 'preserve previous pointer since that is where we need to write
    objPoint = IDFObject(objPoint).nextObjectInClass 'follow chain of objects (linked list)
  Loop
  IDFObject(lastObjectWithPt).nextObjectInClass = maxUsedObject
  IDFObject(maxUsedObject).classType = actClass  'pointer in object list to class
End If
IDFObject(maxUsedObject).nextObjectInClass = 0 'set the next object link to zero to show last defined
IDDClassObjPt(actClass).objectCount = IDDClassObjPt(actClass).objectCount + 1
'calculate the number of fields that should be reserved for this object based on the class
numfields = IDDClassDat(actClass).fieldEnd - IDDClassDat(actClass).fieldStart + 1
'clear the fields for the new object
'This makes sure that the fields are clear for the recently read object
'remember these must be cleared incase not all fields are given values in the
'IDF file for that object, i.e., location has five values, if only three are
'given the other two still should be blank and allocated.
Call resizeValueArray(numfields)
For i = 0 To numfields
'  iField = i + iddclassdat(actClass).fieldStart - 1 original line before LL reported problem
  iField = i + IDDClassDat(actClass).fieldStart
  If IDDField(iField).AN = 1 Then 'alpha
    If IDDField(iField).defaultChoice = 0 Then
      IDFValue(maxUsedValue + i).entry = ""
    Else
      IDFValue(maxUsedValue + i).entry = IDDChoice(IDDField(iField).defaultChoice)
    End If
  Else
    If IDDField(iField).defaultAutosize Then
      IDFValue(maxUsedValue + i).entry = "autosize"
    ElseIf IDDField(iField).defaultAutoCalc Then
      IDFValue(maxUsedValue + i).entry = "autocalculate"
    ElseIf IDDField(iField).defaultValue = 0 Then
      IDFValue(maxUsedValue + i).entry = ""
    Else
      IDFValue(maxUsedValue + i).entry = Str(IDDField(iField).defaultValue) 'STR added to try to address default value for international locations
    End If
  End If
Next i
maxUsedValue = maxUsedValue + numfields
'grdNew.ShowCell grdNew.TopRow - 1, grdNew.LeftCol - 1
End Sub

'-----------------------------------------------------------------------------
' This routine removes the currently selected object
'-----------------------------------------------------------------------------
Sub IDFDeleteObject()
Dim rowSt As Long, rowEnd As Long, colSt As Long, colEnd As Long
Dim li As Long
Dim numColSelected As Long
Dim iCol As Long
Dim colPreserve As Long
Dim objPoint As Long, lastObjectWithPt As Long
Dim actObjSt As Long, actObjEnd As Long
grdNew.GetSelection rowSt, colSt, rowEnd, colEnd
li = lstObjectTypes.ListIndex
numColSelected = 1 + colEnd - colSt
colPreserve = colSt
' walk through pointers to find next empty class pointer
If actCol < 0 Then Exit Sub 'if not a selected "data" column can't delete
If actClass = 0 Then Exit Sub 'if not active can't delete
If IDDClassObjPt(actClass).objectCount = 0 Then Exit Sub 'no more objects
objPoint = IDDClassObjPt(actClass).objectStart
If numColSelected = 1 Then
  If IDDClassObjPt(actClass).objectCount = 1 Then 'this is the case where only one object is defined and it should be deleted and only one object is selected
    IDDClassObjPt(actClass).objectStart = 0
    IDDClassObjPt(actClass).objectCount = 0
    IDFObject(objPoint).nextObjectInClass = isDeleted
  ElseIf objPoint = actObject Then    'if pointing to first object (remember objPoint is on first) and only one object is selected
    If objPoint = 0 Then Exit Sub
    If IDFObject(objPoint).nextObjectInClass > 0 Then
      IDDClassObjPt(actClass).objectStart = IDFObject(objPoint).nextObjectInClass
      IDDClassObjPt(actClass).objectCount = IDDClassObjPt(actClass).objectCount - 1 'reduce the count on the objects
      IDFObject(objPoint).nextObjectInClass = isDeleted 'flag that the object has been deleted
    Else  'should never occur
      MsgBox "Delete Object Error"
    End If
  Else 'the case when something other than the first object is selected but only one object is selected
    If objPoint = 0 Then Exit Sub
    Do While objPoint <> actObject
      lastObjectWithPt = objPoint 'preserve previous pointer since that is where we need to write
      If objPoint = 0 Then Exit Sub
      objPoint = IDFObject(objPoint).nextObjectInClass 'follow chain of objects (linked list)
    Loop
    If lastObjectWithPt > 0 Then
      If objPoint = 0 Then Exit Sub
      IDFObject(lastObjectWithPt).nextObjectInClass = IDFObject(objPoint).nextObjectInClass   'remove link to current object
      IDDClassObjPt(actClass).objectCount = IDDClassObjPt(actClass).objectCount - 1 'reduce the count on the objects
      IDFObject(objPoint).nextObjectInClass = isDeleted 'flag that the object has been deleted
    End If
  End If
Else  ' more than one object is selected
  actObjSt = grdNew.ColData(colSt) ' first object selected
  actObjEnd = grdNew.ColData(colEnd) ' last object selected
  If actObjSt = 0 Or actObjEnd = 0 Then Exit Sub
  If IDDClassObjPt(actClass).objectCount = numColSelected Then 'this is the case where all the objects for a class are selected
    IDDClassObjPt(actClass).objectStart = 0
    IDDClassObjPt(actClass).objectCount = 0
    For iCol = colSt To colEnd
      objPoint = grdNew.ColData(iCol)
      If objPoint > 0 Then
        IDFObject(objPoint).nextObjectInClass = isDeleted
      End If
    Next iCol
  ElseIf objPoint = actObjSt Then    'if first object pointed to is first possible object
    If IDFObject(actObjEnd).nextObjectInClass > 0 Then
      IDDClassObjPt(actClass).objectStart = IDFObject(actObjEnd).nextObjectInClass
      IDDClassObjPt(actClass).objectCount = IDDClassObjPt(actClass).objectCount - numColSelected 'reduce the count on the objects
      For iCol = colSt To colEnd
        objPoint = grdNew.ColData(iCol)
        If objPoint > 0 Then
          IDFObject(objPoint).nextObjectInClass = isDeleted
        End If
      Next iCol
    Else  'should never occur
      MsgBox "Delete Object Error"
    End If
  Else 'the case when something other than the first object is selected and a group of objects is selected
    Do While objPoint <> actObjSt
      lastObjectWithPt = objPoint 'preserve previous pointer since that is where we need to write
      If objPoint = 0 Then Exit Sub
      objPoint = IDFObject(objPoint).nextObjectInClass 'follow chain of objects (linked list)
    Loop
    If lastObjectWithPt > 0 Then
      If objPoint = 0 Then Exit Sub
      IDFObject(lastObjectWithPt).nextObjectInClass = IDFObject(actObjEnd).nextObjectInClass   'remove link to current object
      IDDClassObjPt(actClass).objectCount = IDDClassObjPt(actClass).objectCount - numColSelected 'reduce the count on the objects
      For iCol = colSt To colEnd
        objPoint = grdNew.ColData(iCol)
        If objPoint > 0 Then
          IDFObject(objPoint).nextObjectInClass = isDeleted
        End If
      Next iCol
    End If
  End If
End If
Call FillList
Call FillGrid
If li > lstObjectTypes.ListCount - 1 Then
  li = 0
End If
lstObjectTypes.ListIndex = li
Call lstObjectTypes_Click
If colPreserve > 2 And colPreserve < grdNew.Cols Then
  grdNew.Col = colPreserve
  Call selectCell
  grdNew.ShowCell grdNew.TopRow, grdNew.Col
End If
Call ShowFileAltered
End Sub

'-----------------------------------------------------------------------------
' Creates test files for range tests
'-----------------------------------------------------------------------------
Sub CreateRangeTestFiles()
Dim confirmTest As Long
Dim testCase As Long
Dim testID As String
Dim lastTest As Boolean
Dim fieldUsed As Long, classUsed As Long
Dim runCase As Boolean
Dim i As Long

confirmTest = MsgBox("This function will create a large number of files for testing each field in the IDF file.  EnergyPlus is then called for each file.  It is intended for testers and developers of EnergyPlus only.  It starts an operation that may take hours before it is completed.  Run with caution!", vbOKCancel, "Run Range Tests")
If confirmTest <> 1 Then Exit Sub
Debug.Print "CreateRangeTestFiles --- Started", Now()
Call addNewRunToLog
firstRunInSuite = True
testCase = 1
Do
  Erase logItemState 'clear the log item arrays
  Erase logItemResults
  For i = 1 To numSubTests
    testID = format(testCase, "000000") & "-" & subTestNames(i)
    lastTest = rangeSaveIDF(testCase, subTestNames(i), testID, fieldUsed, classUsed)
    If lastTest Then Exit Do
    runCase = True
    'don't bother running the two past min/max cases if no min and max specified
    If i = STminMinus And IDDField(fieldUsed).minimum = 0 Then runCase = False
    If i = STmaxPlus And IDDField(fieldUsed).maximum = 0 Then runCase = False
    If runCase Then
      If runRangeCase(testID) Then  'must have run and not crashed
        Call collectLogInfo(testCase, i, testID, fieldUsed, classUsed)
      Else
        logItemState(i) = 3 'crashed
      End If
    Else
      logItemState(i) = 4 'not run
    End If
  Next i
  Call writeLogEntry(fieldUsed, classUsed, testCase)
  testCase = testCase + 1
Loop
MsgBox "Completed Range Tests.  Last case " & testID, vbInformation, "Range Test"
Debug.Print "CreateRangeTestFiles --- Ended", Now()
End Sub


'-----------------------------------------------------------------------------
' The following routine is almost identical to the saveIDF routine but is used
' when creating range test cases.
'
'     RANGE TESTS               minMinus  minimum value minus 1% (should fail)
'                               min       minimum value
'                               zero      zero value
'                               max       maximum value
'                               maxPlus   maximum value plus 1% (should fail)
'
'     SENSITIVITY TESTS         nominal   nominal value input
'                               nomMinus  nominal value minus 50%
'                               nomPlus   nominal value plus 50%
'
'-----------------------------------------------------------------------------
Function rangeSaveIDF(rangeItem As Long, subTest As String, testName As String, testedField As Long, testedClass As Long) As Boolean
Dim iClass As Long, iObject As Long, iField As Long
Dim iValue As Long, iComment As Long
Dim firstField As Long, numfields As Long
Dim StartVal As Long, numFilledfields As Long
Dim j As Long, tabPlace As Long
Dim t As String
' variables having to do with the range testing
Dim rangeTestCounter As Long 'counts while going through file to find "current case"
Dim minVal As Double, maxVal As Double, defVal As Double, oldTVal As Double
Dim newTval As Double
Dim foundActiveTest As Boolean ' assume no active test found
Dim curFileName As String
Dim subTestIndex As Long

rangeTestCounter = 0
foundActiveTest = False
On Error Resume Next
curFileName = Left(IDFFileName, Len(IDFFileName) - 4) 'strip off the extension
curFileName = curFileName & "-" & testName & ".idf"
Open curFileName For Output As 3
If Err.Number = 75 Then
  MsgBox "Read Only File - Cannot Save File", vbCritical, "File Error"
  Exit Function
ElseIf Err.Number <> 0 Then
  MsgBox "File Not Saved - Try a Different Path - Error#: " & Err.Number, vbCritical, "File Error"
  Exit Function
End If
On Error GoTo 0
'Open "outtest.idf" For Output As 3
Print #3, "!-Generator IDFEditor in RANGE TESTING MODE "; ver
Print #3, ""
Print #3, "!-NOTE: All comments with '!-' are ignored by the IDFEditor and are generated automatically."
Print #3, "!-      Use '!' comments if they need to be retained when using the IDFEditor."
Print #3, ""
' loop through all classes
For iClass = 1 To maxUsedIDDClass
  If IDDClassObjPt(iClass).objectCount > 0 Then
    Print #3, ""
    Print #3, "!-   ===========  ALL OBJECTS IN CLASS: "; UCase(IDDClassDat(iClass).name); " ==========="
    Print #3, ""
    iObject = IDDClassObjPt(iClass).objectStart
    firstField = IDDClassDat(iClass).fieldStart
    numfields = IDDClassDat(iClass).fieldEnd - firstField + 1
    Do While iObject > 0
      Print #3, IDDClassDat(iClass).name & ","   'print the class name - left justified
      StartVal = IDFObject(iObject).valueStart
      'first go through fields and find last non-blank
      For j = numfields To 1 Step -1
        iValue = StartVal + j - 1
        If IDFValue(iValue).entry <> "" Then
          numFilledfields = j
          Exit For
        End If
      Next j
      For j = 1 To numFilledfields
        iField = firstField + j - 1
        iValue = StartVal + j - 1
        If IDFValue(iValue).commentStart > 0 Then
          For iComment = IDFValue(iValue).commentStart To IDFValue(iValue).commentEnd
            Print #3, "! "; IDFComment(iComment)
          Next iComment
        End If
        t = IDFValue(iValue).entry
        oldTVal = Val(t)
        If IDDField(iField).AN = 1 Then 'an alpha?
          Print #3, Tab(5); t;
          'if an alpha leave more room for alpha prior to field name
          tabPlace = 30
        Else  'if a numeric

          '========== TESTING SECTION START
          ' replace the current value of t with a test value
          If IDFValue(iValue).rangeTestStatus Then  'if part of the range testing
            'Debug.Print "rangeTestCounter", rangeTestCounter
            rangeTestCounter = rangeTestCounter + 1
            If rangeTestCounter = rangeItem Then  'test if we match the test case yet
              foundActiveTest = True
              Print #3,
              testedField = iField
              testedClass = iClass
              If IDDField(iField).type <> 2 Then  'if not integer assume real
                Print #3, "!********************* TEST PARAMETER - REAL"
                'set the min values
                minVal = IDDField(iField).minimum
                If minVal = 0 Then minVal = -9999999
                If IDDField(iField).exclusiveMin Then minVal = minVal + 0.01
                'set the min values
                maxVal = IDDField(iField).maximum
                If maxVal = 0 Then maxVal = 9999999
                If IDDField(iField).exclusiveMax Then maxVal = maxVal - 0.01
                'set the default value
                defVal = IDDField(iField).defaultValue
                Print #3, "!********************* ORIG,MIN,MAX,DEF   ", t, minVal, maxVal, defVal
                Select Case subTest
                  Case subTestNames(STminMinus)      ' minimum value minus 1% (should be flagged)
                    Print #3, "!********************* minimum value minus 1% (should be flagged)"
                    newTval = minVal - (Abs(minVal) * 0.01)
                    subTestIndex = STminMinus
                  Case subTestNames(STminimum)      ' minimum value
                    Print #3, "!********************* minimum value"
                    newTval = minVal
                    subTestIndex = STminimum
                  Case subTestNames(STzero)          ' zero value
                    Print #3, "!********************* zero entry"
                    newTval = 0
                    subTestIndex = STzero
                  Case subTestNames(STmaximum)      ' maximum value
                    Print #3, "!********************* maximum value"
                    newTval = maxVal
                    subTestIndex = STmaximum
                  Case subTestNames(STmaxPlus)      ' maximum value plus 1% (should be flagged)
                    Print #3, "!********************* maximum value plus 1% (should be flagged)"
                    newTval = maxVal + (Abs(maxVal) * 0.01)
                    subTestIndex = STmaxPlus
                  Case subTestNames(STnominal)      ' nominal value
                    Print #3, "!********************* value from file"
                    newTval = oldTVal
                    subTestIndex = STnominal
                  Case subTestNames(STnomMinus)      ' nominal (input) value -50%
                    Print #3, "!********************* value from file minus 50%"
                    newTval = oldTVal * 0.5
                    subTestIndex = STnomMinus
                  Case subTestNames(STnomPlus)      ' nominal (input) value +50%
                    Print #3, "!********************* value from file plus 50%"
                    newTval = oldTVal * 1.5
                    subTestIndex = STnomPlus
                End Select
              Else 'integer value processing
                Print #3, "!********************* TEST PARAMETER - integer"
                'set the min values
                minVal = IDDField(iField).minimum
                If minVal = 0 Then minVal = -9999
                If IDDField(iField).exclusiveMin Then minVal = minVal + 1
                'set the min values
                maxVal = IDDField(iField).maximum
                If maxVal = 0 Then maxVal = 9999
                If IDDField(iField).exclusiveMax Then maxVal = maxVal - 1
                'set the default value
                defVal = IDDField(iField).defaultValue
                Print #3, "!********************* ORIG,MIN,MAX,DEF   ", t, minVal, maxVal, defVal
                Select Case subTest
                  Case subTestNames(STminMinus)      ' minimum value minus 10 (should be flagged)
                    Print #3, "!********************* minimum value minus 10 (should be flagged)"
                    newTval = minVal - 10
                    subTestIndex = STminMinus
                  Case subTestNames(STminimum)      ' minimum value
                    Print #3, "!********************* minimum value"
                    newTval = minVal
                    subTestIndex = STminimum
                  Case subTestNames(STzero)          ' zero value
                    Print #3, "!********************* zero entry"
                    newTval = 0
                    subTestIndex = STzero
                  Case subTestNames(STmaximum)      ' maximum value
                    Print #3, "!********************* maximum value"
                    newTval = maxVal
                    subTestIndex = STmaximum
                  Case subTestNames(STmaxPlus)      ' maximum value plus 10 (should be flagged)
                    Print #3, "!********************* maximum value plus 10 (should be flagged)"
                    newTval = maxVal + 10
                    subTestIndex = STmaxPlus
                  Case subTestNames(STnominal)      ' nominal value
                    Print #3, "!********************* value from file"
                    newTval = oldTVal
                    subTestIndex = STnominal
                  Case subTestNames(STnomMinus)      ' nominal (input) value -50%
                    Print #3, "!********************* value from file minus 50%"
                    newTval = oldTVal - Int(oldTVal * 0.5)
                    subTestIndex = STnomMinus
                  Case subTestNames(STnomPlus)      ' nominal (input) value +50%
                    Print #3, "!********************* value from file plus 50%"
                    newTval = oldTVal + Int(oldTVal * 0.5)
                    subTestIndex = STnomPlus
                End Select
              End If
              Print #3, "!********************* NEW VALUE:      "; newTval
              logItemInputs(subTestIndex) = newTval
              t = Str(newTval)
            End If
          End If
          '========== TESTING SECTION END

          If LTrim(t) <> "" Then  'if blank leave blank - don't convert to a zero
            Print #3, Tab(5); LTrim(Val(t));
          Else
            Print #3, Tab(5);
          End If
          tabPlace = 30
        End If
        If j < numFilledfields Then   'follow every entry with a comma unless last entry than use semicolon
          Print #3, ",";
        Else
          Print #3, ";";
        End If
        Print #3, Tab(tabPlace); "!- "; IDDField(firstField + j - 1).name;
        t = IDDField(firstField + j - 1).Units
        If t <> "" Then
          Print #3, " {"; t; "}"  'include units if not blank
        Else
          Print #3, ""  'just include the carriage return
        End If
      Next j
      iObject = IDFObject(iObject).nextObjectInClass
      Print #3, "" 'leave blank line between objects
    Loop
  End If
Next iClass
Close 3
rangeSaveIDF = Not foundActiveTest
End Function

'-----------------------------------------------------------------------------
' Run a single case for the range testing
'-----------------------------------------------------------------------------
Function runRangeCase(caseName As String) As Boolean
Dim curFileName As String, curPath As String, actDir As String
'change directories
actDir = CurDir
ChDir "c:\energyplus"
ChDrive "c:"
' first delete the existing flag file
If Dir("eplusout") <> "" Then Kill "eplusout.end"
' now create the proper file name
curFileName = Left(IDFFileName, Len(IDFFileName) - 4) 'strip off the extension
curFileName = curFileName & "-" & caseName
' call the NT command shell or the win 95 dos box
ExecCmd "CMD /e:3000 /c RANGE.BAT " & curFileName
' for win95 but doesn't wait properly
'Shell "COMMAND.COM /e:3000 /c RANGE.BAT " & curFileName, vbNormalFocus
If Dir("eplusout.end") <> "" Then 'good run
  runRangeCase = True
Else
  runRangeCase = False
End If
ChDrive actDir
ChDir actDir
End Function

'-----------------------------------------------------------------------------
' Grab outputs from EnergyPlus and put into the log file for the range testing
'-----------------------------------------------------------------------------
Sub collectLogInfo(rangeItem As Long, subTestIndex As Long, testName As String, testedField As Long, testedClass As Long)
Dim curFileName As String, fln As Long, li As String, goodRun As Boolean
Dim parts(100) As String, numParts As Long, valPart1 As Long
Dim readMode As Long '1=datadictionary, 2=contents
Dim varName As String, varFound As Long
Dim i As Long
'clear the pointer numbers for the log items
For i = 1 To numUsedReadVariables
  logItemPointers(i) = 0
Next i
curFileName = Left(IDFFileName, Len(IDFFileName) - 4) 'strip off the extension
curFileName = curFileName & "-" & testName
goodRun = False
fln = FreeFile
On Error Resume Next
Open curFileName & ".err" For Input As fln
If Err.Number <> 0 Then
  'if can't open error file than assumed crashed run
  logItemState(subTestIndex) = 3  'crashed
  Exit Sub
End If
Do While Not EOF(fln)
  Line Input #fln, li
  If InStr(li, "EnergyPlus Terminated--Error(s) Detected") > 0 Then
    logItemState(subTestIndex) = 2 'error
    Close fln
    Exit Sub
  ElseIf InStr(li, "EnergyPlus Completed Successfully--No Errors") > 0 Then
    goodRun = True
    Exit Do
  End If
Loop
Close fln
Debug.Print curFileName, subTestIndex, logItemState(subTestIndex)
' if it was a good run then open ESO file and read values
If goodRun Then
  logItemState(subTestIndex) = 1 'successful
  ' now open and extract results
  readMode = 1 'datadicationary
  fln = FreeFile
  Open curFileName & ".eso" For Input As fln
  If Err.Number <> 0 Then
    'if can't open error file than assumed crashed run
    logItemState(subTestIndex) = 3  'crashed
    Exit Sub
  End If
  Do While Not EOF(fln)
    Line Input #fln, li
    If li = "End of Data Dictionary" Then readMode = 2
    If li = "End of Data" Then Exit Do
    Call commaParse(li, parts, numParts)
    Debug.Print li
    If numParts > 0 Then  'if their is data
      'first item on line is pointer.  If less than 9 than they are "general" pointers that can be ignored
      valPart1 = Int(Val(parts(1)))
      If valPart1 > 9 Then
        If readMode = 1 Then  'if reading data dictionary
          varName = parts(3) & ":" & parts(4)  'this only works for simple records such as variables not the time stamp records
          'assume that every item is a new item
          varFound = 0  'this flag value indicates that no value was found
          For i = 1 To numUsedReadVariables
            If logItemVariables(i) = varName Then
              logItemPointers(i) = valPart1
              varFound = i
              Exit For
            End If
          Next i
          If varFound = 0 Then 'if it was not found then add it to list
            numUsedReadVariables = numUsedReadVariables + 1
            logItemVariables(numUsedReadVariables) = varName
            logItemPointers(numUsedReadVariables) = valPart1
          End If
        Else                  'now reading data
          'This is a "stupid" read routine since it overwrites any item
          'that was found in a previous time step.  This is ok for this
          'application since only one timestep is expected.
          For i = 1 To numUsedReadVariables
            If logItemPointers(i) = valPart1 Then
              logItemResults(subTestIndex, i) = Val(parts(2)) 'grab the value of the second item on record
              Debug.Print "Extracted value", logItemResults(subTestIndex, i)
              Exit For
            End If
          Next i
        End If
      End If
    End If
  Loop
  Close fln
Else
'this should never occur but just incase set it to an "unknown" state
  logItemState(subTestIndex) = 5 'unknown
End If
End Sub

'-----------------------------------------------------------------------------
' this routine creates a log file if it does not exist
' otherwise it adds a row that indicates that it is a new run
'-----------------------------------------------------------------------------
Sub addNewRunToLog()
Dim curFileName As String, fln As Long
Dim i As Long, j As Long
On Error Resume Next
fln = FreeFile
curFileName = Left(IDFFileName, Len(IDFFileName) - 4) 'strip off the extension
curFileName = curFileName & ".rng"
If Dir(curFileName) = "" Then 'this file does not exist so we need to add the headers
  Open curFileName For Output As fln
  If Err.Number <> 0 Then
    MsgBox "Could not create log file - Try a Different Path - Error#: " & Err.Number, vbCritical, "File Error"
    Exit Sub
  End If
  Print #fln, "Created New Range Test Log File: "; Now
  Print #fln,
Else 'now we just need to append a few lines
  Open curFileName For Append As fln
  If Err.Number <> 0 Then
    MsgBox "Could not add to log file - Try a Different Path - Error#: " & Err.Number, vbCritical, "File Error"
    Exit Sub
  End If
  Print #fln,
  Print #fln, "New Suite of Runs: "; Now
  Print #fln,
End If
Close fln
End Sub

'-----------------------------------------------------------------------------
' Write the headers for the range test log file
'-----------------------------------------------------------------------------
Sub writeHeaders(n As Long)
Dim i As Long, j As Long
'write the first line of the log file headers
Print #n, "Index"; vbTab; "TimeStamp"; vbTab; "Class"; vbTab; "Field"; vbTab; "RangeSummary"; vbTab; "MaxError"; vbTab; vbTab;
'write status area of first line
Print #n, "Status"; vbTab;
For i = 1 To numSubTests
  Print #n, vbTab;
Next i
'write sensitivity area of first line
Print #n, vbTab; "Sensitivity"; vbTab; 'blank column after run status
For i = 1 To numUsedReadVariables
  For j = 1 To 5
    Print #n, logItemVariables(i); vbTab; 'just repeat the variable name
  Next j
  Print #n, vbTab;  'leave a group after each sensitivity run
Next i
' state the input values for all subtests
Print #n, vbTab; "Inputs"; vbTab;
For i = 1 To numSubTests
  Print #n, vbTab;
Next i
Print #n, 'end of line
'write the second line of the log file headers
Print #n, "-"; vbTab; "-"; vbTab; "-"; vbTab; "-"; vbTab; "-"; vbTab; "-"; vbTab; vbTab;
'write status area of second line
Print #n, "-"; vbTab;
For i = 1 To numSubTests
  Print #n, subTestNames(i); vbTab;
Next i
' now sensitivity area of second line
Print #n, vbTab; "-"; vbTab; 'blank column after run status
For i = 1 To numUsedReadVariables
  Print #n, subTestNames(STnomMinus); vbTab; subTestNames(STnominal); vbTab; subTestNames(STnomPlus); vbTab;
  Print #n, "Predicted"; vbTab; "AbsError"; vbTab;
  Print #n, vbTab;  'leave a group after each sensitivity run
Next i
' state the input values for all subtests
Print #n, vbTab; vbTab;   'the Inputs block
For i = 1 To numSubTests
  Print #n, subTestNames(i); vbTab;
Next i
Print #n, 'end of line
End Sub

'-----------------------------------------------------------------------------
' this looks at all of the results and displays the answers
'-----------------------------------------------------------------------------
Sub writeLogEntry(testedField As Long, testedClass As Long, testCounter As Long)
Dim fln As Long, i As Long
Dim curFileName As String
Dim logPredValue(numReadVariables) As Double
Dim logPredError(numReadVariables) As Double
Dim logPredErrorMax As Double
Dim goodVariable As Boolean
'before writing results we need to calculate the maximum error
logPredErrorMax = 0
For i = 1 To numUsedReadVariables
    logPredValue(i) = logItemResults(STnominal, i) + (logItemResults(STnominal, i) - logItemResults(STnomMinus, i))
    If logItemResults(STnomPlus, i) <> 0 Then
      logPredError(i) = Abs((logItemResults(STnomPlus, i) - logPredValue(i)) / logItemResults(STnomPlus, i))
    Else
      logPredError(i) = -999
    End If
    If logPredError(i) > logPredErrorMax Then logPredErrorMax = logPredError(i) 'find maximum
Next i
'before writing results, confirm that runs all ran that were supposed to
goodVariable = True
' the nominal cases should all run
If logItemState(STnominal) <> 1 Then goodVariable = False
If logItemState(STnomMinus) <> 1 Then goodVariable = False
If logItemState(STnomPlus) <> 1 Then goodVariable = False
' if any crashed than bad test
If logItemState(STminMinus) = 3 Then goodVariable = False
If logItemState(STminimum) = 3 Then goodVariable = False
If logItemState(STzero) = 3 Then goodVariable = False
If logItemState(STmaximum) = 3 Then goodVariable = False
If logItemState(STmaxPlus) = 3 Then goodVariable = False
' if any unknown than bad test
If logItemState(STminMinus) = 5 Then goodVariable = False
If logItemState(STminimum) = 5 Then goodVariable = False
If logItemState(STzero) = 5 Then goodVariable = False
If logItemState(STmaximum) = 5 Then goodVariable = False
If logItemState(STmaxPlus) = 5 Then goodVariable = False
' if minimum is defined (not zero) than min should run and minminus should error
If IDDField(testedField).minimum <> 0 Then
  If logItemState(STminMinus) <> 2 Then goodVariable = False
  If logItemState(STminimum) <> 1 Then goodVariable = False
End If
' if maximum is defined (not zero) than max should run and maxplus should error
If IDDField(testedField).maximum <> 0 Then
  If logItemState(STmaxPlus) <> 2 Then goodVariable = False
  If logItemState(STmaximum) <> 1 Then goodVariable = False
End If
' open the file and append the current test to it
On Error Resume Next
curFileName = Left(IDFFileName, Len(IDFFileName) - 4) 'strip off the extension
curFileName = curFileName & ".rng"
fln = FreeFile
Open curFileName For Append As fln
If Err.Number <> 0 Then
  MsgBox "Cannot append log file - Error#: " & Err.Number, vbCritical, "File Error"
  Exit Sub
End If
'if first time in suite than add headers
If firstRunInSuite Then
  Call writeHeaders(fln)
  firstRunInSuite = False
End If
'write the record
Print #fln, format(testCounter, "000000"); vbTab;                'test case
Print #fln, Now; vbTab;                        'timestamp
Print #fln, IDDClassDat(testedClass).name; vbTab;  'Class name
Print #fln, IDDField(testedField).name; vbTab;  'Field name
If goodVariable Then
  Print #fln, "Good"; vbTab;                        'range summary
Else
  Print #fln, "Bad"; vbTab;                        'range summary
End If
Print #fln, logPredErrorMax; vbTab;            'maximum error
Print #fln, vbTab;                             'blank column
Print #fln, vbTab;                             'column with title of "STATUS"
For i = 1 To numSubTests
  Select Case logItemState(i)
    Case 0
      Print #fln, "NoEntry"; vbTab;
    Case 1
      Print #fln, "Successful"; vbTab;
    Case 2
      Print #fln, "Error"; vbTab;
    Case 3
      Print #fln, "Crash"; vbTab;
    Case 4
      Print #fln, "NotRun"; vbTab;
    Case 5
      Print #fln, "Unknown"; vbTab;
  End Select
Next i
Print #fln, vbTab;                             'blank column
Print #fln, vbTab;                             'column with title of "Sensitivity"
For i = 1 To numUsedReadVariables
  Print #fln, logItemResults(STnomMinus, i); vbTab; logItemResults(STnominal, i); vbTab; logItemResults(STnomPlus, i); vbTab;
  Print #fln, logPredValue(i); vbTab; logPredError(i); vbTab;
  Print #fln, vbTab;  'leave a group after each sensitivity run
Next i
Print #fln, vbTab;                             'blank column
Print #fln, vbTab;                             'column with title of "Inputs"
For i = 1 To numSubTests
  Print #fln, logItemInputs(i); vbTab;
Next i
Print #fln, 'end of line
Close fln
End Sub

'-----------------------------------------------------------------------------
' Copy the currently selected object to the clipboard
'-----------------------------------------------------------------------------
Sub doCopyObject()
Dim iVal As Long, i As Long, numfields As Long, toClip As String
Dim rowStart As Long, rowEnd As Long, colStart As Long, colEnd As Long
Dim columnSelected As Long
Dim selectedObject As Long
If actObject = 0 Then Exit Sub    'can't duplicate if not on a selected object
grdNew.GetSelection rowStart, colStart, rowEnd, colEnd
'always clear the clipboard before writing to it
Clipboard.Clear
'this string is used to show that the text is an IDF object
toClip = "IDF,"
'loop through the selection to copy multiple columns as necessary
For columnSelected = colStart To colEnd
  selectedObject = grdNew.ColData(columnSelected)
  iVal = IDFObject(selectedObject).valueStart
  numfields = IDDClassDat(actClass).fieldEnd - IDDClassDat(actClass).fieldStart + 1
  'add name of object type
  toClip = toClip & IDDClassDat(actClass).name
  ' now copy values into a string
  For i = 0 To numfields - 1
    toClip = toClip & "," & IDFValue(iVal + i).entry
  Next i
  toClip = toClip & ";"
Next columnSelected
Clipboard.SetText toClip
'enable paste controls
cmdPasteObject.Enabled = True
mnuEditPaste.Enabled = True
End Sub

'-----------------------------------------------------------------------------
' Copy the currently selected object to the clipboard using tab delimting
' appropriate for spreadsheet use.
'-----------------------------------------------------------------------------
Sub doCopyForSpreadsheet()
Dim iVal As Long, jField As Long, numfields As Long, toClip As String
Dim rowStart As Long, rowEnd As Long, colStart As Long, colEnd As Long
Dim columnSelected As Long
Dim selectedObject As Long
Dim iCol
If actObject = 0 Then Exit Sub    'can't duplicate if not on a selected object
grdNew.GetSelection rowStart, colStart, rowEnd, colEnd
'always clear the clipboard before writing to it
Clipboard.Clear
toClip = ""
numfields = IDDClassDat(actClass).fieldEnd - IDDClassDat(actClass).fieldStart + 1
'loop through the selection to copy multiple columns as necessary
For jField = 0 To numfields - 1
  toClip = toClip & IDDField(IDDClassDat(actClass).fieldStart + jField).name & vbTab
  For iCol = colStart To colEnd
    selectedObject = grdNew.ColData(iCol)
    iVal = IDFObject(selectedObject).valueStart
    ' now copy values into a string
    toClip = toClip & IDFValue(iVal + jField).entry
    If iCol < colEnd Then
      toClip = toClip & vbTab
    End If
  Next iCol
  toClip = toClip & vbCrLf
Next jField
Clipboard.SetText toClip
End Sub

'-----------------------------------------------------------------------------
' Returns true or false if the clipboard contains data that may be interpretted
' as an object. It does not validate the object but checks if the general
' text format is consistent with an object in the clipboard. The paste routine
' needs to do the validation.
'-----------------------------------------------------------------------------
Function doesClipContainObject() As Boolean
On Error Resume Next
Dim flag As Boolean
Dim getClip As String
Dim semiLoc As Long
Dim commaLoc As Long
Dim possibleObjName As String
Dim i As Long
flag = False
If Clipboard.GetFormat(vbCFText) Then
  getClip = Clipboard.GetText
  ' If what was put in the clipboard was created by Copy Object in IDF Editor
  ' it has a simple format that is quick to determine if it is an object
  ' that can be pasted into IDF Editor.
  If getClip <> "" Then
    If Left(getClip, 4) = "IDF," Then
      If Right(getClip, 1) = ";" Then
        flag = True
      End If
    End If
  End If
  ' This section of the code if an object is being pasted in from a text file
  ' editor and may have carriage return and line feeds at the end of the line.
  ' It also may contain comment characters "!" and extra spaces. For this
  ' case test if the first string before a comma is the name of an EnergyPlus
  ' object than it is accepted.
  If Not flag Then
    commaLoc = InStr(getClip, ",")
    semiLoc = InStr(getClip, ";")
    If commaLoc > 0 And semiLoc > 0 Then
      possibleObjName = UCase(Trim(Left(getClip, commaLoc - 1)))
      Debug.Print possibleObjName
      For i = 1 To maxUsedIDDClass
        If UCase(IDDClassDat(i).name) = possibleObjName Then
          flag = True
          Exit For
        End If
      Next i
    End If
  End If
End If
doesClipContainObject = flag
End Function

'-----------------------------------------------------------------------------
' Paste the clipboard into the file and display the new object
' Includes validation
' The source of text in the clipboard can be either from an
' IDF Editor copy object or text copied from any other source.
' Due to this the routine needs to be able to handle objects
' with comments (which are ignored) and with carriage returns and
' line feeds.
'-----------------------------------------------------------------------------
Sub doPasteObject()
Dim numParts As Long
Dim parts() As String
Dim objSegments() As String
Dim fromClip As String
Dim found As Long
Dim atLeastOneFound As Boolean
Dim possibleObjectName As String
Dim numfields As Long
Dim wasCollapsedList As Boolean
Dim nextChar As String
Dim isComment As Boolean
Dim curPart As String
Dim i As Long
Dim j As Long
atLeastOneFound = False
If doesClipContainObject Then
  fromClip = Clipboard.GetText
  Debug.Print "Clipboard start:["; fromClip; "]"
  'All characters removed are replaced with spaces
  'remove comments (exclaimation point to carriage return)
  'remove carriage returns
  'remove linefeeds
  isComment = False
  For i = 1 To Len(fromClip)
    nextChar = Mid(fromClip, i, 1)
    Select Case nextChar
      Case "!"
        nextChar = " "
        isComment = True
      Case vbCr
        nextChar = " "
        isComment = False
      Case vbLf
        nextChar = " "
        isComment = False
      Case Else
        If isComment Then
          nextChar = " "
        End If
    End Select
    Mid(fromClip, i, 1) = nextChar
  Next i
  fromClip = Trim(fromClip)
  If Right(fromClip, 1) = ";" Then
    fromClip = Left(fromClip, Len(fromClip) - 1) 'get rid of trailing semicolon
  End If
  If Left(fromClip, 4) = "IDF," Then
    fromClip = Mid(fromClip, 5) 'get rid of leading IDF
  End If
  Debug.Print "Clipboard mid:["; fromClip; "]"
  'fromClip = fromClip & " " - fixed CR6436 by removing this line
  'added to do multiple objects
  objSegments = Split(fromClip, ";")
  For j = 0 To UBound(objSegments)
    parts = Split(objSegments(j), ",")
    numParts = UBound(parts)
    possibleObjectName = UCase(Trim(parts(0))) 'the first field is the name of the object class
    'search through the list of objects to find the matching name
    found = 0
    For i = 1 To maxUsedIDDClass
      If UCase(IDDClassDat(i).name) = possibleObjectName Then
        found = i
        atLeastOneFound = True
        Exit For
      End If
    Next i
    'if the object name was found
    If found > 0 Then
      If mnuViewClassesWithObjs.Checked = True Then
        wasCollapsedList = True
      End If
      If wasCollapsedList Then Call mnuViewClassesWithObjs_Click
      lstObjectTypes.ListIndex = IDDClassObjPt(found).lstObjIndx
      Call lstObjectTypes_Click
      Call IDFNewObject
      'match the number of possible fields and the number of parts
      numfields = IDDClassDat(actClass).fieldEnd - IDDClassDat(actClass).fieldStart + 1
      'If numFields <> numParts - 2 Then
      '  MsgBox "The number of copied fields does not match the number of fields in the target object", vbInformation, "Paste Warning"
      '  If numFields > numParts - 2 Then numFields = numParts - 2
      'End If
      'transfer the values from the clipboard
      Debug.Print
      Debug.Print parts(0)
      For i = 0 To numParts - 1
        curPart = Trim(parts(i + 1))
        Debug.Print "  "; curPart
        IDFValue(maxUsedValue + i - numfields).entry = curPart
      Next i
      If wasCollapsedList Then Call mnuViewClassesWithObjs_Click
    End If
  Next j
  If atLeastOneFound Then 'now update the view
    Call FillList
    lstObjectTypes.ListIndex = IDDClassObjPt(found).lstObjIndx
    Call FillGrid
    'select the last object (should be the one just pasted)
    grdNew.Col = grdNew.Cols - 1
    Call selectCell
    Call ShowFileAltered
  End If
Else
  cmdPasteObject.Enabled = False
  mnuEditPaste.Enabled = False
End If
grdNew.ShowCell grdNew.TopRow, grdNew.Cols - 1
End Sub

'-----------------------------------------------------------------------------
' Search for searchTerm in the Class List (lstObjectTypes)
'-----------------------------------------------------------------------------
Sub findSearchTerm(searchDirection As Integer)
Dim searchTermUpper As String
Dim found As Long
Dim i As Long
If searchTerm <> "" Then
  searchTermUpper = UCase(searchTerm)
  found = -1
  'search upwards
  If searchDirection = findPrev Then
    'search up from current listindex
    For i = lstObjectTypes.ListIndex - 1 To 0 Step -1
      If lstObjectTypes.ItemData(i) > 0 Then
        If InStr(UCase(lstObjectTypes.List(i)), searchTermUpper) > 0 Then
          found = i
          Exit For
        End If
      End If
    Next i
    'search from end of list back to listindex
    If found = -1 Then
      For i = lstObjectTypes.ListCount - 1 To lstObjectTypes.ListIndex + 1 Step -1
        If lstObjectTypes.ItemData(i) > 0 Then
          If InStr(UCase(lstObjectTypes.List(i)), searchTermUpper) > 0 Then
            found = i
            Exit For
          End If
        End If
      Next i
    End If
  'search downward
  Else
    'search down from current listindex
    For i = lstObjectTypes.ListIndex + 1 To lstObjectTypes.ListCount - 1
      If lstObjectTypes.ItemData(i) > 0 Then
        If InStr(UCase(lstObjectTypes.List(i)), searchTermUpper) > 0 Then
          found = i
          Exit For
        End If
      End If
    Next i
    'search from end of list back to listindex
    If found = -1 Then
      For i = 0 To lstObjectTypes.ListIndex - 1
        If lstObjectTypes.ItemData(i) > 0 Then
          If InStr(UCase(lstObjectTypes.List(i)), searchTermUpper) > 0 Then
            found = i
            Exit For
          End If
        End If
      Next i
    End If
  End If
  If found = -1 Then
    MsgBox "Did not find: " & searchTerm, vbInformation, "Find Results"
  Else
    'MsgBox "Found: " & found, vbInformation, "Find Results"
    lstObjectTypes.ListIndex = found
    'Call lstObjectTypes_Click
  End If
End If
End Sub

'-----------------------------------------------------------------------------
' Update the variables that hold the names of variables and meters from
' the RDD file
'
'   autoObjListVar     holds list in format appropriate for pull down list
'   autoObjListMeter   holds list in format appropriate for pull down list
'   autoObjListTimeStamp
'----------------------------------------------------------------------------
Sub updateAutoObjLists()
Dim rddFileName As String
Dim mddFileName As String
Dim curFileDate As Date
Dim i As Integer
On Error Resume Next
If Not IDFUntitled Then
  'use the name of the idf file for the name of the rdd file
  If UCase(Right(IDFFileName, 4)) = ".IDF" Then
    rddFileName = Left(IDFFileName, Len(IDFFileName) - 3) & "rdd"
    mddFileName = Left(IDFFileName, Len(IDFFileName) - 3) & "mdd"
    'the rdd and mdd should have the same time stamp so only check the
    'rdd file time and date
    curFileDate = FileDateTime(rddFileName)
    If Err.Number <> 0 Then
      autoObjListVar = ""
      autoObjListMeter = ""
      Exit Sub
    End If
    If curFileDate > autoObjListTimeStamp Then
      Call readVarMeterDictionary(rddFileName, autoObjListVar, False)
      Call readVarMeterDictionary(mddFileName, autoObjListMeter, True)
      autoObjListTimeStamp = curFileDate
    End If
  End If
Else
  autoObjListVar = ""
  autoObjListMeter = ""
End If
End Sub

'-----------------------------------------------------------------------------
' Read the external file that contains the variable dictionary
'-----------------------------------------------------------------------------
Sub readVarMeterDictionary(fileName As String, autoObjList As String, mddFile As Boolean)
Dim headerLine1 As String
Dim headerLine2 As String
Dim zoneHVAC As String
Dim asterisk As String
Dim duration As String
Dim typeOfLine As String
Dim varMetName As String
Dim bracketLoc As Integer
Dim fln As Integer
On Error Resume Next
fln = FreeFile
Open fileName For Input As fln
If Err.Number <> 0 Then
  autoObjList = ""
  Exit Sub
End If
Line Input #fln, headerLine1
Line Input #fln, headerLine2
'read the rdd and mdd files like they are the original format
'or the object like format:
'
' original:
'  Zone,Average,Outdoor Dry Bulb [C]
'
' object like format (IDF)
'  Report Variable,*,Outdoor Dry Bulb,hourly; !- Zone Average [C]
'  Report Meter,Electricity:Facility,hourly; !- [J]
'
If InStr(headerLine2, "Objects") > 0 Then
  Do While Not EOF(fln)
    If mddFile Then
      Input #fln, typeOfLine, varMetName, duration
    Else
      Input #fln, typeOfLine, asterisk, varMetName, duration
    End If
    autoObjList = autoObjList & varMetName & "|"
  Loop
Else
  Do While Not EOF(fln)
    Input #fln, zoneHVAC, typeOfLine, varMetName
    'strip the brackets off the end of the name
    bracketLoc = InStr(varMetName, "[")
    If bracketLoc > 0 Then
      varMetName = Left(varMetName, bracketLoc - 1)
    End If
    autoObjList = autoObjList & varMetName & "|"
  Loop
End If
Close fln
End Sub

'-----------------------------------------------------------------------------
' Go through the values of the file and display summary of results when values
' are invalid:
'   Numeric values out of range
'   Invalid references
'   Invalid list choices
'-----------------------------------------------------------------------------
Sub doValidityCheck(validityMsg() As String, classForMsg() As Long, objForMsg() As Long, fldForMsg() As Long)
Dim classIndex As Long
Dim firstField As Long
Dim numfields As Long
Dim firstValue As Long
Dim curNumericValue As Double
Dim curAlphaValue As String
Dim outOfRange As Boolean
Dim anyInObjectOutOfRange As Boolean
Dim minimumVal As Double
Dim maximumVal As Double
Dim curField As Long
Dim iObject As Long
Dim j As Long
Dim kClass As Long
Dim fieldIndx As Long
Dim valueIndx As Long
Dim numValidityMsg As Long
Dim sizeValidityMsg As Long
Dim newMsg As String
'resize flag arrays
sizeValidityMsg = 100
numValidityMsg = 0
ReDim validityMsg(sizeValidityMsg)
ReDim classForMsg(sizeValidityMsg)
ReDim objForMsg(sizeValidityMsg)
ReDim fldForMsg(sizeValidityMsg)
'now search through all the objects looking for invalid values
For iObject = 1 To maxUsedObject
  If IDFObject(iObject).nextObjectInClass <> isDeleted Then
    anyInObjectOutOfRange = False
    classIndex = IDFObject(iObject).classType
    firstValue = IDFObject(iObject).valueStart
    firstField = IDDClassDat(classIndex).fieldStart
    numfields = IDDClassDat(classIndex).fieldEnd - firstField + 1
    For j = 1 To numfields
      outOfRange = False
      valueIndx = firstValue + j - 1
      fieldIndx = firstField + j - 1
      curAlphaValue = IDFValue(valueIndx).entry
      newMsg = "" 'clear the current message
      Select Case IDDField(fieldIndx).type
        Case 1, 2 'real or long
          If IDFValue(valueIndx).entry <> "" And LCase(IDFValue(valueIndx).entry) <> "autocalculate" And LCase(IDFValue(valueIndx).entry) <> "autosize" Then
            curNumericValue = Val(curAlphaValue)
            minimumVal = IDDField(fieldIndx).minimum
            maximumVal = IDDField(fieldIndx).maximum
            If IDDField(fieldIndx).maxSpecified Then
              If IDDField(fieldIndx).exclusiveMax Then
                If curNumericValue >= maximumVal Then newMsg = "Exceed maximum value"
              Else
                If curNumericValue > maximumVal Then newMsg = "Exceed maximum value"
              End If
            End If
            If IDDField(fieldIndx).minSpecified Then
              If IDDField(fieldIndx).exclusiveMin Then
                If curNumericValue <= minimumVal Then newMsg = "Below minimum value"
              Else
                If curNumericValue < minimumVal Then newMsg = "Below minimum value"
              End If
            End If
          End If
        Case 4 'choice
          If Not isChoiceValid(curAlphaValue, fieldIndx) Then 'call function that determines if the choice is valid
            newMsg = "Invalid choice"
          End If
        Case 5 'object list
          If Not isReferenceValid(curAlphaValue, fieldIndx) Then 'call function that determines if the reference is valid
            newMsg = "Invalid reference"
          End If
      End Select
      ' if user wants macros to be ignored
      If ignoreValidatingMacros Then
        If InStr(IDFValue(valueIndx).entry, "[") > 0 Then newMsg = ""
        If InStr(IDFValue(valueIndx).entry, "@") > 0 Then newMsg = ""
        If InStr(IDFValue(valueIndx).entry, "$") > 0 Then newMsg = ""
        If InStr(IDFValue(valueIndx).entry, "=") > 0 Then newMsg = ""
        If InStr(IDFValue(valueIndx).entry, "#") > 0 Then newMsg = ""
      End If
      If IDDField(fieldIndx).required And Trim(curAlphaValue) = "" And Not IDDField(fieldIndx).defSpecified Then
        newMsg = "Blank value for required field"
      End If
      If newMsg <> "" Then
        numValidityMsg = numValidityMsg + 1
        If numValidityMsg > sizeValidityMsg Then
          sizeValidityMsg = sizeValidityMsg * 2
          ReDim Preserve validityMsg(sizeValidityMsg)
          ReDim Preserve classForMsg(sizeValidityMsg)
          ReDim Preserve objForMsg(sizeValidityMsg)
          ReDim Preserve fldForMsg(sizeValidityMsg)
        End If
        validityMsg(numValidityMsg) = newMsg & ": " & curAlphaValue & "  (" & IDDClassDat(classIndex).name & " // " & IDFValue(firstValue).entry & " // " & IDDField(fieldIndx).name & ")"
        classForMsg(numValidityMsg) = classIndex
        objForMsg(numValidityMsg) = iObject
        fldForMsg(numValidityMsg) = fieldIndx
      End If
    Next j
  End If
Next iObject
For kClass = 1 To maxUsedIDDClass
  ' do object level checking here
  ' check if the objects that are supposed to be unique are only present once
  newMsg = ""
  If IDDClassDat(kClass).uniqueObject Then
    If IDDClassObjPt(kClass).objectCount > 1 Then
      newMsg = "Multiple instances of the following object are present in the file but the file should have only one of these objects: (" & IDDClassDat(kClass).name & ")"
    End If
  End If
  If newMsg <> "" Then
    numValidityMsg = numValidityMsg + 1
    If numValidityMsg > sizeValidityMsg Then
      sizeValidityMsg = sizeValidityMsg * 2
      ReDim Preserve validityMsg(sizeValidityMsg)
      ReDim Preserve classForMsg(sizeValidityMsg)
      ReDim Preserve objForMsg(sizeValidityMsg)
      ReDim Preserve fldForMsg(sizeValidityMsg)
    End If
    validityMsg(numValidityMsg) = newMsg
    classForMsg(numValidityMsg) = kClass
    objForMsg(numValidityMsg) = IDDClassObjPt(kClass).objectStart
    fldForMsg(numValidityMsg) = IDDClassDat(kClass).fieldStart
  End If
Next kClass
End Sub

'-----------------------------------------------------------------------------
' Check the provided string and field reference value to see if the reference
' is ok or not.
'-----------------------------------------------------------------------------
Function isReferenceValid(curString As String, indexField As Long) As Boolean
Dim iListOfObjList As Long
Dim curObjListName As Long
Dim curObjListItem As Long
Dim refField As Long
Dim refClass As Long
Dim refObject As Long
Dim deltaField As Long
Dim StartVal As Long
Dim found As Boolean
found = False
If curString = "" Then
  isReferenceValid = True 'always show as true if blank
ElseIf IDDField(indexField).autoObjList <> 0 Then 'not any kind of automatically variable or meter list
  isReferenceValid = True 'always show as true variable or meter list (automatic list)
Else
  For iListOfObjList = IDDField(indexField).listOfObjListStart To IDDField(indexField).listOfObjListEnd
    curObjListName = ListOfObjList(iListOfObjList)
    If curObjListName > 0 Then
      curObjListItem = IDDObjListName(curObjListName).objListItemStart
      Do While curObjListItem > 0
        refClass = IDDObjListItem(curObjListItem).classWithRef
        If IDDObjListItem(curObjListItem).fieldWithRef <> fwrClassName Then 'for normal \reference
          refField = IDDObjListItem(curObjListItem).fieldWithRef
          'Debug.Print "  Scanning through "; IDDClassDat(refClass).name, IDDField(refField).name
          refObject = IDDClassObjPt(refClass).objectStart
          deltaField = refField - IDDClassDat(refClass).fieldStart
          Do While refObject > 0
            StartVal = IDFObject(refObject).valueStart
            'the following line actually adds items to the pull down list
            If UCase(Trim(curString)) = UCase(Trim(IDFValue(StartVal + deltaField).entry)) Then
              found = True
              Exit Do
            End If
            refObject = IDFObject(refObject).nextObjectInClass
          Loop
        Else 'for \reference-class-name
          If UCase(curString) = UCase(IDDClassDat(refClass).name) Then
            found = True
          End If
        End If
        If found Then 'if already matched a field don't need to look at read of lists
          Exit For
        End If
        curObjListItem = IDDObjListItem(curObjListItem).nextObjListItem
      Loop
    End If
  Next iListOfObjList
  isReferenceValid = found
End If
End Function


'-----------------------------------------------------------------------------
' Check the provided string and field reference to see if the choice
' is ok or not.
'-----------------------------------------------------------------------------
Function isChoiceValid(curString As String, indexField As Long) As Boolean
Dim found As Boolean
Dim curStringUpper As String
Dim i As Long
If curString = "" Then
  isChoiceValid = True 'always show as true if blank
ElseIf IDDField(indexField).choiceStart = 0 Then
  isChoiceValid = True 'always show true if no choices provided
Else
  found = False
  curStringUpper = UCase(curString)
  For i = IDDField(indexField).choiceStart To IDDField(indexField).choiceEnd
    If curStringUpper = UCase(IDDChoice(i)) Then
      found = True
      Exit For
    End If
  Next i
  If found Then isChoiceValid = True
End If
End Function

'-----------------------------------------------------------------------------
'This displays errors in a pop up dialog box
'-----------------------------------------------------------------------------
Function errDisplayChild(msg As String, section As String) As Boolean
Dim response As Integer
errCnt = errCnt + 1
If errCnt > 10 Then
  MsgBox "Too many errors were found when working with the file. Please examine the file in a text editor to fix.", vbCritical, "ERROR"
  errCnt = 0
  errDisplayChild = True 'signals that no more processing should occur and the file be closed.
Else
  'check for version here and if an old version add to the message and make ok-cancel choice otherwise just leave message along and us ok only
  Call getIDFVersion(True)
  If isVersionMismatch Then
    response = MsgBox(msg + _
    vbCrLf + vbCrLf + "============================================================================================================" + _
    vbCrLf + vbCrLf + "The IDF file is version " + IDFVersion + " but this does not match the version of EnergyPlus: " + IDDVersion + _
    vbCrLf + "Please press CANCEL to stop processing this file or OK to continue processing this file", vbOKCancel + vbQuestion, section)
    If response = vbCancel Then
      errDisplayChild = True
    Else
      errDisplayChild = False
    End If
  Else
    MsgBox msg, vbQuestion, section
    errDisplayChild = False 'signals that processing can continue.
  End If
End If
End Function

'-----------------------------------------------------------------------------
' CEPTChange: Fills the Classes combo based on selection of ClassCategory
'-----------------------------------------------------------------------------
Sub FillClassCombo(ClassCategory As String)
Dim i As Long, j As Long, t As String
Dim topItem As Long
Dim cntObjUsed As Long
Dim curShowAllClasses As Boolean
topItem = lstObjectTypes.TopIndex

'check if any objects are used and if not do not allow
'the showAllClasses = false to be activated
curShowAllClasses = showAllClasses
If Not showAllClasses Then
  For i = 1 To maxUsedIDDClass
    cntObjUsed = cntObjUsed + IDDClassObjPt(i).objectCount
  Next i
  If cntObjUsed = 0 Then
    showAllClasses = True
    mnuViewClassesWithObjs.Checked = False
    curShowAllClasses = True
  End If
End If
For i = 1 To maxUsedClassGroup ' get index of IDDClassGroup
  If IDDClassGroup(i).name = ClassCategory Then
    Exit For
  End If
Next i
cboClasses.Clear
For j = IDDClassGroup(i).classStart To IDDClassGroup(i + 1).classStart - 1
  'Debug.Print "Object Count", IDDClassdat(j).name, iddclassdat(j).objectCount
  If curShowAllClasses Then
    If IDDClassObjPt(j).objectCount > 0 Then
      t = Right("0000" & RTrim(LTrim(Str(IDDClassObjPt(j).objectCount))), 4)
    Else
      t = "------"
    End If
    cboClasses.AddItem IDDClassDat(j).name ' CEPTChange: Added to fill class combo
    cboClasses.ItemData(cboClasses.NewIndex) = j ' CEPTChange: Added to fill class combo
  Else
    If IDDClassObjPt(j).objectCount > 0 Then
      t = Right("0000" & RTrim(LTrim(Str(IDDClassObjPt(j).objectCount))), 4)
      cboClasses.AddItem IDDClassDat(j).name ' CEPTChange: Added to fill class combo
      cboClasses.ItemData(cboClasses.NewIndex) = j ' CEPTChange: Added to fill class combo
    End If
  End If
Next j
End Sub

'-----------------------------------------------------------------------------
' Add to the recently used node name list that is displayed in the node dialog
'-----------------------------------------------------------------------------
Sub addToRecentlyUsedNodeNames(nameIn As String)
Dim found As Boolean
Dim i As Integer
If mostRecentUsedNodeName = 0 Then 'first item
  recentlyUsedNodeNames(1) = nameIn
  mostRecentUsedNodeName = 1
Else 'adding to a list that already contains items
  'first make sure not already on the list
  found = False
  For i = 1 To UBound(recentlyUsedNodeNames)
    If recentlyUsedNodeNames(i) = nameIn Then
      found = True
      Exit For
    End If
  Next i
  If Not found Then
    mostRecentUsedNodeName = mostRecentUsedNodeName + 1
    If mostRecentUsedNodeName > UBound(recentlyUsedNodeNames) Then
      mostRecentUsedNodeName = 1
    End If
    recentlyUsedNodeNames(mostRecentUsedNodeName) = nameIn
    End If
End If
End Sub

'-----------------------------------------------------------------------------
' The working routine that is called by search and replace dialog
' to find the item being searched
'-----------------------------------------------------------------------------
Public Sub searchFind(inSearchTerm As String, inIsEntireFieldValue As Boolean, outIdfValue() As String, outIdfName() As String, outIddObj() As Long, outIddFld() As Long, outIdfValueIndx() As Long, outIdfObjIndx() As Long)
Dim iObject As Long
Dim jField As Long
Dim classIndex As Long
Dim firstValue As Long
Dim firstField As Long
Dim numfields As Long
Dim curValue As String
Dim fieldIndx As Long
Dim valueIndx As Long
Dim searchTermUpper As String
Dim sizeOutArrays As Long
Dim maxUsedOutArrays As Long
Dim found As Boolean
maxUsedOutArrays = 0
sizeOutArrays = 10
ReDim outIdfValue(sizeOutArrays)
ReDim outIdfName(sizeOutArrays)
ReDim outIddObj(sizeOutArrays)
ReDim outIddFld(sizeOutArrays)
ReDim outIdfValueIndx(sizeOutArrays)
ReDim outIdfObjIndx(sizeOutArrays)
searchTermUpper = UCase(inSearchTerm)
For iObject = 1 To maxUsedObject
  If IDFObject(iObject).nextObjectInClass <> isDeleted Then
    classIndex = IDFObject(iObject).classType
    firstValue = IDFObject(iObject).valueStart
    firstField = IDDClassDat(classIndex).fieldStart
    numfields = IDDClassDat(classIndex).fieldEnd - firstField + 1
    For jField = 1 To numfields
      valueIndx = firstValue + jField - 1
      fieldIndx = firstField + jField - 1
      curValue = IDFValue(valueIndx).entry
      'test if string has been found
      found = False
      If inIsEntireFieldValue Then  'entire field value
        If UCase(curValue) = searchTermUpper Then found = True
      Else 'search for only the substring
        If InStr(UCase(curValue), searchTermUpper) > 0 Then found = True
      End If
      If found Then
        maxUsedOutArrays = maxUsedOutArrays + 1
        'if arrays are not large enough to hold another result increase their size
        If maxUsedOutArrays > sizeOutArrays Then
          sizeOutArrays = sizeOutArrays * 2
          ReDim Preserve outIdfValue(sizeOutArrays)
          ReDim Preserve outIdfName(sizeOutArrays)
          ReDim Preserve outIddObj(sizeOutArrays)
          ReDim Preserve outIddFld(sizeOutArrays)
          ReDim Preserve outIdfValueIndx(sizeOutArrays)
          ReDim Preserve outIdfObjIndx(sizeOutArrays)
        End If
        outIdfValue(maxUsedOutArrays) = curValue
        outIdfValueIndx(maxUsedOutArrays) = valueIndx
        outIdfName(maxUsedOutArrays) = IDFValue(firstValue).entry
        outIddObj(maxUsedOutArrays) = classIndex
        outIddFld(maxUsedOutArrays) = fieldIndx
        outIdfObjIndx(maxUsedOutArrays) = iObject
      End If
    Next jField
  End If
Next iObject
End Sub

'-----------------------------------------------------------------------------
' The working routine that is called by search and replace dialog
' to replace the selected items found by previous search
' The routine double checks if the value being searched is actually
' in the value array but does not do any other searching.
'-----------------------------------------------------------------------------
Public Sub replaceFound(inSearchTerm As String, inReplaceTerm As String, inIsEntireFieldValue As Boolean, inSelectToReplace() As Boolean, inIdfValueIndx() As Long, outCount As Long)
Dim origValue As String
Dim newValue As String
Dim curValInd As Long
Dim searchTermUpper As String
Dim origValueUpper As String
Dim locSearchTerm As Integer
Dim i As Long
If inSearchTerm = "" Then Exit Sub
If inReplaceTerm = "" Then Exit Sub
outCount = 0
searchTermUpper = UCase(inSearchTerm)
For i = 1 To UBound(inSelectToReplace)
  If inSelectToReplace(i) Then
    curValInd = inIdfValueIndx(i)
    'make sure index is in valid range
    newValue = ""
    If curValInd > 0 And curValInd <= maxUsedValue Then
      ' depending on if entire value is being replaced or not
      origValue = IDFValue(curValInd).entry
      origValueUpper = UCase(origValue)
      If inIsEntireFieldValue Then
        If UCase(origValue) = searchTermUpper Then 'this should always be the case since it was previously found
          newValue = inReplaceTerm
        Else
          newValue = origValue
        End If
      Else
        locSearchTerm = InStr(origValueUpper, searchTermUpper)
        If locSearchTerm > 0 Then 'this should always be the case since it was previously found
          If locSearchTerm > 1 Then
            newValue = Left(origValue, locSearchTerm - 1) 'pick up the text to the left of found text
          End If
          newValue = newValue & inReplaceTerm 'add replaced term
          If locSearchTerm + Len(inSearchTerm) <= Len(origValue) Then
            newValue = newValue & Mid(origValue, locSearchTerm + Len(inSearchTerm))  'add the rest of original string
          End If
        Else
          newValue = origValue
        End If
      End If
      IDFValue(curValInd).entry = newValue
      outCount = outCount + 1
    End If
  End If
Next i
Call FillGrid
End Sub

'-----------------------------------------------------------------------------
' Move to a new class and object and field for current selection
' used in external search and replace function for goto
'-----------------------------------------------------------------------------
Public Sub jumpToObjectField(inClass As Long, inObj As Long, inFldIndx As Long)
Dim i As Long
Dim foundClass As Long
Dim foundCol As Long
Dim foundRow As Long
'search for and go to the correct class
foundClass = 0
For i = 0 To lstObjectTypes.ListCount - 1
  If lstObjectTypes.ItemData(i) = inClass Then
    foundClass = i
    Exit For
  End If
Next i
If foundClass > 0 Then
  lstObjectTypes.Selected(i) = True
End If
Call FillGrid
'search for and go to the correct object in the class
foundCol = 0
For i = 1 To grdNew.Cols - 1
  If grdNew.ColData(i) = inObj Then
    foundCol = i
    Exit For
  End If
Next i
'compute row
foundRow = 2
If inFldIndx >= IDDClassDat(inClass).fieldStart Then
  If inFldIndx <= IDDClassDat(inClass).fieldEnd Then
    foundRow = 1 + inFldIndx - IDDClassDat(inClass).fieldStart
  End If
End If
If foundCol > 0 Then
  grdNew.Select foundRow, foundCol
  grdNew.ShowCell foundRow, foundCol
End If
End Sub


'-----------------------------------------------------------------------------
' If multiple cells are selected and Fill Right is used then the value
' of left most cells are placed into the remaining cells selected
' Note that control-D is used because it stands for "dextral"
'-----------------------------------------------------------------------------
Sub fillGridToRight()
Dim rowSt As Long, rowEnd As Long, colSt As Long, colEnd As Long
Dim iRow As Long
Dim iCol As Long
Dim curActRow As Long
Dim sourceObject As Long
Dim sourceValue As Long
Dim destObject As Long
Dim destValue As Long
Dim li As Long

grdNew.GetSelection rowSt, colSt, rowEnd, colEnd
If colEnd > colSt Then 'is more than column selected if so then can fill right
  li = lstObjectTypes.ListIndex
  For iRow = rowSt To rowEnd
    curActRow = iRow - 1
    sourceObject = grdNew.ColData(colSt)
    sourceValue = IDFObject(sourceObject).valueStart + curActRow
    For iCol = colSt + 1 To colEnd 'for each additional column
      destObject = grdNew.ColData(iCol)
      destValue = IDFObject(destObject).valueStart + curActRow
      IDFValue(destValue).entry = IDFValue(sourceValue).entry
      IDFValue(destValue).isExpression = IDFValue(sourceValue).isExpression
      IDFValue(destValue).leadingSpaces = IDFValue(sourceValue).leadingSpaces
    Next iCol
  Next iRow
  Call ShowFileAltered
  Call FillList
  lstObjectTypes.ListIndex = li
  Call FillGrid
  Call selectCell
  grdNew.Select rowSt, colSt, rowEnd, colEnd
End If
End Sub

'-----------------------------------------------------------------------------
' Create list of other locations that use the selected cell
'-----------------------------------------------------------------------------
Sub updateJumpList()
Dim iObject As Long
Dim classIndex As Long
Dim firstValue As Long
Dim firstField As Long
Dim numfields  As Long
Dim valueIndx As Long
Dim fieldIndx As Long
Dim jFld As Long
Dim curFieldName As String
Dim menuCounter As Integer
Dim i As Integer

menuCounter = 0
For i = 1 To maxJumpMenuSize
  mnuJumpItem(i).Visible = False
  mnuJumpItem(i).Caption = ""
  jmpClass(i) = 0
  jmpObj(i) = 0
  jmpFld(i) = 0
Next i

'first the name of the name of the field that is being search
If IDFValue(actValue).entry <> "" Then
  If IDDField(actField).type = 6 Or IDDField(actField).type = 5 Then 'if node or reference
    curFieldName = IDFValue(actValue).entry
  ElseIf actField = IDDClassDat(actClass).fieldStart And IDDField(actField).type = 3 Then 'or if first field and text
    curFieldName = IDFValue(actValue).entry
  End If
End If
Debug.Print "Finding places to jump based on: "; curFieldName
If curFieldName <> "" Then
  firstValue = IDFObject(actObject).valueStart
  firstField = IDDClassDat(actClass).fieldStart
  For iObject = 1 To maxUsedObject
    If IDFObject(iObject).nextObjectInClass <> isDeleted Then
      classIndex = IDFObject(iObject).classType
      firstValue = IDFObject(iObject).valueStart
      firstField = IDDClassDat(classIndex).fieldStart
      numfields = IDDClassDat(classIndex).fieldEnd - firstField + 1
      For jFld = 1 To numfields
        valueIndx = firstValue + jFld - 1
        fieldIndx = firstField + jFld - 1
        'check if a node or a reference or name in first field of object
        If IDDField(fieldIndx).type = 6 Or IDDField(fieldIndx).type = 5 Or (jFld = 1 And IDDField(fieldIndx).type = 3) Then
          If IDFValue(valueIndx).entry = curFieldName Then
            mnuJumpItem(menuCounter).Caption = IDFValue(firstValue).entry & " [" & IDDClassDat(classIndex).name & "]"
            mnuJumpItem(menuCounter).Visible = True
            jmpClass(menuCounter) = classIndex
            jmpObj(menuCounter) = iObject
            jmpFld(menuCounter) = fieldIndx
            menuCounter = menuCounter + 1
            If menuCounter > maxJumpMenuSize Then Exit For
          End If
        End If
      Next jFld
    End If
    If menuCounter > 20 Then Exit For
  Next iObject
Else
  mnuJumpItem(0).Caption = "No destination"
  mnuJumpItem(0).Visible = True
End If
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
