VERSION 5.00
Object = "{48E59290-9880-11CF-9754-00AA00C00908}#1.0#0"; "MSINET.OCX"
Object = "{831FDD16-0C5C-11D2-A9FC-0000F8754DA1}#2.1#0"; "MSCOMCTL.OCX"
Object = "{F9043C88-F6F2-101A-A3C9-08002B2F49FB}#1.2#0"; "ComDlg32.OCX"
Begin VB.Form eplUI 
   BorderStyle     =   1  'Fixed Single
   Caption         =   "EP-Launch"
   ClientHeight    =   7455
   ClientLeft      =   1050
   ClientTop       =   1620
   ClientWidth     =   8835
   Icon            =   "epl-ui.frx":0000
   LinkTopic       =   "Form1"
   MaxButton       =   0   'False
   ScaleHeight     =   7455
   ScaleWidth      =   8835
   StartUpPosition =   1  'CenterOwner
   Begin VB.Frame frameUtility 
      BorderStyle     =   0  'None
      Caption         =   "Frame1"
      Height          =   6255
      Left            =   240
      TabIndex        =   28
      Top             =   480
      Width           =   8295
      Begin VB.CommandButton cmdUtilityOutput6 
         Caption         =   "Open 6"
         Height          =   375
         Left            =   3960
         TabIndex        =   104
         Top             =   4680
         Width           =   1785
      End
      Begin VB.CommandButton cmdUtilityOutput5 
         Caption         =   "Open 5"
         Height          =   375
         Left            =   2040
         TabIndex        =   103
         Top             =   4680
         Width           =   1785
      End
      Begin VB.CommandButton cmdUtilityOutput4 
         Caption         =   "Open 4"
         Height          =   375
         Left            =   120
         TabIndex        =   102
         Top             =   4680
         Width           =   1785
      End
      Begin VB.CommandButton cmdUtilityOutput3 
         Caption         =   "Open 3"
         Height          =   375
         Left            =   3960
         TabIndex        =   44
         Top             =   4200
         Width           =   1785
      End
      Begin VB.CommandButton cmdUtilityOutput2 
         Caption         =   "Open 2"
         Height          =   375
         Left            =   2040
         TabIndex        =   43
         Top             =   4200
         Width           =   1785
      End
      Begin VB.ComboBox cmbUtility 
         Height          =   315
         Left            =   600
         Style           =   2  'Dropdown List
         TabIndex        =   42
         Top             =   120
         Width           =   1815
      End
      Begin VB.Frame frameUtilityWeather 
         Caption         =   "Weather File"
         Height          =   1215
         Left            =   120
         TabIndex        =   36
         Top             =   2880
         Width           =   8175
         Begin VB.CommandButton cmdUtilityWeatherBrowse 
            Caption         =   "Browse.."
            Height          =   375
            Left            =   240
            TabIndex        =   37
            Top             =   720
            Width           =   1215
         End
         Begin VB.Label lblUtilityWeatherFile 
            Caption         =   "<file path>\<file name>"
            Height          =   495
            Left            =   120
            TabIndex        =   38
            Top             =   240
            Width           =   7935
         End
      End
      Begin VB.Frame frameUtilityInput 
         Caption         =   "Input File"
         Height          =   1215
         Left            =   120
         TabIndex        =   33
         Top             =   1560
         Width           =   8175
         Begin VB.CommandButton cmdUtilityIDFEdit 
            Caption         =   "Edit - IDF Editor"
            Height          =   375
            Left            =   6720
            TabIndex        =   41
            Top             =   720
            Width           =   1335
         End
         Begin VB.CommandButton cmdUtilityTextEdit 
            Caption         =   "Edit - Text Editor"
            Height          =   375
            Left            =   5280
            TabIndex        =   40
            Top             =   720
            Width           =   1335
         End
         Begin VB.CommandButton cmdUtilityInputBrowse 
            Caption         =   "Browse.."
            Height          =   375
            Left            =   240
            TabIndex        =   34
            Top             =   720
            Width           =   1215
         End
         Begin VB.Label lblUtilityInputFile 
            Caption         =   "<file path>\<file name>"
            Height          =   495
            Left            =   120
            TabIndex        =   35
            Top             =   240
            Width           =   7935
         End
      End
      Begin VB.Frame frameUtilityAbout 
         Caption         =   "About"
         Height          =   975
         Left            =   120
         TabIndex        =   32
         Top             =   480
         Width           =   8175
         Begin VB.Label lblUtilityAbout 
            Caption         =   "lt"
            Height          =   615
            Left            =   120
            TabIndex        =   39
            Top             =   240
            Width           =   7935
            WordWrap        =   -1  'True
         End
      End
      Begin VB.CommandButton cmdUtilityOutput1 
         Caption         =   "Open 1"
         Height          =   375
         Left            =   120
         TabIndex        =   31
         Top             =   4200
         Width           =   1785
      End
      Begin VB.CommandButton cmdUtilityRun 
         Caption         =   "Run Utility"
         Height          =   375
         Left            =   6000
         TabIndex        =   30
         Top             =   4200
         Width           =   2175
      End
      Begin VB.Label Label2 
         Caption         =   "Utility"
         Height          =   255
         Left            =   120
         TabIndex        =   45
         Top             =   160
         Width           =   495
      End
   End
   Begin VB.Frame frameSingle 
      BorderStyle     =   0  'None
      Caption         =   "frameSingle"
      Height          =   6255
      Left            =   240
      TabIndex        =   1
      Top             =   480
      Width           =   8295
      Begin VB.Timer queueTimer 
         Interval        =   500
         Left            =   4920
         Top             =   5760
      End
      Begin VB.CommandButton cmdSimulate 
         Caption         =   "Simulate..."
         Height          =   375
         Left            =   6720
         TabIndex        =   14
         Top             =   5880
         Width           =   1575
      End
      Begin VB.Frame frameView 
         Caption         =   "View Results"
         Height          =   2775
         Left            =   0
         TabIndex        =   12
         Top             =   3000
         Width           =   8295
         Begin VB.Frame frameViewAllOut 
            BorderStyle     =   0  'None
            Height          =   2175
            Left            =   600
            TabIndex        =   53
            Top             =   360
            Width           =   7455
            Begin VB.CommandButton cmdSpSZ 
               Caption         =   "SpSZ"
               Height          =   255
               Left            =   6360
               TabIndex        =   106
               ToolTipText     =   "Zone sizing and flow rates"
               Top             =   1560
               Width           =   1020
            End
            Begin VB.CommandButton cmdPerfLogCSV 
               Caption         =   "PerfLog.csv"
               Height          =   255
               Left            =   6360
               TabIndex        =   105
               ToolTipText     =   "Basement preprocessor monthly ground temperatures"
               Top             =   1200
               Width           =   1020
            End
            Begin VB.CommandButton cmdTblXML 
               Caption         =   "Table XML"
               Height          =   255
               Left            =   6360
               TabIndex        =   101
               ToolTipText     =   "XML version of tabular output report which contains summarized results"
               Top             =   720
               Width           =   1020
            End
            Begin VB.CommandButton cmdSlabErr 
               Caption         =   "Slab Err"
               Height          =   255
               Left            =   5280
               TabIndex        =   100
               ToolTipText     =   "Slab preprocessor error file."
               Top             =   1920
               Width           =   900
            End
            Begin VB.CommandButton cmdSlab 
               Caption         =   "Slab"
               Height          =   255
               Left            =   5280
               TabIndex        =   99
               ToolTipText     =   "Slab preprocessor IDF intermediate file."
               Top             =   1560
               Width           =   900
            End
            Begin VB.CommandButton cmdSlabOut 
               Caption         =   "Slab Out"
               Height          =   255
               Left            =   5280
               TabIndex        =   98
               ToolTipText     =   "Slab preprocessor output"
               Top             =   1200
               Width           =   900
            End
            Begin VB.CommandButton cmdBsmtCSV 
               Caption         =   "Bsmt CSV"
               Height          =   255
               Left            =   6360
               TabIndex        =   97
               ToolTipText     =   "Basement preprocessor monthly ground temperatures"
               Top             =   0
               Width           =   1020
            End
            Begin VB.CommandButton cmdBsmtAudit 
               Caption         =   "Bsmt Audit"
               Height          =   255
               Left            =   5280
               TabIndex        =   96
               ToolTipText     =   "Basement preprocessor run-time output"
               Top             =   720
               Width           =   900
            End
            Begin VB.CommandButton cmdBsmt 
               Caption         =   "Bsmt"
               Height          =   255
               Left            =   5280
               TabIndex        =   95
               ToolTipText     =   "Basement preprocessor IDF intermediate file."
               Top             =   360
               Width           =   900
            End
            Begin VB.CommandButton cmdBsmtOut 
               Caption         =   "Bsmt Out"
               Height          =   255
               Left            =   5280
               TabIndex        =   94
               ToolTipText     =   "Basement preprocessor output"
               Top             =   0
               Width           =   900
            End
            Begin VB.CommandButton cmdMain 
               Caption         =   "Variables"
               Height          =   255
               Left            =   0
               TabIndex        =   84
               ToolTipText     =   "Output:Variable and Output:Meter outputs (.csv, .txt, or .tab)"
               Top             =   720
               Width           =   900
            End
            Begin VB.CommandButton cmdERR 
               Caption         =   "Errors"
               Height          =   255
               Left            =   960
               TabIndex        =   83
               ToolTipText     =   "EnergyPlus error messages and warnings"
               Top             =   0
               Width           =   900
            End
            Begin VB.CommandButton cmdTable 
               Caption         =   "Tables"
               Height          =   255
               Left            =   0
               TabIndex        =   82
               ToolTipText     =   "Results summarized into monthly and binned reports."
               Top             =   0
               Width           =   900
            End
            Begin VB.CommandButton cmdESO 
               Caption         =   "ESO"
               Height          =   255
               Left            =   4320
               TabIndex        =   81
               ToolTipText     =   "Output:Variable and Output:Meter outputs before processing into Variable file"
               Top             =   1200
               Width           =   900
            End
            Begin VB.CommandButton cmdRDD 
               Caption         =   "RDD"
               Height          =   255
               Left            =   960
               TabIndex        =   80
               ToolTipText     =   "Report data dictionary containing variables that may be requested."
               Top             =   360
               Width           =   900
            End
            Begin VB.CommandButton cmdEIO 
               Caption         =   "EIO"
               Height          =   255
               Left            =   0
               TabIndex        =   79
               ToolTipText     =   "Standard and optional reports summarizing inputs, environments, constructions, sizing, etc"
               Top             =   1200
               Width           =   900
            End
            Begin VB.CommandButton cmdBND 
               Caption         =   "BND"
               Height          =   255
               Left            =   4320
               TabIndex        =   78
               ToolTipText     =   "HVAC branch and node details"
               Top             =   0
               Width           =   900
            End
            Begin VB.CommandButton cmdMTR 
               Caption         =   "MTR"
               Height          =   255
               Left            =   4320
               TabIndex        =   77
               ToolTipText     =   "Output:Meter and Output:Meter:MeterFileOnly outputs before processing into Meter file"
               Top             =   1560
               Width           =   900
            End
            Begin VB.CommandButton cmdMTD 
               Caption         =   "MTD"
               Height          =   255
               Left            =   960
               TabIndex        =   76
               ToolTipText     =   "Meter details to see what is on each meter"
               Top             =   1200
               Width           =   900
            End
            Begin VB.CommandButton cmdMeter 
               Caption         =   "Meters"
               Height          =   255
               Left            =   0
               TabIndex        =   75
               ToolTipText     =   "Output:Meter and Output:Meter:MeterFileOnly outputs (Meter.csv, Meter.txt, or Meter.tab)"
               Top             =   360
               Width           =   900
            End
            Begin VB.CommandButton cmdZSZ 
               Caption         =   "ZSZ"
               Height          =   255
               Left            =   960
               TabIndex        =   74
               ToolTipText     =   "Zone sizing and flow rates"
               Top             =   1560
               Width           =   900
            End
            Begin VB.CommandButton cmdSSZ 
               Caption         =   "SSZ"
               Height          =   255
               Left            =   960
               TabIndex        =   73
               ToolTipText     =   "System sizing and flow rates"
               Top             =   1920
               Width           =   900
            End
            Begin VB.CommandButton cmdAudit 
               Caption         =   "Audit"
               Height          =   255
               Left            =   3120
               TabIndex        =   72
               ToolTipText     =   "Echo of input with errors, warnings, and filled-in defaults"
               Top             =   1920
               Width           =   900
            End
            Begin VB.CommandButton cmdSLN 
               Caption         =   "SLN"
               Height          =   255
               Left            =   4320
               TabIndex        =   71
               ToolTipText     =   "Surface lines and coordinates."
               Top             =   720
               Width           =   900
            End
            Begin VB.CommandButton cmdDBG 
               Caption         =   "DBG"
               Height          =   255
               Left            =   4320
               TabIndex        =   70
               ToolTipText     =   "Debug output"
               Top             =   360
               Width           =   900
            End
            Begin VB.CommandButton cmdSVG 
               Caption         =   "SVG"
               Height          =   255
               Left            =   0
               TabIndex        =   69
               ToolTipText     =   "Drawing of HVAC system layout."
               Top             =   1560
               Width           =   900
            End
            Begin VB.CommandButton cmdEPMIDF 
               Caption         =   "EPMIDF"
               Height          =   255
               Left            =   2160
               TabIndex        =   68
               ToolTipText     =   "EPMacro idf file output"
               Top             =   1560
               Width           =   900
            End
            Begin VB.CommandButton cmdEPMDET 
               Caption         =   "EPMDET"
               Height          =   255
               Left            =   2160
               TabIndex        =   67
               ToolTipText     =   "EPMacro echo of input with errors and macro expansions"
               Top             =   1920
               Width           =   900
            End
            Begin VB.CommandButton cmdEXPIDF 
               Caption         =   "EXPIDF"
               Height          =   255
               Left            =   2160
               TabIndex        =   66
               ToolTipText     =   "ExpandObjects idf file output"
               Top             =   1200
               Width           =   900
            End
            Begin VB.CommandButton cmdMAP 
               Caption         =   "MAP"
               Height          =   255
               Left            =   2160
               TabIndex        =   65
               ToolTipText     =   "Daylighting map."
               Top             =   720
               Width           =   900
            End
            Begin VB.CommandButton cmdDXF 
               Caption         =   "DXF"
               Height          =   255
               Left            =   0
               TabIndex        =   64
               ToolTipText     =   "DXF drawing of building surfaces"
               Top             =   1920
               Width           =   900
            End
            Begin VB.CommandButton cmdIN 
               Caption         =   "DE IN"
               Height          =   255
               Left            =   2160
               TabIndex        =   63
               ToolTipText     =   "DElight input file."
               Top             =   0
               Width           =   900
            End
            Begin VB.CommandButton cmdOUT 
               Caption         =   "DE OUT"
               Height          =   255
               Left            =   2160
               TabIndex        =   62
               ToolTipText     =   "DElight output file."
               Top             =   360
               Width           =   900
            End
            Begin VB.CommandButton cmdELDMP 
               Caption         =   "ELDMP"
               Height          =   255
               Left            =   3120
               TabIndex        =   61
               ToolTipText     =   "DElight file"
               Top             =   0
               Width           =   900
            End
            Begin VB.CommandButton cmdDFDMP 
               Caption         =   "DFDMP"
               Height          =   255
               Left            =   3120
               TabIndex        =   60
               ToolTipText     =   "DElight file"
               Top             =   360
               Width           =   900
            End
            Begin VB.CommandButton cmdScreen 
               Caption         =   "Screen"
               Height          =   255
               Left            =   3120
               TabIndex        =   59
               ToolTipText     =   "DElight file"
               Top             =   720
               Width           =   900
            End
            Begin VB.CommandButton cmdSHD 
               Caption         =   "SHD"
               Height          =   255
               Left            =   3120
               TabIndex        =   58
               ToolTipText     =   "Shadowing Details"
               Top             =   1200
               Width           =   900
            End
            Begin VB.CommandButton cmdVRML 
               Caption         =   "VRML"
               Height          =   255
               Left            =   3120
               TabIndex        =   57
               ToolTipText     =   "Virtual Reality Modeling Language drawing of building surfaces"
               Top             =   1560
               Width           =   900
            End
            Begin VB.CommandButton cmdMDD 
               Caption         =   "MDD"
               Height          =   255
               Left            =   960
               TabIndex        =   56
               ToolTipText     =   "Report data dictionary containing variables that may be requested."
               Top             =   720
               Width           =   900
            End
            Begin VB.CommandButton cmdProcCSV 
               Caption         =   "Proc CSV"
               Height          =   255
               Left            =   4320
               TabIndex        =   55
               ToolTipText     =   "Processed CSV file with simple statistics from main CSV file."
               Top             =   1920
               Width           =   900
            End
            Begin VB.CommandButton cmdEDD 
               Caption         =   "EDD"
               Height          =   255
               Left            =   6360
               TabIndex        =   54
               ToolTipText     =   "EMS Actuator List"
               Top             =   360
               Width           =   1020
            End
         End
         Begin VB.Frame frameViewSelectOut 
            BorderStyle     =   0  'None
            Height          =   2175
            Left            =   600
            TabIndex        =   48
            Top             =   360
            Width           =   7455
            Begin VB.CommandButton cmdViewSet 
               Caption         =   "Set 8"
               Height          =   375
               Index           =   7
               Left            =   5760
               TabIndex        =   93
               Top             =   1440
               Width           =   1700
            End
            Begin VB.CommandButton cmdViewSet 
               Caption         =   "Set 7"
               Height          =   375
               Index           =   6
               Left            =   5760
               TabIndex        =   92
               Top             =   960
               Width           =   1700
            End
            Begin VB.CommandButton cmdViewSet 
               Caption         =   "Set 6"
               Height          =   375
               Index           =   5
               Left            =   5760
               TabIndex        =   91
               Top             =   480
               Width           =   1700
            End
            Begin VB.CommandButton cmdViewSet 
               Caption         =   "Set 5"
               Height          =   375
               Index           =   4
               Left            =   5760
               TabIndex        =   90
               Top             =   0
               Width           =   1700
            End
            Begin VB.CommandButton cmdViewSet 
               Caption         =   "Set 4"
               Height          =   375
               Index           =   3
               Left            =   3960
               TabIndex        =   89
               Top             =   1440
               Width           =   1700
            End
            Begin VB.CommandButton cmdViewSet 
               Caption         =   "Set 3"
               Height          =   375
               Index           =   2
               Left            =   3960
               TabIndex        =   88
               Top             =   960
               Width           =   1700
            End
            Begin VB.CommandButton cmdViewSet 
               Caption         =   "Set 2"
               Height          =   375
               Index           =   1
               Left            =   3960
               TabIndex        =   87
               Top             =   480
               Width           =   1700
            End
            Begin VB.CommandButton cmdViewDefine 
               Caption         =   "Define.."
               Height          =   315
               Left            =   0
               TabIndex        =   86
               Top             =   1800
               Width           =   855
            End
            Begin VB.CommandButton cmdViewSet 
               Caption         =   "Set 1"
               Height          =   375
               Index           =   0
               Left            =   3960
               TabIndex        =   85
               Top             =   0
               Width           =   1700
            End
            Begin VB.CommandButton cmdViewSet 
               Caption         =   "HTML"
               Height          =   375
               Index           =   11
               Left            =   1920
               TabIndex        =   52
               Top             =   480
               Width           =   1700
            End
            Begin VB.CommandButton cmdViewSet 
               Caption         =   "Spreadsheets"
               Height          =   375
               Index           =   10
               Left            =   120
               TabIndex        =   51
               Top             =   480
               Width           =   1700
            End
            Begin VB.CommandButton cmdViewSet 
               Caption         =   "Drawing File"
               Height          =   375
               Index           =   9
               Left            =   1920
               TabIndex        =   50
               Top             =   0
               Width           =   1700
            End
            Begin VB.CommandButton cmdViewSet 
               Caption         =   "Text Output Files"
               Height          =   375
               Index           =   8
               Left            =   120
               TabIndex        =   49
               Top             =   0
               Width           =   1700
            End
         End
         Begin MSComctlLib.TabStrip tabViewResults 
            Height          =   2415
            Left            =   120
            TabIndex        =   47
            Top             =   240
            Width           =   8055
            _ExtentX        =   14208
            _ExtentY        =   4260
            MultiRow        =   -1  'True
            Placement       =   2
            _Version        =   393216
            BeginProperty Tabs {1EFB6598-857C-11D1-B16A-00C0F0283628} 
               NumTabs         =   2
               BeginProperty Tab1 {1EFB659A-857C-11D1-B16A-00C0F0283628} 
                  Caption         =   "Sets"
                  ImageVarType    =   2
               EndProperty
               BeginProperty Tab2 {1EFB659A-857C-11D1-B16A-00C0F0283628} 
                  Caption         =   "All"
                  ImageVarType    =   2
               EndProperty
            EndProperty
         End
         Begin VB.Label lblViewResults 
            Caption         =   "View Results"
            BeginProperty Font 
               Name            =   "MS Sans Serif"
               Size            =   8.25
               Charset         =   0
               Weight          =   700
               Underline       =   0   'False
               Italic          =   0   'False
               Strikethrough   =   0   'False
            EndProperty
            Height          =   255
            Left            =   720
            TabIndex        =   13
            Top             =   0
            Width           =   1935
         End
      End
      Begin VB.Frame frmInputFile 
         Caption         =   "Input File"
         Height          =   1335
         Left            =   0
         TabIndex        =   2
         Top             =   0
         Width           =   8295
         Begin VB.ComboBox cmbInput 
            Height          =   315
            Left            =   120
            Style           =   2  'Dropdown List
            TabIndex        =   6
            Top             =   360
            Width           =   8055
         End
         Begin VB.CommandButton cmdInputBrowse 
            Caption         =   "Browse..."
            Height          =   375
            Left            =   120
            TabIndex        =   5
            Top             =   840
            Width           =   1215
         End
         Begin VB.CommandButton cmdInputEdit 
            Caption         =   "Edit - Text Editor"
            Height          =   375
            Left            =   4920
            TabIndex        =   4
            Top             =   840
            Width           =   1575
         End
         Begin VB.CommandButton cmdIDFEdit 
            Caption         =   "Edit - IDF Editor"
            Height          =   375
            Left            =   6600
            Style           =   1  'Graphical
            TabIndex        =   3
            Top             =   840
            Width           =   1575
         End
         Begin VB.Label lblInputFile 
            Caption         =   "Input File"
            BeginProperty Font 
               Name            =   "MS Sans Serif"
               Size            =   8.25
               Charset         =   0
               Weight          =   700
               Underline       =   0   'False
               Italic          =   0   'False
               Strikethrough   =   0   'False
            EndProperty
            Height          =   255
            Left            =   120
            TabIndex        =   7
            Top             =   120
            Width           =   1935
         End
      End
      Begin MSComDlg.CommonDialog CommonDialog1 
         Left            =   2520
         Top             =   5640
         _ExtentX        =   847
         _ExtentY        =   847
         _Version        =   393216
      End
      Begin VB.Frame frmWeatherFile 
         Caption         =   "Weather File"
         Height          =   1335
         Left            =   0
         TabIndex        =   8
         Top             =   1560
         Width           =   8280
         Begin VB.CommandButton cmdWeatherBrowse 
            Caption         =   "Browse..."
            Height          =   375
            Left            =   120
            TabIndex        =   10
            Top             =   840
            Width           =   1215
         End
         Begin VB.ComboBox cmbWeather 
            Height          =   315
            Left            =   120
            Style           =   2  'Dropdown List
            TabIndex        =   9
            Top             =   360
            Width           =   8025
         End
         Begin VB.Label lblWeatherFile 
            Caption         =   "Weather File"
            BeginProperty Font 
               Name            =   "MS Sans Serif"
               Size            =   8.25
               Charset         =   0
               Weight          =   700
               Underline       =   0   'False
               Italic          =   0   'False
               Strikethrough   =   0   'False
            EndProperty
            Height          =   255
            Left            =   120
            TabIndex        =   11
            Top             =   120
            Width           =   1935
         End
      End
      Begin VB.Image Image1 
         Height          =   600
         Left            =   3600
         Picture         =   "epl-ui.frx":08CA
         Top             =   5640
         Visible         =   0   'False
         Width           =   600
      End
   End
   Begin VB.Frame frameGroup 
      BorderStyle     =   0  'None
      Caption         =   "Frame1"
      Height          =   6255
      Left            =   240
      TabIndex        =   15
      Top             =   480
      Width           =   8295
      Begin VB.Frame frmGroupFile 
         Caption         =   "Group File"
         Height          =   1935
         Left            =   0
         TabIndex        =   19
         Top             =   120
         Width           =   8295
         Begin VB.CommandButton cmdNewGroup 
            Caption         =   "New Group..."
            Height          =   375
            Left            =   120
            TabIndex        =   26
            Top             =   360
            Width           =   1215
         End
         Begin VB.ComboBox cmbGroup 
            Height          =   315
            Left            =   120
            Style           =   2  'Dropdown List
            TabIndex        =   22
            Top             =   960
            Width           =   8055
         End
         Begin VB.CommandButton cmdGroupBrowse 
            Caption         =   "Browse..."
            Height          =   375
            Left            =   120
            TabIndex        =   21
            Top             =   1440
            Width           =   1215
         End
         Begin VB.CommandButton cmdGroupEdit 
            Caption         =   "Edit..."
            Height          =   375
            Left            =   6960
            TabIndex        =   20
            Top             =   1440
            Width           =   1215
         End
      End
      Begin VB.CommandButton cmdViewGroupError 
         Caption         =   "View Group Error File..."
         Height          =   375
         Left            =   120
         TabIndex        =   27
         Top             =   2280
         Width           =   1935
      End
      Begin VB.CommandButton cmdSimulateGroup 
         Caption         =   "Simulate Group..."
         Height          =   375
         Left            =   6600
         TabIndex        =   18
         Top             =   4200
         Width           =   1575
      End
   End
   Begin VB.Frame frameHistory 
      BorderStyle     =   0  'None
      Caption         =   "Frame1"
      Height          =   6255
      Left            =   120
      TabIndex        =   23
      Top             =   480
      Width           =   8295
      Begin MSComctlLib.TreeView treeHistory 
         Height          =   5655
         Left            =   120
         TabIndex        =   25
         Top             =   120
         Width           =   8175
         _ExtentX        =   14420
         _ExtentY        =   9975
         _Version        =   393217
         LabelEdit       =   1
         LineStyle       =   1
         Style           =   7
         Appearance      =   1
      End
      Begin VB.CommandButton cmdViewHistoryFile 
         Caption         =   " View File ..."
         Height          =   375
         Left            =   3360
         TabIndex        =   24
         Top             =   5880
         Width           =   1575
      End
      Begin VB.Label Label1 
         Caption         =   "Label1"
         Height          =   255
         Left            =   120
         TabIndex        =   29
         Top             =   240
         Width           =   975
      End
   End
   Begin InetCtlsObjects.Inet Inet1 
      Left            =   4800
      Top             =   6120
      _ExtentX        =   1005
      _ExtentY        =   1005
      _Version        =   393216
   End
   Begin VB.CommandButton cmdExit 
      Cancel          =   -1  'True
      Caption         =   "Exit"
      Height          =   375
      Left            =   6960
      TabIndex        =   17
      Top             =   6960
      Width           =   1575
   End
   Begin MSComctlLib.TabStrip tabMain 
      Height          =   6735
      Left            =   60
      TabIndex        =   0
      Top             =   120
      Width           =   8655
      _ExtentX        =   15266
      _ExtentY        =   11880
      MultiRow        =   -1  'True
      _Version        =   393216
      BeginProperty Tabs {1EFB6598-857C-11D1-B16A-00C0F0283628} 
         NumTabs         =   4
         BeginProperty Tab1 {1EFB659A-857C-11D1-B16A-00C0F0283628} 
            Caption         =   "Single Input File"
            ImageVarType    =   2
         EndProperty
         BeginProperty Tab2 {1EFB659A-857C-11D1-B16A-00C0F0283628} 
            Caption         =   "Group of Input Files"
            ImageVarType    =   2
         EndProperty
         BeginProperty Tab3 {1EFB659A-857C-11D1-B16A-00C0F0283628} 
            Caption         =   "History"
            ImageVarType    =   2
         EndProperty
         BeginProperty Tab4 {1EFB659A-857C-11D1-B16A-00C0F0283628} 
            Caption         =   "Utilities"
            ImageVarType    =   2
         EndProperty
      EndProperty
   End
   Begin VB.Label lblCheckingUpdates 
      Caption         =   "Checking for Updates"
      ForeColor       =   &H80000011&
      Height          =   375
      Left            =   3360
      TabIndex        =   46
      Top             =   7080
      Width           =   3375
   End
   Begin VB.Shape Shape1 
      Height          =   255
      Left            =   4680
      Top             =   120
      Width           =   615
   End
   Begin VB.Label lblIDDVersion 
      Height          =   255
      Left            =   240
      TabIndex        =   16
      Top             =   7080
      Width           =   6615
   End
   Begin VB.Line Line1 
      X1              =   0
      X2              =   17640
      Y1              =   0
      Y2              =   0
   End
   Begin VB.Menu mnuFile 
      Caption         =   "&File"
      Begin VB.Menu mnuFileIn 
         Caption         =   "Select &Input File..."
      End
      Begin VB.Menu mnuFileWeather 
         Caption         =   "Select &Weather File..."
      End
      Begin VB.Menu mnuFileGroup 
         Caption         =   "Select &Group File..."
      End
      Begin VB.Menu mnuSep 
         Caption         =   "-"
      End
      Begin VB.Menu mnuFileTransitionVersion 
         Caption         =   "Transition &Version..."
      End
      Begin VB.Menu mnuFileSep4 
         Caption         =   "-"
      End
      Begin VB.Menu mnuFileNewGroup 
         Caption         =   "New Group..."
      End
      Begin VB.Menu mnuFileSep3 
         Caption         =   "-"
      End
      Begin VB.Menu mnuFileSim 
         Caption         =   "&Simulate Single File..."
      End
      Begin VB.Menu mnuFileSimGroup 
         Caption         =   "S&imulate Group of Files..."
      End
      Begin VB.Menu mnuFileSep2 
         Caption         =   "-"
      End
      Begin VB.Menu mnuFileStop 
         Caption         =   "&Cancel Additional Simulations.."
         Shortcut        =   ^C
      End
      Begin VB.Menu mnuFileExit 
         Caption         =   "E&xit"
      End
   End
   Begin VB.Menu mnuEdit 
      Caption         =   "&Edit"
      Begin VB.Menu mnuEditUndo 
         Caption         =   "Undo"
         Enabled         =   0   'False
      End
      Begin VB.Menu mnuEditCut 
         Caption         =   "Cut"
         Enabled         =   0   'False
      End
      Begin VB.Menu mnuEditCopy 
         Caption         =   "Copy"
         Enabled         =   0   'False
      End
      Begin VB.Menu mnuEditPaste 
         Caption         =   "Paste"
         Enabled         =   0   'False
      End
      Begin VB.Menu mnuEditSep 
         Caption         =   "-"
      End
      Begin VB.Menu mnuEditInputText 
         Caption         =   "Input with &Text Editor"
      End
      Begin VB.Menu mnuEditIDF 
         Caption         =   "Input with &IDF Editor"
      End
      Begin VB.Menu mnuEditWeather 
         Caption         =   "&Weather with Text Editor"
      End
      Begin VB.Menu mnuEditPost 
         Caption         =   "&Postprocessor Command"
      End
   End
   Begin VB.Menu mnuView 
      Caption         =   "&View"
      Begin VB.Menu mnuViewOut 
         Caption         =   "Text &Output Files"
      End
      Begin VB.Menu mnuViewDrawing 
         Caption         =   "&Drawing Files"
      End
      Begin VB.Menu mnuViewSpreadsheet 
         Caption         =   "&Spreadsheets"
      End
      Begin VB.Menu viewHTMLfile 
         Caption         =   "&HTML File"
      End
      Begin VB.Menu mnuViewGroup 
         Caption         =   "&Group File"
      End
      Begin VB.Menu viewSelectedHistory 
         Caption         =   "&Selected History File"
      End
      Begin VB.Menu mnuViewSep0 
         Caption         =   "-"
      End
      Begin VB.Menu mnuViewEEREIO 
         Caption         =   "&ERR/EIO/BND Output Files Only"
         Shortcut        =   {F2}
      End
      Begin VB.Menu mnuViewSingleFile 
         Caption         =   "Sin&gle File"
         Begin VB.Menu mnuViewCSV 
            Caption         =   "Variable"
            Shortcut        =   {F4}
         End
         Begin VB.Menu mnuViewESO 
            Caption         =   "ESO File"
            Shortcut        =   {F5}
         End
         Begin VB.Menu mnuViewRDD 
            Caption         =   "RDD File"
            Shortcut        =   {F6}
         End
         Begin VB.Menu mnuViewMDD 
            Caption         =   "MDD File"
            Shortcut        =   +^{F3}
         End
         Begin VB.Menu mnuViewEIO 
            Caption         =   "EIO File"
            Shortcut        =   {F7}
         End
         Begin VB.Menu mnuViewErr 
            Caption         =   "ERR File"
            Shortcut        =   {F8}
         End
         Begin VB.Menu mnuViewBND 
            Caption         =   "BND File"
            Shortcut        =   {F9}
         End
         Begin VB.Menu mnuViewMTR 
            Caption         =   "MTR File"
            Shortcut        =   {F11}
         End
         Begin VB.Menu mnuViewMTD 
            Caption         =   "MTD File"
            Shortcut        =   {F12}
         End
         Begin VB.Menu mnuViewMETER 
            Caption         =   "METER File"
            Shortcut        =   ^{F4}
         End
         Begin VB.Menu mnuViewZSZ 
            Caption         =   "ZSZ File"
            Shortcut        =   ^{F5}
         End
         Begin VB.Menu mnuViewSSZ 
            Caption         =   "SSZ File"
            Shortcut        =   ^{F6}
         End
         Begin VB.Menu mnuViewSpSZ 
            Caption         =   "SpSZ File"
         End
         Begin VB.Menu mnuViewAUDIT 
            Caption         =   "AUDIT File"
            Shortcut        =   ^{F8}
         End
         Begin VB.Menu mnuViewSLN 
            Caption         =   "SLN File"
            Shortcut        =   ^{F9}
         End
         Begin VB.Menu mnuViewDBG 
            Caption         =   "DBG File"
            Shortcut        =   ^{F11}
         End
         Begin VB.Menu mnuViewSHD 
            Caption         =   "SHD File"
            Shortcut        =   ^{F12}
         End
         Begin VB.Menu mnuViewSVG 
            Caption         =   "HVAC Diagram-SVG"
            Shortcut        =   +{F4}
         End
         Begin VB.Menu mnuViewEPMIDF 
            Caption         =   "EPMIDF File"
            Shortcut        =   +{F5}
         End
         Begin VB.Menu mnuViewEPMDET 
            Caption         =   "EPMDET File"
            Shortcut        =   +{F6}
         End
         Begin VB.Menu mnuViewMAP 
            Caption         =   "MAP File"
            Shortcut        =   +{F7}
         End
         Begin VB.Menu mnuViewTABLE 
            Caption         =   "TABLE File"
            Shortcut        =   +{F8}
         End
         Begin VB.Menu mnuViewVRML 
            Caption         =   "VRML File"
            Shortcut        =   +{F11}
         End
         Begin VB.Menu mnuViewDXF 
            Caption         =   "DXF File"
            Shortcut        =   +{F12}
         End
         Begin VB.Menu mnuViewDelightIN 
            Caption         =   "Delight IN"
            Shortcut        =   +^{F4}
         End
         Begin VB.Menu mnuViewDelightOut 
            Caption         =   "Delight OUT"
            Shortcut        =   +^{F5}
         End
         Begin VB.Menu mnuViewDelightELDMP 
            Caption         =   "Delight ELDMP"
            Shortcut        =   +^{F6}
         End
         Begin VB.Menu mnuViewDelightDFDMP 
            Caption         =   "Delight DFDMP"
            Shortcut        =   +^{F7}
         End
         Begin VB.Menu mnuViewExpIDF 
            Caption         =   "EXPIDF File"
            Shortcut        =   +^{F8}
         End
         Begin VB.Menu mnuViewErrGrp 
            Caption         =   "Group Error errgrp"
            Shortcut        =   +^{F9}
         End
         Begin VB.Menu mnuViewVCpErr 
            Caption         =   "VCpErr"
            Shortcut        =   +^{F11}
         End
         Begin VB.Menu mnuViewScreen 
            Caption         =   "Screen"
            Shortcut        =   +^{F12}
         End
         Begin VB.Menu mnuViewProcCSV 
            Caption         =   "Proc CSV"
         End
         Begin VB.Menu mnuViewEdd 
            Caption         =   "EDD File"
         End
         Begin VB.Menu mnuViewBsmtOut 
            Caption         =   "Bsmt Out"
         End
         Begin VB.Menu mnuViewBsmt 
            Caption         =   "Bsmt"
         End
         Begin VB.Menu mnuViewBsmtAudit 
            Caption         =   "Bsmt Audit"
         End
         Begin VB.Menu mnuViewBsmtCSV 
            Caption         =   "Bsmt CSV"
         End
         Begin VB.Menu mnuViewSlabOut 
            Caption         =   "Slab Out"
         End
         Begin VB.Menu mnuViewSlab 
            Caption         =   "Slab"
         End
         Begin VB.Menu mnuViewSlabErr 
            Caption         =   "Slab Err"
         End
         Begin VB.Menu mnuViewPerfLogCSV 
            Caption         =   "PerfLog CSV"
         End
      End
      Begin VB.Menu mnuViewSep6 
         Caption         =   "-"
      End
      Begin VB.Menu mnuViewFolderInput 
         Caption         =   "Input File Folder"
      End
      Begin VB.Menu mnuViewFolderWeather 
         Caption         =   "Weather File Folder"
      End
      Begin VB.Menu mnuViewFolderGroup 
         Caption         =   "Group File Folder"
      End
      Begin VB.Menu mnuViewSep1 
         Caption         =   "-"
      End
      Begin VB.Menu mnuViewOptions 
         Caption         =   "Options..."
      End
   End
   Begin VB.Menu mnuHelp 
      Caption         =   "&Help"
      Begin VB.Menu mnuHelpQuickstart 
         Caption         =   "EnergyPlus QuickStart Webpage"
      End
      Begin VB.Menu mnuHelpEssentials 
         Caption         =   "EnergyPlus Essentials"
      End
      Begin VB.Menu mnuHelpIORef 
         Caption         =   "EnergyPlus Input/Output Reference"
         Shortcut        =   {F1}
      End
      Begin VB.Menu mnuHelpOutDetails 
         Caption         =   "EnergyPlus Output Details and Examples"
      End
      Begin VB.Menu mnuHelpEngRef 
         Caption         =   "EnergyPlus Engineering Reference"
      End
      Begin VB.Menu mnuHelpSep2 
         Caption         =   "-"
      End
      Begin VB.Menu mnuHelpPlantAppl 
         Caption         =   "Plant Application Guide"
      End
      Begin VB.Menu mnuHelpEMS 
         Caption         =   "EMS Application Guide"
      End
      Begin VB.Menu mnuHelpCompliance 
         Caption         =   "Using EnergyPlus for Compliance"
      End
      Begin VB.Menu mnuHelpExtInterface 
         Caption         =   "External Interface Application Guide"
      End
      Begin VB.Menu mnuHelpSep3 
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
      Begin VB.Menu mnuHelpDiv2 
         Caption         =   "-"
      End
      Begin VB.Menu mnuHelpCheckUpdates 
         Caption         =   "Check for Updates..."
      End
      Begin VB.Menu mnuHelpViewUpdateList 
         Caption         =   "View Entire Update List on Web..."
      End
      Begin VB.Menu mnuHelpDiv3 
         Caption         =   "-"
      End
      Begin VB.Menu mnuHelpEPL 
         Caption         =   "Using EP-Launch Help..."
      End
      Begin VB.Menu mnuHelpAbout 
         Caption         =   "&About EP-Launch..."
      End
   End
