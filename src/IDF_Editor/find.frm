VERSION 5.00
Begin VB.Form frmFind
   BorderStyle     =   3  'Fixed Dialog
   Caption         =   "Find Class"
   ClientHeight    =   1140
   ClientLeft      =   2760
   ClientTop       =   3750
   ClientWidth     =   6840
   LinkTopic       =   "Form1"
   MaxButton       =   0   'False
   MinButton       =   0   'False
   ScaleHeight     =   1140
   ScaleWidth      =   6840
   ShowInTaskbar   =   0   'False
   Begin VB.TextBox txtFindInput
      Height          =   285
      Left            =   1440
      TabIndex        =   3
      Top             =   120
      Width           =   3975
   End
   Begin VB.CommandButton OKButton
      Caption         =   "Find"
      Default         =   -1  'True
      Height          =   375
      Left            =   5520
      TabIndex        =   1
      Top             =   120
      Width           =   1215
   End
   Begin VB.CommandButton CancelButton
      Cancel          =   -1  'True
      Caption         =   "Cancel"
      Height          =   375
      Left            =   5520
      TabIndex        =   0
      Top             =   600
      Width           =   1215
   End
   Begin VB.Label Label1
      Caption         =   "Find What:"
      Height          =   255
      Left            =   135
      TabIndex        =   2
      Top             =   165
      Width           =   1440
   End
End
Attribute VB_Name = "frmFind"
Attribute VB_GlobalNameSpace = False
Attribute VB_Creatable = False
Attribute VB_PredeclaredId = True
Attribute VB_Exposed = False

Option Explicit

Private Sub CancelButton_Click()
searchTerm = ""
Unload Me
End Sub

Private Sub Form_Activate()
txtFindInput.SetFocus
End Sub

Private Sub OKButton_Click()
searchTerm = txtFindInput.Text
Unload Me
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
