VERSION 5.00
Begin VB.Form crashMessage
   BorderStyle     =   4  'Fixed ToolWindow
   Caption         =   " "
   ClientHeight    =   8100
   ClientLeft      =   45
   ClientTop       =   285
   ClientWidth     =   6630
   ControlBox      =   0   'False
   LinkTopic       =   "Form1"
   MaxButton       =   0   'False
   MinButton       =   0   'False
   ScaleHeight     =   8100
   ScaleWidth      =   6630
   ShowInTaskbar   =   0   'False
   StartUpPosition =   1  'CenterOwner
   Begin VB.PictureBox Picture2
      AutoSize        =   -1  'True
      BorderStyle     =   0  'None
      Height          =   480
      Left            =   6000
      Picture         =   "crashMessage.frx":0000
      ScaleHeight     =   480
      ScaleWidth      =   480
      TabIndex        =   5
      Top             =   240
      Width           =   480
   End
   Begin VB.PictureBox Picture1
      AutoSize        =   -1  'True
      BorderStyle     =   0  'None
      Height          =   480
      Left            =   240
      Picture         =   "crashMessage.frx":0442
      ScaleHeight     =   480
      ScaleWidth      =   480
      TabIndex        =   1
      Top             =   240
      Width           =   480
   End
   Begin VB.CommandButton cmdOK
      Caption         =   "OK"
      Default         =   -1  'True
      Height          =   375
      Left            =   2520
      TabIndex        =   0
      Top             =   7680
      Width           =   1455
   End
   Begin VB.Label Label4
      Caption         =   $"crashMessage.frx":0884
      Height          =   855
      Left            =   240
      TabIndex        =   14
      Top             =   3720
      Width           =   6135
      WordWrap        =   -1  'True
   End
   Begin VB.Line Line4
      X1              =   120
      X2              =   6600
      Y1              =   3600
      Y2              =   3600
   End
   Begin VB.Label Label1
      Caption         =   "Check the EnergyPlus Installation"
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
      TabIndex        =   13
      Top             =   3360
      Width           =   4455
   End
   Begin VB.Label Label6
      Caption         =   "You are using version: "
      Height          =   255
      Left            =   240
      TabIndex        =   12
      Top             =   2520
      Width           =   4215
   End
   Begin VB.Line Line3
      X1              =   120
      X2              =   6480
      Y1              =   6600
      Y2              =   6600
   End
   Begin VB.Label Label10
      Caption         =   "Check for System Messages"
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
      Top             =   6360
      Width           =   2535
   End
   Begin VB.Line Line2
      X1              =   120
      X2              =   6480
      Y1              =   4920
      Y2              =   4920
   End
   Begin VB.Line Line1
      X1              =   120
      X2              =   6480
      Y1              =   1200
      Y2              =   1200
   End
   Begin VB.Label Label9
      Caption         =   "About the Crash"
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
      TabIndex        =   10
      Top             =   4680
      Width           =   2535
   End
   Begin VB.Label Label8
      Caption         =   "Crash Data"
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
      TabIndex        =   9
      Top             =   960
      Width           =   2535
   End
   Begin VB.Label Label5
      Caption         =   $"crashMessage.frx":09A6
      Height          =   735
      Left            =   240
      TabIndex        =   8
      Top             =   6720
      Width           =   6135
      WordWrap        =   -1  'True
   End
   Begin VB.Label Label7
      Caption         =   $"crashMessage.frx":0A5D
      Height          =   1215
      Left            =   240
      TabIndex        =   7
      Top             =   5040
      Width           =   6135
      WordWrap        =   -1  'True
   End
   Begin VB.Label lblEnergyPlusVersion
      Caption         =   "    lblEnergyPlusVersion"
      Height          =   255
      Left            =   240
      TabIndex        =   6
      Top             =   2880
      Width           =   6375
   End
   Begin VB.Label lblInputFile
      Caption         =   $"crashMessage.frx":0C10
      Height          =   735
      Left            =   480
      TabIndex        =   4
      Top             =   1680
      Width           =   5895
   End
   Begin VB.Label Label3
      Caption         =   "The active EnergyPlus input file is named:"
      Height          =   255
      Left            =   240
      TabIndex        =   3
      Top             =   1320
      Width           =   4215
   End
   Begin VB.Label Label2
      Alignment       =   2  'Center
      Caption         =   "EnergyPlus Crashed"
      BeginProperty Font
         Name            =   "MS Sans Serif"
         Size            =   9.75
         Charset         =   0
         Weight          =   700
         Underline       =   0   'False
         Italic          =   0   'False
         Strikethrough   =   0   'False
      EndProperty
      Height          =   375
      Left            =   840
      TabIndex        =   2
      Top             =   360
      Width           =   4935
   End
End
Attribute VB_Name = "crashMessage"
Attribute VB_GlobalNameSpace = False
Attribute VB_Creatable = False
Attribute VB_PredeclaredId = True
Attribute VB_Exposed = False
Option Explicit

Private Sub cmdOK_Click()
Unload Me
End Sub

Private Sub Form_Load()
If Len(eplUI.inputFileName) > 70 Then
  lblInputFile.Caption = simpleWrap(eplUI.crashFileName, 70)
Else
  lblInputFile.Caption = eplUI.crashFileName
End If
lblEnergyPlusVersion.Caption = "    " & eplUI.EnergyPlusVer
Call eplUI.checkIfApplicationDirectoryGood
End Sub

Private Function simpleWrap(strIn As String, wrapAt As Integer) As String
Dim strOut As String
Dim i As Integer
For i = 1 To Len(strIn)
  strOut = strOut & Mid(strIn, i, 1)
  If i Mod wrapAt = 0 Then
    strOut = strOut & vbCrLf
  End If
Next i
simpleWrap = strOut
End Function


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
Private Sub Label8_Click()

End Sub