End
Attribute VB_Name = "eplUI"
Attribute VB_GlobalNameSpace = False
Attribute VB_Creatable = False
Attribute VB_PredeclaredId = True
Attribute VB_Exposed = False
Option Explicit
Const maxInputListSize = 20 'number of items allowed in input file pull down list
Const maxWeatherListSize = 20  'number of items allowed in weather file pull down list
Const maxGroupListSize = 20  'number of items allowed in group file pull down list
Const batchFileName = "EPL-run.bat"
Const noWeatherFile = "No Weather File"
Dim q As String
Public inputFileName As String
Dim outputFileName As String
Dim weatherFileName As String
Dim groupFileName As String
Public textEditFileName As String
Public spreadsheetFileName As String
Public pdfViewerFileName As String
Public xmlViewerFileName As String
Public dxfViewFileName As String
Public vrmlAppFileName As String
Public htmlViewFileName As String
Public svgViewFileName As String
Public esoViewFileName As String
Dim curOSver As Integer
Dim selectedInputIndex As Integer
Dim selectedWeatherIndex As Integer
Dim selectedGroupIndex As Integer
Dim selectInputFilterIndex As Integer
Public lastInputDirectory As String
Public lastWeatherDirectory As String
Public lastGroupDirectory As String
Public lastGrpInputDirectory As String
Public lastGrpWeatherDirectory As String
Public EnergyPlusVer As String
Dim platform As String
Dim curTab As Integer
Dim iddVersion As String
Dim previousVersion As String
Dim currentVersion As String
Dim transitionFileName As String
Public pauseDuringRun As Boolean
Public useWideView As Boolean
Public useSimAboveView As Boolean
Public testViewConvertOld As Boolean
Public minimizeGroupCmd As Boolean
Public tabWithSpreadsheet As Boolean
Public allowGT250Col As Boolean
Public convertESOMTRIP As Boolean
Public minimizeSingleCmd As Boolean
Public createCSVprocFile As Boolean
Public CreateRunEPBatch As Boolean
Public enableParametricPreprocessor As Boolean
Public numberOfSimProcessesAllowed As Integer
Public viewAllOutputTabSelected As Boolean
Dim firstActivateCall As Boolean
Dim appPath As String
Public crashFileName As String

Public updateLastAnchor As String 'stored in registry
Public updateLastDate As String   'stored in registry
Public updatePageURL As String    'stored in registry
Public updateAutoCheck As Boolean 'stored in regisry

Dim folderList() As String
Dim numFolderList As Integer

Private Type utilProgType
    name As String
    about As String
    enableInput As Boolean
    enableWthr As Boolean
    enableInTextEdit As Boolean
    IDFEdOpt As String 'blank if no IDF Editor support
    inExt As String
    inExtAlt As String  'alternate input extension (used for htm)
    outExt1 As String   'blank if no output
    outExt2 As String   'blank if no output
    outExt3 As String   'blank if no output
    outExt3Alt As String 'alternate output extension for outExt3 (used for htm)
    outExt4 As String   'blank if no output
    outExt5 As String   'blank if no output
    outExt6 As String   'blank if no output
    applicationFile As String
    batchFile As String
    appIsSpreadsheet As Boolean
    curInputFile As String
    curWeatherFile As String
    useInputAsExeArgument As Boolean 'when not using batch file, pass the argument when running exe
    fileSuffix As String 'something added to file root before extension that is removed before opening outExt's
    includeExtensionInCall As Boolean 'if the extension should be included when the file is passed to the application
    waitUntilUtilityExits As Boolean 'true if EP-Launch should wait until the called program is done
End Type
Const numUtilProg = 9
Dim utilProg(numUtilProg) As utilProgType

Private Type SimQueueType
   status As Integer
   nameIn As String
   nameWthr As String
   nameOut As String
   nameGrp As String
   counter As Integer
   messagesShow As Boolean
   tempFolder As String
   processHandle As Long
   eplusoutEndText As String
   reasonCancelled As String
   usedOutFileName As String
   timeStart As Date
End Type
Const qStatNotRun = 1
Const qStatActive = 2
Const qStatDoneOk = 3
Const qStatDoneCancelled = 4
Const qStatDoneCrashed = 5
Dim simQueue() As SimQueueType
Dim numSimQueue As Long
Dim queueBeingResized As Boolean 'used to lock out operations while the queue is being added to and the array possibily being resized
'Dim startSimulationActive As Boolean 'used to prevent the startSimulation routine from being executed more than once for a given time.

Private Declare Function FindExecutable Lib "shell32.dll" Alias "FindExecutableA" (ByVal lpFile As String, ByVal lpDirectory As String, ByVal lpresults As String) As Long

Private Declare Function GetShortPathName Lib "kernel32.dll" Alias "GetShortPathNameA" (ByVal lpszLongPath As String, ByVal lpszShortPath As String, ByVal cchBuffer As Long) As Long


' The following code is from VBnet and is used for file associations
'
''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
' Copyright ©1996-2007 VBnet, Randy Birch, All Rights Reserved.
' Some pages may also contain other copyrights by the author.
''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
' Distribution: You can freely use this code in your own
'               applications, but you may not reproduce
'               or publish this code on any web site,
'               online service, or distribute as source
'               on any media without express permission.
''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
Private Const REG_SZ As Long = &H1
Private Const REG_DWORD As Long = &H4
Private Const HKEY_CLASSES_ROOT As Long = &H80000000
Private Const HKEY_CURRENT_USER As Long = &H80000001
Private Const HKEY_LOCAL_MACHINE As Long = &H80000002
Private Const HKEY_USERS As Long = &H80000003
Private Const ERROR_SUCCESS As Long = 0
Private Const ERROR_BADDB As Long = 1009
Private Const ERROR_BADKEY As Long = 1010
Private Const ERROR_CANTOPEN As Long = 1011
Private Const ERROR_CANTREAD As Long = 1012
Private Const ERROR_CANTWRITE As Long = 1013
Private Const ERROR_OUTOFMEMORY As Long = 14
Private Const ERROR_INVALID_PARAMETER As Long = 87
Private Const ERROR_ACCESS_DENIED As Long = 5
Private Const ERROR_MORE_DATA As Long = 234
Private Const ERROR_NO_MORE_ITEMS As Long = 259
Private Const KEY_ALL_ACCESS As Long = &HF003F
Private Const REG_OPTION_NON_VOLATILE As Long = 0
Private Declare Function RegCloseKey Lib "advapi32.dll" (ByVal hKey As Long) As Long
Private Declare Function RegCreateKeyEx Lib "advapi32" Alias "RegCreateKeyExA" (ByVal hKey As Long, ByVal lpSubKey As String, ByVal Reserved As Long, _
    ByVal lpClass As String, ByVal dwOptions As Long, ByVal samDesired As Long, ByVal lpSecurityAttributes As Long, phkResult As Long, lpdwDisposition As Long) As Long
Private Declare Function RegOpenKeyEx Lib "advapi32" Alias "RegOpenKeyExA" (ByVal hKey As Long, ByVal lpSubKey As String, ByVal ulOptions As Long, _
    ByVal samDesired As Long, phkResult As Long) As Long
Private Declare Function RegSetValueEx Lib "advapi32" Alias "RegSetValueExA" (ByVal hKey As Long, ByVal lpValueName As String, ByVal Reserved As Long, _
   ByVal dwType As Long, lpValue As Any, ByVal cbData As Long) As Long



'============================
'============================
'============================
' routines to do a shelled process
'============================
'============================
'============================

      Private Type STARTUPINFO
         cb As Long
         lpReserved As String
         lpDesktop As String
         lpTitle As String
         dwX As Long
         dwY As Long
         dwXSize As Long
         dwYSize As Long
         dwXCountChars As Long
         dwYCountChars As Long
         dwFillAttribute As Long
         dwFlags As Long
         wShowWindow As Integer
         cbReserved2 As Integer
         lpReserved2 As Long
         hStdInput As Long
         hStdOutput As Long
         hStdError As Long
      End Type

      Private Type PROCESS_INFORMATION
         hProcess As Long
         hThread As Long
         dwProcessID As Long
         dwThreadID As Long
      End Type

      ' used for getting version
      Private Declare Function GetVersion Lib "kernel32" () As Long

      Private Declare Function WaitForSingleObject Lib "kernel32" (ByVal _
         hHandle As Long, ByVal dwMilliseconds As Long) As Long

      Private Declare Function CreateProcessA Lib "kernel32" (ByVal _
         lpApplicationName As Long, ByVal lpCommandLine As String, ByVal _
         lpProcessAttributes As Long, ByVal lpThreadAttributes As Long, _
         ByVal bInheritHandles As Long, ByVal dwCreationFlags As Long, _
         ByVal lpEnvironment As Long, ByVal lpCurrentDirectory As Long, _
         lpStartupInfo As STARTUPINFO, lpProcessInformation As _
         PROCESS_INFORMATION) As Long

      Private Declare Function CloseHandle Lib "kernel32" (ByVal _
         hObject As Long) As Long

      Private Const NORMAL_PRIORITY_CLASS = &H20&
      Private Const INFINITE = -1&

      Public Sub ExecCmd(cmdLine$)
         Dim proc As PROCESS_INFORMATION
         Dim start As STARTUPINFO
         Dim Ret As Long

         ' Initialize the STARTUPINFO structure:
         start.cb = Len(start)
         'to minimize the window
         'start.wShowWindow = 2
         'start.dwFlags = &H1
         'to use a different title bar
         start.lpTitle = "EnergyPlus Process"

         ' Start the shelled application:
         Ret = CreateProcessA(0&, cmdLine$, 0&, 0&, 1&, _
            NORMAL_PRIORITY_CLASS, 0&, 0&, start, proc)

         ' Wait for the shelled application to finish:
         Ret = WaitForSingleObject(proc.hProcess, INFINITE)
         Ret = CloseHandle(proc.hProcess)
      End Sub

      Public Sub ExecCmdWindowControl(cmdLine$, minimizeWindow As Boolean)
         Dim proc As PROCESS_INFORMATION
         Dim start As STARTUPINFO
         Dim Ret As Long

         ' Initialize the STARTUPINFO structure:
         start.cb = Len(start)
         'to minimize the window
         If minimizeWindow Then
           start.wShowWindow = 7
           start.dwFlags = &H1
         End If
         'to use a different title bar
         start.lpTitle = "EnergyPlus Process"

         ' Start the shelled application:
         Ret = CreateProcessA(0&, cmdLine$, 0&, 0&, 1&, _
            NORMAL_PRIORITY_CLASS, 0&, 0&, start, proc)

         ' Wait for the shelled application to finish:
         Ret = WaitForSingleObject(proc.hProcess, INFINITE)
         Ret = CloseHandle(proc.hProcess)
      End Sub

      Public Function ExecCmdNoWait(indexToDisplay As Long, cmdLine$, minimizeWindow As Boolean) As Long
         Dim proc As PROCESS_INFORMATION
         Dim start As STARTUPINFO
         Dim Ret As Long

         ' Initialize the STARTUPINFO structure:
         start.cb = Len(start)
         'to minimize the window
         If minimizeWindow Then
           start.wShowWindow = 7
           start.dwFlags = &H1
         End If
         'to use a different title bar
         start.lpTitle = Format(indexToDisplay, "0000") & " - EnergyPlus Process"

         ' Start the shelled application:
         Ret = CreateProcessA(0&, cmdLine$, 0&, 0&, 1&, _
            NORMAL_PRIORITY_CLASS, 0&, 0&, start, proc)

         ' don't wait just return the process handle to check with
         ExecCmdNoWait = proc.hProcess
      End Function

      Public Function isProcessDone(curHProcess As Long) As Boolean
         Dim Ret As Long
         Ret = WaitForSingleObject(curHProcess, 0)
         'MsgBox "ret: " & Str(Ret)
         If Ret = 0& Then
            Ret = CloseHandle(curHProcess)
            isProcessDone = True
         Else
            isProcessDone = False
         End If
      End Function

''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
' Copyright ©1996-2007 VBnet, Randy Birch, All Rights Reserved.
' Some pages may also contain other copyrights by the author.
''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
' Distribution: You can freely use this code in your own
'               applications, but you may not reproduce
'               or publish this code on any web site,
'               online service, or distribute as source
'               on any media without express permission.
''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
Public Sub CreateAssociation()
   Dim sPath As String
  'File Associations begin with a listing
  'of the default extension under HKEY_CLASSES_ROOT.
  'So the first step is to create that
  'root extension item. We'll use an extension
  'of .xxx to make it easy to locate in the registry.
   CreateNewKey ".idf", HKEY_CLASSES_ROOT
   CreateNewKey ".imf", HKEY_CLASSES_ROOT
   CreateNewKey ".epg", HKEY_CLASSES_ROOT
  'To the extension just added, add a
  'subitem where the registry will look for
  'commands relating to the .xxx extension
  '("MyApp.Document"). Its type is String (REG_SZ)
   SetKeyValue ".idf", "", "EP-Launch.Document", REG_SZ
   SetKeyValue ".imf", "", "EP-Launch.Document", REG_SZ
   SetKeyValue ".epg", "", "EP-Launch.Document", REG_SZ
  'Create the 'MyApp.Document' item under
  'HKEY_CLASSES_ROOT. This is where you'll put
  'the command line to execute or other shell
  'statements necessary.
   CreateNewKey "EP-Launch.Document\shell\open\command", HKEY_CLASSES_ROOT
  'Set its default item to "MyApp Document".
  'This is what is displayed in Explorer against
  'files with the .xxx extension. Its type is
  'String (REG_SZ)
   SetKeyValue "EP-Launch.Document", "", "EP-Launch Document", REG_SZ
  'Finally, add the path to myapp.exe
  'Remember to add %1 as the final command
  'parameter to assure the app opens the passed
  'command line item, and enclose the path in quotes
  'to ensure the long path name is received by the app.
  '(results in '"c:\LongPathname\Myapp.exe" "%1")
  'Again, its type is string.
   sPath = ShortName(appPath & "EP-Launch.exe") & " %1"
   SetKeyValue "EP-Launch.Document\shell\open\command", "", sPath, REG_SZ
  'All done
   MsgBox "The EP-Launch file associations have been made!", vbInformation, "File Association"
End Sub
Private Function SetValueEx(ByVal hKey As Long, sValueName As String, lType As Long, vValue As Variant) As Long
   Dim nValue As Long
   Dim sValue As String
   Select Case lType
      Case REG_SZ
         sValue = vValue & Chr$(0)
         SetValueEx = RegSetValueEx(hKey, sValueName, 0&, lType, ByVal sValue, Len(sValue))
      Case REG_DWORD
         nValue = vValue
         SetValueEx = RegSetValueEx(hKey, sValueName, 0&, lType, ByVal nValue, 4)
   End Select
End Function
Private Sub CreateNewKey(sNewKeyName As String, lPredefinedKey As Long)
  'handle to the new key
   Dim hKey As Long
   Dim result As Long
   Call RegCreateKeyEx(lPredefinedKey, sNewKeyName, 0&, vbNullString, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, 0&, hKey, result)
   Call RegCloseKey(hKey)
End Sub
Private Sub SetKeyValue(sKeyName As String, sValueName As String, vValueSetting As Variant, lValueType As Long)
  'handle of opened key
   Dim hKey As Long
  'open the specified key
   Call RegOpenKeyEx(HKEY_CLASSES_ROOT, sKeyName, 0, KEY_ALL_ACCESS, hKey)
   Call SetValueEx(hKey, sValueName, lValueType, vValueSetting)
   Call RegCloseKey(hKey)
End Sub


'***************************************************************************
' Converts Long FileName to Short FileName
'***************************************************************************
Public Function ShortName(LongPath As String) As String
    Dim ShortPath As String
    Dim Ret As Long
    Const MAX_PATH = 260
    If LongPath = "" Then
        Exit Function
    End If
    ShortPath = Space$(MAX_PATH)
    Ret = GetShortPathName(LongPath, ShortPath, MAX_PATH)
    If Ret Then
        ShortName = Left$(ShortPath, Ret)
    End If
End Function







Private Sub cmdViewDefine_Click()
frmDefineViewResults.Show vbModal
Call checkOutputButtonsFiles
End Sub


'=======================================================
' Handler for all of the "select" view output file buttons
'=======================================================
Private Sub cmdViewSet_Click(Index As Integer)
Dim setNum As Integer
setNum = Index + 1 'sets are 1 to 12 but indexes are 0 to 11
Call viewOutputSet(setNum)
End Sub

'=======================================================
' Check for updates after the interface is loaded but
' since the Activate event can fire every time the
' main window is switched to, a flag is added to call it
' only once.
'=======================================================
Private Sub Form_Activate()
If firstActivateCall Then
  If updateAutoCheck Then Call checkForUpdatesNow(False)
  Call checkIfApplicationDirectoryGood
  firstActivateCall = False
End If
End Sub


'============================
'============================
'============================
'  COMMAND BUTTON ACTIVATIONS
'============================
'============================
'============================

Private Sub cmdExit_Click()
Call ExitEPL
End Sub
Private Sub cmdGroupBrowse_Click()
Call SelectGroup
End Sub
Private Sub cmdGroupEdit_Click()
Call RunGroupEdit
End Sub
Private Sub cmdIDFEdit_Click()
Call RunIDFEdit
End Sub
Private Sub cmdInputBrowse_Click()
Call SelectInput
End Sub
Private Sub cmdWeatherBrowse_Click()
Call SelectWeather
End Sub
Private Sub cmdInputEdit_Click()
Call RunTextEditor
End Sub
Private Sub cmdNewGroup_Click()
Call runCreateNewRunQueue
End Sub
Private Sub cmdSimulate_Click()
Call RunSimulation
End Sub
Private Sub cmdSimulateGroup_Click()
Call RunSimulationGroup
End Sub
Private Sub cmdViewHistoryFile_Click()
Call runViewSelectedFileFromHistory
End Sub

'from group tab
Private Sub cmdViewGroupError_Click()
Call RunGrpErrEdit
End Sub



Private Sub cmdAudit_Click()
Call RunOutputEditorSingleFile(".AUDIT")
End Sub

Private Sub cmdBND_Click()
Call RunOutputEditorSingleFile(".BND")
End Sub

Private Sub cmdDBG_Click()
Call RunOutputEditorSingleFile(".DBG")
End Sub

Private Sub cmdDFDMP_Click()
Call RunOutputEditorSingleFile("DElight.dfdmp")
End Sub

Private Sub cmdDXF_Click()
Call runViewDrawing
End Sub

Private Sub cmdEDD_Click()
Call RunOutputEditorSingleFile(".EDD")
End Sub

Private Sub cmdEIO_Click()
Call RunOutputEditorSingleFile(".EIO")
End Sub

Private Sub cmdELDMP_Click()
Call RunOutputEditorSingleFile("DElight.eldmp")
End Sub

Private Sub cmdEPMDET_Click()
Call RunOutputEditorSingleFile(".EPMDET")
End Sub

Private Sub cmdEPMIDF_Click()
Call RunOutputEditorSingleFile(".EPMIDF")
End Sub

Private Sub cmdERR_Click()
Call RunOutputEditorSingleFile(".ERR")
End Sub

Private Sub cmdESO_Click()
Call showESOfile
End Sub

Private Sub cmdEXPIDF_Click()
Call RunOutputEditorSingleFile(".EXPIDF")
End Sub

Private Sub cmdIN_Click()
Call RunOutputEditorSingleFile("DElight.IN")
End Sub

Private Sub cmdMain_Click()
Call viewMainCSV
End Sub

Private Sub cmdMAP_Click()
Call runOutputSpreadsheetSingleFile("Map.csv")
Call RunOutputEditorSingleFile("Map.tab")
End Sub

Private Sub cmdMeter_Click()
Call viewMETERfile
End Sub

Private Sub cmdMTD_Click()
Call RunOutputEditorSingleFile(".MTD")
End Sub

Private Sub cmdMTR_Click()
Call RunOutputEditorSingleFile(".MTR")
End Sub

Private Sub cmdOUT_Click()
Call RunOutputEditorSingleFile("DElight.out")
End Sub

Private Sub cmdProcCSV_Click()
Call runOutputSpreadsheetSingleFile("-Proc.csv")
End Sub

Private Sub cmdRDD_Click()
Call RunOutputEditorSingleFile(".RDD")
End Sub

Private Sub cmdMDD_Click()
Call RunOutputEditorSingleFile(".MDD")
End Sub

Private Sub cmdScreen_Click()
Call runOutputSpreadsheetSingleFile("Screen.csv")
End Sub

Private Sub cmdSHD_Click()
Call RunOutputEditorSingleFile(".SHD")
End Sub

Private Sub cmdSLN_Click()
Call RunOutputEditorSingleFile(".SLN")
End Sub

Private Sub cmdSSZ_Click()
Call viewSSZFile
End Sub

Private Sub cmdSVG_Click()
Call showSingleSVGFile(".SVG")
End Sub

Private Sub cmdTable_Click()
Call viewTABLEfile
End Sub

Private Sub cmdVRML_Click()
Call runOutputVRMLdisplay
End Sub

Private Sub cmdZSZ_Click()
Call viewZSZFile
End Sub

Private Sub cmdSpSZ_Click()
Call viewSpSZFile
End Sub

Private Sub cmdBsmt_Click()
Call RunOutputEditorSingleFile(".bsmt")
End Sub

Private Sub cmdBsmtAudit_Click()
Call RunOutputEditorSingleFile("_bsmt.audit")
End Sub

Private Sub cmdBsmtCSV_Click()
Call runOutputSpreadsheetSingleFile("_bsmt.csv")
End Sub

Private Sub cmdBsmtOut_Click()
Call RunOutputEditorSingleFile("_bsmt.out")
End Sub

Private Sub cmdSlab_Click()
Call RunOutputEditorSingleFile(".slab")
End Sub

Private Sub cmdSlabErr_Click()
Call RunOutputEditorSingleFile("_slab.ger")
End Sub

Private Sub cmdSlabOut_Click()
Call RunOutputEditorSingleFile("_slab.out")
End Sub

Private Sub cmdTblXML_Click()
Call showSingleXMLFile("Table.xml")
End Sub

Private Sub cmdPerfLogCSV_Click()
Call runOutputSpreadsheetSingleFile("_perflog.csv")
End Sub




'============================
'============================
'============================
'  MENU ACTIVATIONS
'============================
'============================
'============================

' File
Private Sub mnuFileIn_Click()
Call SelectInput
End Sub
Private Sub mnuFileNewGroup_Click()
Call runCreateNewRunQueue
End Sub
Private Sub mnuFileGroup_Click()
Call SelectGroup
End Sub



Private Sub mnuFileStop_Click()
Call cancelAdditionalSimulations
End Sub

Private Sub mnuFileWeather_Click()
Call SelectWeather
End Sub

Private Sub mnuFileSim_Click()
Call RunSimulation
End Sub
Private Sub mnuFileSimGroup_Click()
Call RunSimulationGroup
End Sub
Private Sub mnuFileExit_Click()
Call ExitEPL
End Sub
Private Sub mnuFileTransitionVersion_Click()
Call transitionInputFile
End Sub

' Edit
Private Sub mnuEditInputText_Click()
Call RunTextEditor
End Sub
Private Sub mnuEditIDF_Click()
Call RunIDFEdit
End Sub
Private Sub mnuEditWeather_Click()
Call RunWeatherEdit
End Sub
Private Sub mnuEditPost_Click()
Call RunPostEdit
End Sub





' View
Private Sub mnuViewDrawing_Click()
Call viewOutputSet(outSetDrawingFiles)
End Sub

Private Sub mnuViewEEREIO_Click()
Call RunOutputEditorEEREIO
End Sub
Private Sub mnuViewGroup_Click()
Call RunGroupEdit
End Sub

Private Sub mnuViewOptions_Click()
frmOptions.Show vbModal
Call resetWideView
Call resetSimAboveView
End Sub



Private Sub queueTimer_Timer()
Call manageSimulationQueue
End Sub

