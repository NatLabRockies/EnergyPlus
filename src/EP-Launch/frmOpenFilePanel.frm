VERSION 5.00
Begin VB.Form frmOpenFilePanel
   BorderStyle     =   4  'Fixed ToolWindow
   Caption         =   "Quick Open Panel for Single Simulation"
   ClientHeight    =   2400
   ClientLeft      =   45
   ClientTop       =   285
   ClientWidth     =   6510
   LinkTopic       =   "Form1"
   MaxButton       =   0   'False
   MinButton       =   0   'False
   ScaleHeight     =   2400
   ScaleWidth      =   6510
   ShowInTaskbar   =   0   'False
   StartUpPosition =   3  'Windows Default
   Begin VB.CommandButton cmdEDD
      Caption         =   "EDD"
      Height          =   255
      Left            =   5400
      TabIndex        =   30
      ToolTipText     =   "HVAC branch and node details"
      Top             =   120
      Width           =   900
   End
   Begin VB.CommandButton cmdProcCSV
      Caption         =   "Proc CSV"
      Height          =   255
      Left            =   4440
      TabIndex        =   29
      ToolTipText     =   "Processed CSV file with simple statistics from main CSV file."
      Top             =   2040
      Width           =   900
   End
   Begin VB.CommandButton cmdMDD
      Caption         =   "MDD"
      Height          =   255
      Left            =   1080
      TabIndex        =   28
      ToolTipText     =   "Report data dictionary containing variables that may be requested."
      Top             =   840
      Width           =   900
   End
   Begin VB.CommandButton cmdVRML
      Caption         =   "VRML"
      Height          =   255
      Left            =   3240
      TabIndex        =   27
      ToolTipText     =   "DElight file"
      Top             =   1680
      Width           =   900
   End
   Begin VB.CommandButton cmdSHD
      Caption         =   "SHD"
      Height          =   255
      Left            =   3240
      TabIndex        =   26
      ToolTipText     =   "DElight file"
      Top             =   1320
      Width           =   900
   End
   Begin VB.CommandButton cmdScreen
      Caption         =   "Screen"
      Height          =   255
      Left            =   3240
      TabIndex        =   25
      ToolTipText     =   "DElight file"
      Top             =   840
      Width           =   900
   End
   Begin VB.CommandButton cmdDFDMP
      Caption         =   "DFDMP"
      Height          =   255
      Left            =   3240
      TabIndex        =   24
      ToolTipText     =   "DElight file"
      Top             =   480
      Width           =   900
   End
   Begin VB.CommandButton cmdELDMP
      Caption         =   "ELDMP"
      Height          =   255
      Left            =   3240
      TabIndex        =   23
      ToolTipText     =   "DElight file"
      Top             =   120
      Width           =   900
   End
   Begin VB.CommandButton cmdOUT
      Caption         =   "DE OUT"
      Height          =   255
      Left            =   2280
      TabIndex        =   22
      ToolTipText     =   "DElight output file."
      Top             =   480
      Width           =   900
   End
   Begin VB.CommandButton cmdIN
      Caption         =   "DE IN"
      Height          =   255
      Left            =   2280
      TabIndex        =   21
      ToolTipText     =   "DElight input file."
      Top             =   120
      Width           =   900
   End
   Begin VB.CommandButton cmdDXF
      Caption         =   "DXF"
      Height          =   255
      Left            =   120
      TabIndex        =   20
      ToolTipText     =   "DXF drawing of building surfaces"
      Top             =   2040
      Width           =   900
   End
   Begin VB.CommandButton cmdMAP
      Caption         =   "MAP"
      Height          =   255
      Left            =   2280
      TabIndex        =   19
      ToolTipText     =   "Daylighting map."
      Top             =   840
      Width           =   900
   End
   Begin VB.CommandButton cmdEXPIDF
      Caption         =   "EXPIDF"
      Height          =   255
      Left            =   2280
      TabIndex        =   18
      ToolTipText     =   "ExpandObjects idf file output"
      Top             =   1320
      Width           =   900
   End
   Begin VB.CommandButton cmdEPMDET
      Caption         =   "EPMDET"
      Height          =   255
      Left            =   2280
      TabIndex        =   17
      ToolTipText     =   "EPMacro echo of input with errors and macro expansions"
      Top             =   2040
      Width           =   900
   End
   Begin VB.CommandButton cmdEPMIDF
      Caption         =   "EPMIDF"
      Height          =   255
      Left            =   2280
      TabIndex        =   16
      ToolTipText     =   "EPMacro idf file output"
      Top             =   1680
      Width           =   900
   End
   Begin VB.CommandButton cmdSVG
      Caption         =   "SVG"
      Height          =   255
      Left            =   120
      TabIndex        =   15
      ToolTipText     =   "Drawing of HVAC system layout."
      Top             =   1680
      Width           =   900
   End
   Begin VB.CommandButton cmdDBG
      Caption         =   "DBG"
      Height          =   255
      Left            =   4440
      TabIndex        =   14
      ToolTipText     =   "Debug output"
      Top             =   480
      Width           =   900
   End
   Begin VB.CommandButton cmdSLN
      Caption         =   "SLN"
      Height          =   255
      Left            =   4440
      TabIndex        =   13
      ToolTipText     =   "Surface lines and coordinates."
      Top             =   840
      Width           =   900
   End
   Begin VB.CommandButton cmdAudit
      Caption         =   "Audit"
      Height          =   255
      Left            =   3240
      TabIndex        =   12
      ToolTipText     =   "Echo of input with errors, warnings, and filled-in defaults"
      Top             =   2040
      Width           =   900
   End
   Begin VB.CommandButton cmdSSZ
      Caption         =   "SSZ"
      Height          =   255
      Left            =   1080
      TabIndex        =   11
      ToolTipText     =   "System sizing and flow rates"
      Top             =   2040
      Width           =   900
   End
   Begin VB.CommandButton cmdZSZ
      Caption         =   "ZSZ"
      Height          =   255
      Left            =   1080
      TabIndex        =   10
      ToolTipText     =   "Zone sizing and flow rates"
      Top             =   1680
      Width           =   900
   End
   Begin VB.CommandButton cmdMeter
      Caption         =   "Meter"
      Height          =   255
      Left            =   120
      TabIndex        =   9
      ToolTipText     =   "Report Meter and Report MeterFileOnly outputs (Meter.csv, Meter.txt, or Meter.tab)"
      Top             =   480
      Width           =   900
   End
   Begin VB.CommandButton cmdMTD
      Caption         =   "MTD"
      Height          =   255
      Left            =   1080
      TabIndex        =   8
      ToolTipText     =   "Meter details to see what is on each meter"
      Top             =   1320
      Width           =   900
   End
   Begin VB.CommandButton cmdMTR
      Caption         =   "MTR"
      Height          =   255
      Left            =   4440
      TabIndex        =   7
      ToolTipText     =   "Report Meter and Report MeterFileOnly outputs before processing into Main Results file"
      Top             =   1680
      Width           =   900
   End
   Begin VB.CommandButton cmdBND
      Caption         =   "BND"
      Height          =   255
      Left            =   4440
      TabIndex        =   6
      ToolTipText     =   "HVAC branch and node details"
      Top             =   120
      Width           =   900
   End
   Begin VB.CommandButton cmdEIO
      Caption         =   "EIO"
      Height          =   255
      Left            =   120
      TabIndex        =   5
      ToolTipText     =   "Standard and optional reports summarizing inputs, environments, constructions, sizing, etc"
      Top             =   1320
      Width           =   900
   End
   Begin VB.CommandButton cmdRDD
      Caption         =   "RDD"
      Height          =   255
      Left            =   1080
      TabIndex        =   4
      ToolTipText     =   "Report data dictionary containing variables that may be requested."
      Top             =   480
      Width           =   900
   End
   Begin VB.CommandButton cmdESO
      Caption         =   "ESO"
      Height          =   255
      Left            =   4440
      TabIndex        =   3
      ToolTipText     =   "Report Variable and Report Meter outputs before processing into Main Results file"
      Top             =   1320
      Width           =   900
   End
   Begin VB.CommandButton cmdTable
      Caption         =   "Table"
      Height          =   255
      Left            =   120
      TabIndex        =   2
      ToolTipText     =   "Results summarized into monthly and binned reports."
      Top             =   840
      Width           =   900
   End
   Begin VB.CommandButton cmdERR
      Caption         =   "ERR"
      Height          =   255
      Left            =   1080
      TabIndex        =   1
      ToolTipText     =   "EnergyPlus error messages and warnings"
      Top             =   120
      Width           =   900
   End
   Begin VB.CommandButton cmdMain
      Caption         =   "CSV"
      Height          =   255
      Left            =   120
      TabIndex        =   0
      ToolTipText     =   "Report variable and report meter outputs (.csv, .txt, or .tab)"
      Top             =   120
      Width           =   900
   End
