VERSION 5.00
Begin VB.Form frmDefineViewResults
   BorderStyle     =   3  'Fixed Dialog
   Caption         =   "Define Files to View"
   ClientHeight    =   6465
   ClientLeft      =   45
   ClientTop       =   435
   ClientWidth     =   4275
   LinkTopic       =   "Form1"
   MaxButton       =   0   'False
   MinButton       =   0   'False
   ScaleHeight     =   6465
   ScaleWidth      =   4275
   ShowInTaskbar   =   0   'False
   StartUpPosition =   1  'CenterOwner
   Begin VB.CommandButton cmdClose
      Caption         =   "Close"
      Height          =   375
      Left            =   2640
      TabIndex        =   9
      Top             =   6000
      Width           =   1455
   End
   Begin VB.ListBox lstFilesToOpen
      Height          =   5685
      Left            =   2160
      Style           =   1  'Checkbox
      TabIndex        =   8
      Top             =   120
      Width           =   1935
   End
   Begin VB.OptionButton optUserButton
      Caption         =   "Set 8"
      Height          =   375
      Index           =   7
      Left            =   1080
      TabIndex        =   7
      Top             =   1560
      Width           =   855
   End
   Begin VB.OptionButton optUserButton
      Caption         =   "Set 7"
      Height          =   375
      Index           =   6
      Left            =   1080
      TabIndex        =   6
      Top             =   1080
      Width           =   855
   End
   Begin VB.OptionButton optUserButton
      Caption         =   "Set 6"
      Height          =   375
      Index           =   5
      Left            =   1080
      TabIndex        =   5
      Top             =   600
      Width           =   855
   End
   Begin VB.OptionButton optUserButton
      Caption         =   "Set 5"
      Height          =   375
      Index           =   4
      Left            =   1080
      TabIndex        =   4
      Top             =   120
      Width           =   855
   End
   Begin VB.OptionButton optUserButton
      Caption         =   "Set 4"
      Height          =   375
      Index           =   3
      Left            =   120
      TabIndex        =   3
      Top             =   1560
      Width           =   855
   End
   Begin VB.OptionButton optUserButton
      Caption         =   "Set 3"
      Height          =   375
      Index           =   2
      Left            =   120
      TabIndex        =   2
      Top             =   1080
      Width           =   855
   End
   Begin VB.OptionButton optUserButton
      Caption         =   "Set 2"
      Height          =   375
      Index           =   1
      Left            =   120
      TabIndex        =   1
      Top             =   600
      Width           =   855
   End
   Begin VB.OptionButton optUserButton
      Caption         =   "Set 1"
      Height          =   375
      Index           =   0
      Left            =   120
      TabIndex        =   0
      Top             =   120
      Value           =   -1  'True
      Width           =   855
   End
End
Attribute VB_Name = "frmDefineViewResults"
Attribute VB_GlobalNameSpace = False
Attribute VB_Creatable = False
Attribute VB_PredeclaredId = True
Attribute VB_Exposed = False
Dim curSet As Integer

Private Sub cmdClose_Click()
Unload Me
End Sub

Private Sub Form_Load()
curSet = 1
optUserButton(curSet - 1).Value = True
Call refreshList
End Sub

Sub refreshList()
Dim iOutKind As Integer
lstFilesToOpen.Clear
For iOutKind = 1 To numOutputKinds
  lstFilesToOpen.AddItem (outputKind(iOutKind).suffix)
  If outputKind(iOutKind).outSet(curSet) Then
    lstFilesToOpen.Selected(iOutKind - 1) = True
  End If
Next iOutKind
lstFilesToOpen.ListIndex = 0
End Sub


Private Sub lstFilesToOpen_ItemCheck(Item As Integer)
Dim curSuffix As Integer
curSuffix = Item + 1
outputKind(curSuffix).outSet(curSet) = lstFilesToOpen.Selected(Item)
End Sub

Private Sub optUserButton_Click(Index As Integer)
curSet = Index + 1
Call refreshList
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
