VERSION 5.00
Begin VB.Form frmRunStatus
   BorderStyle     =   3  'Fixed Dialog
   Caption         =   "EnergyPlus Run Status"
   ClientHeight    =   3600
   ClientLeft      =   2760
   ClientTop       =   3750
   ClientWidth     =   13860
   LinkTopic       =   "Form1"
   MaxButton       =   0   'False
   MinButton       =   0   'False
   ScaleHeight     =   3600
   ScaleWidth      =   13860
   ShowInTaskbar   =   0   'False
   StartUpPosition =   1  'CenterOwner
   Begin VB.ListBox lstStatus
      Height          =   2790
      Left            =   840
      TabIndex        =   2
      Top             =   120
      Width           =   12855
   End
   Begin VB.PictureBox Picture1
      BorderStyle     =   0  'None
      Height          =   615
      Left            =   240
      Picture         =   "frmRunStatus.frx":0000
      ScaleHeight     =   615
      ScaleWidth      =   495
      TabIndex        =   1
      Top             =   120
      Width           =   495
   End
   Begin VB.CommandButton OKButton
      Caption         =   "OK"
      Default         =   -1  'True
      Height          =   375
      Left            =   12480
      TabIndex        =   0
      Top             =   3120
      Width           =   1215
   End
End
Attribute VB_Name = "frmRunStatus"
Attribute VB_GlobalNameSpace = False
Attribute VB_Creatable = False
Attribute VB_PredeclaredId = True
Attribute VB_Exposed = False

Option Explicit

Private Sub OKButton_Click()
Unload Me
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