End
Attribute VB_Name = "frmOpenFilePanel"
Attribute VB_GlobalNameSpace = False
Attribute VB_Creatable = False
Attribute VB_PredeclaredId = True
Attribute VB_Exposed = False
Option Explicit

Private Sub cmdAudit_Click()
Call eplUI.RunOutputEditorSingleFile(".AUDIT")
End Sub

Private Sub cmdBND_Click()
Call eplUI.RunOutputEditorSingleFile(".BND")
'eplUI.SetFocus
End Sub

Private Sub cmdDBG_Click()
Call eplUI.RunOutputEditorSingleFile(".DBG")
End Sub

Private Sub cmdDFDMP_Click()
Call eplUI.RunOutputEditorSingleFile("DElight.dfdmp")
End Sub

Private Sub cmdDXF_Click()
Call eplUI.runViewDrawing
End Sub

Private Sub cmdEDD_Click()
Call eplUI.RunOutputEditorSingleFile(".EDD")
End Sub

Private Sub cmdEIO_Click()
Call eplUI.RunOutputEditorSingleFile(".EIO")
End Sub

Private Sub cmdELDMP_Click()
Call eplUI.RunOutputEditorSingleFile("DElight.eldmp")
End Sub

Private Sub cmdEPMDET_Click()
Call eplUI.RunOutputEditorSingleFile(".EPMDET")
End Sub