Private Sub viewSelectedHistory_Click()
Call runViewSelectedFileFromHistory
End Sub
' view single files
Private Sub mnuViewEIO_Click()
Call RunOutputEditorSingleFile(".EIO")
End Sub
Private Sub mnuViewERR_Click()
Call RunOutputEditorSingleFile(".ERR")
End Sub
Private Sub mnuViewESO_Click()
Call showESOfile
End Sub
Private Sub mnuViewMTD_Click()
Call RunOutputEditorSingleFile(".MTD")
End Sub
Private Sub mnuViewMTR_Click()
Call RunOutputEditorSingleFile(".MTR")
End Sub
Private Sub mnuViewAUDIT_Click()
Call RunOutputEditorSingleFile(".AUDIT")
End Sub
Private Sub mnuViewBND_Click()
Call RunOutputEditorSingleFile(".BND")
End Sub
Private Sub mnuViewDBG_Click()
Call RunOutputEditorSingleFile(".DBG")
End Sub
Private Sub mnuViewRDD_Click()
Call RunOutputEditorSingleFile(".RDD")
End Sub
Private Sub mnuViewMDD_Click()
Call RunOutputEditorSingleFile(".MDD")
End Sub
Private Sub mnuViewSLN_Click()
Call RunOutputEditorSingleFile(".SLN")
End Sub
Private Sub mnuViewEPMIDF_Click()
Call RunOutputEditorSingleFile(".EPMIDF")
End Sub
Private Sub mnuViewEPMDET_Click()
Call RunOutputEditorSingleFile(".EPMDET")
End Sub
Private Sub mnuViewDXF_Click()
Call runViewDrawing
End Sub
Private Sub mnuViewMAP_Click()
Call runOutputSpreadsheetSingleFile("Map.csv")
End Sub
Private Sub mnuViewSHD_Click()
Call RunOutputEditorSingleFile(".SHD")
End Sub
Private Sub mnuViewSVG_Click()
Call showSingleSVGFile(".SVG")
End Sub
Private Sub mnuViewExpIDF_Click()
Call RunOutputEditorSingleFile(".expidf")
End Sub
Private Sub mnuViewDelightDFDMP_Click()
Call RunOutputEditorSingleFile("DElight.dfdmp")
End Sub
Private Sub mnuViewDelightELDMP_Click()
Call RunOutputEditorSingleFile("DElight.eldmp")
End Sub
Private Sub mnuViewDelightIN_Click()
Call RunOutputEditorSingleFile("DElight.in")
End Sub
Private Sub mnuViewDelightOut_Click()
Call RunOutputEditorSingleFile("DElight.out")
End Sub
Private Sub mnuViewErrGrp_Click()
Call RunGrpErrEdit
End Sub
Private Sub mnuViewSSZ_Click()
Call viewSSZFile
End Sub
Private Sub mnuViewZSZ_Click()
Call viewZSZFile
End Sub
Private Sub mnuViewSpSZ_Click()
Call viewSpSZFile
End Sub
Private Sub mnuViewTABLE_Click()
Call viewTABLEfile
End Sub
Private Sub mnuViewCSV_Click()
Call viewMainCSV
End Sub
Private Sub mnuViewMETER_Click()
Call viewMETERfile
End Sub
Private Sub mnuViewVCpErr_Click()
Call RunOutputEditorSingleFile(".VCpErr")
End Sub
Private Sub mnuViewScreen_Click()
Call runOutputSpreadsheetSingleFile("Screen.csv")
End Sub
Private Sub mnuViewVRML_Click()
Call runOutputVRMLdisplay
End Sub
Private Sub mnuViewProcCSV_Click()
Call runOutputSpreadsheetSingleFile("-Proc.csv")
End Sub
Private Sub mnuViewEdd_Click()
Call RunOutputEditorSingleFile(".EDD")
End Sub
Private Sub mnuViewSlab_Click()
Call RunOutputEditorSingleFile(".slab")
End Sub
Private Sub mnuViewSlabErr_Click()
Call RunOutputEditorSingleFile("_slab.ger")
End Sub
Private Sub mnuViewSlabOut_Click()
Call RunOutputEditorSingleFile("_slab.out")
End Sub
Private Sub mnuViewBsmt_Click()
Call RunOutputEditorSingleFile(".bsmt")
End Sub
Private Sub mnuViewBsmtAudit_Click()
Call RunOutputEditorSingleFile("_bsmt.audit")
End Sub
Private Sub mnuViewBsmtOut_Click()
Call RunOutputEditorSingleFile("_bsmt.out")
End Sub
Private Sub mnuViewBsmtCSV_Click()
Call runOutputSpreadsheetSingleFile("_bsmt.csv")
End Sub
Private Sub mnuViewPerfLogCSV_Click()
Call runOutputSpreadsheetSingleFile("_perflog.csv")
End Sub



'open folders containing files
Private Sub mnuViewFolderGroup_Click()
Call openFolderContaining(groupFileName)
End Sub
Private Sub mnuViewFolderInput_Click()
Call openFolderContaining(inputFileName)
End Sub
Private Sub mnuViewFolderWeather_Click()
Call openFolderContaining(weatherFileName)
End Sub

'other view menu items
Private Sub mnuViewOut_Click()
Call viewOutputSet(outSetTextOutputFiles)
End Sub
Private Sub mnuViewSpreadsheet_Click()
Call viewOutputSet(outSetSpreadsheets)
End Sub

' Help
Private Sub mnuHelpAbout_Click()
frmAbout.Show
End Sub
Private Sub mnuHelpEPDocs_Click()
'Call startAcrobat("EPlusMainMenu.pdf")
Call viewWebPage(appPath & "Documentation\index.html")
End Sub
Private Sub mnuHelpEssentials_Click()
'Call startAcrobat("EnergyPlusEssentials.pdf")
Call viewWebPage("https://energyplus.readthedocs.io/en/latest/essentials/essentials.html")
End Sub
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
'Call startAcrobat("OtherInformation.pdf") change for 1.09 version on Dec 12, 2002
'Call startAcrobat("AuxiliaryPrograms.pdf")
Call viewWebPage("https://energyplus.readthedocs.io/en/latest/auxiliary-programs/auxiliary-programs.html")
End Sub
Private Sub mnuHelpCompliance_Click()
Call startAcrobat("UsingEnergyPlusForCompliance.pdf")
End Sub
Private Sub mnuHelpPlantAppl_Click()
Call startAcrobat("PlantApplicationGuide.pdf")
End Sub
Private Sub mnuHelpEMS_Click()
'Call startAcrobat("EMSApplicationGuide.pdf")
Call viewWebPage("https://energyplus.readthedocs.io/en/latest/ems-application-guide/ems-application-guide.html")
End Sub
Private Sub mnuHelpExtInterface_Click()
Call startAcrobat("ExternalInterfacesApplicationGuide.pdf")
End Sub
Private Sub mnuHelpTips_Click()
'Call startAcrobat("TipsAndTricksUsingEnergyPlus.pdf")
Call viewWebPage("https://energyplus.readthedocs.io/en/latest/tips_and_tricks/tips_and_tricks.html")
End Sub
Private Sub mnuHelpAcknowledge_Click()
'Call startAcrobat("Acknowledgments.pdf")
Call viewWebPage("https://energyplus.readthedocs.io/en/latest/acknowledgments/acknowledgments.html")
End Sub
Private Sub mnuHelpCheckUpdates_Click()
Call checkForUpdatesNow(True)
End Sub
Private Sub mnuHelpViewUpdateList_Click()
Call viewWebPage(updatePageURL)
End Sub
Private Sub mnuHelpQuickstart_Click()
Call viewWebPage("https://energyplus.net/quick-start")
End Sub