Private Sub cmdEPMIDF_Click()
Call eplUI.RunOutputEditorSingleFile(".EPMIDF")
End Sub

Private Sub cmdERR_Click()
Call eplUI.RunOutputEditorSingleFile(".ERR")
End Sub

Private Sub cmdESO_Click()
Call eplUI.showESOfile
End Sub

Private Sub cmdEXPIDF_Click()
Call eplUI.RunOutputEditorSingleFile(".EXPIDF")
End Sub


Private Sub cmdIN_Click()
Call eplUI.RunOutputEditorSingleFile("DElight.IN")
End Sub

Private Sub cmdMain_Click()
Call eplUI.viewMainCSV
End Sub

Private Sub cmdMAP_Click()
Call eplUI.runOutputSpreadsheetSingleFile("Map.csv")
End Sub


Private Sub cmdMeter_Click()
Call eplUI.viewMETERfile
End Sub

Private Sub cmdMTD_Click()
Call eplUI.RunOutputEditorSingleFile(".MTD")
End Sub

Private Sub cmdMTR_Click()
Call eplUI.RunOutputEditorSingleFile(".MTR")
End Sub

Private Sub cmdOUT_Click()
Call eplUI.RunOutputEditorSingleFile("DElight.out")
End Sub

Private Sub cmdProcCSV_Click()
Call eplUI.runOutputSpreadsheetSingleFile("-Proc.csv")
End Sub

Private Sub cmdRDD_Click()
Call eplUI.RunOutputEditorSingleFile(".RDD")
End Sub

Private Sub cmdMDD_Click()
Call eplUI.RunOutputEditorSingleFile(".MDD")
End Sub

Private Sub cmdScreen_Click()
Call eplUI.runOutputSpreadsheetSingleFile("Screen.csv")
End Sub

Private Sub cmdSHD_Click()
Call eplUI.RunOutputEditorSingleFile(".SHD")
End Sub

Private Sub cmdSLN_Click()
Call eplUI.RunOutputEditorSingleFile(".SLN")
End Sub

Private Sub cmdSSZ_Click()
Call eplUI.viewSSZFile
End Sub

Private Sub cmdSVG_Click()
Call eplUI.showSingleSVGFile(".SVG")
End Sub

Private Sub cmdTable_Click()
Call eplUI.viewTABLEfile
End Sub

Private Sub cmdVRML_Click()
Call eplUI.runOutputVRMLdisplay
End Sub

Private Sub cmdZSZ_Click()
Call eplUI.viewZSZFile
End Sub

Private Sub Form_Unload(Cancel As Integer)
Me.Hide
eplUI.viewQuickOpenPanel = False
Cancel = True
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