'=======================================================
' Start Adobe Acrobat Reader With a File
'=======================================================
Sub startAcrobat(s)
Dim shellString As String
On Error Resume Next
Debug.Print pdfViewerFileName
shellString = ShortName(pdfViewerFileName) & " " & ShortName(appPath & "Documentation\" & s)
Shell shellString, vbNormalFocus
If Err.Number <> 0 Then
  MsgBox "Error initiating acrobat to view documentation file.", vbExclamation, "Help Cancelled"
  Exit Sub
End If
End Sub

'=======================================================
'  Menu Help
'=======================================================
Private Sub mnuHelpEPL_Click()
MsgBox "Using EP-Launch is documented in GettingStarted under the heading " & q & "Running EnergyPlus" & q & vbCrLf & vbCrLf & _
"To open the GettingStarted documentation, use the HELP menu and select EnergyPlus Getting Started. " & vbCrLf & _
"This will open a PDF file called GettingStarted.pdf. In GettingStarted.pdf select the section on " & q & "Running EnergyPlus" & _
q, vbInformation, "EP-Launch Help"
End Sub

'==========================
'  Drop Down List Routines
Private Sub cmbInput_Click()
Dim i As Integer
inputFileName = cmbInput.List(cmbInput.ListIndex)
selectedInputIndex = cmbInput.ListIndex
outputFileName = NoExtension(inputFileName)
Debug.Print "New Input File Selected: "; inputFileName
Debug.Print "                   path: "; pathOnly(inputFileName)
Select Case UCase(Right(RTrim(inputFileName), 3))
  Case "IDF"
    eplUI.cmdIDFEdit.Enabled = True
    eplUI.mnuEditIDF.Enabled = True
    cmbWeather.Enabled = True
    cmdWeatherBrowse.Enabled = True
    frmWeatherFile.Enabled = True
    frameView.Enabled = True
    mnuViewOut.Enabled = True
    mnuViewDrawing.Enabled = True
    mnuViewSpreadsheet.Enabled = True
    viewHTMLfile.Enabled = True
    mnuViewEEREIO.Enabled = True
    mnuViewSingleFile.Enabled = True
  Case "IMF"
    eplUI.cmdIDFEdit.Enabled = False
    eplUI.mnuEditIDF.Enabled = False
    cmbWeather.Enabled = True
    cmdWeatherBrowse.Enabled = True
    frmWeatherFile.Enabled = True
    frameView.Enabled = True
    mnuViewOut.Enabled = True
    mnuViewDrawing.Enabled = True
    mnuViewSpreadsheet.Enabled = True
    viewHTMLfile.Enabled = True
    mnuViewEEREIO.Enabled = True
    mnuViewSingleFile.Enabled = True
End Select
Call checkOutputButtonsFiles
Call warnIfBothIdfImf
End Sub

Private Sub cmbWeather_Click()
weatherFileName = cmbWeather.List(cmbWeather.ListIndex)
selectedWeatherIndex = cmbWeather.ListIndex
Debug.Print "New Weather File Selected: "; weatherFileName
End Sub
Private Sub cmbGroup_Click()
groupFileName = cmbGroup.List(cmbGroup.ListIndex)
selectedGroupIndex = cmbGroup.ListIndex
Debug.Print "New Group File Selected: "; groupFileName
Call checkOutputButtonsFiles
End Sub

Private Sub tabMain_Click()
curTab = tabMain.SelectedItem.Index
Select Case curTab
  Case 1 'single
    frameSingle.Left = 240
    frameGroup.Left = -30000
    frameHistory.Left = -30000
    frameUtility.Left = -30000
  Case 2 'group
    frameSingle.Left = -30000
    frameGroup.Left = 240
    frameHistory.Left = -30000
    frameUtility.Left = -30000
  Case 3 'history
    frameSingle.Left = -30000
    frameGroup.Left = -30000
    frameHistory.Left = 240
    frameUtility.Left = -30000
  Case 4 'utility
    frameSingle.Left = -30000
    frameGroup.Left = -30000
    frameHistory.Left = -30000
    frameUtility.Left = 240
End Select
End Sub


Private Sub tabViewResults_Click()
Select Case tabViewResults.SelectedItem.Index
  Case 1 'groups of output files
    frameViewAllOut.Left = -30000
    frameViewSelectOut.Left = 600
    viewAllOutputTabSelected = False
  Case 2 'single output file
    frameViewAllOut.Left = 600
    frameViewSelectOut.Left = -30000
    viewAllOutputTabSelected = True
End Select
End Sub

'============================
'============================
'============================
'  File Opening Routines
'============================
'============================
'============================
'
'=======================================================
' Select the input file open dialog
'=======================================================
Sub SelectInput()
Dim oldFN As String
oldFN = inputFileName
CommonDialog1.fileName = ""
CommonDialog1.DialogTitle = "Find EnergyPlus Input File"
CommonDialog1.Filter = "EnergyPlus Input (*.idf)|*.idf|EnergyPlus Macro (*.imf)|*.imf|EnergyPlus Input or Macro (*.idf;*.imf)|*.idf;*.imf"
CommonDialog1.FilterIndex = selectInputFilterIndex
CommonDialog1.Flags = &H800 Or &H1000 Or cdlOFNHideReadOnly
CommonDialog1.InitDir = lastInputDirectory
CommonDialog1.ShowOpen
inputFileName = CommonDialog1.fileName
If inputFileName = "" Then inputFileName = oldFN
outputFileName = NoExtension(inputFileName)
selectInputFilterIndex = CommonDialog1.FilterIndex
lastInputDirectory = pathOnly(inputFileName)
Call addItemToInputList(inputFileName)
 End Sub

'=======================================================
' Add an item to the input list
'=======================================================
Sub addItemToInputList(nameOfItem As String)
Dim existInList As Integer, i As Integer
' check if it is in the pull down list already
Err.Clear
On Error Resume Next
If nameOfItem <> "" Then
  existInList = -1
  For i = 0 To cmbInput.ListCount
    If cmbInput.List(i) = nameOfItem Then
      existInList = i
      Exit For
    End If
  Next i
  If existInList > -1 And cmbInput.ListIndex >= 0 Then 'it was already in the list just point to it
    cmbInput.ListIndex = existInList
  Else  'add it to the beginning of the list
    cmbInput.AddItem nameOfItem, 0
    cmbInput.ListIndex = 0
    Err.Clear
    'if the list has gotten two long then get rid of last item
    If cmbInput.ListCount > maxInputListSize Then cmbInput.RemoveItem cmbInput.ListCount - 1
    If Err.Number <> 0 Then
      MsgBox "Error removing last item from list"
      Err.Clear
    End If
  End If
End If
End Sub

'=======================================================
' Select the weather file open dialog
'=======================================================
Sub SelectWeather()
Dim existInList As Integer, i As Integer
Dim oldFN As String
oldFN = weatherFileName
CommonDialog1.fileName = ""
CommonDialog1.DialogTitle = "Find EnergyPlus Weather File"
CommonDialog1.Filter = "EnergyPlus Weather (*.epw)|*.epw"
CommonDialog1.FilterIndex = 1
CommonDialog1.Flags = &H800 Or &H1000
CommonDialog1.InitDir = lastWeatherDirectory
CommonDialog1.ShowOpen
weatherFileName = CommonDialog1.fileName
If weatherFileName = "" Then weatherFileName = oldFN
' check if it is in the pull down list already
lastWeatherDirectory = pathOnly(weatherFileName)
If weatherFileName <> "" Then
  existInList = -1
  For i = 0 To cmbWeather.ListCount
    If cmbWeather.List(i) = weatherFileName Then
      existInList = i
      Exit For
    End If
  Next i
  If existInList > -1 And cmbWeather.ListIndex >= 0 Then 'it was already in the list just point to it
    cmbWeather.ListIndex = existInList
  Else  'add it to the beginning of the list
    cmbWeather.AddItem weatherFileName, 1
    cmbWeather.ListIndex = 1
    'if the list has gotten two long then get rid of last item
    If cmbWeather.ListCount > maxWeatherListSize Then cmbWeather.RemoveItem maxWeatherListSize - 1
  End If
End If
End Sub

'=======================================================
' Select the group file open dialog
'=======================================================
Sub SelectGroup()
Dim oldFN As String
oldFN = groupFileName
CommonDialog1.fileName = ""
CommonDialog1.DialogTitle = "Find EnergyPlus Group File"
CommonDialog1.Filter = "EnergyPlus Group (*.epg)|*.epg"
CommonDialog1.FilterIndex = 1
CommonDialog1.Flags = &H800 Or &H1000
CommonDialog1.InitDir = lastGroupDirectory
CommonDialog1.ShowOpen
groupFileName = CommonDialog1.fileName
If groupFileName = "" Then groupFileName = oldFN
' check if it is in the pull down list already
lastGroupDirectory = pathOnly(groupFileName)
Call addItemToGroupList(groupFileName)
End Sub

'=======================================================
' Select the group file open dialog
'=======================================================
Sub addItemToGroupList(nameOfItem As String)
Dim existInList As Integer, i As Integer
On Error Resume Next
If nameOfItem <> "" Then
  existInList = -1
  For i = 0 To cmbGroup.ListCount
    If cmbGroup.List(i) = nameOfItem Then
      existInList = i
      Exit For
    End If
  Next i
  If existInList > -1 And cmbGroup.ListIndex >= 0 Then 'it was already in the list just point to it
    cmbGroup.ListIndex = existInList
  Else  'add it to the beginning of the list
    cmbGroup.AddItem nameOfItem, 0
    cmbGroup.ListIndex = 0
    'if the list has gotten two long then get rid of last item
    If cmbGroup.ListCount > maxGroupListSize Then cmbGroup.RemoveItem maxGroupListSize - 1
  End If
End If
End Sub

'=======================================================
' Select a program to view the specified kind of file
'=======================================================
Public Function SelectApplication(dialogPrompt As String, oldAppFileName) As String
Dim oldFN As String
Dim newFN As String
oldFN = oldAppFileName
CommonDialog1.fileName = oldAppFileName
CommonDialog1.DialogTitle = dialogPrompt
CommonDialog1.Filter = "Executable (*.exe)|*.exe|All Files (*.*)|*.*"
CommonDialog1.FilterIndex = 1
CommonDialog1.Flags = &H800 Or &H1000
CommonDialog1.ShowOpen
newFN = CommonDialog1.fileName
If newFN <> "" Then
  SelectApplication = newFN
Else
  SelectApplication = oldFN
End If
End Function

'============================
'============================
'============================
' Run All Programs
'============================
'============================
'============================

'=======================================================
' Run either a specific simulation or a set of parametric simulations
'=======================================================
Sub RunSimulation()
If enableParametricPreprocessor Then
  If Not doesIDFcontainParametrics(inputFileName) Then
    'no parametric objects found so run the file
    Call addToSimulationQueue(inputFileName, weatherFileName, NoExtension(inputFileName), "", 0, True)
  Else 'found parametric objects so run preprocessor
    Call RunParametricInput(inputFileName, weatherFileName)
  End If
Else 'don't check for parametric object and just run the file
  Call addToSimulationQueue(inputFileName, weatherFileName, NoExtension(inputFileName), "", 0, True)
End If
End Sub

'=======================================================
' Adds the input file and weather file combination
' to the list of runs in the simulation queue that
' need to be simulated. A timer driven function
' will actually perform the simulations
'=======================================================
Sub addToSimulationQueue(inName As String, wthrName As String, outName As String, grpName As String, counterIn As Integer, showMessages As Boolean)
Dim sizeSimQueue As Integer
Dim IDFversion As String
Dim msg As String
Dim cancelDueToVersionCheck As Boolean
'Version checking
cancelDueToVersionCheck = False
If testViewConvertOld Then
  If currentVersion <> "" Then
    IDFversion = checkIDFVersion(inName)
    If IDFversion = previousVersion Then
      msg = "The file:" & vbCrLf & vbCrLf
      msg = msg & "  " & inName & vbCrLf & vbCrLf
      msg = msg & "is an old version (" & previousVersion & "). "
      msg = msg & "To update the file to the latest version ("
      msg = msg & currentVersion & ") use the TRANSITION VERSION option under the FILE menu " & vbCrLf & vbCrLf
      msg = msg & "Proceed with simulation using old version?"
      If MsgBox(msg, vbOKCancel, "Check File Version") = vbCancel Then
        cancelDueToVersionCheck = True
      End If
    ElseIf IDFversion = "VERSION NOT FOUND" Then
      msg = "The VERSION object was not found in the file:" & vbCrLf & vbCrLf
      msg = msg & "  " & inName & vbCrLf & vbCrLf
      msg = msg & "You may wish to else turn off version checking under View..Option..Miscellaneous." & vbCrLf & vbCrLf
      msg = msg & "The simulation will proceed when you press OK."
      MsgBox msg, vbInformation, "VERSION object missing"
      cancelDueToVersionCheck = False
    ElseIf isNewerVersion(IDFversion, currentVersion) Then
      msg = "The file:" & vbCrLf & vbCrLf
      msg = msg & "  " & inName & vbCrLf & vbCrLf
      msg = msg & "is an not the same version (" & IDFversion & ") as the EnergyPlus version (" & currentVersion & ")."
      msg = msg & "You may need download EnergyPlus from http://www.energyplus.net/." & vbCrLf & vbCrLf
      msg = msg & "Proceed with simulation using the different version?"
      If MsgBox(msg, vbOKCancel, "Check File Version") = vbCancel Then
        cancelDueToVersionCheck = True
      End If
    ElseIf IDFversion <> currentVersion Then
      msg = "The file:" & vbCrLf & vbCrLf
      msg = msg & "  " & inName & vbCrLf & vbCrLf
      msg = msg & "is an old version (" & IDFversion & "). "
      msg = msg & "To update the file to the latest version ("
      msg = msg & currentVersion & ") use the IDFVersionUpdater program on the Utility tab. "
      msg = msg & "And you may need to also download additional Transition programs from http://www.energyplus.net/ on the Extras page." & vbCrLf & vbCrLf
      msg = msg & "Proceed with simulation using old version?"
      If MsgBox(msg, vbOKCancel, "Check File Version") = vbCancel Then
        cancelDueToVersionCheck = True
      End If
    End If
  End If
End If
'lock the queue when resizing so that other routines don't try to use it as the same time
queueBeingResized = True
'resize array if needed
sizeSimQueue = UBound(simQueue)
If numSimQueue >= sizeSimQueue Then
  ReDim Preserve simQueue(sizeSimQueue * 2)
End If
'assign to next spot in array
numSimQueue = numSimQueue + 1
simQueue(numSimQueue).status = qStatNotRun
simQueue(numSimQueue).nameIn = inName
simQueue(numSimQueue).nameWthr = wthrName
simQueue(numSimQueue).nameOut = outName
simQueue(numSimQueue).nameGrp = grpName
simQueue(numSimQueue).counter = counterIn
simQueue(numSimQueue).messagesShow = showMessages
simQueue(numSimQueue).timeStart = -1 'flag set so elapsed time does not show until it is set
'unlock the queue since resizing complete
queueBeingResized = False
' results of the version checking flag
If cancelDueToVersionCheck Then
  simQueue(numSimQueue).reasonCancelled = "The file has a VERSION object that does not match the version of EnergyPlus being used."
  simQueue(numSimQueue).status = qStatDoneCancelled
End If
' verify that the run should not be cancelled
If inName = "" Then
  simQueue(numSimQueue).reasonCancelled = "No Input File Selected"
  simQueue(numSimQueue).status = qStatDoneCancelled
End If
If wthrName = "" Then
  simQueue(numSimQueue).reasonCancelled = "No Weather File Selected"
  simQueue(numSimQueue).status = qStatDoneCancelled
End If
If outName = "" Then
  simQueue(numSimQueue).reasonCancelled = "No Output File Specified"
  simQueue(numSimQueue).status = qStatDoneCancelled
End If
If isContainsInvalids(inName) Then
  simQueue(numSimQueue).reasonCancelled = "The IDF file path contains one or more invalid characters: &^,=%" & Chr(34)
  simQueue(numSimQueue).status = qStatDoneCancelled
End If
If isContainsInvalids(wthrName) Then
  simQueue(numSimQueue).reasonCancelled = "The weather file path contains one or more invalid characters: &^,=%" & Chr(34)
  simQueue(numSimQueue).status = qStatDoneCancelled
End If
If isContainsInvalids(outName) Then
  simQueue(numSimQueue).reasonCancelled = "The output file path contains one or more invalid characters: &^,=%" & Chr(34)
  simQueue(numSimQueue).status = qStatDoneCancelled
End If
If isContainsInvalids(appPath) Then
  simQueue(numSimQueue).reasonCancelled = "The path that EP-Launch is located contains one or more invalid characters: &^,=%" & Chr(34)
  simQueue(numSimQueue).status = qStatDoneCancelled
End If
' if this is the first item in the simulation queue start
' managing the queue without delay so that it will start
' to run the simulation
If numSimQueue = 1 Then
  Call manageSimulationQueue
End If
End Sub

'=======================================================
' The main routine checkes if any simulations in the queue
' need to be performed. If no items are left in the queue
' then message is shown that everything is completed.
' This routine is called by a timer every second or less.
'=======================================================
Sub manageSimulationQueue()
Dim prevStatus As Long
Dim curStatus As Long
Dim countNotRun As Long
Dim countActive As Long
Dim countDone As Long
Dim countCrashes As Long
Dim firstNotRun As Long
Dim textFromErr As String
Dim flNum As Integer
Dim i As Long
queueTimer.Enabled = False 'turn off the timer to prevent this routine from being started more than once
Debug.Print "Begin manageSimulationQueue: "; Time()
If queueBeingResized Then Exit Sub 'if the queue is being resized or added to at this exact time of this call, skip the routine
firstNotRun = 0
On Error Resume Next
'update status of all previously active simulations
For i = 1 To numSimQueue
  prevStatus = simQueue(i).status
  ' Check if the status of a simulation in the simQueue
  ' is still running or if it is complete.
  If prevStatus = qStatActive Then
    If isProcessDone(simQueue(i).processHandle) Then
      'precess is complete but still need to classify
      'if ok or crashed (if cancelled it would already be classified)
      Err.Clear
      flNum = FreeFile
      Open simQueue(i).usedOutFileName & ".err" For Input As flNum
      'Call MsgBox("opening " & simQueue(i).usedOutFileName & ".err: " & Err.Description, vbInformation)
      If Err.Number <> 0 Then
        curStatus = qStatDoneCrashed
      Else
        Do While Not EOF(flNum)
            Line Input #flNum, textFromErr
            'Call MsgBox("read " & simQueue(i).usedOutFileName & ".err: " & textFromErr & " error: " & Err.Description, vbInformation)
        Loop
        'Call MsgBox(textFromErr, vbInformation)
        If Err.Number <> 0 Then
          curStatus = qStatDoneCrashed
        Else
            If InStr(textFromErr, "EnergyPlus") = 0 Or InStr(textFromErr, "Elapsed Time") = 0 Then
              curStatus = qStatDoneCrashed
            Else
              curStatus = qStatDoneOk
              simQueue(i).eplusoutEndText = Mid(textFromErr, 17) 'remove leading asterisks and spaces
            End If
        End If
        Close flNum
      End If
      'clear out remaining files
      Kill simQueue(i).tempFolder & "\eplusout.end"
      Kill simQueue(i).tempFolder & "\counter.inc"
      Kill simQueue(i).tempFolder & "\RUNEP.BAT"
    Else
      curStatus = prevStatus
    End If
  Else
    curStatus = prevStatus
  End If
  Select Case curStatus
    Case qStatNotRun
      countNotRun = countNotRun + 1
      If firstNotRun = 0 Then firstNotRun = i
    Case qStatActive
      countActive = countActive + 1
    Case qStatDoneOk
      countDone = countDone + 1
      If prevStatus = qStatActive Then
        Call postProcessSimulation(i)
      End If
    Case qStatDoneCancelled
      countDone = countDone + 1
      If prevStatus = qStatActive Then
        Call postProcessSimulation(i)
      End If
    Case qStatDoneCrashed
      countDone = countDone + 1
      countCrashes = countCrashes + 1
      If prevStatus = qStatActive Then
        Call postProcessSimulation(i)
      End If
  End Select
  simQueue(i).status = curStatus
Next i
If countNotRun > 0 Then
  'numberOfSimProcessesAllowed add to options and setting saving and getting
  If countActive < numberOfSimProcessesAllowed Then
    Call startSimulation(firstNotRun) 'only start one - the rest will start the next time
  End If
  eplUI.Caption = "EP-Launch - SIMULATION IN PROGRESS"
  eplUI.Refresh
End If
If numSimQueue > 0 And countDone = numSimQueue Then
  'display dialog showing all finished
  eplUI.Caption = "EP-Launch"
  eplUI.Refresh
  frmRunStatus.lstStatus.Clear
  For i = 1 To numSimQueue
    frmRunStatus.lstStatus.AddItem simQueue(i).nameIn
    frmRunStatus.lstStatus.AddItem simQueue(i).nameWthr
    Select Case simQueue(i).status
      Case qStatNotRun
        frmRunStatus.lstStatus.AddItem "NOT RUN.       " & simQueue(i).eplusoutEndText
      Case qStatActive
        frmRunStatus.lstStatus.AddItem "ACTIVE.        " & simQueue(i).eplusoutEndText
      Case qStatDoneOk
        frmRunStatus.lstStatus.AddItem "Run Complete.  " & simQueue(i).eplusoutEndText
      Case qStatDoneCancelled
        frmRunStatus.lstStatus.AddItem "Cancelled.     " & simQueue(i).eplusoutEndText & " " & simQueue(i).reasonCancelled
      Case qStatDoneCrashed
        frmRunStatus.lstStatus.AddItem "Crashed.       " & simQueue(i).eplusoutEndText
        crashFileName = simQueue(i).nameIn
        Call crashErrorMessage
    End Select
    'frmRunStatus.lstStatus.AddItem simQueue(i).eplusoutEndText
    frmRunStatus.lstStatus.AddItem ""
  Next i
  If simQueue(1).timeStart <> -1 Then
    frmRunStatus.lstStatus.AddItem "Total elapsed clock time: " & DateDiff("s", simQueue(1).timeStart, Time()) & " (seconds)"
  End If
  frmRunStatus.Show vbModal
  'remove directories that had good statuses
  For i = 1 To UBound(simQueue)
    RmDir simQueue(i).tempFolder
  Next i
  'clear queue
  numSimQueue = 0
  For i = 1 To UBound(simQueue)
    simQueue(i).counter = 0
    simQueue(i).eplusoutEndText = ""
    simQueue(i).messagesShow = False
    simQueue(i).nameGrp = ""
    simQueue(i).nameIn = ""
    simQueue(i).nameOut = ""
    simQueue(i).nameWthr = ""
    simQueue(i).processHandle = 0
    simQueue(i).reasonCancelled = ""
    simQueue(i).status = 0
    simQueue(i).tempFolder = ""
    simQueue(i).usedOutFileName = ""
    simQueue(i).timeStart = -1  'flag value to indicate no time has started yet.
  Next i
  Call checkOutputButtonsFiles
End If
Debug.Print "Complete manageSimulationQueue: "; Time()
queueTimer.Enabled = True 'turn on the timer
End Sub

'=======================================================
' Start the simulation based on the queue index provided
' and perform all checking
' This routine should not be executed more than once at
' a given time.
'=======================================================
Sub startSimulation(simQueueIndex As Long)
Dim inName As String
Dim wthrName As String
Dim outName As String
Dim currentDirectory As String
Dim flNum As Integer
Dim passedInputFileName As String
Dim passedOutFileName As String
Dim passedWthrFileName As String
Dim runBatchFile As String
Dim cmdLn As String
Dim curHandle As Integer
Dim outFN As Integer
Dim minWindow As Boolean
Dim showMessages As Boolean
Dim lineOfBatch As String
Dim i As Integer
'If startSimulationActive Then Exit Sub 'exit if already running once
'startSimulationActive = True
Debug.Print "Begin StartSimulation :"; simQueueIndex, Time()
' assign local variables
inName = simQueue(simQueueIndex).nameIn
wthrName = simQueue(simQueueIndex).nameWthr
outName = simQueue(simQueueIndex).nameOut
showMessages = simQueue(simQueueIndex).messagesShow
simQueue(simQueueIndex).timeStart = Time()
' check if all files available first
On Error Resume Next
flNum = FreeFile
Open inName For Input As flNum
If Err.Number <> 0 Then 'file not found
  simQueue(simQueueIndex).reasonCancelled = "Input File Not Found - Err 1: " & inName
  simQueue(simQueueIndex).status = qStatDoneCancelled
  Debug.Print "Exit StartSimulation at input file not found"; Err.Description
'  startSimulationActive = False
  Exit Sub
End If
Close flNum
If wthrName <> noWeatherFile Then
  Open wthrName For Input As flNum
  If Err.Number <> 0 Then 'file not found
    simQueue(simQueueIndex).reasonCancelled = "Weather File Not Found: " & inName
    simQueue(simQueueIndex).status = qStatDoneCancelled
    Debug.Print "Exit StartSimulation at Weather File Not Found"; Err.Description
'    startSimulationActive = False
    Exit Sub
  End If
  Close flNum
End If
' if international input file name then use the short file name
If checkIfInternationalFileName(inName) Then
  passedInputFileName = NoExtension(ShortName(inName))
  Debug.Print "                  short: "; passedInputFileName
Else
  passedInputFileName = NoExtension(inName)
End If
' if international output file name then use the short file name
If checkIfInternationalFileName(outName) Then
  passedOutFileName = ShortName(outName)
  Debug.Print "                  short: "; passedOutFileName
  If passedOutFileName = "" Then
    passedOutFileName = NoExtension(passedInputFileName) 'this is a workaround and may not always work
  End If
Else
  passedOutFileName = outName
End If
simQueue(simQueueIndex).usedOutFileName = passedOutFileName

' call the routine that ensures that the path with all directories exists.
Call makeSubdir(outName)
' if international weather file name then use the short file name
If checkIfInternationalFileName(wthrName) Then
  passedWthrFileName = ShortName(wthrName)
  Debug.Print "                  short: "; passedWthrFileName
Else
  passedWthrFileName = wthrName
End If
' check if all output files is open by another application
For i = 1 To numOutputKinds
  If checkIfFileIsLocked(passedOutFileName & outputKind(i).suffix) Then
    simQueue(simQueueIndex).reasonCancelled = "File Locked: " & passedOutFileName & outputKind(i).suffix
    simQueue(simQueueIndex).status = qStatDoneCancelled
    Debug.Print "Exit StartSimulation at file locked " & passedOutFileName & outputKind(i).suffix, Err.Description
'    startSimulationActive = False
    Exit Sub
  End If
Next i
' is batch file available?
Err.Clear
Open appPath & batchFileName For Input As flNum
If Err.Number <> 0 Then 'file not found
  simQueue(simQueueIndex).reasonCancelled = "Batch File Not Found"
  simQueue(simQueueIndex).status = qStatDoneCancelled
  Debug.Print "Exit StartSimulation at Batch File Not Found"; Err.Description
'  startSimulationActive = False
  Exit Sub
End If
Close flNum
' set flag if the window should be minimized
minWindow = False
If minimizeGroupCmd And Not showMessages Then
  minWindow = True
End If
If minimizeSingleCmd Then
  minWindow = True
End If
'create temporary directory if not already present
simQueue(simQueueIndex).tempFolder = upDirectory(passedOutFileName) & "\EPTEMP-" & Format(simQueueIndex, "00000000")
ChDrive Left(simQueue(simQueueIndex).tempFolder, 1)
MkDir simQueue(simQueueIndex).tempFolder
ChDir simQueue(simQueueIndex).tempFolder
'remove all files present
Kill simQueue(simQueueIndex).tempFolder & "\*.*"
'copy the perflog.csv file
' Name passedOutFileName & outputKind(okPerfLogCSV).suffix As simQueue(simQueueIndex).tempFolder & "\eplusout_perflog.csv"
Err.Clear
'Option to create a batch file RunEP.bat to run EnergyPlus. This is the normal
'way EnergyPlus is envoked but some security settings may not allow software to
'create batch files. For those users the batch file EPL-RUN batch file is called
'directly using arguments.
If CreateRunEPBatch Then
  outFN = FreeFile
  runBatchFile = "RUNEP.BAT"
  Open runBatchFile For Output As outFN
  If Err.Number > 0 Then
    simQueue(simQueueIndex).reasonCancelled = "Temporary Batch File Could Not Be Created. " & Err.Description
    simQueue(simQueueIndex).status = qStatDoneCancelled
'    startSimulationActive = False
    Debug.Print "Exit StartSimulation at Temporary Batch File Could Not Be Created"; Err.Description
    Exit Sub
  End If
  'start writing batch file
  'On Error GoTo 0
  Print #outFN, "SET epin="; passedInputFileName 'pass the input file name without extension
  Print #outFN, "SET epout="; passedOutFileName  'pass the output file name without extension
  Print #outFN, "SET epwthr="; passedWthrFileName
  If wthrName <> noWeatherFile Then
    Print #outFN, "SET eptype=EP"
  Else
    Print #outFN, "SET eptype=NONE"
  End If
  If pauseDuringRun And Not minWindow Then
    Print #outFN, "SET pausing=Y"
  Else
    Print #outFN, "SET pausing=N"
  End If
  'output of the type of extension (either idf or imf) added with v1.04
  Print #outFN, "SET epinext="; extensionOnly(inName)
  If allowGT250Col Then
    Print #outFN, "SET maxcol=nolimit"
  Else
    Print #outFN, "SET maxcol=250"
  End If
  If convertESOMTRIP Then
    Print #outFN, "SET convESO=Y"
  Else
    Print #outFN, "SET convESO=N"
  End If
  If createCSVprocFile Then
    Print #outFN, "SET procCSV=Y"
  Else
    Print #outFN, "SET procCSV=N"
  End If
  Print #outFN, "SET epPath="; appPath
  ' now copy the batch file contents to the temporary batch file
  flNum = FreeFile
  Open appPath & batchFileName For Input As flNum
  If Err.Number > 0 Then
    simQueue(simQueueIndex).reasonCancelled = "Original batch file could not be opened to be copied. " & Err.Description
    simQueue(simQueueIndex).status = qStatDoneCancelled
'    startSimulationActive = False
    Debug.Print "Exit StartSimulation at Original batch file could not be opened to be copied"; Err.Description
    Exit Sub
  End If
  Do While Not EOF(flNum)
    Line Input #flNum, lineOfBatch
    Print #outFN, lineOfBatch
    'Debug.Print "Line of batch file: "; lineOfBatch
  Loop
  Close flNum
  Close outFN
Else
  'Create the command line for running the batch file and on pass
  'all parameters on the command line.
  runBatchFile = appPath & batchFileName 'use EPL-RUN.BAT as the batch file that is run directly
  '%epin% or %1
  cmdLn = " " & q & passedInputFileName & q & " "
  '%epout% or %2
  cmdLn = cmdLn & q & passedOutFileName & q & " "
  '%epinext% or %3
  cmdLn = cmdLn & extensionOnly(inName) & " "
  '%epwthr% or %4
  cmdLn = cmdLn & q & passedWthrFileName & q & " "
  '%eptype% or %5
  If wthrName <> noWeatherFile Then
    cmdLn = cmdLn & "EP "
  Else
    cmdLn = cmdLn & "NONE "
  End If
  '%pausing% or %6
  If pauseDuringRun And Not minWindow Then
    cmdLn = cmdLn & "Y "
  Else
    cmdLn = cmdLn & "N "
  End If
  '%maxcol% or %7
  If allowGT250Col Then
    cmdLn = cmdLn & "nolimit "
  Else
    cmdLn = cmdLn & "250 "
  End If
  '%convESO% or %8
  If convertESOMTRIP Then
    cmdLn = cmdLn & "Y "
  Else
    cmdLn = cmdLn & "N "
  End If
  '%procCSV% or %9
  If createCSVprocFile Then
    cmdLn = cmdLn & "Y "
  Else
    cmdLn = cmdLn & "N "
  End If
  'if using parameter passing, append the command line parameters to the batch file name
  runBatchFile = runBatchFile & cmdLn
End If
'write the counter.inc file
outFN = FreeFile
Open "counter.inc" For Output As outFN
Print #outFN, ""
Print #outFN, "! From counter.inc created by EP-Launch when running groups of files"
Print #outFN, ""
Print #outFN, "##set1 counter "; simQueue(simQueueIndex).counter
Print #outFN, ""
Close outFN
'run the command line to call the batch file and don't wait for it to end
Debug.Print "Simulate Command Line:"; runBatchFile
simQueue(simQueueIndex).status = qStatActive
curHandle = ExecCmdNoWait(simQueueIndex, "CMD /e:3000 /c " & runBatchFile, minWindow)
If Err.Number > 0 Then
  simQueue(simQueueIndex).reasonCancelled = "Temporary Batch File " & runBatchFile & " Could Not Be Run"
  simQueue(simQueueIndex).status = qStatDoneCancelled
  Debug.Print "Exit StartSimulation at Temporary Batch File " & runBatchFile & " Could Not Be Run"; Err.Description
'  startSimulationActive = False
  Exit Sub
End If
simQueue(simQueueIndex).processHandle = curHandle
ChDrive appPath
ChDir appPath
Debug.Print "End StartSimulation :"; simQueueIndex, Time()
'startSimulationActive = False
End Sub

'=======================================================
' After a simulation has been completed this routine
' cleans up
'=======================================================
Sub postProcessSimulation(simQueueIndex As Long)
Dim inName As String
Dim outName As String
Dim passedOutFileName As String
Dim wthrName As String
Dim grpName As String
Dim counter As Integer
Dim grpErrorName As String
Dim historyLine As String
Dim egFL As Integer
Dim outFN As Integer
Dim i As Integer
On Error Resume Next
inName = simQueue(simQueueIndex).nameIn
grpName = simQueue(simQueueIndex).nameGrp
outName = simQueue(simQueueIndex).nameOut
passedOutFileName = simQueue(simQueueIndex).usedOutFileName
wthrName = simQueue(simQueueIndex).nameWthr
counter = simQueue(simQueueIndex).counter
If outName <> passedOutFileName Then
  For i = 1 To numOutputKinds
    Name passedOutFileName & outputKind(i).suffix As outName & outputKind(i).suffix
  Next i
End If
Err.Clear 'likely that one of these renaming commands did not run but we don't care
'create the line for the history file
historyLine = Format(Now, "YYYY-MMM-DD HH:MM:SS") & ","
If simQueue(simQueueIndex).status <> qStatDoneCrashed Then
  historyLine = historyLine & simQueue(simQueueIndex).eplusoutEndText & ","
Else
  historyLine = historyLine & "CRASHED,"
End If
historyLine = historyLine & inName & "," & wthrName & "," & outName & "," & grpName & ","
'MsgBox outName, vbDefaultButton1, "DEBUG"
For i = 1 To numOutputKinds
  historyLine = historyLine & includeIfExist(outName & outputKind(i).suffix) & ","
Next i
'now output the line for the history/log file
outFN = FreeFile
Open appPath & "history.csv" For Append As outFN
Print #outFN, historyLine
Close outFN
Call displayNewHistoryLine(historyLine)
'append error to the group error file if run as a group
If grpName <> "" Then
  grpErrorName = NoExtension(grpName) & ".errgrp"
  Err.Clear
  egFL = FreeFile
  Open grpErrorName For Append As egFL
  If Err.Number = 0 Then
    Print #egFL, ""
    Print #egFL, ""
    Print #egFL, "====================================================================================================================="
    Print #egFL, "Input File:     "; inName
    Print #egFL, "Weather File:   "; wthrName
    Print #egFL, "Output Files:   "; outName
    Print #egFL, "Output Time:    "; FileDateTime(outName & ".err")
    Print #egFL, "Counter:        "; counter
    Print #egFL, "---------------------------------------------------------------------------------------------------------------------"
    Close egFL
    Call appendTextFile(grpErrorName, outName & ".err")
  End If
End If
End Sub

'=======================================================
' Set all unrun files in the queue to cancelled
'=======================================================
Sub cancelAdditionalSimulations()
Dim i As Integer
For i = 1 To numSimQueue
  If simQueue(i).status = qStatNotRun Then
    simQueue(i).status = qStatDoneCancelled
    simQueue(i).eplusoutEndText = "Cancelled by user"
  End If
Next i
MsgBox "Additional Simulations Cancelled", vbOKOnly, "EP-Launch"
End Sub

'=======================================================
' Runs the transition program on the current input file
'=======================================================
Sub transitionInputFile()
Dim oldidfName As String
Dim oldIdfNameWithSuffix As String
Dim errFileName As String
Dim difNewFileName As String
Dim extraIdfNewName As String
Dim currentDirectory As String
Dim oldVerSuffix As String
Dim oldRviName As String
Dim oldRviNameWithSuffix As String
Dim extraRviNewName As String
Dim oldMviName As String
Dim oldMviNameWithSuffix As String
Dim extraMviNewName As String
Dim firstPeriodPos As Long

On Error Resume Next
'create the file suffix for the old version
firstPeriodPos = InStr(previousVersion, ".")
If firstPeriodPos = 2 Then
    oldVerSuffix = "_V" & Left(previousVersion, 1) & Mid(previousVersion, 3, 1) & Mid(previousVersion, 5, 1)
Else
    oldVerSuffix = "_V" & Left(previousVersion, 2) & Mid(previousVersion, 4, 1) & Mid(previousVersion, 6, 1)
End If
'save current directory
currentDirectory = CurDir
ChDir appPath & "PreProcess\IDFVersionUpdater"
ChDrive Left(appPath, 1)
'First make sure no OLDIDF file exists
oldidfName = NoExtension(inputFileName) & ".idfold"
errFileName = NoExtension(inputFileName) & ".VCpErr"
difNewFileName = NoExtension(inputFileName) & ".difnew"
extraIdfNewName = NoExtension(inputFileName) & ".idfnew"
oldIdfNameWithSuffix = NoExtension(inputFileName) & oldVerSuffix & ".idf"
'files for processing the RVI file
oldRviName = NoExtension(inputFileName) & ".rviold"
extraRviNewName = NoExtension(inputFileName) & ".rvinew"
oldRviNameWithSuffix = NoExtension(inputFileName) & oldVerSuffix & ".rvi"
'files for processing the RVI file
oldMviName = NoExtension(inputFileName) & ".mviold"
extraMviNewName = NoExtension(inputFileName) & ".mvinew"
oldMviNameWithSuffix = NoExtension(inputFileName) & oldVerSuffix & ".mvi"

If checkIfFileExists(oldidfName) Then
  MsgBox "Since the IDFOLD file already exists no translation will be performed.", vbCritical, "Transition Error"
  Exit Sub
End If
If checkIfFileExists(oldIdfNameWithSuffix) Then
  MsgBox "Since the " & oldVerSuffix & ".IDF  file already exists no translation will be performed.", vbCritical, "Transition Error"
  Exit Sub
End If
If Not checkIfFileExists(transitionFileName) Then
  MsgBox "Transition utility file not found", vbExclamation, "Transition Error"
End If
If checkIfFileExists(errFileName) Then Kill errFileName
If checkIfFileExists(difNewFileName) Then Kill difNewFileName
Err.Clear
ExecCmd transitionFileName & " " & q & inputFileName & q
If Err.Number <> 0 Then
  MsgBox "Cannot start transition program", vbCritical, "Transition Error"
  Exit Sub
End If
'check if an error occured in the translation
If checkIfFileExists(errFileName) Then
  If MsgBox("Errors occurred during transition.  Please see the VCpErr file for details. " & vbCrLf & vbCrLf & "Would you like to see the VCpErr file now?", vbYesNo + vbCritical, "Transition Error") = vbYes Then
    Call RunOutputEditorSingleFile(".VCpErr")
  End If
End If
'process the IDF related files
If checkIfFileExists(difNewFileName) Then Kill difNewFileName
If checkIfFileExists(extraIdfNewName) Then Kill extraIdfNewName
If checkIfFileExists(oldidfName) Then
  Name oldidfName As oldIdfNameWithSuffix
End If
'process the RVI related files
If checkIfFileExists(oldRviName) Then
  If checkIfFileExists(extraRviNewName) Then Kill extraRviNewName
  Name oldRviName As oldRviNameWithSuffix
End If
'process the MVI related files
If checkIfFileExists(oldMviName) Then
  If checkIfFileExists(extraMviNewName) Then Kill extraMviNewName
  Name oldMviName As oldMviNameWithSuffix
End If
ChDir currentDirectory
End Sub



'=======================================================
' If the file exists then pass the string back otherwise
' pass back an empty string
'=======================================================
Function includeIfExist(nameOfFile As String) As String
Dim filelength  As Long
On Error Resume Next
'check the length of all files before trying to open them
' don't really need the length but it makes for a quick test
Err.Clear
filelength = FileLen(nameOfFile)
If Err.Number = 0 Then
  includeIfExist = nameOfFile
Else
  includeIfExist = ""
End If
End Function


'=======================================================
' Crash Error Message
' Added in 1.08 version on Oct 17, 2002
'=======================================================
Sub crashErrorMessage()
'show the error message
crashMessage.Show vbModal
End Sub


'=======================================================
' Check if certain characters are part of a string that
' are invalid if used as file names or directory names.
'=======================================================
Function isContainsInvalids(inString) As Boolean
Dim res As Boolean
res = False
If InStr(inString, "&") > 0 Then res = True
If InStr(inString, "^") > 0 Then res = True
If InStr(inString, ",") > 0 Then res = True
If InStr(inString, "=") > 0 Then res = True
If InStr(inString, ";") > 0 Then res = True
If InStr(inString, "%") > 0 Then res = True
isContainsInvalids = res
End Function

'=======================================================
' Call the text editor with the input file
'=======================================================
Sub RunTextEditor()
If textEditFileName = "" Then
  MsgBox "No Text Editor Selected. Go to VIEW OPTIONS and select TEXT EDITOR to select program.", vbExclamation, "Edit Cancelled"
  Exit Sub
End If
On Error Resume Next
Shell textEditFileName & " " & q & inputFileName & q, vbNormalFocus
If Err.Number <> 0 Then
  MsgBox "Error initiating text editor.  This is likely due to the text editor program being deleted or moved. Please use the VIEW OPTIONS menu to select a new TEXT EDITOR program.", vbExclamation, "Edit Cancelled"
  Exit Sub
End If
End Sub

'=======================================================
' View a set of output files
'=======================================================
Sub viewOutputSet(setNum As Integer)
Dim iOutKind As Integer
Dim doOpen As Boolean
Dim spreadsheetFilesToOpen As String
On Error Resume Next
For iOutKind = 1 To numOutputKinds
  doOpen = False
  If outputKind(iOutKind).outSet(setNum) Then
    If outputKind(iOutKind).viewer And ovpTextEditor Then
      If Not outputKind(iOutKind).containsTabs Then
        doOpen = True
      Else ' one of the files with tabs
        If Not tabWithSpreadsheet Then
          doOpen = True
        End If
      End If
      If doOpen Then Call RunOutputEditorSingleFile(outputKind(iOutKind).suffix)
    ElseIf outputKind(iOutKind).viewer And ovpDrawingViewer Then
      Call runViewDrawing
    ElseIf outputKind(iOutKind).viewer And ovpVRMLviewer Then
      Call runOutputVRMLdisplay
    ElseIf outputKind(iOutKind).viewer And ovpSpreadsheet Then
      If Not outputKind(iOutKind).containsTabs Then
        doOpen = True
      Else ' one of the files with tabs
        If tabWithSpreadsheet Then
          doOpen = True
        End If
      End If
      If doOpen Then
        spreadsheetFilesToOpen = spreadsheetFilesToOpen & fileNameInQuotesIfExist(outputKind(iOutKind).suffix)
      End If
    ElseIf outputKind(iOutKind).viewer And ovpDiagramming Then
      Call showSingleSVGFile(".SVG")
    ElseIf outputKind(iOutKind).viewer And ovpHTMLBrowser Then
      Call showSingleHTMLFile("Table.html")
    ElseIf outputKind(iOutKind).viewer And ovpESOviewer Then
      Call showESOfile
    ElseIf outputKind(iOutKind).viewer And ovpXMLviewer Then
      Call showSingleXMLFile("Table.XML")
    Else
      'do nothing - no viewer associated with that kind of file (for example .SQL)
    End If
  End If
Next iOutKind
'now open any spreadsheet files that are queued up
If spreadsheetFilesToOpen <> "" Then
  If spreadsheetFileName = "" Then
    MsgBox "No Spreadsheet Program Selected.  To view the spreadsheet you need a program that can display a CSV file. Go to VIEW OPTIONS and select SPREADSHEET to select program.", vbExclamation, "Run Cancelled"
    Exit Sub
  End If
  Shell spreadsheetFileName & " /e " & spreadsheetFilesToOpen, vbNormalFocus
  If Err.Number <> 0 Then
    MsgBox "Error initiating spreadsheet.This is likely due to the spreadsheet program being deleted or moved. Please go to the VIEW OPTIONS menu to select a new SPREADSHEET program.", vbExclamation, "Spreadsheet Cancelled"
    Exit Sub
  End If
End If
Err.Clear
End Sub

'=======================================================
' If the output file exists with the suffix (end of file
' name and extension) then return the full file name
' in quotes.
'=======================================================
Function fileNameInQuotesIfExist(inSuffix As String) As String
On Error Resume Next
Err.Clear
If checkIfFileExists(outputFileName & inSuffix) Then
  fileNameInQuotesIfExist = " " & q & outputFileName & inSuffix & q & " "
End If
End Function

'=======================================================
' call the text editor with the output files
'=======================================================
Sub RunOutputEditorEEREIO()
Dim filelength As Long
If textEditFileName = "" Then
  MsgBox "No Text Editor Selected.  Go to VIEW OPTIONS and select TEXT EDITOR to select program.", vbExclamation, "Edit Cancelled"
  Exit Sub
End If
On Error Resume Next
Call RunOutputEditorSingleFile(".EIO")
Call RunOutputEditorSingleFile(".BND")
Call RunOutputEditorSingleFile(".ERR")
End Sub

'=======================================================
' Run the output editor with a single file as shown
' with the file extension
' NOTE: the file extension is preceded by a "."
' and a file ending with extension is allowed such
' as "Zsz.txt"
'=======================================================
Sub RunOutputEditorSingleFile(fileExtension As String)
Dim filelength As Long
If textEditFileName = "" Then
  MsgBox "No Text Editor Selected.  Go to VIEW OPTIONS and select TEXT EDITOR to select program.", vbExclamation, "Edit Cancelled"
  Exit Sub
End If
On Error Resume Next
'check the length of all files before trying to open them
' don't really need the length but it makes for a quick test
Err.Clear
filelength = FileLen(outputFileName & LCase(fileExtension))
If Err.Number = 0 Then
    Shell textEditFileName & " " & q & outputFileName & LCase(fileExtension) & q, vbNormalFocus
    If Err.Number <> 0 Then
        MsgBox "Error initiating text editor for file type" & fileExtension & ". This is likely due to the text editor program being deleted or moved. Please go to the VIEW OPTIONS menu to select a new TEXT EDITOR program.", vbExclamation, "Edit Cancelled"
        Exit Sub
    End If
End If
End Sub

'=======================================================
' Open a single HTML file with a internet browser
'=======================================================
Sub showSingleHTMLFile(fileExtension As String)
Dim filelength As Long
If htmlViewFileName = "" Then
  MsgBox "No HTML Browser Selected.  Go to VIEW OPTIONS and select HTML BROWSER to select program.", vbExclamation, "Edit Cancelled"
  Exit Sub
End If
On Error Resume Next
'check the length of all files before trying to open them
' don't really need the length but it makes for a quick test
Err.Clear
filelength = FileLen(outputFileName & LCase(fileExtension))
If Err.Number = 0 Then
    Shell htmlViewFileName & " " & q & outputFileName & LCase(fileExtension) & q, vbNormalFocus
    If Err.Number <> 0 Then
        MsgBox "Error initiating HTML browser for file type" & fileExtension & ". This is likely due to the HTML browser program being deleted or moved. Please go to VIEW OPTIONS to select a new HTML BROWSER program.", vbExclamation, "Edit Cancelled"
        Exit Sub
    End If
End If
End Sub

'=======================================================
' Show the ESO file with a special viewer. If no special
' ESO viewer has been selected use a text editor.
'=======================================================
Sub showESOfile()
If esoViewFileName <> "" Then
  If checkIfFileExists(outputFileName & ".eso") Then
    Shell esoViewFileName & " " & q & outputFileName & ".eso" & q, vbNormalFocus
  End If
Else
  Call RunOutputEditorSingleFile(".ESO")
End If
End Sub

'=======================================================
' Open SVG file
'=======================================================
Sub showSingleSVGFile(fileExtension As String)
Dim filelength As Long
If svgViewFileName = "" Then
  MsgBox "No SVG Diagram Viewer Selected.  Go to VIEW OPTIONS and select DIAGRAMMING to select program.", vbExclamation, "Edit Cancelled"
  Exit Sub
End If
On Error Resume Next
'check the length of all files before trying to open them
' don't really need the length but it makes for a quick test
Err.Clear
filelength = FileLen(outputFileName & LCase(fileExtension))
If Err.Number = 0 Then
    Shell svgViewFileName & " " & q & outputFileName & LCase(fileExtension) & q, vbNormalFocus
    If Err.Number <> 0 Then
        MsgBox "Error initiating SVG browser for file type" & fileExtension & ". This is likely due to the SVG browser program being deleted or moved. Please go to VIEW OPTIONS to select a new DIAGRAMMING program.", vbExclamation, "Edit Cancelled"
        Exit Sub
    End If
End If
End Sub

'=======================================================
' Open XML file
'=======================================================
Sub showSingleXMLFile(fileExtension As String)
Dim filelength As Long
If xmlViewerFileName = "" Then
  MsgBox "No XML Viewer Selected.  Go to VIEW OPTIONS and select XML to select program.", vbExclamation, "Edit Cancelled"
  Exit Sub
End If
On Error Resume Next
'check the length of all files before trying to open them
' don't really need the length but it makes for a quick test
Err.Clear
filelength = FileLen(outputFileName & LCase(fileExtension))
If Err.Number = 0 Then
    Shell xmlViewerFileName & " " & q & outputFileName & LCase(fileExtension) & q, vbNormalFocus
    If Err.Number <> 0 Then
        MsgBox "Error initiating XML viewer for file type" & fileExtension & ". This is likely due to the XML viewer program being deleted or moved. Please go to VIEW OPTIONS to select a new XML program.", vbExclamation, "Edit Cancelled"
        Exit Sub
    End If
End If
End Sub

'=======================================================
' call the text editor with the postprocessor file
'=======================================================
Sub RunPostEdit()
If textEditFileName = "" Then
  MsgBox "No Text Editor Selected.  Go to VIEW OPTIONS and select TEXT EDITOR to select program.", vbExclamation, "Run Cancelled"
  Exit Sub
End If
On Error Resume Next
Shell textEditFileName & " " & q & outputFileName & ".rvi" & q, vbNormalFocus
If Err.Number <> 0 Then
  MsgBox "Error initiating text editor 6. This is likely due to the text editor program being deleted or moved. Please go to VIEW OPTION menu to select a new TEXT EDITOR program.", vbExclamation, "Edit Cancelled"
  Exit Sub
End If
End Sub

'=======================================================
' call the text editor with the weather file
'=======================================================
Sub RunWeatherEdit()
If textEditFileName = "" Then
  MsgBox "No Text Editor Selected.  Go to VIEW OPTIONS and select TEXT EDITOR to select program.", vbExclamation, "Run Cancelled"
  Exit Sub
End If
On Error Resume Next
Shell textEditFileName & " " & weatherFileName, vbNormalFocus
If Err.Number <> 0 Then
  MsgBox "Error initiating text editor 7. This is likely due to the text editor program being deleted or moved. Please go to the VIEW OPTION menu to select a new TEXT EDITOR program.", vbExclamation, "Edit Cancelled"
  Exit Sub
End If
End Sub

'=======================================================
' call the text editor with the Group Error (errgrp) file
'=======================================================
Sub RunGrpErrEdit()
Dim grpErrorName As String
If textEditFileName = "" Then
  MsgBox "No Text Editor Selected.  Go to VIEW OPTIONS and select TEXT EDITOR to select program.", vbExclamation, "Run Cancelled"
  Exit Sub
End If
On Error Resume Next
grpErrorName = NoExtension(groupFileName) & ".errgrp"
Shell textEditFileName & " " & q & grpErrorName & q, vbNormalFocus
If Err.Number <> 0 Then
  MsgBox "Error initiating text editor 8. This is likely due to the text editor program being deleted or moved. Please go to the VIEW OPTIONS menu to select a new TEXT EDITOR program.", vbExclamation, "Edit Cancelled"
  Exit Sub
End If
End Sub

'=======================================================
' call the output spreadsheet with a single output file
'=======================================================
Sub runOutputSpreadsheetSingleFile(fileExtension As String)
Dim filelength As Long
Dim fileToOpen As String
On Error Resume Next
If spreadsheetFileName = "" Then
  MsgBox "No Spreadsheet Program Selected.  To view the spreadsheet you need a program that can display a CSV file. Go to VIEW OPTIONS and select SPREADSHEET to select program.", vbExclamation, "Run Cancelled"
  Exit Sub
End If
Debug.Print spreadsheetFileName
'if file is present then open it
Err.Clear
fileToOpen = outputFileName & fileExtension
filelength = FileLen(fileToOpen)
If Err.Number <> 0 Then Exit Sub
' now open any files that were selected
Shell spreadsheetFileName & " /e " & q & fileToOpen & q, vbNormalFocus
If Err.Number <> 0 Then
  MsgBox "Error initiating spreadsheet. This is likely due to the spreadsheet program being deleted or moved. Please go to the VIEW OPTIONS menu to select a new SPREADSHEET program.", vbExclamation, "Spreadsheet Cancelled"
  Exit Sub
End If
End Sub


'=======================================================
' call the drawing viewer with the DXF output file
'=======================================================
Sub runViewDrawing()
Dim dxfFileTime As Date
Dim inpFileTime As Date
Dim filelength As Long
On Error Resume Next
filelength = FileLen(outputFileName & ".dxf")
If Err.Number = 0 Then
  'file exists check if date is older then input file
  dxfFileTime = FileDateTime(outputFileName & ".dxf")
  inpFileTime = FileDateTime(inputFileName)
  Debug.Print "Times of idf and dxf files: "; inpFileTime, dxfFileTime
  If inpFileTime > dxfFileTime Then
    Call RunEPDrawGUI
  End If
Else
  Call RunEPDrawGUI
End If
filelength = FileLen(outputFileName & ".dxf")
If Err.Number <> 0 Then
  MsgBox "No drawing file found or is older than input file and EPDrawGUI not working.  Try rerunning simulation.", vbExclamation, "Viewing Cancelled"
Else
  If dxfViewFileName = "" Then
    MsgBox "No Drawing File Viewer Selected.  To view a drawing you need a program that can display a DXF file.  Go to VIEW OPTIONS and select DRAWING VIEWER to select program.", vbExclamation, "Viewing Cancelled"
    Exit Sub
  End If
  Debug.Print dxfViewFileName
  'now call the viewer with the output file
  Shell dxfViewFileName & " " & q & outputFileName & ".dxf" & q, vbNormalFocus
  If Err.Number <> 0 Then
    MsgBox "Error initiating drawing file viewer. This is likely due to the drawing file viewer program being deleted or moved. Please use the VIEW OPTIONS menu to select a new DRAWING VIEWER program.", vbExclamation, "View Cancelled"
    Exit Sub
  End If
End If
Call checkOutputButtonsFiles
End Sub

'=======================================================
' call the VRML drawing viewer with the VRML (wrl) output file
'=======================================================
Sub runOutputVRMLdisplay()
Dim filelength As Long
Dim fileToOpen As String
On Error Resume Next
'if file does not exist, exit routine before searching for viewer
Err.Clear
fileToOpen = outputFileName & ".wrl"
filelength = FileLen(fileToOpen)
If Err.Number <> 0 Then Exit Sub
If vrmlAppFileName = "" Then
  MsgBox "No VRML Program Selected.  To view the VRML file you need a program that can display a WRL file.  See links on http://cic.nist.gov/vrml/vbdetect.html for some viewers.  Go to VIEW OPTIONS and select VRML VIEWER to select program.", vbExclamation, "VRML Display Cancelled"
  Exit Sub
End If
Debug.Print vrmlAppFileName
' now open any files that were selected
Shell vrmlAppFileName & " " & q & fileToOpen & q, vbNormalFocus
If Err.Number <> 0 Then
  MsgBox "Error initiating VRML file viewer. This is likely due to the VRML program being deleted or moved. Please go to the VIEW OPTIONS menu to select a VRML VIEWER program.", vbExclamation, "VRML Display Cancelled"
  Exit Sub
End If
End Sub


'=======================================================
' Run the EPDrawGUI program to create a DXF file
' only called when the DXF file is older than the
' input file or when it is not present
'=======================================================
Sub RunEPDrawGUI()
Dim fileNoExt As String
Dim dxfFile As String
Dim filelength As Long
Dim fileForInput As String
Dim curDirectory As String
On Error Resume Next
dxfFile = outputFileName & ".dxf"
'check if EPDrawGUI present
Err.Clear
filelength = FileLen(appPath & "Preprocess\EPDraw\EPDrawGUI.exe")
If Err.Number <> 0 Then
  MsgBox "The EPDrawGUI program could not be found.", vbExclamation, "Drawing Cancelled"
  Exit Sub
End If
'check on idf/idm file
filelength = FileLen(inputFileName)
If Err.Number <> 0 Then
  MsgBox "Input file could not be found.", vbExclamation, "Drawing Cancelled"
  Exit Sub
End If
'if an IMF file run EP-Macro first
fileNoExt = NoExtension(inputFileName)
If UCase(Right(RTrim(inputFileName), 3)) = "IMF" Then
  Call runEPMacro
  fileForInput = fileNoExt & ".epmidf"
  filelength = FileLen(fileForInput)
  If Err.Number <> 0 Then
    MsgBox "Converted input file could not be found.", vbExclamation, "Drawing Cancelled"
    Exit Sub
  End If
Else
  fileForInput = inputFileName
End If
'store the current directory and go to the application location (also EPDrawGUI location)
curDirectory = CurDir
ChDir appPath
ChDrive appPath
'run EPDrawGUI
'MsgBox appPath & "Preprocess\EPDraw\EPDrawGUI.exe  " & q & fileForInput & q
ExecCmd appPath & "Preprocess\EPDraw\EPDrawGUI.exe " & q & fileForInput & q
'MsgBox "After EPDrawGUI is called in RunEPDrawGUI in EP-Launch"

' change back to the old current directory
ChDir curDirectory
ChDrive curDirectory
'check for errors
If Err.Number <> 0 Then
  MsgBox "Error initiating EPDrawGUI.  This is likely due to the EPDrawGUI program being deleted or moved. Please check if it is in the installed in Preprocess\EPDraw subdirectory of EnergyPlus.", vbExclamation, "Drawing Cancelled"
  Exit Sub
End If
End Sub

'=======================================================
' Run the EPMacro program to convert the IMF to IDF file
' so that it can be converted to an DXF by EPDrawGUI
' It needs an in.imf file and creates out.idf and audit.out
' which are renamed xxx.epmidf and xxx.epmdet
'=======================================================
Sub runEPMacro()
Dim fileNoExt As String
Dim filelength As Long
Dim curDirectory As String
On Error Resume Next
fileNoExt = NoExtension(inputFileName)
'check if EPMacro.exe file is present
Err.Clear
filelength = FileLen(appPath & "EPMacro.exe")
If Err.Number <> 0 Then
  MsgBox "The EPMacro program could not be found.", vbExclamation, "EPMacro Cancelled"
  Exit Sub
End If
'remove any existing in.imf file and ignore errors
Kill appPath & "in.imf"
Err.Clear
'copy the imf file to the in.imf file.
FileCopy inputFileName, appPath & "in.imf"
If Err.Number <> 0 Then
  MsgBox "Copying the imf file to the temporary in.imf file failed.", vbExclamation, "EPMacro Cancelled"
  Exit Sub
End If
'store the current directory and go to the application location (also WinEPDraw location(??))
curDirectory = CurDir
ChDir appPath
ChDrive appPath
'run EPMacro
ExecCmd appPath & "EPMacro"
' change back to the old current directory
ChDir curDirectory
ChDrive curDirectory
'check for errors
If Err.Number <> 0 Then
  MsgBox "Running EPMacro failed.", vbExclamation, "EPMacro Cancelled"
  Exit Sub
End If
'remove any existing epmXXX files and ignore errors
Kill fileNoExt & ".epmidf"
Err.Clear
Kill fileNoExt & ".epmdet"
Err.Clear
'copy audit.out and out.idf to xxx.epmXXX files
FileCopy appPath & "out.idf", fileNoExt & ".epmidf"
If Err.Number <> 0 Then
  MsgBox "Copying out.idf failed.", vbExclamation, "EPMacro Cancelled"
  Exit Sub
End If
FileCopy appPath & "audit.out", fileNoExt & ".epmdet"
If Err.Number <> 0 Then
  MsgBox "Copying audit.out failed.", vbExclamation, "EPMacro Cancelled"
  Exit Sub
End If
End Sub

'=======================================================
' call the IDF Editor with the input file
'=======================================================
Sub RunIDFEdit()
Dim olddir
'Shell idfEditorFileName & " " & inputFileName, vbNormalFocus
'Exit Sub
If UCase(Right(RTrim(inputFileName), 3)) = "IMF" Then
  MsgBox "Cannot edit .imf files with IDF-Editor"
  Exit Sub
End If
On Error Resume Next
olddir = CurDir
ChDrive Left(appPath, 1)
ChDir appPath & "preprocess\idfeditor\"
'MsgBox "current directory: " & CurDir, vbExclamation
Shell "idfeditor.exe " & inputFileName, vbNormalFocus
ChDrive Left(olddir, 1)
ChDir olddir
'Shell "c:\energyplus\preprocess\idfeditor\idfeditor.exe " & inputFileName
If Err.Number <> 0 Then
  MsgBox "Error initiating IDFEditor.  This is likely due to the IDF Editor program being deleted or moved. Please confirm that it is in the Preprocessor\IDF Editor subdirectory of the installed EnergyPlus directory." & inputFileName, vbExclamation, "Edit Cancelled"
  Exit Sub
End If
End Sub

'=======================================================
' Call a text editor and display the group file
'=======================================================
Sub RunGroupEdit()
If textEditFileName = "" Then
  MsgBox "No Text Editor Selected", vbExclamation, "Edit Cancelled"
  Exit Sub
End If
On Error Resume Next
Shell textEditFileName & " " & q & groupFileName & q, vbNormalFocus
If Err.Number <> 0 Then
  MsgBox "Error initiating text editor for group file. This is likely due to the text editor program being deleted or moved. Please use the FILE menu to select a new text editor program.", vbExclamation, "Edit Cancelled"
  Exit Sub
End If
End Sub

'============================
'============================
'============================
' Support and Debug Routines
'============================
'============================
'============================

'=======================================================
'  Starts when the form loads - main start of program
'=======================================================
Private Sub Form_Load()
Dim longCommand As String
Dim existInList As Integer
Dim arFN As String
Dim commandExtension As String
Dim i As Integer
Debug.Print "============================"
Debug.Print "NEW RUN OF EPL "; Now()
'create a varialbe that points to the current EP-Launch location with trailing slash
appPath = App.path
If Right(appPath, 1) <> "\" Then appPath = appPath & "\"
q = Chr(34) 'quote symbol
eplUI.Caption = "EP-Launch"
curOSver = getOSVersion
cmbWeather.AddItem noWeatherFile
pdfViewerFileName = findProgramUsingExtension("pdf")
Call setupUtilityTab
Call initializeOutputKindsAndSets 'from MainModule
Call GetAllSettings
Call GetIDDVersion
'default window organization
frameSingle.Left = 240
frameGroup.Left = -30000
frameHistory.Left = -30000
frameUtility.Left = -30000
'hide the check for updates lable
lblCheckingUpdates.Visible = False
'parse the command line
Debug.Print "CommandLine: ", Command
longCommand = GetLongFileName(Command)
If longCommand = "\" Then longCommand = ""
'debugging msgbox for command line issues
'MsgBox "[" & longCommand & "]", vbExclamation, "The command line within brackets"
Debug.Print "Passed command line: "; Command, longCommand
If longCommand <> "" Then
  commandExtension = extensionOnly(longCommand)
  Select Case commandExtension
    Case "idf", "imf"
      existInList = -1
      For i = 0 To cmbInput.ListCount
        If cmbInput.List(i) = longCommand Then
          existInList = i
          Exit For
        End If
      Next i
      If existInList > -1 And cmbInput.ListIndex >= 0 Then 'it was already in the list just point to it
        cmbInput.ListIndex = existInList
      Else  'add it to the beginning of the list
        cmbInput.AddItem longCommand, 0
        cmbInput.ListIndex = 0
        'if the list has gotten two long then get rid of last item
    '    If cmbInput.ListCount > maxInputListSize Then cmbInput.RemoveItem maxInputListSize + 1
        If cmbInput.ListCount > maxInputListSize Then cmbInput.RemoveItem cmbInput.ListCount - 1
      End If
      lastInputDirectory = pathOnly(longCommand)
      curTab = 1
      frameSingle.Left = 240
      frameGroup.Left = -30000
      frameHistory.Left = -30000
      frameUtility.Left = -30000
    Case "epg"
      Call addItemToGroupList(longCommand)
      curTab = 2
      tabMain.Tabs(curTab).Selected = True
      frameSingle.Left = -30000
      frameGroup.Left = 240
      frameHistory.Left = -30000
      frameUtility.Left = -30000
    Case Else
      MsgBox "Invalid file extension in file: " & longCommand, vbCritical, "Extension"
      curTab = 1
      frameSingle.Left = 240
      frameGroup.Left = -30000
      frameHistory.Left = -30000
      frameUtility.Left = -30000
  End Select
End If
Call readHistoryFile
Call getTransitionVersions
'initial setup for utility tab
Call setupUtilityTab
cmbUtility.Clear
For i = 1 To numUtilProg
  cmbUtility.AddItem utilProg(i).name
Next i
cmbUtility.ListIndex = 0
Call redrawUtilityTab
firstActivateCall = True
'establish original size for simQueue
ReDim simQueue(2)
numSimQueue = 0
'startSimulationActive = False
End Sub

'=======================================================
' This catches the control box close
'=======================================================
Private Sub Form_queryunLoad(Cancel As Integer, unloadmode As Integer)
If numSimQueue > 0 Then
  MsgBox "Cannot exit while simulations are running", vbInformation, "EP-Launch"
  Cancel = True
Else
  Call ExitEPL
End If
End Sub

'=======================================================
'  Get version of the operating system
'=======================================================
Function getOSVersion() As Integer
If GetVersion() And &H80000000 Then
  getOSVersion = 1  'Win 95/98/ME
Else
  getOSVersion = 2  'NT/2000
End If
End Function

'=======================================================
'  Routine when exiting EP Launch
'=======================================================
Sub ExitEPL()
Dim response As Integer
If numSimQueue > 0 Then
  response = MsgBox("Would you like to exit while simulations are still running", vbYesNo, "EP-Launch")
  If response = vbNo Then
    Exit Sub
  End If
End If
Call SaveAllSettings
Debug.Print "UNLOAD " & Now()
End
End Sub

'=======================================================
'  Remove extenstion from string
'=======================================================
Function NoExtension(n As String)
Dim i As Integer
i = InStrRev(n, ".")    'get last dot
If i > 1 Then
  NoExtension = Left(n, i - 1)
Else
  NoExtension = n
End If
End Function

'=======================================================
'  Return only the extension from the file given
'=======================================================
Function extensionOnly(s As String) As String
Dim i As Integer
i = InStrRev(s, ".") 'get last dot
If i > 1 Then
  extensionOnly = LCase(Mid(s, i + 1))  'make lowercase
Else
  extensionOnly = ""
End If
End Function

'=======================================================
' Returns only the path for the file and path provided
'=======================================================
Function pathOnly(pathWithFile As String) As String
Dim slpt As Integer
slpt = InStrRev(pathWithFile, "\") 'finds last slash
If slpt > 1 Then
  pathOnly = Left(pathWithFile, slpt)
Else
  pathOnly = "c:\"
End If
End Function

'=======================================================
' Returns only the path for the file and path provided
'=======================================================
Function fileWithExt(pathWithFile As String) As String
Dim slpt As Integer
slpt = InStrRev(pathWithFile, "\") 'finds last slash
If slpt > 1 Then
  fileWithExt = Mid(pathWithFile, slpt + 1)
Else
  fileWithExt = ""
End If
End Function

'============================
'============================
'============================
' routines that interact with registry
'============================
'============================
'============================

'=======================================================
' Place values in the registry to store values between runs
'=======================================================
Sub SaveAllSettings()
Dim setString As String
Dim i As Integer
Dim j As Integer
SaveSetting "EP-Launch", "Pointers", "FirstUse", "False"
SaveSetting "EP-Launch", "Pointers", "TextEditor", textEditFileName
SaveSetting "EP-Launch", "Pointers", "Spreadsheet", spreadsheetFileName
SaveSetting "EP-Launch", "Pointers", "dxfViewer", dxfViewFileName
SaveSetting "EP-Launch", "Pointers", "VRMLapp", vrmlAppFileName
SaveSetting "EP-Launch", "Pointers", "htmlViewer", htmlViewFileName
SaveSetting "EP-Launch", "Pointers", "svgViewer", svgViewFileName
SaveSetting "EP-launch", "Pointers", "esoViewer", esoViewFileName
SaveSetting "EP-launch", "Pointers", "pdfViewer", pdfViewerFileName
SaveSetting "EP-launch", "Pointers", "xmlViewer", xmlViewerFileName
' save location of program
SaveSetting "EP-Launch", "Location", "Left", Str(eplUI.Left)
SaveSetting "EP-Launch", "Location", "Top", Str(eplUI.Top)
If useWideView Then
    SaveSetting "EP-Launch", "Location", "Wide", "True"
Else
    SaveSetting "EP-Launch", "Location", "Wide", "False"
End If
If useSimAboveView Then
    SaveSetting "EP-Launch", "Location", "SimAboveView", "True"
Else
    SaveSetting "EP-Launch", "Location", "SimAboveView", "False"
End If
If pauseDuringRun Then
    SaveSetting "EP-Launch", "Location", "Pause", "True"
Else
    SaveSetting "EP-Launch", "Location", "Pause", "False"
End If
If minimizeGroupCmd Then
    SaveSetting "EP-Launch", "Location", "MinGroupCmd", "True"
Else
    SaveSetting "EP-Launch", "Location", "MinGroupCmd", "False"
End If
If minimizeSingleCmd Then
    SaveSetting "EP-Launch", "Location", "MinSingleCmd", "True"
Else
    SaveSetting "EP-Launch", "Location", "MinSingleCmd", "False"
End If
If tabWithSpreadsheet Then
    SaveSetting "EP-Launch", "Location", "TabWithSpreadsheet", "True"
Else
    SaveSetting "EP-Launch", "Location", "TabWithSpreadsheet", "False"
End If
If testViewConvertOld Then
   SaveSetting "EP-Launch", "Location", "TestViewConvert", "True"
Else
   SaveSetting "EP-Launch", "Location", "TestViewConvert", "False"
End If
If allowGT250Col Then
    SaveSetting "EP-Launch", "Location", "AllowMore250Col", "True"
Else
    SaveSetting "EP-Launch", "Location", "AllowMore250Col", "False"
End If
If convertESOMTRIP Then
    SaveSetting "EP-Launch", "Location", "convertESOMTRIntoIP", "True"
Else
    SaveSetting "EP-Launch", "Location", "convertESOMTRIntoIP", "False"
End If
If createCSVprocFile Then
    SaveSetting "EP-Launch", "Location", "CSVproc", "True"
Else
    SaveSetting "EP-Launch", "Location", "CSVproc", "False"
End If
If CreateRunEPBatch Then
    SaveSetting "EP-Launch", "Location", "CreateRunEPbat", "True"
Else
    SaveSetting "EP-Launch", "Location", "CreateRunEPbat", "False"
End If
If enableParametricPreprocessor Then
    SaveSetting "EP-Launch", "Location", "UseParametricPreprocessor", "True"
Else
    SaveSetting "EP-Launch", "Location", "UseParametricPreprocessor", "False"
End If
SaveSetting "EP-Launch", "Location", "MaxNumProcesses", Str(numberOfSimProcessesAllowed)
If viewAllOutputTabSelected Then
    SaveSetting "EP-Launch", "Location", "ViewAllOutputTabSelected", "True"
Else
    SaveSetting "EP-Launch", "Location", "ViewAllOutputTabSelected", "False"
End If



' save recently used input file list
SaveSetting "EP-Launch", "RecentInputs", "InputIndex", Str(selectedInputIndex)
SaveSetting "EP-Launch", "RecentInputs", "LastDir", lastInputDirectory
For i = 0 To cmbInput.ListCount
  SaveSetting "EP-Launch", "RecentInputs", Chr(i + 65), cmbInput.List(i)
Next i
For i = cmbInput.ListCount To maxInputListSize - 1 'put empties
  SaveSetting "EP-Launch", "RecentInputs", Chr(i + 65), "-"
Next i

' save weather info
SaveSetting "EP-Launch", "RecentWeather", "WeatherIndex", Str(selectedWeatherIndex)
SaveSetting "EP-Launch", "RecentWeather", "LastDir", lastWeatherDirectory
For i = 1 To cmbWeather.ListCount
  SaveSetting "EP-Launch", "RecentWeather", Chr(i + 65), cmbWeather.List(i)
Next i
For i = cmbWeather.ListCount To maxWeatherListSize - 1 'put empties
  SaveSetting "EP-Launch", "RecentWeather", Chr(i + 65), "-"
Next i

' save group info
SaveSetting "EP-Launch", "RecentGroup", "GroupIndex", Str(selectedGroupIndex)
SaveSetting "EP-Launch", "RecentGroup", "LastDir", lastGroupDirectory
SaveSetting "EP-Launch", "RecentGroup", "GrpInputDir", lastGrpInputDirectory
SaveSetting "EP-Launch", "RecentGroup", "GrpWeatherDir", lastGrpWeatherDirectory
For i = 0 To cmbGroup.ListCount
  SaveSetting "EP-Launch", "RecentGroup", Chr(i + 65), cmbGroup.List(i)
Next i
For i = cmbGroup.ListCount To maxGroupListSize - 1 'put empties
  SaveSetting "EP-Launch", "RecentGroup", Chr(i + 65), "-"
Next i
'save util tab input and weather files
For i = 1 To numUtilProg
  If utilProg(i).enableInput Then
    SaveSetting "EP-Launch", "RecentUtilIn", Trim(utilProg(i).name), utilProg(i).curInputFile
  End If
  If utilProg(i).enableWthr Then
    SaveSetting "EP-Launch", "RecentUtilWthr", Trim(utilProg(i).name), utilProg(i).curWeatherFile
  End If
Next i
'save update checking status
SaveSetting "EP-Launch", "UpdateCheck", "LastAnchor", updateLastAnchor
SaveSetting "EP-Launch", "UpdateCheck", "LastDate", updateLastDate
SaveSetting "EP-Launch", "UpdateCheck", "CheckURL", updatePageURL
If updateAutoCheck Then
  SaveSetting "EP-Launch", "UpdateCheck", "AutoCheck", "True"
Else
  SaveSetting "EP-Launch", "UpdateCheck", "AutoCheck", "False"
End If
'save user defined sets of output files to open
For i = 1 To numOutputSets - 4 'don't include the four predefined
  setString = ""
  For j = 1 To numOutputKinds
    If outputKind(j).outSet(i) Then
      setString = setString & outputKind(j).suffix & " "
    End If
  Next j
  SaveSetting "EP-Launch", "ViewResultsSet", "Set-" & Trim(Str(i)), setString
Next i
End Sub

'=======================================================
' Retrieve values from the registry from previous runs and set default values
'=======================================================
Sub GetAllSettings()
Dim listItemPtr As Integer
Dim i As Integer
Dim j As Integer
Dim k As Integer
Dim mainTopEst As Long
Dim mainLeftEst As Long
Dim firstUse As String, t As String, X As Double
Dim setString As String
Dim parts() As String
Dim wthrCount As Integer
firstUse = GetSetting("EP-Launch", "Pointers", "FirstUse", "True")
Debug.Print "FirstUse?", firstUse
If firstUse = "True" Then
  Debug.Print "CurrentPath: "; appPath
  If appPath = "D:\projects\EnergyPlus-EPLaunch\release 1.05" Then appPath = "c:\EnergyPlus" 'for development only
  textEditFileName = findProgramUsingExtension("txt")
  spreadsheetFileName = findProgramUsingExtension("xls")
  dxfViewFileName = findProgramUsingExtension("dxf")
  htmlViewFileName = findProgramUsingExtension("html")
  svgViewFileName = findProgramUsingExtension("svg")
  vrmlAppFileName = findProgramUsingExtension("wrl")
  esoViewFileName = findProgramUsingExtension("eso")
  pdfViewerFileName = findProgramUsingExtension("pdf")
  xmlViewerFileName = findProgramUsingExtension("xml")
  Call SaveAllSettings
  On Error Resume Next
  ChDir appPath & "ExampleFiles"
  t = dir("*.idf")
  Do While t <> "" And cmbInput.ListCount <= maxInputListSize
    cmbInput.AddItem appPath & "ExampleFiles\" & t
    t = dir
  Loop
  If cmbInput.ListCount > 0 Then cmbInput.ListIndex = 0
  wthrCount = 0
  ChDir appPath & "WeatherData"
  t = dir("*.epw")
  Do While t <> ""
    cmbWeather.AddItem appPath & "WeatherData\" & t
    wthrCount = wthrCount + 1
    t = dir
    If wthrCount >= 25 Then Exit Do
  Loop
'  t = dir("*.wea")
'  Do While t <> ""
'    cmbWeather.AddItem appPath & "WeatherData\" & t
'    t = dir
'  Loop
  useWideView = False
  useSimAboveView = False
  tabWithSpreadsheet = False
  testViewConvertOld = True
  mainTopEst = (Screen.Height - 6495) / 2
  mainLeftEst = (Screen.Width - 8790) / 2
  Call resetSimAboveView
  selectedInputIndex = 0
  If cmbWeather.ListCount > 1 Then
    cmbWeather.ListIndex = 1
    selectedWeatherIndex = 1
  Else
    selectedWeatherIndex = 0
    cmbWeather.ListIndex = 0
  End If
  lastWeatherDirectory = appPath
  lastInputDirectory = appPath
  pauseDuringRun = False
  minimizeGroupCmd = False
  minimizeSingleCmd = False
  allowGT250Col = False
  convertESOMTRIP = False
  createCSVprocFile = False
  CreateRunEPBatch = False
  enableParametricPreprocessor = True
  numberOfSimProcessesAllowed = 2
  viewAllOutputTabSelected = False
  'update checking
  updateLastAnchor = ""
  updateLastDate = "1/1/2008"
  updatePageURL = "http://energyplus.net/epupdate.htm"
  updateAutoCheck = True
  Call SaveAllSettings
Else
  textEditFileName = GetSetting("EP-Launch", "Pointers", "TextEditor")
  If textEditFileName = "" Then textEditFileName = findProgramUsingExtension("txt")
  spreadsheetFileName = GetSetting("EP-Launch", "Pointers", "Spreadsheet")
  If spreadsheetFileName = "" Then spreadsheetFileName = findProgramUsingExtension("xls")
  dxfViewFileName = GetSetting("EP-Launch", "Pointers", "dxfViewer")
  If dxfViewFileName = "" Then dxfViewFileName = findProgramUsingExtension("dxf")
  vrmlAppFileName = GetSetting("EP-Launch", "Pointers", "VRMLapp")
  If vrmlAppFileName = "" Then vrmlAppFileName = findProgramUsingExtension("wrl")
  htmlViewFileName = GetSetting("EP-Launch", "Pointers", "htmlViewer")
  If htmlViewFileName = "" Then htmlViewFileName = findProgramUsingExtension("html")
  svgViewFileName = GetSetting("EP-Launch", "Pointers", "svgViewer")
  If svgViewFileName = "" Then svgViewFileName = findProgramUsingExtension("svg")
  esoViewFileName = GetSetting("EP-Launch", "Pointers", "esoViewer")
  If esoViewFileName = "" Then esoViewFileName = findProgramUsingExtension("eso")
  pdfViewerFileName = GetSetting("EP-Launch", "Pointers", "pdfViewer")
  If pdfViewerFileName = "" Then pdfViewerFileName = findProgramUsingExtension("pdf")
  xmlViewerFileName = GetSetting("EP-Launch", "Pointers", "xmlViewer")
  If xmlViewerFileName = "" Then xmlViewerFileName = findProgramUsingExtension("xml")
  For i = 0 To maxInputListSize
    t = GetSetting("EP-Launch", "RecentInputs", Chr(i + 65))
    If t <> "-" And t <> "" Then cmbInput.AddItem t
  Next i
  If cmbInput.ListCount > 0 Then cmbInput.ListIndex = 0
  For i = 0 To maxWeatherListSize
    t = GetSetting("EP-Launch", "RecentWeather", Chr(i + 65))
    If t <> "-" And t <> "" Then cmbWeather.AddItem t
  Next i
  If cmbWeather.ListCount > 0 Then cmbWeather.ListIndex = 0
  For i = 0 To maxGroupListSize
    t = GetSetting("EP-Launch", "RecentGroup", Chr(i + 65))
    If t <> "-" And t <> "" Then cmbGroup.AddItem t
  Next i
  If cmbGroup.ListCount > 0 Then cmbGroup.ListIndex = 0
  'If cmbWeather.ListCount > 1 Then cmbWeather.ListIndex = 1  to fix 6061
  'LEFT
  X = Val(GetSetting("EP-Launch", "Location", "Left"))
  If X <= 0 Or X > (Screen.Width - 1000) Then
    X = 1000
  End If
  eplUI.Left = X
  If eplUI.Left = 0 Then eplUI.Left = 1000
  'TOP
  X = Val(GetSetting("EP-Launch", "Location", "Top"))
  If X <= 0 Or X > (Screen.Height - 1000) Then
    X = 1000
  End If
  eplUI.Top = X
  If eplUI.Top = 0 Then eplUI.Top = 1000
  'WIDE
  If Left(GetSetting("EP-Launch", "Location", "Wide"), 1) = "T" Then
    useWideView = True
  Else
    useWideView = False
  End If
  'WIDE
  If Left(GetSetting("EP-Launch", "Location", "Wide"), 1) = "T" Then
    useWideView = True
  Else
    useWideView = False
  End If
  Call resetWideView
  If Left(GetSetting("EP-Launch", "Location", "TabWithSpreadsheet"), 1) = "T" Then
    tabWithSpreadsheet = True
  Else
    tabWithSpreadsheet = False
  End If
  If Left(GetSetting("EP-Launch", "Location", "SimAboveView"), 1) = "T" Then
    useSimAboveView = True
  Else
    useSimAboveView = False
  End If
  Call resetSimAboveView
  If Left(GetSetting("EP-Launch", "Location", "Pause"), 1) = "T" Then
    pauseDuringRun = True
  Else
    pauseDuringRun = False
  End If
  If Left(GetSetting("EP-Launch", "Location", "MinGroupCmd"), 1) = "T" Then
    minimizeGroupCmd = True
  Else
    minimizeGroupCmd = False
  End If
  If Left(GetSetting("EP-Launch", "Location", "MinSingleCmd"), 1) = "T" Then
    minimizeSingleCmd = True
  Else
    minimizeSingleCmd = False
  End If
  'Warn if old version
  If Left(GetSetting("EP-Launch", "Location", "TestViewConvert"), 1) = "F" Then
    testViewConvertOld = False
  Else
    testViewConvertOld = True
  End If
  If Left(GetSetting("EP-Launch", "Location", "AllowMore250Col"), 1) = "F" Then
    allowGT250Col = False
  Else
    allowGT250Col = True
  End If
  If Left(GetSetting("EP-Launch", "Location", "convertESOMTRIntoIP"), 1) = "T" Then
    convertESOMTRIP = True
  Else
    convertESOMTRIP = False
  End If
  If Left(GetSetting("EP-Launch", "Location", "CSVproc"), 1) = "T" Then
    createCSVprocFile = True
  Else
    createCSVprocFile = False
  End If
  If Left(GetSetting("EP-Launch", "Location", "CreateRunEPbat"), 1) = "T" Then
    CreateRunEPBatch = True
  Else
    CreateRunEPBatch = False
  End If
  If Left(GetSetting("EP-Launch", "Location", "UseParametricPreprocessor"), 1) = "F" Then
    enableParametricPreprocessor = False
  Else
    enableParametricPreprocessor = True
  End If
  'Number of simultaneous threads for simulation processes
  numberOfSimProcessesAllowed = Val(GetSetting("EP-Launch", "Location", "MaxNumProcesses"))
  If numberOfSimProcessesAllowed <= 1 Or numberOfSimProcessesAllowed > 1024 Then
    numberOfSimProcessesAllowed = 2
  End If
  If Left(GetSetting("EP-Launch", "Location", "ViewAllOutputTabSelected"), 1) = "T" Then
    viewAllOutputTabSelected = True
    Set tabViewResults.SelectedItem = tabViewResults.Tabs(2)
  Else
    viewAllOutputTabSelected = False
  End If
  If cmbInput.ListCount > 1 Then
    listItemPtr = Val(GetSetting("EP-Launch", "RecentInputs", "InputIndex"))
    If listItemPtr > cmbInput.ListCount - 1 Then
      listItemPtr = 1
    End If
    cmbInput.ListIndex = listItemPtr
  End If
  If cmbWeather.ListCount > 1 Then
    listItemPtr = Val(GetSetting("EP-Launch", "RecentWeather", "WeatherIndex"))
    If listItemPtr > cmbWeather.ListCount - 1 Then
      listItemPtr = 1
    End If
    cmbWeather.ListIndex = listItemPtr
  End If
  If cmbGroup.ListCount > 1 Then
    listItemPtr = Val(GetSetting("EP-Launch", "RecentGroup", "GroupIndex"))
    If listItemPtr > cmbGroup.ListCount - 1 Then
      listItemPtr = 1
    End If
    cmbGroup.ListIndex = listItemPtr
  End If
  lastInputDirectory = GetSetting("EP-Launch", "RecentInputs", "LastDir")
  lastWeatherDirectory = GetSetting("EP-Launch", "RecentWeather", "LastDir")
  lastGroupDirectory = GetSetting("EP-Launch", "RecentGroup", "LastDir")
  lastGrpInputDirectory = GetSetting("EP-Launch", "RecentGroup", "GrpInputDir")
  lastGrpWeatherDirectory = GetSetting("EP-Launch", "RecentGroup", "GrpWeatherDir")
  'get util tab files
  For i = 1 To numUtilProg
    utilProg(i).curInputFile = GetSetting("EP-Launch", "RecentUtilIn", utilProg(i).name)
    utilProg(i).curWeatherFile = GetSetting("EP-Launch", "RecentUtilWthr", utilProg(i).name)
  Next i
  'update checking
  updateLastAnchor = GetSetting("EP-Launch", "UpdateCheck", "LastAnchor")
  updateLastDate = GetSetting("EP-Launch", "UpdateCheck", "LastDate")
  If updateLastDate = "" Then updateLastDate = "1/1/2008"
  updatePageURL = GetSetting("EP-Launch", "UpdateCheck", "CheckURL")
  'If updatePageURL = "" Then updatePageURL = "http://gard.com/ep/epupdate.htm"
  If updatePageURL = "" Then updatePageURL = "http://energyplus.net/epupdate.htm"
  If Left(GetSetting("EP-Launch", "UpdateCheck", "AutoCheck"), 1) = "F" Then
    updateAutoCheck = False
  Else
    updateAutoCheck = True
  End If
  'user defined sets of output files to open
  For i = 1 To numOutputSets - 4 'don't include the four predefined
    setString = GetSetting("EP-Launch", "ViewResultsSet", "Set-" & Trim(Str(i)))
    parts = Split(setString, " ")
    For j = 0 To UBound(parts)
      parts(j) = UCase(Trim(parts(j)))
      For k = 1 To numOutputKinds
        If (parts(j) = UCase(outputKind(k).suffix)) Then
          outputKind(k).outSet(i) = True
          Exit For
        End If
      Next k
    Next j
  Next i
End If
End Sub

'=======================================================
' View either wide or normal screen
'=======================================================
Sub resetWideView()
If useWideView Then
    eplUI.Width = 14820
    cmbInput.Width = 14110
    cmbWeather.Width = 14110
    cmbGroup.Width = 14110
    treeHistory.Width = 14205
    frmInputFile.Width = 14310
    frmWeatherFile.Width = 14310
    frmGroupFile.Width = 14325
    frameSingle.Width = 14325
    frameGroup.Width = 14325
    frameHistory.Width = 14325
    frameUtility.Width = 14325
    tabMain.Width = 14565
    frameUtilityAbout.Width = 14105
    frameUtilityInput.Width = 14105
    frameUtilityWeather.Width = 14105
    lblUtilityAbout.Width = 13950
    lblUtilityInputFile.Width = 13950
    lblUtilityWeatherFile.Width = 13950
Else
    eplUI.Width = 8790
'    eplUI.Width = 11640
    cmbInput.Width = 8080
    cmbWeather.Width = 8080
    cmbGroup.Width = 8080
    treeHistory.Width = 8175
    frmInputFile.Width = 8280
    frmWeatherFile.Width = 8280
    frmGroupFile.Width = 8295
    frameSingle.Width = 8295
    frameGroup.Width = 8295
    frameHistory.Width = 8295
    frameUtility.Width = 8295
    tabMain.Width = 8535
    frameUtilityAbout.Width = 8175
    frameUtilityInput.Width = 8175
    frameUtilityWeather.Width = 8175
    lblUtilityAbout.Width = 7935
    lblUtilityInputFile.Width = 7935
    lblUtilityWeatherFile.Width = 7935
End If
End Sub

'=======================================================
' Move the simulate button above the view section
'=======================================================
Sub resetSimAboveView()
If useSimAboveView Then
  frmInputFile.BorderStyle = 0
  frmWeatherFile.BorderStyle = 0
  frameView.BorderStyle = 0
  frameView.Left = 2200
  cmdSimulate.Top = 3480
  cmdSimulate.Left = 240
  lblInputFile.Visible = True
  lblWeatherFile.Visible = True
  lblViewResults.Visible = True
  cmdInputBrowse.Top = 720
  cmdInputEdit.Top = 720
  cmdIDFEdit.Top = 720
  cmdWeatherBrowse.Top = 720
Else
  frmInputFile.BorderStyle = 1
  frmWeatherFile.BorderStyle = 1
  frameView.BorderStyle = 1
  frameView.Left = 0
  cmdSimulate.Top = 5880
  cmdSimulate.Left = 6600
  lblInputFile.Visible = False
  lblWeatherFile.Visible = False
  lblViewResults.Visible = False
  cmdInputBrowse.Top = 840
  cmdInputEdit.Top = 840
  cmdIDFEdit.Top = 840
  cmdWeatherBrowse.Top = 840
End If
End Sub

'=======================================================
' From Karl Moore via VB-World Sample Code
'=======================================================
Public Function GetLongFileName(ByVal ShortFileName As String) As String
    Dim intPos As Integer
    Dim strLongFileName As String
    Dim strDirName As String
    If checkIfFileExists(ShortFileName) Then
        'Format the filename for later processing
        ShortFileName = ShortFileName & "\"
        'Grab the position of the first real slash
        intPos = InStr(4, ShortFileName, "\")
        'Loop round all the directories and files
        'in ShortFileName, grabbing the full names
        'of everything within it.
        While intPos
            strDirName = dir(Left(ShortFileName, intPos - 1), _
                vbNormal + vbHidden + vbSystem + vbDirectory)
            If strDirName = "" Then
                GetLongFileName = ""
                Exit Function
            End If
            strLongFileName = strLongFileName & "\" & strDirName
            intPos = InStr(intPos + 1, ShortFileName, "\")
        Wend
        'Return the completed long file name
        GetLongFileName = Left(ShortFileName, 2) & strLongFileName
    Else
        GetLongFileName = ShortFileName
    End If
End Function

'=======================================================
' Returns the path to a program that handles the extension
' provided. The extension should not have a period but should
' be the simple three (or more) letter extension such as:
'    txt
'=======================================================
Public Function findProgramUsingExtension(extensionIn As String) As String
Dim searchFileName As String
Dim fn As Integer
Dim testFile As String
Dim progFound As String
Dim tempDir As String
On Error Resume Next
fn = FreeFile
tempDir = Environ("temp")
If tempDir = "" Then tempDir = Environ("tmp")
If tempDir = "" Then tempDir = "c:"
If Right(tempDir, 1) <> "\" Then tempDir = tempDir & "\"
testFile = tempDir & "test." & Trim(extensionIn)
Open testFile For Output As fn
Print #fn, "This is a test file from the EP-Launch Program which as distributed with EnergyPlus and should have been deleted.  You can delete this at any time."
Close fn
'MsgBox "Find Program using extension. Check for file:" & vbCrLf & vbCrLf & testFile
searchFileName = String(250, 0)
Call FindExecutable(testFile, vbNullString, searchFileName)
progFound = Left(searchFileName, InStr(searchFileName, vbNullChar) - 1) 'clean up
'if text file and no other program found use notepad
If extensionIn = "TXT" Then
  If progFound = "" Then progFound = "NOTEPAD.EXE"
End If
Debug.Print "Program found for ["; extensionIn; "] is ["; progFound; "]"
Kill testFile
findProgramUsingExtension = progFound
End Function


'=======================================================
' Retrieve and display the IDD version
'=======================================================
Sub GetIDDVersion()
Dim fl As Integer, verLine As String
On Error GoTo ErrorSkip
fl = FreeFile
Open appPath & "energy+.idd" For Input As fl
Line Input #fl, verLine
' 12345678901234567890
' !IDD_Version 22.1.0
If Left(verLine, 12) = "!IDD_Version" Then
  lblIDDVersion.Caption = "EnergyPlus " & Mid(verLine, 14)
  iddVersion = Mid(verLine, 14)
Else
  lblIDDVersion.Caption = ""
  iddVersion = ""
End If
EnergyPlusVer = lblIDDVersion.Caption
ErrorSkip:
Close fl
End Sub

'=======================================================
' Returns flag to true if the file name
' or path includes an international character
' (above ASCII 127)
'
' Note:
' On some systems 8.3 files names are not available
' on specific drive letters. To fix this, use the following
' command in a CMD session with ADMINSTRATOR rights:
'
'   fsutil 8dot3name set c: 0
'
' Where c: is the drive letter. This will only create 8.3 filenames
' for files and directories after this. Existing ones will not be created
' automatically. To test if 8.3 file names are availabe use dir/x
'=======================================================
Function checkIfInternationalFileName(fileNameWithPath As String) As Boolean
Dim i As Integer
Dim isNonUS As Boolean
isNonUS = False
For i = 1 To Len(fileNameWithPath)
  'if a character that is above 127 is used (i.e. non-US ascii character) then
  'use input file name that is short file name
  If AscW(Mid(fileNameWithPath, i, 1)) > 127 Then
    isNonUS = True
  End If
Next i
checkIfInternationalFileName = isNonUS
End Function

'=======================================================
' Check if the specified file is open and locked by
' another application and if it is display a warning
' message.
'=======================================================
Function checkIfFileIsLocked(nameOfFile As String) As Boolean
Dim fileOpen As Boolean
Dim filelength As Long
Dim fl As Integer
On Error Resume Next
fileOpen = False
Err.Clear
filelength = FileLen(nameOfFile)
If Err.Number = 0 Then  'if no error then see if it is locked.
  fl = FreeFile
  ' Open nameOfFile For Output As fl
  Open nameOfFile For Append As fl
  If Err.Number = 55 Or Err.Number = 70 Then
    MsgBox "The following file must be closed before running EnergyPlus again: " & vbCrLf & vbCrLf & nameOfFile, vbExclamation, "Run Cancelled"
    fileOpen = True
  ElseIf Err.Number > 0 Then
    Debug.Print "check file is locked", nameOfFile, Err.Number
  End If
  Close fl
End If
checkIfFileIsLocked = fileOpen
End Function

'=======================================================
' Display dialog to create a new queued set of runs
'=======================================================
Sub runCreateNewRunQueue()
Dim groupFile As String
groupFile = ""
frmNewQueue.Show vbModal
groupFile = frmNewQueue.queueFileName
Debug.Print "groupFile: ", groupFile
If groupFile <> "" Then
  Call addItemToGroupList(groupFile)
End If
End Sub

'=======================================================
' Display a run in the history window based on
' a line of the history file
'=======================================================
Sub displayNewHistoryLine(lineOfHistory As String)
Dim parts() As String
Dim nd As Node
Dim itemRoot As String
Dim i As Integer
parts = Split(lineOfHistory, ",")
itemRoot = parts(0) & "  " & fileWithExt(parts(2)) & ", " & fileWithExt(parts(3)) & ", " & fileWithExt(parts(4))
'if start of tree
If treeHistory.Nodes.Count = 0 Then
  Set nd = treeHistory.Nodes.Add(, , "Root", itemRoot)
  For i = 1 To UBound(parts)
    If parts(i) <> "" Then Set nd = treeHistory.Nodes.Add("Root", tvwChild, , parts(i))
  Next i
Else
  Set nd = treeHistory.Nodes.Add("Root", tvwLast, itemRoot, itemRoot)
  For i = 1 To UBound(parts)
    If parts(i) <> "" Then Set nd = treeHistory.Nodes.Add(itemRoot, tvwChild, , parts(i))
  Next i
End If
End Sub

'=======================================================
' if the history file exists, read and display it.
'=======================================================
Sub readHistoryFile()
Dim histLine As String
Dim fl As Integer
On Error Resume Next
fl = FreeFile
Open appPath & "history.csv" For Input As fl
If Err.Number = 0 Then
  Do While Not EOF(fl)
    Line Input #fl, histLine
    Call displayNewHistoryLine(histLine)
  Loop
  Close fl
End If
End Sub

'=======================================================
' Run the group of file defined in the simulation
'=======================================================
Sub RunSimulationGroup()
Dim parts() As String
Dim lineOfGroup As String
Dim flNum As Long
Dim numTotalRuns As Integer
Dim currentRunCounter As Integer
'Dim oldWinTitle As String
Dim delay As Single
Dim grpErrorName As String
Dim appFileName As String
Dim currentDirectory As String
Dim curIDF As String
Dim invalidCharactersFound As Boolean
Dim wthrNameOrFlag As String
Dim i As Integer
On Error Resume Next
flNum = FreeFile
delay = 0.05
numTotalRuns = 0
currentRunCounter = 0
invalidCharactersFound = False
'oldWinTitle = eplUI.Caption
grpErrorName = NoExtension(groupFileName) & ".errgrp"
Kill grpErrorName
Err.Clear
Open groupFileName For Input As flNum
If Err.Number = 0 Then
  'first pass is to count the number of runs
  Do While Not EOF(flNum)
    Line Input #flNum, lineOfGroup
    If Left(LTrim(lineOfGroup), 1) <> "!" Then
      numTotalRuns = numTotalRuns + 1
    End If
  Loop
  Close flNum
  If numTotalRuns = 0 Then
    MsgBox "No runs are enabled in the group run file", vbExclamation, "Run Group Simulation"
  End If
  'second pass is to actually perform the runs
  Open groupFileName For Input As flNum
  Do While Not EOF(flNum)
    Line Input #flNum, lineOfGroup
    If Left(LTrim(lineOfGroup), 1) <> "!" Then
      lineOfGroup = Replace(lineOfGroup, vbTab, " ") 'get rid of tabs for CR7195
      parts = Split(lineOfGroup, ",")
      For i = 0 To UBound(parts)
        parts(i) = Trim(parts(i))
        If isContainsInvalids(parts(i)) Then
          invalidCharactersFound = True
        End If
        If InStr(parts(i), "%") Then invalidCharactersFound = True
      Next i
      currentRunCounter = currentRunCounter + 1
      Call Pause(delay)
      'eplUI.Caption = Trim(Val(currentRunCounter)) & " of " & Trim(Val(numTotalRuns)) & "   " & oldWinTitle
      'eplUI.Refresh
      DoEvents
      Call Pause(delay)
      'if the weather file name is blank treat as "no weather file" as per CR7591
      If Len(Trim(parts(1))) > 0 Then
        wthrNameOrFlag = parts(1)
      Else
        wthrNameOrFlag = noWeatherFile
      End If
      If enableParametricPreprocessor Then
        If Not doesIDFcontainParametrics(parts(0)) Then
          'no parametric objects found so run the file
          Call addToSimulationQueue(parts(0), wthrNameOrFlag, parts(2), groupFileName, CInt(parts(3)), False)
        Else 'found parametric objects so run preprocessor
          Call RunParametricInput(parts(0), wthrNameOrFlag, parts(2), groupFileName)
        End If
      Else 'don't check for parametric object and just run the file
        Call addToSimulationQueue(parts(0), wthrNameOrFlag, parts(2), groupFileName, CInt(parts(3)), False)
      End If
    End If
  Loop
  Close flNum
End If
Call Pause(delay)
'eplUI.Caption = oldWinTitle
'eplUI.Refresh
DoEvents
Call Pause(delay)
If invalidCharactersFound Then
  Call MsgBox("One or more files or file paths contain invalid characters. Remove the invalid characters and rerun. Invalid characters include &^,=;%", vbExclamation, "Error")
End If
End Sub

'=======================================================
' Run a suite of simulations based on parametric objects
' present in file. Run ParametricPreprocessor as first
' step.
'=======================================================
Sub RunParametricInput(InIDFfile As String, InWthrFile As String, Optional outName As String, Optional grpName As String)
Dim appFileName As String
Dim curIDF As String
Dim curIDFwPath As String
Dim numTotalRuns As Integer
Dim currentRunCounter As Integer
Dim currentDirectory As String
Dim recDateTime As Date
Dim curDateTime As Date
'Dim oldWinTitle As String
Dim delay As Single
'Note that outName and grpName are optional and are only supplied when called from a group run

recDateTime = Now() 'get the current date and timestamp
recDateTime = DateAdd("n", -1, recDateTime) 'subtract one minute just to provide cushion
'oldWinTitle = eplUI.Caption
delay = 0.05
On Error Resume Next
'MsgBox "InIDF: " & InIDFfile & vbCrLf & "InWthr: " & InWthrFile & vbCrLf & "OutName: " & outName & vbCrLf & "GrpName: " & grpName, vbInformation, "Parametric arguments"

Err.Clear 'don't worry if files not found
'run parametric preprocessor
appFileName = appPath & "PreProcess\ParametricPreprocessor\ParametricPreprocessor.exe"
If Not checkIfFileExists(appFileName) Then
  MsgBox "Application not present: " & vbCrLf & vbCrLf & appFileName, vbExclamation, "Parametric Simulation Run Cancelled"
  Exit Sub
End If
If isContainsInvalids(InIDFfile) Then
   MsgBox "Input file name contains characters: &^,=%" & Chr(34), vbExclamation, "Parametric Simulation Run Cancelled"
   Exit Sub
End If
'save current directory
currentDirectory = CurDir
ChDrive Left(appFileName, 1)
ChDir pathOnly(appFileName)
'Shell appFileName & " " & q & InIDFfile & q, vbNormalFocus
ExecCmd appFileName & " " & q & InIDFfile & q
If Err.Number <> 0 Then
   MsgBox "Error initiating ParametricPreprocessor using " & appFileName, vbExclamation, "Parametric Simulation Run Cancelled"
   Exit Sub
Else
   Kill pathOnly(InIDFfile) & "parametric.int"
End If
Err.Clear
ChDrive Left(currentDirectory, 1)
ChDir pathOnly(currentDirectory)
Call Pause(1)
'count how many files are part of parametric group
numTotalRuns = 0
curIDF = dir(NoExtension(InIDFfile) & "-*.idf")
Do While curIDF <> ""
  numTotalRuns = numTotalRuns + 1
  curIDF = dir
Loop
If numTotalRuns = 0 Then
  MsgBox "Either errors were found or no runs are enabled in the parametric input file", vbExclamation, "Run Parametric Simulation"
End If
'run each simulation
currentRunCounter = 0
curIDF = dir(NoExtension(InIDFfile) & "-*.idf")
curIDFwPath = pathOnly(InIDFfile) & curIDF
Do While curIDF <> ""
  curDateTime = FileDateTime(curIDFwPath)
  If curDateTime > recDateTime Then 'only include files that were recently created
    currentRunCounter = currentRunCounter + 1
    Call Pause(delay)
    'eplUI.Caption = Trim(Val(currentRunCounter)) & " of " & Trim(Val(numTotalRuns)) & "   " & oldWinTitle
    'eplUI.Refresh
    DoEvents
    Call Pause(delay)
    If IsMissing(grpName) Or IsMissing(outName) Or outName = "" Or grpName = "" Then
      Call addToSimulationQueue(curIDFwPath, InWthrFile, NoExtension(curIDFwPath), "", currentRunCounter, False)
    Else
      Call addToSimulationQueue(curIDFwPath, InWthrFile, pathOnly(outName) & NoExtension(curIDF), grpName, currentRunCounter, False)
    End If
  End If
  curIDF = dir
  curIDFwPath = pathOnly(InIDFfile) & curIDF 'add back the full path to the file
Loop
Call Pause(delay)
'eplUI.Caption = oldWinTitle
'eplUI.Refresh
DoEvents
Call Pause(delay)
ChDir currentDirectory
Call checkOutputButtonsFiles
End Sub


'=======================================================
' Returns TRUE if the file specified includes one of the
' PARAMETRIC objects.
'=======================================================
Function doesIDFcontainParametrics(inFile As String) As Boolean
Dim lineIn As String
Dim flv As Integer
Dim found As Boolean
Dim startTime As Single

doesIDFcontainParametrics = False
startTime = Timer
flv = FreeFile
On Error Resume Next
Open inFile For Input As flv
If Err.Number <> 0 Then Exit Function
found = False
Do While Not EOF(flv)
  Line Input #flv, lineIn
  lineIn = Replace(lineIn, vbTab, " ") 'get rid of tabs and replace with spaces
  lineIn = LTrim(lineIn)               'remove all leading spaces
  If Left(lineIn, 1) <> "!" Then
    If UCase(Left(lineIn, 11)) = "PARAMETRIC:" Then
      lineIn = UCase(lineIn)
      If InStr(lineIn, "PARAMETRIC:SETVALUEFORRUN") > 0 Then found = True
      If InStr(lineIn, "PARAMETRIC:LOGIC") > 0 Then found = True
      If InStr(lineIn, "PARAMETRIC:RUNCONTROL") > 0 Then found = True
      If InStr(lineIn, "PARAMETRIC:FILENAMESUFFIX") > 0 Then found = True
      If found Then
        doesIDFcontainParametrics = True
        Exit Do
      End If
    End If
  End If
Loop
Close flv
Debug.Print "Time to check for parametric objects: "; Timer - startTime, doesIDFcontainParametrics, lineIn
End Function

'=======================================================
' from a string containing a path, create all of the
' directories necessary to make the path a valid
' path
'=======================================================
Sub makeSubdir(pathString As String)
Dim pathNoFile As String
Dim startSlash As Integer
Dim endSlash As Integer
Dim curPath As String
Dim newDir As String
On Error Resume Next
'save the current directory and drive
curPath = CurDir
pathNoFile = pathOnly(pathString)
'get drive letter and change to that drive
If Mid(pathString, 2, 1) = ":" Then
  ChDrive pathString
  ChDir "\"
End If
startSlash = InStr(pathNoFile, "\")  'find first slash
Do While startSlash > 0
  endSlash = InStr(startSlash + 1, pathNoFile, "\") 'finds end slash
  If endSlash > startSlash + 1 Then
    newDir = Mid(pathNoFile, startSlash + 1, endSlash - (startSlash + 1))
    MkDir newDir
    ChDir newDir
  End If
  startSlash = endSlash
Loop
'restore original path
ChDrive curPath
ChDir curPath
End Sub

'=======================================================
' Display the file selected from the history list
'=======================================================
Sub runViewSelectedFileFromHistory()
Dim selectedFile As String
Dim filelength As Long
On Error Resume Next
selectedFile = treeHistory.SelectedItem.Text
'check the length of all files before trying to open them
' don't really need the length but it makes for a quick test
Err.Clear
filelength = FileLen(selectedFile)
If Err.Number <> 0 Then
  MsgBox "File not found: " & vbCrLf & vbCrLf & selectedFile, vbExclamation, "View Cancelled"
  Exit Sub
End If
Select Case UCase(extensionOnly(selectedFile))
  Case "DXF"
    If dxfViewFileName = "" Then
      MsgBox "No Drawing File Viewer Selected.  To view a drawing you need a program that can display a DXF file.  You will now be prompted to locate such a program on your computer.", vbExclamation, "Viewing Cancelled"
      dxfViewFileName = SelectApplication("Find Drawing File Viewer", dxfViewFileName)
      If dxfViewFileName = "" Then Exit Sub
    End If
    Shell dxfViewFileName & " " & q & selectedFile & q, vbNormalFocus
  Case "SVG"
    If svgViewFileName = "" Then
      MsgBox "No SVG Browser Selected", vbExclamation, "Edit Cancelled"
      Exit Sub
    End If
    Shell svgViewFileName & " " & q & selectedFile & q, vbNormalFocus
  Case "WRL"
    If vrmlAppFileName = "" Then
      MsgBox "No VRML Application Selected", vbExclamation, "Edit Cancelled"
      Exit Sub
    End If
    Shell vrmlAppFileName & " " & q & selectedFile & q, vbNormalFocus
  Case "HTML"
    If htmlViewFileName = "" Then
      MsgBox "No HTML Browser Selected", vbExclamation, "Edit Cancelled"
      Exit Sub
    End If
    Shell htmlViewFileName & " " & q & selectedFile & q, vbNormalFocus
  Case "CSV"
    If spreadsheetFileName = "" Then
      MsgBox "No Spreadsheet Program Selected.  To view the spreadsheet you need a program that can display a CSV file.  You will now be prompted to locate such a program on your computer.", vbExclamation, "Run Cancelled"
      spreadsheetFileName = SelectApplication("Find Spreadsheet Program", spreadsheetFileName)
      If spreadsheetFileName = "" Then Exit Sub
    End If
    Shell spreadsheetFileName & " /e " & q & selectedFile & q, vbNormalFocus
  Case "TAB"
    If tabWithSpreadsheet Then
      If spreadsheetFileName = "" Then
        MsgBox "No Spreadsheet Program Selected.  To view the spreadsheet you need a program that can display a CSV file.  You will now be prompted to locate such a program on your computer.", vbExclamation, "Run Cancelled"
        spreadsheetFileName = SelectApplication("Find Spreadsheet Program", spreadsheetFileName)
        If spreadsheetFileName = "" Then Exit Sub
      End If
    Shell spreadsheetFileName & " /e " & q & selectedFile & q, vbNormalFocus
    Else
      If textEditFileName = "" Then
        MsgBox "No Text Editor Selected", vbExclamation, "Edit Cancelled"
        Exit Sub
      End If
      Shell textEditFileName & " " & q & selectedFile & q, vbNormalFocus
      If Err.Number <> 0 Then
         MsgBox "Error initiating text editor for " & selectedFile & ". This is likely due to the text editor program being deleted or moved. Please use the FILE menu to select a new text editor program.", vbExclamation, "Edit Cancelled"
         Exit Sub
      End If
    End If
  Case Else 'handle like a text file
    If textEditFileName = "" Then
      MsgBox "No Text Editor Selected", vbExclamation, "Edit Cancelled"
      Exit Sub
    End If
    Shell textEditFileName & " " & q & selectedFile & q, vbNormalFocus
    If Err.Number <> 0 Then
       MsgBox "Error initiating text editor for " & selectedFile & ". This is likely due to the text editor program being deleted or moved. Please use the FILE menu to select a new text editor program.", vbExclamation, "Edit Cancelled"
       Exit Sub
    End If
End Select
End Sub

'=======================================================
' Pause the program for a number of seconds but allow
' events to be processed.
'=======================================================
Sub Pause(ByVal numSecond As Single)
Dim timerStart As Single
Dim noEvent As Integer
timerStart = Timer
Do While Timer - timerStart < numSecond
  noEvent = DoEvents()
  ' Midnight fix
  If Timer < timerStart Then
    timerStart = timerStart - 24 * 60 * 60
  End If
Loop
End Sub

'=======================================================
'Open the folder containing the file specified
'=======================================================
Sub openFolderContaining(nameOfFile As String)
Shell "explorer.exe " & pathOnly(nameOfFile), vbMaximizedFocus
End Sub

'=======================================================
' Check if any files associated with a button are present
' and enable button if they are
'=======================================================
Sub checkOutputButtonsFiles()
Dim showSetButton(numOutputSets) As Boolean
Dim iSet As Integer
Dim jKind As Integer
On Error Resume Next
'TEXT BUTTON CHECKING
cmdZSZ.Enabled = False
cmdSSZ.Enabled = False
cmdSpSZ.Enabled = False
cmdMAP.Enabled = False
cmdESO.Enabled = False
cmdRDD.Enabled = False
cmdMDD.Enabled = False
cmdEIO.Enabled = False
cmdSLN.Enabled = False
cmdMTR.Enabled = False
cmdMTD.Enabled = False
cmdBND.Enabled = False
cmdDBG.Enabled = False
cmdAudit.Enabled = False
cmdTable.Enabled = False
cmdOUT.Enabled = False
cmdEXPIDF.Enabled = False
cmdEPMIDF.Enabled = False
cmdEPMDET.Enabled = False
cmdELDMP.Enabled = False
cmdDFDMP.Enabled = False
cmdERR.Enabled = False
cmdSVG.Enabled = False
cmdDXF.Enabled = False
cmdVRML.Enabled = False
cmdMain.Enabled = False
cmdMeter.Enabled = False
cmdSHD.Enabled = False
cmdScreen.Enabled = False
cmdProcCSV.Enabled = False
cmdEDD.Enabled = False
cmdBsmtOut.Enabled = False
cmdBsmt.Enabled = False
cmdBsmtAudit.Enabled = False
cmdBsmtCSV.Enabled = False
cmdPerfLogCSV.Enabled = False
cmdSlabOut.Enabled = False
cmdSlab.Enabled = False
cmdSlabErr.Enabled = False
cmdTblXML.Enabled = False
If checkIfFileExists(outputFileName & ".ERR") Then
  cmdERR.Enabled = True
End If
If checkIfFileExists(outputFileName & ".ESO") Then
  cmdESO.Enabled = True
End If
If checkIfFileExists(outputFileName & ".RDD") Then
  cmdRDD.Enabled = True
End If
If checkIfFileExists(outputFileName & ".MDD") Then
  cmdMDD.Enabled = True
End If
If checkIfFileExists(outputFileName & ".EIO") Then
  cmdEIO.Enabled = True
End If
If checkIfFileExists(outputFileName & ".SLN") Then
  cmdSLN.Enabled = True
End If
If checkIfFileExists(outputFileName & ".SHD") Then
  cmdSHD.Enabled = True
End If
If checkIfFileExists(outputFileName & "ZSZ.TXT") Then
  cmdZSZ.Enabled = True
End If
If checkIfFileExists(outputFileName & "SpSZ.TXT") Then
  cmdSpSZ.Enabled = True
End If
If checkIfFileExists(outputFileName & "SSZ.TXT") Then
  cmdSSZ.Enabled = True
End If
If checkIfFileExists(outputFileName & ".MTR") Then
  cmdMTR.Enabled = True
End If
If checkIfFileExists(outputFileName & ".MTD") Then
  cmdMTD.Enabled = True
End If
If checkIfFileExists(outputFileName & ".BND") Then
  cmdBND.Enabled = True
End If
If checkIfFileExists(outputFileName & ".DBG") Then
  cmdDBG.Enabled = True
End If
If checkIfFileExists(outputFileName & "MAP.TXT") Then
  cmdMAP.Enabled = True
End If
If checkIfFileExists(outputFileName & ".AUDIT") Then
  cmdAudit.Enabled = True
End If
If checkIfFileExists(outputFileName & "TABLE.TXT") Then
  cmdTable.Enabled = True
End If
If checkIfFileExists(outputFileName & ".EPMIDF") Then
  cmdEPMIDF.Enabled = True
End If
If checkIfFileExists(outputFileName & ".EPMDET") Then
  cmdEPMDET.Enabled = True
End If
If checkIfFileExists(outputFileName & ".TXT") Then
End If
If checkIfFileExists(outputFileName & ".EXPIDF") Then
  cmdEXPIDF.Enabled = True
End If
If checkIfFileExists(outputFileName & "DElight.in") Then
  cmdIN.Enabled = True
Else
  cmdIN.Enabled = False
End If
If checkIfFileExists(outputFileName & "DElight.out") Then
  cmdOUT.Enabled = True
End If
If checkIfFileExists(outputFileName & "DElight.eldmp") Then
  cmdELDMP.Enabled = True
End If
If checkIfFileExists(outputFileName & "DElight.dfdmp") Then
  cmdDFDMP.Enabled = True
End If
If checkIfFileExists(outputFileName & ".EDD") Then
  cmdEDD.Enabled = True
End If
If checkIfFileExists(outputFileName & "_bsmt.out") Then
  cmdBsmtOut.Enabled = True
End If
If checkIfFileExists(outputFileName & ".bsmt") Then
  cmdBsmt.Enabled = True
End If
If checkIfFileExists(outputFileName & "_bsmt.audit") Then
  cmdBsmtAudit.Enabled = True
End If
If checkIfFileExists(outputFileName & "_bsmt.csv") Then
  cmdBsmtCSV.Enabled = True
End If
If checkIfFileExists(outputFileName & ".slab") Then
  cmdSlab.Enabled = True
End If
If checkIfFileExists(outputFileName & "_slab.ger") Then
  cmdSlabErr.Enabled = True
End If
If checkIfFileExists(outputFileName & "_slab.out") Then
  cmdSlabOut.Enabled = True
End If
If checkIfFileExists(outputFileName & "Table.xml") Then
  cmdTblXML.Enabled = True
End If
If checkIfFileExists(outputFileName & "_perflog.csv") Then
  cmdPerfLogCSV.Enabled = True
End If

If Not tabWithSpreadsheet Then
  If checkIfFileExists(outputFileName & "ZSZ.TAB") Then
    cmdZSZ.Enabled = True
  End If
  If checkIfFileExists(outputFileName & "SpSZ.TAB") Then
    cmdSpSZ.Enabled = True
  End If
  If checkIfFileExists(outputFileName & "SSZ.TAB") Then
    cmdSSZ.Enabled = True
  End If
  If checkIfFileExists(outputFileName & "MAP.TAB") Then
    cmdMAP.Enabled = True
  End If
  If checkIfFileExists(outputFileName & "TABLE.TAB") Then
    cmdTable.Enabled = True
  End If
  If checkIfFileExists(outputFileName & ".TAB") Then
    cmdMain.Enabled = True
  End If
  If checkIfFileExists(outputFileName & "METER.TAB") Then
    cmdMeter.Enabled = True
  End If
End If
If checkIfFileExists(NoExtension(groupFileName) & ".errgrp") Then
  cmdViewGroupError.Enabled = True
Else
  cmdViewGroupError.Enabled = False
End If
'DRAWING BUTTON CHECKING
'  - don't do since EPDrawGUI can be run instead
If checkIfFileExists(outputFileName & ".DXF") Then
  cmdDXF.Enabled = True
End If
If checkIfFileExists(outputFileName & ".SVG") Then
  cmdSVG.Enabled = True
End If
If checkIfFileExists(outputFileName & ".WRL") Then
  cmdVRML.Enabled = True
End If
'SPREADSHEET BUTTON CHECKING
If checkIfFileExists(outputFileName & ".CSV") Then
  cmdMain.Enabled = True
End If
If checkIfFileExists(outputFileName & "Zsz.csv") Then
  cmdZSZ.Enabled = True
End If
If checkIfFileExists(outputFileName & "Spsz.csv") Then
  cmdSpSZ.Enabled = True
End If
If checkIfFileExists(outputFileName & "Ssz.csv") Then
  cmdSSZ.Enabled = True
End If
If checkIfFileExists(outputFileName & "Map.csv") Then
  cmdMAP.Enabled = True
End If
If checkIfFileExists(outputFileName & "Table.csv") Then
  cmdTable.Enabled = True
End If
If checkIfFileExists(outputFileName & "Meter.csv") Then
  cmdMeter.Enabled = True
End If
If checkIfFileExists(outputFileName & "Screen.csv") Then
  cmdScreen.Enabled = True
End If
If checkIfFileExists(outputFileName & "-Proc.csv") Then
  cmdProcCSV.Enabled = True
End If
If tabWithSpreadsheet Then
  If checkIfFileExists(outputFileName & "Zsz.tab") Then
    cmdZSZ.Enabled = True
  End If
  If checkIfFileExists(outputFileName & "Spsz.tab") Then
    cmdSpSZ.Enabled = True
  End If
  If checkIfFileExists(outputFileName & "Ssz.tab") Then
    cmdSSZ.Enabled = True
  End If
  If checkIfFileExists(outputFileName & "Map.tab") Then
    cmdMAP.Enabled = True
  End If
  If checkIfFileExists(outputFileName & "Table.tab") Then
    cmdTable.Enabled = True
  End If
  If checkIfFileExists(outputFileName & ".tab") Then
    cmdMain.Enabled = True
  End If
  If checkIfFileExists(outputFileName & "Meter.tab") Then
    cmdMeter.Enabled = True
  End If
End If
'HTML BUTTON CHECKING
If checkIfFileExists(outputFileName & "Table.html") Then
  cmdTable.Enabled = True
End If
'Now check if the buttons on the "SET" tab should
'be enabled or not. Any sets with a DXF item should be
'enabled since EPDrawGUI gets called.
'clear the flags
For iSet = 1 To numOutputSets
  showSetButton(iSet) = False
Next iSet
'go through the suffixes seeing if the file exists
For jKind = 1 To numOutputKinds
  'see if any of the sets include this file type
  If checkIfFileExists(outputFileName & outputKind(jKind).suffix) Then
    For iSet = 1 To numOutputSets
      If outputKind(jKind).outSet(iSet) Then
        showSetButton(iSet) = True
        'Exit For
      End If
    Next iSet
  End If
Next jKind
'if any files do exist show the button
For iSet = 1 To numOutputSets
  cmdViewSet(iSet - 1).Enabled = showSetButton(iSet)
Next iSet
cmdViewSet(outSetDrawingFiles - 1).Enabled = True
End Sub


'=======================================================
' If the file exists
'=======================================================
Public Function checkIfFileExists(nameOfFile As String) As Boolean
Dim filelength  As Long
On Error Resume Next
'check the length of all files before trying to open them
' don't really need the length but it makes for a quick test
Err.Clear
filelength = FileLen(nameOfFile)
If Err.Number = 0 Then
  checkIfFileExists = True
Else
  checkIfFileExists = False
End If
End Function

'=======================================================
' Append one text file to another
'=======================================================
Sub appendTextFile(logFileName As String, toAppendFileName As String)
Dim lineOfFile As String
Dim flLog As Integer
Dim flApp As Integer
flLog = FreeFile
On Error Resume Next
Open logFileName For Append As flLog
If Err.Number <> 0 Then Exit Sub
flApp = FreeFile
Open toAppendFileName For Input As flApp
If Err.Number <> 0 Then
  Close flLog
  Exit Sub
End If
Do While Not EOF(flApp)
  Line Input #flApp, lineOfFile
  Print #flLog, lineOfFile
  If Err.Number <> 0 Then Exit Do
Loop
Close flApp
Close flLog
End Sub

'=======================================================
' Check if the selected file name has both an IDF and
' an IMF file associated with it and provide a warning
'=======================================================
Sub warnIfBothIdfImf()
Dim nameNoExt As String
Dim idfFound As Boolean
Dim imfFound As Boolean
Dim filelength As Long
On Error Resume Next
nameNoExt = NoExtension(inputFileName)
Err.Clear
filelength = FileLen(nameNoExt & ".idf")
If Err.Number = 0 Then
  idfFound = True
Else
  idfFound = False
End If
Err.Clear
filelength = FileLen(nameNoExt & ".imf")
If Err.Number = 0 Then
  imfFound = True
Else
  imfFound = False
End If
If idfFound And imfFound Then
  MsgBox "The selected file name was found with both an IDF extension and an IMF extension. This may cause problems since both files create the output files using exactly the same names. You may want to rename one of the files.", vbInformation, "IDF and IMF both found"
End If
End Sub

'=======================================================
' View main csv file
'=======================================================
Sub viewMainCSV()
Dim filelength As Long
' check first for text file
On Error Resume Next
Err.Clear
filelength = FileLen(outputFileName & ".txt")
If Err.Number = 0 Then
  Call RunOutputEditorSingleFile(".txt")
  Exit Sub
End If
Err.Clear
filelength = FileLen(outputFileName & ".csv")
If Err.Number = 0 Then
  Call runOutputSpreadsheetSingleFile(".csv")
  Exit Sub
End If
Err.Clear
filelength = FileLen(outputFileName & ".tab")
If Err.Number = 0 Then
  If tabWithSpreadsheet Then
    Call runOutputSpreadsheetSingleFile(".tab")
  Else
    Call RunOutputEditorSingleFile(".tab")
  End If
  Exit Sub
End If
End Sub

'=======================================================
' View METER file
'=======================================================
Sub viewMETERfile()
Dim filelength As Long
' check first for text file
On Error Resume Next
Err.Clear
filelength = FileLen(outputFileName & "Meter.txt")
If Err.Number = 0 Then
  Call RunOutputEditorSingleFile("Meter.txt")
  Exit Sub
End If
Err.Clear
filelength = FileLen(outputFileName & "Meter.csv")
If Err.Number = 0 Then
  Call runOutputSpreadsheetSingleFile("Meter.csv")
  Exit Sub
End If
Err.Clear
filelength = FileLen(outputFileName & "Meter.tab")
If Err.Number = 0 Then
  If tabWithSpreadsheet Then
    Call runOutputSpreadsheetSingleFile("Meter.tab")
  Else
    Call RunOutputEditorSingleFile("Meter.tab")
  End If
  Exit Sub
End If
End Sub

'=======================================================
' View SSZ file
'=======================================================
Sub viewSSZFile()
Dim filelength As Long
' check first for text file
On Error Resume Next
Err.Clear
filelength = FileLen(outputFileName & "Ssz.txt")
If Err.Number = 0 Then
  Call RunOutputEditorSingleFile("Ssz.txt")
  Exit Sub
End If
Err.Clear
filelength = FileLen(outputFileName & "Ssz.csv")
If Err.Number = 0 Then
  Call runOutputSpreadsheetSingleFile("Ssz.csv")
  Exit Sub
End If
Err.Clear
filelength = FileLen(outputFileName & "Ssz.tab")
If Err.Number = 0 Then
  If tabWithSpreadsheet Then
    Call runOutputSpreadsheetSingleFile("Ssz.tab")
  Else
    Call RunOutputEditorSingleFile("Ssz.tab")
  End If
  Exit Sub
End If
End Sub

'=======================================================
' View ZSZ file
'=======================================================
Sub viewZSZFile()
Dim filelength As Long
' check first for text file
On Error Resume Next
Err.Clear
filelength = FileLen(outputFileName & "Zsz.txt")
If Err.Number = 0 Then
  Call RunOutputEditorSingleFile("Zsz.txt")
  Exit Sub
End If
Err.Clear
filelength = FileLen(outputFileName & "Zsz.csv")
If Err.Number = 0 Then
  Call runOutputSpreadsheetSingleFile("Zsz.csv")
  Exit Sub
End If
Err.Clear
filelength = FileLen(outputFileName & "Zsz.tab")
If Err.Number = 0 Then
  If tabWithSpreadsheet Then
    Call runOutputSpreadsheetSingleFile("Zsz.tab")
  Else
    Call RunOutputEditorSingleFile("Zsz.tab")
  End If
  Exit Sub
End If
End Sub


'=======================================================
' View SpSZ file
'=======================================================
Sub viewSpSZFile()
Dim filelength As Long
' check first for text file
On Error Resume Next
Err.Clear
filelength = FileLen(outputFileName & "Spsz.txt")
If Err.Number = 0 Then
  Call RunOutputEditorSingleFile("Spsz.txt")
  Exit Sub
End If
Err.Clear
filelength = FileLen(outputFileName & "Spsz.csv")
If Err.Number = 0 Then
  Call runOutputSpreadsheetSingleFile("Spsz.csv")
  Exit Sub
End If
Err.Clear
filelength = FileLen(outputFileName & "Spsz.tab")
If Err.Number = 0 Then
  If tabWithSpreadsheet Then
    Call runOutputSpreadsheetSingleFile("Spsz.tab")
  Else
    Call RunOutputEditorSingleFile("Spsz.tab")
  End If
  Exit Sub
End If
End Sub

'=======================================================
' View TABLE file
'=======================================================
Sub viewTABLEfile()
Dim filelength As Long
' check first for text file
On Error Resume Next
Err.Clear
filelength = FileLen(outputFileName & "Table.txt")
If Err.Number = 0 Then
  Call RunOutputEditorSingleFile("Table.txt")
  Exit Sub
End If
Err.Clear
filelength = FileLen(outputFileName & "Table.csv")
If Err.Number = 0 Then
  Call runOutputSpreadsheetSingleFile("Table.csv")
  Exit Sub
End If
Err.Clear
filelength = FileLen(outputFileName & "Table.tab")
If Err.Number = 0 Then
  If tabWithSpreadsheet Then
    Call runOutputSpreadsheetSingleFile("Table.tab")
  Else
    Call RunOutputEditorSingleFile("Table.tab")
  End If
  Exit Sub
End If
Err.Clear
filelength = FileLen(outputFileName & "Table.html")
If Err.Number = 0 Then
  Call showSingleHTMLFile("Table.html")
  Exit Sub
End If
End Sub

'=======================================================
' Look for the transition file and grab the version
' numbers from it.
'
' Assumes the file name is in the form:
'
'    Transition-V1-2-2-to-V1-2-3.exe
'
'=======================================================
Sub getTransitionVersions()
Dim transProg As String
Dim shortIDDversion
Dim iddFirstPeriodPos As Integer
Dim toPos As Integer
Dim exePos As Integer
Dim i As Integer
iddFirstPeriodPos = InStr(iddVersion, ".")
'make short version using dashes depending on version in format 9.6.1 or 22.1.0
If iddFirstPeriodPos = 3 Then
    shortIDDversion = Left(iddVersion, 2) & "-" & Mid(iddVersion, 4, 1) & "-" & Mid(iddVersion, 6, 1)
Else
    shortIDDversion = Left(iddVersion, 1) & "-" & Mid(iddVersion, 3, 1) & "-" & Mid(iddVersion, 5, 1)
End If
transProg = dir(appPath & "PreProcess\IDFVersionUpdater\Transition-V?-?-?-to-V" & shortIDDversion & ".exe")
If transProg = "" Then
    transProg = dir(appPath & "PreProcess\IDFVersionUpdater\Transition-V??-?-?-to-V" & shortIDDversion & ".exe")
End If
transitionFileName = transProg
If transProg <> "" Then
  'convert dashes to periods
  For i = 1 To Len(transProg)
    If Mid(transProg, i, 1) = "-" Then
      Mid(transProg, i) = "."
    End If
  Next i
  toPos = InStr(transProg, "to")
  exePos = InStr(transProg, "exe")
  '  Transition-V1-2-1-to-V1-2-2.exe
  '  12345678901234567890123456789012
  previousVersion = Mid(transProg, 13, toPos - 14)
  currentVersion = Mid(transProg, toPos + 4, exePos - (toPos + 5))
  'MsgBox previousVersion & vbCrLf & vbCrLf & currentVersion, vbExclamation, "getTransitionVersions debug"
End If
End Sub

'=======================================================
' Return the version string of the file
'
' if no Version object found in file return special
' flag string "VERSION NOT FOUND"
'=======================================================
Function checkIDFVersion(inFile As String) As String
Dim lineIn As String
Dim nextLine As String
Dim objString As String
Dim flv As Integer
Dim semiPos As Integer
Dim commaPos As Integer
Dim firstPeriodPos As Integer
Dim versionFound As String
Dim i As Integer
Dim startTime As Single
Dim versionObjectFound As Boolean
Dim foundEOF As Boolean
Dim lineCount As Long
startTime = Timer
flv = FreeFile
checkIDFVersion = ""
On Error Resume Next
Open inFile For Input As flv
If Err.Number <> 0 Then Exit Function
versionObjectFound = False
Call getNextLine(flv, lineIn, foundEOF, True) 'get first line and clear buffer
lineCount = 1
Do
  'Line Input #flv, lineIn
  lineIn = LTrim(lineIn)
  If Left(lineIn, 1) <> "!" Then
    If InStr(UCase(lineIn), "VERSION,") > 0 Then
      versionObjectFound = True
      Call getNextLine(flv, nextLine, foundEOF)
      nextLine = LTrim(nextLine)
      'trim comment at end of line
      objString = removeTabs(trimOffComment(lineIn) & trimOffComment(nextLine))
      commaPos = InStr(objString, ",")
      semiPos = InStr(objString, ";")
      If semiPos > commaPos Then
        versionFound = Mid(objString, commaPos + 1, (semiPos - commaPos) - 1)
        versionFound = Trim(versionFound)
        firstPeriodPos = InStr(versionFound, ".")
        If firstPeriodPos = 3 Then 'such as version "22.1"
            If Len(versionFound) = 4 Then
              versionFound = versionFound & ".0"
            End If
            versionFound = Left(versionFound, 6) 'trim build number if included, such as: 22.0.0.028 to 22.0.0 CR7622
        Else 'such as version "9.6"
            If Len(versionFound) = 3 Then
              versionFound = versionFound & ".0"
            End If
            versionFound = Left(versionFound, 5) 'trim build number if included, such as: 3.0.0.028 to 3.0.0 CR7622
        End If
      Else
        versionFound = ""
      End If
      checkIDFVersion = versionFound
      Exit Do
    End If
  End If
  'If EOF(flv) Then Exit For
  If foundEOF Then Exit Do
  Call getNextLine(flv, lineIn, foundEOF)
  lineCount = lineCount + 1
Loop
Close flv
If Not versionObjectFound Then
  checkIDFVersion = "VERSION NOT FOUND"
End If
'MsgBox "Version of IDF: " & checkIDFVersion & " -- " & lineCount & " -- " & Timer - startTime, vbOKOnly, "CheckIDFVersion debug"
Debug.Print "Time to check for version: "; Timer - startTime
End Function

'=======================================================
' Return true if version A is newer than version B and
' false if not or if it cannot be determined
'=======================================================
Function isNewerVersion(versionA, versionB) As Boolean
Dim aVerNum As Long
Dim bVerNum As Long
aVerNum = convertVersionToNumber(versionA)
bVerNum = convertVersionToNumber(versionB)
If aVerNum > 0 And bVerNum > 0 Then
    If aVerNum > bVerNum Then
        isNewerVersion = True
    Else
        isNewerVersion = False
    End If
Else
    isNewerVersion = False
End If
End Function

'=======================================================
' Convert a version string in the format of 8.5.1 into
' an integer of 80501
'=======================================================
Function convertVersionToNumber(versionString) As Long
Dim parts() As String
Dim verNum As Long
Dim i As Integer
parts = Split(versionString, ".")
verNum = 0
If UBound(parts) = 2 Then
    For i = 0 To UBound(parts)
        If IsNumeric(parts(i)) Then
            verNum = verNum * 100 + Val(parts(i))
        Else
            verNum = 0 ' if any parts are not numeric then exit function with a zero.
            Exit For
        End If
    Next i
End If
convertVersionToNumber = verNum
'MsgBox "convertVersionToNumber: " & versionString & " into " & verNum, vbOKOnly
End Function


'=======================================================
' Remove the part of the string
'=======================================================
Function trimOffComment(stringWithComment As String) As String
Dim explPtPos As Integer
explPtPos = InStr(stringWithComment, "!")
If explPtPos > 1 Then
  trimOffComment = Left(stringWithComment, explPtPos - 1)
ElseIf explPtPos = 1 Then
  trimOffComment = ""
Else
  trimOffComment = stringWithComment
End If
End Function

'=======================================================
' Remove tabs from the  string
'=======================================================
Function removeTabs(stringIn As String) As String
Dim stringOut As String
Dim i As Integer
For i = 1 To Len(stringIn)
  If Mid(stringIn, i, 1) <> vbTab Then
    stringOut = stringOut & Mid(stringIn, i, 1)
  End If
Next
removeTabs = stringOut
End Function


'=======================================================
' Configure the options on the utility tab including
' the name of the program, what options are available
' and how to run the program.
'=======================================================
Sub setupUtilityTab()
'initialize the array the
utilProg(1).name = "Basement"
utilProg(1).about = "3-d ground heat transfer tool used to calculate monthly outside " & _
"surface temperatures for basement walls and floor. The outside surface " & _
"temperature is at the plane between the outside insulation (or the soil if " & _
" there is no insulation) and the basement wall or floor."
utilProg(1).enableInput = True
utilProg(1).enableWthr = True
utilProg(1).enableInTextEdit = True
utilProg(1).IDFEdOpt = " /idd:BasementGHT.idd"
utilProg(1).inExt = "IDF"
utilProg(1).outExt1 = "_bsmt.AUDIT"
utilProg(1).outExt2 = "_bsmt.OUT"
utilProg(1).outExt3 = "_bsmt.CSV"
utilProg(1).outExt4 = ""
utilProg(1).outExt5 = ""
utilProg(1).outExt6 = ""
utilProg(1).applicationFile = "PreProcess\GrndTempCalc\Basement.exe"
utilProg(1).appIsSpreadsheet = False
utilProg(1).batchFile = "PreProcess\GrndTempCalc\RunBasement.bat"
utilProg(1).useInputAsExeArgument = False
utilProg(1).fileSuffix = ""
utilProg(1).includeExtensionInCall = False
utilProg(1).waitUntilUtilityExits = True
'
utilProg(2).name = "CalcSoilSurfTemp"
utilProg(2).about = "The program calculates three important parameters for the simulation of " & _
"the earth tube: the annual average soil surface temperature, the amplitude of soil surface temperature, " & _
"the phase constant of soil surface temperature. "
utilProg(2).enableInput = True
utilProg(2).enableWthr = False
utilProg(2).enableInTextEdit = True
utilProg(2).IDFEdOpt = ""
utilProg(2).inExt = "EPW"
utilProg(2).outExt1 = ".OUT"
utilProg(2).outExt2 = ""
utilProg(2).outExt3 = ""
utilProg(2).outExt4 = ""
utilProg(2).outExt5 = ""
utilProg(2).outExt6 = ""
utilProg(2).applicationFile = "PreProcess\CalcSoilSurfTemp\CalcSoilSurfTemp.exe"
utilProg(2).appIsSpreadsheet = False
utilProg(2).batchFile = "PreProcess\CalcSoilSurfTemp\RunCalcSoilSurfTemp.bat"
utilProg(2).useInputAsExeArgument = False
utilProg(2).fileSuffix = ""
utilProg(2).includeExtensionInCall = False
utilProg(2).waitUntilUtilityExits = True
'
utilProg(3).name = "CoeffCheck"
utilProg(3).about = "A program to print out a performance map given a bi-quadratic performance curve."
utilProg(3).enableInput = True
utilProg(3).enableWthr = False
utilProg(3).enableInTextEdit = True
utilProg(3).IDFEdOpt = ""
utilProg(3).inExt = "CCI"
utilProg(3).outExt1 = ".CCO"
utilProg(3).outExt2 = ""
utilProg(3).outExt3 = ""
utilProg(3).outExt4 = ""
utilProg(3).outExt5 = ""
utilProg(3).outExt6 = ""
utilProg(3).applicationFile = "PreProcess\CoeffConv\CoeffCheck.exe"
utilProg(3).appIsSpreadsheet = False
utilProg(3).batchFile = "PreProcess\CoeffConv\EPL-Check.BAT"
utilProg(3).useInputAsExeArgument = False
utilProg(3).fileSuffix = ""
utilProg(3).includeExtensionInCall = False
utilProg(3).waitUntilUtilityExits = True
'
utilProg(4).name = "CoeffConv"
utilProg(4).about = "A program to convert DOE-2 temperature dependent curves in Fahrenheit to " & _
"EnergyPlus curves in Centigrade. The program converts the Doe-2 coefficients of a " & _
"biquadratic curve to the equivalent EPlus biquadratic curve coefficients."
utilProg(4).enableInput = True
utilProg(4).enableWthr = False
utilProg(4).enableInTextEdit = True
utilProg(4).IDFEdOpt = ""
utilProg(4).inExt = "COI"
utilProg(4).outExt1 = ".COO"
utilProg(4).outExt2 = ""
utilProg(4).outExt3 = ""
utilProg(4).outExt4 = ""
utilProg(4).outExt5 = ""
utilProg(4).outExt6 = ""
utilProg(4).applicationFile = "PreProcess\CoeffConv\CoeffConv.exe"
utilProg(4).appIsSpreadsheet = False
utilProg(4).batchFile = "PreProcess\CoeffConv\EPL-Conv.BAT"
utilProg(4).useInputAsExeArgument = False
utilProg(4).fileSuffix = ""
utilProg(4).includeExtensionInCall = False
utilProg(4).waitUntilUtilityExits = True
'
utilProg(5).name = "Slab"
utilProg(5).about = "3-d ground heat transfer tool used to calculate monthly outside surface " & _
"temperatures for a slab-in-grade floor.  The outside surface temperature is at " & _
"the plane between the floor slab and the soil or outside insulation."
utilProg(5).enableInput = True
utilProg(5).enableWthr = True
utilProg(5).enableInTextEdit = True
utilProg(5).IDFEdOpt = " /idd:SlabGHT.idd"
utilProg(5).inExt = "IDF"
utilProg(5).outExt1 = "_slab.GER"
utilProg(5).outExt2 = "_slab.OUT"
utilProg(5).outExt3 = "_slab.GTP"
utilProg(5).outExt4 = ""
utilProg(5).outExt5 = ""
utilProg(5).outExt6 = ""
utilProg(5).applicationFile = "PreProcess\GrndTempCalc\Slab.exe"
utilProg(5).appIsSpreadsheet = False
utilProg(5).batchFile = "PreProcess\GrndTempCalc\RunSlab.bat"
utilProg(5).useInputAsExeArgument = False
utilProg(5).fileSuffix = ""
utilProg(5).includeExtensionInCall = False
utilProg(5).waitUntilUtilityExits = True
'
utilProg(6).name = "Weather"
utilProg(6).about = "Translates and extends typical weather data into the EnergyPlus format and " & _
"calculates the horizontal infrared radiation intensity values. The utility also prepares a " & _
"statistical summary of the weather data set as part of the processing."
utilProg(6).enableInput = False
utilProg(6).enableWthr = False
utilProg(6).enableInTextEdit = True
utilProg(6).IDFEdOpt = ""
utilProg(6).inExt = ""
utilProg(6).outExt1 = ""
utilProg(6).outExt2 = ""
utilProg(6).outExt3 = ""
utilProg(6).outExt4 = ""
utilProg(6).outExt5 = ""
utilProg(6).outExt6 = ""
utilProg(6).applicationFile = "PreProcess\WeatherConverter\Weather.exe"
utilProg(6).appIsSpreadsheet = False
utilProg(6).batchFile = ""
utilProg(6).useInputAsExeArgument = False
utilProg(6).fileSuffix = ""
utilProg(6).includeExtensionInCall = False
utilProg(6).waitUntilUtilityExits = True
'
utilProg(7).name = "AppGPostProcess"
utilProg(7).about = "Averages four sets of simulation results for use with Appendix G of ASHRAE " & _
"Standard 90.1.  The four simulations must end with the names -G000, -G090, -G180 and -G270. " & _
"The utility will average the HTML, main CSV, and Meter CSV files. All file must be the same directory." & _
"Select the -G000 as the input file."
utilProg(7).enableInput = True
utilProg(7).enableWthr = False
utilProg(7).enableInTextEdit = False
utilProg(7).IDFEdOpt = ""
utilProg(7).inExt = "HTML"
utilProg(7).inExtAlt = "HTM"
utilProg(7).outExt1 = "-GAVG.csv"
utilProg(7).outExt2 = "-GAVGMeter.csv"
utilProg(7).outExt3 = "-GAVGTable.html"
utilProg(7).outExt3Alt = "-GAVGTable.htm"
utilProg(7).outExt4 = "-AppGErr.txt"
utilProg(7).outExt5 = ""
utilProg(7).outExt6 = ""
utilProg(7).applicationFile = "PostProcess\AppGPostProcess.exe"
utilProg(7).appIsSpreadsheet = False
utilProg(7).batchFile = ""
utilProg(7).useInputAsExeArgument = True
utilProg(7).fileSuffix = "-G000Table"
utilProg(7).includeExtensionInCall = True
utilProg(7).waitUntilUtilityExits = True
'
utilProg(8).name = "IDFVersionUpdater"
utilProg(8).about = "Converts a file created for an older version of EnergyPlus into a file compatible " & _
"with the current version of EnergyPlus. " & _
"The utility may require additional files to be downloaded prior to use. "
utilProg(8).enableInput = False
utilProg(8).enableWthr = False
utilProg(8).enableInTextEdit = True
utilProg(8).IDFEdOpt = ""
utilProg(8).inExt = ""
utilProg(8).outExt1 = ""
utilProg(8).outExt2 = ""
utilProg(8).outExt3 = ""
utilProg(8).outExt4 = ""
utilProg(8).outExt5 = ""
utilProg(8).outExt6 = ""
utilProg(8).applicationFile = "PreProcess\IDFVersionUpdater\IDFVersionUpdater.exe"
utilProg(8).appIsSpreadsheet = False
utilProg(8).batchFile = ""
utilProg(8).useInputAsExeArgument = False
utilProg(8).fileSuffix = ""
utilProg(8).includeExtensionInCall = False
utilProg(8).waitUntilUtilityExits = False
'
utilProg(9).name = "EPDrawGUI"
utilProg(9).about = "Creates a DXF file from an EnergyPlus IDF file."
utilProg(9).enableInput = False
utilProg(9).enableWthr = False
utilProg(9).enableInTextEdit = True
utilProg(9).IDFEdOpt = ""
utilProg(9).inExt = ""
utilProg(9).outExt1 = ""
utilProg(9).outExt2 = ""
utilProg(9).outExt3 = ""
utilProg(9).outExt4 = ""
utilProg(9).outExt5 = ""
utilProg(9).outExt6 = ""
utilProg(9).applicationFile = "PreProcess\EPDraw\EPDrawGUI.exe"
utilProg(9).appIsSpreadsheet = False
utilProg(9).batchFile = ""
utilProg(9).useInputAsExeArgument = False
utilProg(9).fileSuffix = ""
utilProg(9).includeExtensionInCall = False
utilProg(9).waitUntilUtilityExits = False

'
' NOT SUPPORTED
'
'utilProg(10).name = "EP-Compare"
'utilProg(10).about = "Graphs the values from the tabular report for multiple simulations side by side."
'utilProg(10).enableInput = False
'utilProg(10).enableWthr = False
'utilProg(10).enableInTextEdit = True
'utilProg(10).IDFEdOpt = ""
'utilProg(10).inExt = ""
'utilProg(10).outExt1 = ""
'utilProg(10).outExt2 = ""
'utilProg(10).outExt3 = ""
'utilProg(10).outExt4 = ""
'utilProg(10).outExt5 = ""
'utilProg(10).outExt6 = ""
'utilProg(10).applicationFile = "PostProcess\EP-Compare\EP-Compare.exe"
'utilProg(10).appIsSpreadsheet = False
'utilProg(10).batchFile = ""
'utilProg(10).useInputAsExeArgument = False
'utilProg(10).fileSuffix = ""
'utilProg(10).includeExtensionInCall = False
'utilProg(10).waitUntilUtilityExits = False
'
'utilProg(8).name = "View3D"
'utilProg(8).about = "EnergyPlus has the capability of accepting user defined view factors for special research " & _
'"situations. This option is not recommended for general use. However, when a user desires to supply view " & _
'"factors, this auxiliary program can be used to calculate them for a variety of configurations."
'utilProg(8).enableInput = False
'utilProg(8).enableWthr = False
'utilProg(8).IDFEdOpt = ""
'utilProg(8).inExt = ""
'utilProg(8).outExt1 = ""
'utilProg(8).outExt2 = ""
'utilProg(8).outExt3 = ""
'utilProg(8).applicationFile = "PreProcess\ViewFactorCalculation\ViewFactorInterface.xls"
'utilProg(8).appIsSpreadsheet = True
'utilProg(8).batchFile = ""
'
'utilProg(2).name = "Blast Translator"
'utilProg(2).about = "The BLAST Translator will produce an IDF file from an existing BLAST Input File usually " & _
'"called <something>.bin. For anyone that is unfamiliar, BLAST stands for the Building Loads " & _
'"Analysis and Systems Thermodynamics computer program."
'utilProg(2).enableInput = True
'utilProg(2).enableWthr = False
'utilProg(2).IDFEdOpt = ""
'utilProg(2).inExt = "BIN"
'utilProg(2).outExt1 = "IDF"
'utilProg(2).outExt2 = ""
'utilProg(2).outExt3 = ""
'utilProg(2).applicationFile = ""
'utilProg(2).appIsSpreadsheet = False
'utilProg(2).batchFile = ""
End Sub

'=======================================================
' Change the captions and state of all controls on
' the utility tab based on which is selected
'=======================================================
Sub redrawUtilityTab()
Dim curUtil As Integer
Dim outFile As String
curUtil = cmbUtility.ListIndex + 1
lblUtilityAbout.Caption = utilProg(curUtil).name & " - " & utilProg(curUtil).about
frameUtilityInput.Visible = utilProg(curUtil).enableInput
If utilProg(curUtil).IDFEdOpt <> "" Then
  cmdUtilityIDFEdit.Visible = True
Else
  cmdUtilityIDFEdit.Visible = False
End If
cmdUtilityTextEdit.Visible = utilProg(curUtil).enableInTextEdit
frameUtilityWeather.Visible = utilProg(curUtil).enableWthr
'for each output extension see if one is specified and show the button
'if a file exists enable the button.
If utilProg(curUtil).outExt1 <> "" Then
    cmdUtilityOutput1.Visible = True
    cmdUtilityOutput1.Caption = "Open " & utilProg(curUtil).outExt1
    outFile = changeExtension(utilProg(curUtil).curInputFile, utilProg(curUtil).outExt1, utilProg(curUtil).fileSuffix)
    If checkIfFileExists(outFile) Then
      cmdUtilityOutput1.Enabled = True
    Else
      cmdUtilityOutput1.Enabled = False
    End If
Else
    cmdUtilityOutput1.Visible = False
End If
If utilProg(curUtil).outExt2 <> "" Then
    cmdUtilityOutput2.Visible = True
    cmdUtilityOutput2.Caption = "Open " & utilProg(curUtil).outExt2
    outFile = changeExtension(utilProg(curUtil).curInputFile, utilProg(curUtil).outExt2, utilProg(curUtil).fileSuffix)
    If checkIfFileExists(outFile) Then
      cmdUtilityOutput2.Enabled = True
    Else
      cmdUtilityOutput2.Enabled = False
    End If
Else
    cmdUtilityOutput2.Visible = False
End If
If utilProg(curUtil).outExt3 <> "" Then
    cmdUtilityOutput3.Visible = True
    cmdUtilityOutput3.Caption = "Open " & utilProg(curUtil).outExt3
    outFile = changeExtension(utilProg(curUtil).curInputFile, utilProg(curUtil).outExt3, utilProg(curUtil).fileSuffix)
    If checkIfFileExists(outFile) Then
      cmdUtilityOutput3.Enabled = True
    Else
      'check if alternative output extension file exists
      If utilProg(curUtil).outExt3Alt <> "" Then
        outFile = changeExtension(utilProg(curUtil).curInputFile, utilProg(curUtil).outExt3Alt, utilProg(curUtil).fileSuffix)
        If checkIfFileExists(outFile) Then
          cmdUtilityOutput3.Enabled = True
        Else
          cmdUtilityOutput3.Enabled = False
        End If
      Else
        cmdUtilityOutput3.Enabled = False
      End If
    End If
Else
    cmdUtilityOutput3.Visible = False
End If

If utilProg(curUtil).outExt4 <> "" Then
    cmdUtilityOutput4.Visible = True
    cmdUtilityOutput4.Caption = "Open " & utilProg(curUtil).outExt4
    outFile = changeExtension(utilProg(curUtil).curInputFile, utilProg(curUtil).outExt4, utilProg(curUtil).fileSuffix)
    If checkIfFileExists(outFile) Then
      cmdUtilityOutput4.Enabled = True
    Else
      cmdUtilityOutput4.Enabled = False
    End If
Else
    cmdUtilityOutput4.Visible = False
End If
If utilProg(curUtil).outExt5 <> "" Then
    cmdUtilityOutput5.Visible = True
    cmdUtilityOutput5.Caption = "Open " & utilProg(curUtil).outExt5
    outFile = changeExtension(utilProg(curUtil).curInputFile, utilProg(curUtil).outExt5, utilProg(curUtil).fileSuffix)
    If checkIfFileExists(outFile) Then
      cmdUtilityOutput5.Enabled = True
    Else
      cmdUtilityOutput5.Enabled = False
    End If
Else
    cmdUtilityOutput5.Visible = False
End If
If utilProg(curUtil).outExt6 <> "" Then
    cmdUtilityOutput6.Visible = True
    cmdUtilityOutput6.Caption = "Open " & utilProg(curUtil).outExt6
    outFile = changeExtension(utilProg(curUtil).curInputFile, utilProg(curUtil).outExt6, utilProg(curUtil).fileSuffix)
    If checkIfFileExists(outFile) Then
      cmdUtilityOutput6.Enabled = True
    Else
      cmdUtilityOutput6.Enabled = False
    End If
Else
    cmdUtilityOutput6.Visible = False
End If

frameUtilityInput.Caption = "Input File (" & utilProg(curUtil).inExt & ")"
cmdUtilityRun.Caption = "Run " & utilProg(curUtil).name
lblUtilityInputFile.Caption = utilProg(curUtil).curInputFile
lblUtilityWeatherFile.Caption = utilProg(curUtil).curWeatherFile
End Sub


'=======================================================
' Event when pull down list on utility tab changes
'=======================================================
Private Sub cmbUtility_Click()
Call redrawUtilityTab
End Sub


'=======================================================
' Browse for Input file on Utility Tab
'=======================================================
Private Sub cmdUtilityInputBrowse_Click()
Dim oldFN As String
Dim curUtil As Integer
Dim newFN As String
curUtil = cmbUtility.ListIndex + 1
oldFN = utilProg(curUtil).curInputFile
CommonDialog1.fileName = oldFN
CommonDialog1.DialogTitle = "Find " & utilProg(curUtil).name & " Input File"
If utilProg(curUtil).inExtAlt = "" Then
  CommonDialog1.Filter = utilProg(curUtil).name & " (*." & utilProg(curUtil).inExt & ") |*." & utilProg(curUtil).inExt
Else
  CommonDialog1.Filter = utilProg(curUtil).name & " (*." & utilProg(curUtil).inExt & ";*." & utilProg(curUtil).inExtAlt & ") |*." & utilProg(curUtil).inExt & ";*." & utilProg(curUtil).inExtAlt
End If
CommonDialog1.FilterIndex = 1
CommonDialog1.Flags = &H800 Or &H1000 Or cdlOFNHideReadOnly
If oldFN = "" Then
  CommonDialog1.InitDir = appPath
Else
  CommonDialog1.InitDir = pathOnly(oldFN)
End If
CommonDialog1.ShowOpen
newFN = CommonDialog1.fileName
If newFN <> "" Then utilProg(curUtil).curInputFile = newFN
lblUtilityInputFile.Caption = utilProg(curUtil).curInputFile
Call redrawUtilityTab
End Sub

'=======================================================
' Browse for Weather file on Utility Tab
'=======================================================
Private Sub cmdUtilityWeatherBrowse_Click()
Dim oldFN As String
Dim curUtil As Integer
Dim newFN As String
curUtil = cmbUtility.ListIndex + 1
oldFN = utilProg(curUtil).curWeatherFile
CommonDialog1.fileName = oldFN
CommonDialog1.DialogTitle = "Find EnergyPlus Weather File"
CommonDialog1.Filter = "EnergyPlus Weather (*.epw)|*.epw"
CommonDialog1.FilterIndex = 1
CommonDialog1.Flags = &H800 Or &H1000 Or cdlOFNHideReadOnly
If oldFN = "" Then
  CommonDialog1.InitDir = appPath
Else
  CommonDialog1.InitDir = pathOnly(oldFN)
End If
CommonDialog1.ShowOpen
newFN = CommonDialog1.fileName
If newFN <> "" Then utilProg(curUtil).curWeatherFile = newFN
lblUtilityWeatherFile.Caption = utilProg(curUtil).curWeatherFile
Call redrawUtilityTab
End Sub

'=======================================================
' Generic routine to start the text editor with any
' file name passed.
' This is similar to RunOutputEditorSingleFile but instead
' of just an extension the entire file name is passed.
'=======================================================
Sub RunTextEditorOnFile(fileToEdit As String)
If textEditFileName = "" Then
  MsgBox "No Text Editor Selected", vbExclamation, "Edit Cancelled"
  Exit Sub
End If
If checkIfFileExists(fileToEdit) Then
  On Error Resume Next
  Shell textEditFileName & " " & q & fileToEdit & q, vbNormalFocus
  If Err.Number <> 0 Then
    MsgBox "Error initiating text editor.  This is likely due to the text editor program being deleted or moved. Please use the FILE menu to select a new text editor program.", vbExclamation, "Edit Cancelled"
    Exit Sub
  End If
End If
End Sub

'=======================================================
' Edit the input file on the utility tab
'=======================================================
Private Sub cmdUtilityTextEdit_Click()
Dim curUtil As Integer
curUtil = cmbUtility.ListIndex + 1
Call RunTextEditorOnFile(utilProg(curUtil).curInputFile)
End Sub

'=======================================================
' Edit the output files on the utility tab
'=======================================================
Private Sub cmdUtilityOutput1_Click()
Dim curUtil As Integer
Dim outFileName As String
curUtil = cmbUtility.ListIndex + 1
outFileName = changeExtension(utilProg(curUtil).curInputFile, utilProg(curUtil).outExt1, utilProg(curUtil).fileSuffix)
Call RunUtilityOutputEditor(outFileName)
End Sub
Private Sub cmdUtilityOutput2_Click()
Dim curUtil As Integer
Dim outFileName As String
curUtil = cmbUtility.ListIndex + 1
outFileName = changeExtension(utilProg(curUtil).curInputFile, utilProg(curUtil).outExt2, utilProg(curUtil).fileSuffix)
Call RunUtilityOutputEditor(outFileName)
End Sub
Private Sub cmdUtilityOutput3_Click()
Dim curUtil As Integer
Dim outFileName As String
curUtil = cmbUtility.ListIndex + 1
If extensionOnly(utilProg(curUtil).curInputFile) <> "htm" Then
  outFileName = changeExtension(utilProg(curUtil).curInputFile, utilProg(curUtil).outExt3, utilProg(curUtil).fileSuffix)
Else
  outFileName = changeExtension(utilProg(curUtil).curInputFile, utilProg(curUtil).outExt3Alt, utilProg(curUtil).fileSuffix)
End If
Call RunUtilityOutputEditor(outFileName)
End Sub
Private Sub cmdUtilityOutput4_Click()
Dim curUtil As Integer
Dim outFileName As String
curUtil = cmbUtility.ListIndex + 1
outFileName = changeExtension(utilProg(curUtil).curInputFile, utilProg(curUtil).outExt4, utilProg(curUtil).fileSuffix)
Call RunUtilityOutputEditor(outFileName)
End Sub
Private Sub cmdUtilityOutput5_Click()
Dim curUtil As Integer
Dim outFileName As String
curUtil = cmbUtility.ListIndex + 1
outFileName = changeExtension(utilProg(curUtil).curInputFile, utilProg(curUtil).outExt5, utilProg(curUtil).fileSuffix)
Call RunUtilityOutputEditor(outFileName)
End Sub
Private Sub cmdUtilityOutput6_Click()
Dim curUtil As Integer
Dim outFileName As String
curUtil = cmbUtility.ListIndex + 1
outFileName = changeExtension(utilProg(curUtil).curInputFile, utilProg(curUtil).outExt6, utilProg(curUtil).fileSuffix)
Call RunUtilityOutputEditor(outFileName)
End Sub

'=======================================================
' open the appropriate output file viewer for the
' type of file used in the utility tab
'=======================================================
Sub RunUtilityOutputEditor(fileName As String)
Dim extension As String
extension = extensionOnly(fileName)
If extension = "html" Or extension = "htm" Then
 If htmlViewFileName = "" Then
    MsgBox "No HTML Browser Selected", vbExclamation, "Edit Cancelled"
    Exit Sub
  End If
  Shell htmlViewFileName & " " & q & fileName & q, vbNormalFocus
ElseIf extension = "csv" Then
  If spreadsheetFileName = "" Then
    MsgBox "No Spreadsheet Program Selected.  To view the spreadsheet you need a program that can display a CSV file.  You will now be prompted to locate such a program on your computer.", vbExclamation, "Run Cancelled"
    spreadsheetFileName = SelectApplication("Find Spreadsheet Program", spreadsheetFileName)
    If spreadsheetFileName = "" Then Exit Sub
  End If
  Shell spreadsheetFileName & " /e " & q & fileName & q, vbNormalFocus
Else
  Call RunTextEditorOnFile(fileName)
End If
End Sub

'=======================================================
' Change the extension on the input file to the new
' one specified. The new extension does not need a period
' but if it is included it is not repeated
'=======================================================
Function changeExtension(fileName As String, newExtension As String, suffixToRemove As String) As String
Dim s As String
Dim lenSuffix As Integer
s = NoExtension(fileName)
lenSuffix = Len(suffixToRemove)
If lenSuffix > 0 Then
  If UCase(Right(s, lenSuffix)) = UCase(suffixToRemove) Then
    s = Left(s, Len(s) - lenSuffix)
    changeExtension = s & newExtension
  Else
    changeExtension = s & newExtension
  End If
Else
  If InStr(newExtension, ".") > 0 Then
    changeExtension = s & newExtension
  Else
    changeExtension = s & "." & newExtension
  End If
End If
Debug.Print "change extension"
Debug.Print "   original file:"; fileName
Debug.Print "   new extension:"; newExtension
Debug.Print "  suffixToRemove:"; suffixToRemove
Debug.Print "          result:"; changeExtension
End Function

'=======================================================
' Run the utility program
'=======================================================
Private Sub cmdUtilityRun_Click()
Dim appFileName As String
Dim batchFileName As String
Dim utilInFileName As String
Dim utilOutFilename As String
Dim utilWthrFileName As String
Dim utilInPassNoExt As String
Dim utilWthrPassNoExt As String
Dim utilInNoExt As String
Dim utilWthrNoExt As String
Dim utilInExtOnly As String
Dim curUtil As Integer
Dim currentDirectory As String
Dim useBatch As Boolean
Dim useShortName As Boolean
Dim lineOfBatch As String
Dim obFN As Integer
Dim tbFN As Integer
Dim cmdLine As String
Dim handleUtil As Long
If numSimQueue > 0 Then
  MsgBox "Cannot use utility tab while simulations are running", vbInformation, "Run Utility"
  Exit Sub
End If
curUtil = cmbUtility.ListIndex + 1
If utilProg(curUtil).appIsSpreadsheet Then
  'application is a spreadsheet (View3D)
  'first check if spreadsheet application has been defined.
  If spreadsheetFileName = "" Then
    spreadsheetFileName = findProgramUsingExtension("xls")
  End If
  If spreadsheetFileName = "" Then
    MsgBox "No Spreadsheet Program Selected. Please select a spreadsheet program.  You will now be prompted to locate such a program on your computer.", vbExclamation, "Utility Cancelled"
    spreadsheetFileName = SelectApplication("Find Spreadsheet Program", spreadsheetFileName)
    If spreadsheetFileName = "" Then Exit Sub
  End If
  appFileName = appPath & utilProg(curUtil).applicationFile
  If checkIfFileExists(appFileName) Then
    On Error Resume Next
    Err.Clear
    ' now open any files that were selected
    Shell spreadsheetFileName & " /e " & q & appFileName & q, vbNormalFocus
    If Err.Number <> 0 Then
      MsgBox "Error initiating spreadsheet. This is likely due to the spreadsheet program being deleted or moved. Please use the FILE menu to select a new spreadsheet program.", vbExclamation, "Spreadsheet Cancelled"
      Exit Sub
    End If
  End If
Else
  'application is an exe or batch file
  'use local variables for file names
  appFileName = appPath & utilProg(curUtil).applicationFile
  utilInFileName = utilProg(curUtil).curInputFile
  utilWthrFileName = utilProg(curUtil).curWeatherFile
  If utilProg(curUtil).batchFile <> "" Then
    useBatch = True
    batchFileName = appPath & utilProg(curUtil).batchFile
  Else
    useBatch = False
    batchFileName = ""
  End If
  'check if files are present and does not contain invalid characters
  If Not checkIfFileExists(appFileName) Then
    MsgBox "Application not present: " & utilProg(curUtil).name, vbExclamation, "Utility Run Cancelled"
    Exit Sub
  End If
  If isContainsInvalids(appFileName) Then
    MsgBox "Utility file name characters: &^,=%" & Chr(34), vbExclamation, "Utility Run Cancelled"
    Exit Sub
  End If
  If useBatch Then
    If Not checkIfFileExists(batchFileName) Then
      MsgBox "Batch file needed for utility: " & utilProg(curUtil).name, vbExclamation, "Utility Run Cancelled"
      Exit Sub
    End If
    If isContainsInvalids(batchFileName) Then
      MsgBox "Batch file name contains characters: &^,=%" & Chr(34), vbExclamation, "Utility Run Cancelled"
      Exit Sub
    End If
  End If
  If utilProg(curUtil).enableInput Then
    If utilInFileName = "" Then
      MsgBox "No input file selected for utility: " & utilProg(curUtil).name, vbExclamation, "Utility Run Cancelled"
      Exit Sub
    End If
    If Not checkIfFileExists(utilInFileName) Then
      MsgBox "Specified file missing for utility: " & utilProg(curUtil).name & vbCrLf & vbCrLf & utilInFileName, vbExclamation, "Utility Run Cancelled"
      Exit Sub
    End If
    If isContainsInvalids(utilInFileName) Then
      MsgBox "Input file name contains characters: &^,=%" & Chr(34), vbExclamation, "Utility Run Cancelled"
      Exit Sub
    End If
  End If
  If utilProg(curUtil).enableWthr Then
    If utilWthrFileName = "" Then
      MsgBox "No weather file selected for utility: " & utilProg(curUtil).name, vbExclamation, "Utility Run Cancelled"
      Exit Sub
    End If
    If Not checkIfFileExists(utilWthrFileName) Then
      MsgBox "Specified weather file missing for utility: " & utilProg(curUtil).name & vbCrLf & vbCrLf & utilWthrFileName, vbExclamation, "Utility Run Cancelled"
      Exit Sub
    End If
    If isContainsInvalids(utilWthrFileName) Then
      MsgBox "Weather file name contains characters: &^,=%" & Chr(34), vbExclamation, "Utility Run Cancelled"
      Exit Sub
    End If
  End If
  'save current directory
  currentDirectory = CurDir
  'go to directory with utility
  If useBatch Then
    ChDrive Left(batchFileName, 1)
    ChDir pathOnly(batchFileName)
  Else
    ChDrive Left(appFileName, 1)
    ChDir pathOnly(appFileName)
  End If
  'Remove extensions from active files
  utilInNoExt = NoExtension(utilInFileName)
  utilInExtOnly = extensionOnly(utilInFileName)
  utilWthrNoExt = NoExtension(utilWthrFileName)
  'if international file use the short name instead
  'only need to worry about keeping track with input file
  'since need to rename it after running utility
  useShortName = checkIfInternationalFileName(utilInFileName)
  If useShortName Then
    utilInPassNoExt = NoExtension(ShortName(utilInFileName))
  Else
    utilInPassNoExt = NoExtension(utilInFileName)
  End If
  If checkIfInternationalFileName(utilWthrFileName) Then '
    utilWthrPassNoExt = NoExtension(ShortName(utilWthrFileName))
  Else
    utilWthrPassNoExt = NoExtension(utilWthrFileName)
  End If
  'check if output files are locked
  If utilProg(curUtil).outExt1 <> "" Then
    If checkIfFileIsLocked(utilInNoExt & utilProg(curUtil).outExt1) Then Exit Sub
  End If
  If utilProg(curUtil).outExt2 <> "" Then
    If checkIfFileIsLocked(utilInNoExt & utilProg(curUtil).outExt2) Then Exit Sub
  End If
  If utilProg(curUtil).outExt3 <> "" Then
    If checkIfFileIsLocked(utilInNoExt & utilProg(curUtil).outExt3) Then Exit Sub
  End If
  If utilProg(curUtil).outExt3Alt <> "" Then
    If checkIfFileIsLocked(utilInNoExt & utilProg(curUtil).outExt3Alt) Then Exit Sub
  End If
  If utilProg(curUtil).outExt4 <> "" Then
    If checkIfFileIsLocked(utilInNoExt & utilProg(curUtil).outExt4) Then Exit Sub
  End If
  If utilProg(curUtil).outExt5 <> "" Then
    If checkIfFileIsLocked(utilInNoExt & utilProg(curUtil).outExt5) Then Exit Sub
  End If
  If utilProg(curUtil).outExt6 <> "" Then
    If checkIfFileIsLocked(utilInNoExt & utilProg(curUtil).outExt6) Then Exit Sub
  End If
  'create temporary batch file
  On Error Resume Next
  If useBatch Then
    If CreateRunEPBatch Then 'create a batch file - used for most cases
      tbFN = FreeFile
      Open "RUNUTIL.BAT" For Output As tbFN
      If Err.Number > 0 Then
        MsgBox "Temporary batch file could not be created for utility: " & utilProg(curUtil).name, vbExclamation, "Utility Run Cancelled"
        Exit Sub
      End If
      'write the SET command to the top of the temporary batch file
      If utilProg(curUtil).enableInput Then
        If utilProg(curUtil).includeExtensionInCall Then
          Print #tbFN, "SET inout="; utilInPassNoExt & "." & utilInExtOnly
        Else
          Print #tbFN, "SET inout="; utilInPassNoExt
        End If
      End If
      If utilProg(curUtil).enableWthr Then
        Print #tbFN, "SET wthr="; utilWthrPassNoExt
      End If
      'copy the rest of the batch file
      obFN = FreeFile
      Open batchFileName For Input As obFN
      ' now copy the batch file contents to the temporary batch file
      Do While Not EOF(obFN)
        Line Input #obFN, lineOfBatch
        Print #tbFN, lineOfBatch
      Loop
      Close obFN
      Close tbFN
      cmdLine = "RUNUTIL.bat"
    Else 'don't create a batch file for users with security systems that don't allow batch file creation
      cmdLine = fileWithExt(batchFileName) & " "
      '%1 or %inout%
      If utilProg(curUtil).includeExtensionInCall Then
        cmdLine = cmdLine & q & utilInPassNoExt & "." & utilInExtOnly & q & " "
      Else
        cmdLine = cmdLine & q & utilInPassNoExt & q & " "
      End If
      '%2 or %wthr%
      cmdLine = cmdLine & q & utilWthrPassNoExt & q & " "
      '%3 or PARAM is a flag for the batch file
      cmdLine = cmdLine & " PARAM "
    End If
    'Run the batch file
    Debug.Print "Utility Command Line:"; cmdLine
    If curOSver = 1 Then  'windows 95/98
      ExecCmdWindowControl cmdLine, False 'this will call the batch file and has an environment defined.
    ElseIf curOSver = 2 Then  'windows NT/2000/XP/Vista(?)
      ExecCmdWindowControl "CMD /e:3000 /c " & cmdLine, False
'DEBUGGING ONLY
'     The /k option instead of /c does not close the command window (need to type exit to proceed)
'      ExecCmdWindowControl "CMD /e:3000 /k " & cmdLine, False
    Else  'if something else just try it with default environment sizie
      ExecCmdWindowControl cmdLine, False
    End If
    If Err.Number > 0 Then
      MsgBox "Temporary Batch File Could Not Be Run", vbExclamation, "Utility Run Cancelled"
      Exit Sub
    End If
  Else 'no batch file needed
    Err.Clear
    If utilProg(curUtil).useInputAsExeArgument Then
      If utilProg(curUtil).includeExtensionInCall Then
        If utilProg(curUtil).waitUntilUtilityExits Then
          ExecCmdWindowControl appFileName & " " & q & utilInPassNoExt & "." & utilInExtOnly & q, False
        Else
          handleUtil = ExecCmdNoWait(0, appFileName & " " & q & utilInPassNoExt & "." & utilInExtOnly & q, False)
        End If
      Else
        If utilProg(curUtil).waitUntilUtilityExits Then
          ExecCmdWindowControl appFileName & " " & q & utilInPassNoExt & q, False
        Else
          handleUtil = ExecCmdNoWait(0, appFileName & " " & q & utilInPassNoExt & q, False)
        End If
      End If
    Else
      If utilProg(curUtil).waitUntilUtilityExits Then
        ExecCmdWindowControl appFileName, False
      Else
        handleUtil = ExecCmdNoWait(0, appFileName, False)
      End If
    End If
    If Err.Number <> 0 Then
      MsgBox "Error initiating utility: " & utilProg(curUtil).name & " using " & appFileName, vbExclamation, "Utility Run Cancelled"
      Exit Sub
    End If
  End If
  'fix international file names
  If useShortName Then
    If utilProg(curUtil).outExt1 <> "" Then
      Name utilInPassNoExt & utilProg(curUtil).outExt1 As utilInNoExt & utilProg(curUtil).outExt1
    End If
    If utilProg(curUtil).outExt2 <> "" Then
      Name utilInPassNoExt & utilProg(curUtil).outExt2 As utilInNoExt & utilProg(curUtil).outExt2
    End If
    If utilProg(curUtil).outExt3 <> "" Then
      Name utilInPassNoExt & utilProg(curUtil).outExt3 As utilInNoExt & utilProg(curUtil).outExt3
    End If
    If utilProg(curUtil).outExt3Alt <> "" Then
      Name utilInPassNoExt & utilProg(curUtil).outExt3Alt As utilInNoExt & utilProg(curUtil).outExt3Alt
    End If
    If utilProg(curUtil).outExt4 <> "" Then
      Name utilInPassNoExt & utilProg(curUtil).outExt4 As utilInNoExt & utilProg(curUtil).outExt4
    End If
    If utilProg(curUtil).outExt5 <> "" Then
      Name utilInPassNoExt & utilProg(curUtil).outExt5 As utilInNoExt & utilProg(curUtil).outExt5
    End If
    If utilProg(curUtil).outExt6 <> "" Then
      Name utilInPassNoExt & utilProg(curUtil).outExt6 As utilInNoExt & utilProg(curUtil).outExt6
    End If
  End If
  Err.Clear 'likely that one of these renaming commands did not run but we don't care
  'restore current directory
  ChDir currentDirectory
End If
Call redrawUtilityTab
End Sub

'=======================================================
' Activate IDF Editor with the special IDD for a utility
' and the active file.
'=======================================================
Private Sub cmdUtilityIDFEdit_Click()
Dim olddir
Dim curUtil As Integer
curUtil = cmbUtility.ListIndex + 1
On Error Resume Next
olddir = CurDir
ChDrive Left(appPath, 1)
ChDir appPath & "preprocess\idfeditor\"
'MsgBox "current directory: " & CurDir, vbExclamation
'MsgBox "files:" & vbCrLf & utilProg(curUtil).curInputFile & vbCrLf & utilProg(curUtil).IDFEdOpt, vbInformation, "UtilityIDFEdit"
Shell "idfeditor.exe " & utilProg(curUtil).curInputFile & utilProg(curUtil).IDFEdOpt, vbNormalFocus
ChDrive Left(olddir, 1)
ChDir olddir
'Shell "c:\energyplus\preprocess\idfeditor\idfeditor.exe " & inputFileName
If Err.Number <> 0 Then
  MsgBox "Error initiating IDFEditor.  This is likely due to the IDF Editor program being deleted or moved. Please confirm that it is in the Preprocessor\IDF Editor subdirectory of the installed EnergyPlus directory." & inputFileName, vbExclamation, "Edit Cancelled"
  Exit Sub
End If
End Sub

'=======================================================
' Check for updates to EnergyPlus on the internet
'=======================================================
Sub checkForUpdatesNow(isManualCheck As Boolean)
Dim pageText As String
Dim lines() As String
Dim isInTOC As Boolean
Dim endOfAnchor As Integer
Dim anchor As String
Dim firstNewAnchor As String
Dim lastNewAnchor As String
Dim foundOldAnchor As Boolean
Dim response As Integer
Dim doCheck As Boolean
Dim i As Integer
On Error GoTo updateError
lblCheckingUpdates.Visible = True
'-----------------------------------------
'for debugging only
'updateLastAnchor = "#4.0.0-0000"
'updateLastAnchor = "#4.0.0-0001"
'updateLastAnchor = "#4.0.0-0002"
'updateLastAnchor = "#4.0.0-0003"
'updateLastAnchor = "#4.0.0-this is a long anchor"
'updateLastAnchor = ""
' normally updatePageURL is set by registry settings - this overrides it.
'updatePageURL = "http://gard.com/ep/epupdate.htm"
'updatePageURL = "http://gard.com/ep/epupdateLF.htm"
'updatePageURL = "http://gard.com/ep/epupdateCRLF.htm"
'updatePageURL = "http://gard.com/ep/epupdateCR.htm"
'updatePageURL = "http://gard.com/ep/junk.htm"
'updatePageURL = "http://eeredevapps1.nrel.gov/buildings/energyplus/update/epupdate.htm"
'updatePageURL = "http://energyplus.net/epupdate.htm"
'updateLastDate = "9/2/2009 12:47:47 AM"
'-----------------------------------------

'check every seven days
If Not IsDate(updateLastDate) Then
  doCheck = True
ElseIf DateDiff("d", updateLastDate, Now) >= 7 Or isManualCheck Then
  doCheck = True
Else
  doCheck = False
End If
If doCheck Then
  pageText = Inet1.OpenURL(updatePageURL, icString)
  'added to allow for LF (unix), CRLF (msdos), or CR (macintosh) line endings
  If InStr(pageText, vbCrLf) > 0 Then
    lines = Split(pageText, vbCrLf)
  ElseIf InStr(pageText, vbCr) > 0 Then
    lines = Split(pageText, vbCr)
  ElseIf InStr(pageText, vbLf) > 0 Then
    lines = Split(pageText, vbLf)
  Else 'assume lf if nothing else?
    lines = Split(pageText, vbLf)
  End If
  isInTOC = False
  foundOldAnchor = False
  If updateLastAnchor = "" Then foundOldAnchor = True
  For i = 1 To UBound(lines)
    lines(i) = Trim(lines(i))
    If lines(i) = "<!-- ENDLIST -->" Then
      isInTOC = False
      Exit For
    End If
    If isInTOC Then
      If Left(lines(i), 11) = "<br><a href" Then
        endOfAnchor = InStr(11, lines(i), ">")
        If endOfAnchor > 11 Then
          anchor = Mid(lines(i), 14, endOfAnchor - 15)
          Debug.Print "Anchor found: "; anchor
          If foundOldAnchor Then
            lastNewAnchor = anchor
            If firstNewAnchor = "" Then firstNewAnchor = anchor
          End If
          If anchor = updateLastAnchor Then
            Debug.Print "Last anchor found"
            foundOldAnchor = True
          End If
        End If
      End If
    End If
    If lines(i) = "<!-- STARTLIST -->" Then
      isInTOC = True
    End If
  Next i
  'check if any new anchors (updates) are available
  If (firstNewAnchor <> "" Or Not foundOldAnchor) And pageText <> "" Then
    response = MsgBox("An update to the EnergyPlus simulation program, files, or utilities is now available." & _
                      vbCrLf & vbCrLf & "Would you like to review the updates available?", vbYesNo + vbCritical, "Check for Updates")
    If response = vbYes Then
      Call viewWebPage(updatePageURL & firstNewAnchor)
      If foundOldAnchor Then
        updateLastAnchor = lastNewAnchor
      Else
        updateLastAnchor = anchor 'if the old anchor was never found, just set it to the last anchor ever used.
      End If
    End If
    Debug.Print "first new anchor: "; firstNewAnchor
    Debug.Print "last new achnor:  "; lastNewAnchor
  Else
    If isManualCheck Then
      MsgBox "No new updates listed since the last check", vbInformation + vbOKOnly, "Check for Updates"
    End If
  End If
  updateLastDate = Now 'always change the date to today so that the user is not reminded every start up.
Else
  Debug.Print "Within seven days of last update check."
End If
lblCheckingUpdates.Visible = False
Exit Sub

updateError:
Debug.Print "check for updates now error: ", Err.Number, Err.Description
lblCheckingUpdates.Visible = False
End Sub

'=======================================================
' View a web page given the full URL as the input parameter
'=======================================================
Sub viewWebPage(page As String)
If htmlViewFileName = "" Then
  Exit Sub
End If
On Error Resume Next
Err.Clear
If Err.Number = 0 Then
    Shell htmlViewFileName & " " & q & page & q, vbNormalFocus
    If Err.Number <> 0 Then
        MsgBox "Error initiating HTML browser. This is likely due to the HTML browser program being deleted or moved. Please go to VIEW OPTIONS to select a new HTML BROWSER program.", vbExclamation, "Edit Cancelled"
        Exit Sub
    End If
End If
End Sub

'=======================================================
' Report the dates of all files in the EnergyPlus
' directory and subdirectories
'=======================================================
Sub reportFileDates()
Dim curFile As String
Dim curFileWpath As String
Dim curFileTime As String
Dim curFold As String
Dim curFoldWpath As String
Dim foundNewFold As Boolean
Dim curParent As String
Dim i As Integer

Open appPath & "filedatelist.txt" For Output As 23
'establish original list of folders
ReDim folderList(2)
numFolderList = 0
i = 1
curParent = appPath
Call addToFolderList(curParent)
Do
  curFold = dir(curParent & "*.*", vbDirectory)
  Do While curFold <> ""
    curFoldWpath = curParent & curFold
    If GetAttr(curFoldWpath) = vbDirectory Then
      If curFold <> "." And curFold <> ".." Then
        Print #23, curFoldWpath
        Call addToFolderList(curFoldWpath & "\")
      End If
    End If
    curFold = dir
  Loop
  i = i + 1
  If i > numFolderList Then Exit Do
  curParent = folderList(i)
Loop
For i = 1 To numFolderList
  curFile = dir(folderList(i) & "*.*")
  Do While curFile <> ""
    curFileWpath = folderList(i) & curFile
    curFileTime = FileDateTime(curFileWpath)
    Print #23, curFileTime, curFileWpath
    curFile = dir
  Loop
Next i
Close 23
End Sub


'=======================================================
' Add a folder to the list of folders and redim array
' if necessary
'=======================================================
Sub addToFolderList(folderIn As String)
Dim sizeFolderList As Integer
'resize array if needed
sizeFolderList = UBound(folderList)
If numFolderList >= sizeFolderList Then
  ReDim Preserve folderList(sizeFolderList * 2)
End If
'assign to next spot in array
numFolderList = numFolderList + 1
folderList(numFolderList) = folderIn
End Sub

'=======================================================
' Removes the last slash and what ever is right of it
' This effectively is like moving up one directory for a string
' that does not end in a slash.  If it does end in a slash it simply
' removes it.  If no slash is found than it returns the original string.
'=======================================================
Function upDirectory(inPath As String) As String
Dim p As String
Dim i As Long, lastSlash As Long
p = inPath
For i = 1 To Len(p)
  If Mid(p, i, 1) = "\" Then lastSlash = i
Next i
If lastSlash > 2 Then
  upDirectory = Left(p, lastSlash - 1)
Else
  upDirectory = p
End If
End Function

'-----------------------------------------------------------------------------
' Abstracts the reading of a line of file to enable reading files with
' mixed carriage return and line feed characters
'-----------------------------------------------------------------------------
Sub getNextLine(fileHandle As Integer, readLine As String, EOFflag As Boolean, Optional clearBuffer As Boolean = False)
Static readBuffer As String
Dim posLineBreak As Integer
Dim lineIn As String
If clearBuffer Then readBuffer = ""
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

'=======================================================
' Check the application directory and see if it is likely
' to cause problems because of space or not being
' able to write files there (such as when EnergyPlus
' is installed in c:\program files\EnergyPlus
'=======================================================
Sub checkIfApplicationDirectoryGood()
' assume it is a good directory
Dim directoryCanBeWritten As Boolean
Dim outFN As Integer
Dim testFile As String
directoryCanBeWritten = True
Debug.Print "checkIfApplicationDirectoryGood apppath:", appPath
On Error Resume Next
Err.Clear
outFN = FreeFile
testFile = "test.bat"
' MsgBox appPath & testFile
Open appPath & testFile For Output As outFN
If Err.Number <> 0 Then
    directoryCanBeWritten = False
End If
Close outFN
Kill appPath & testFile
If Err.Number <> 0 Then
    directoryCanBeWritten = False
End If
If Not directoryCanBeWritten Then
    MsgBox "Invalid application directory:" + vbCrLf + vbCrLf + appPath + vbCrLf + vbCrLf + "You should consider uninstalling EnergyPlus and installing it in a directory such as c:\EnergyPlusVx-x-x that requires no special permission to write files.", vbInformation, "EP-Launch ERROR"
ElseIf InStr(appPath, " ") > 0 And Not CreateRunEPBatch Then
    MsgBox "Invalid application directory:" + vbCrLf + vbCrLf + appPath + vbCrLf + vbCrLf + "You should consider uninstalling EnergyPlus and installing it in a directory such as c:\EnergyPlusVx-x-x that has no spaces in the path. As an alternative you can also go to VIEW .. OPTIONS .. MISCELLANEOUS and check Create Batch File to Run EnergyPlus.", vbInformation, "EP-Launch ERROR"
End If
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
