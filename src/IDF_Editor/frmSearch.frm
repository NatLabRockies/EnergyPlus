VERSION 5.00
Begin VB.Form frmSearch
   BorderStyle     =   3  'Fixed Dialog
   Caption         =   "Search and Replace"
   ClientHeight    =   4680
   ClientLeft      =   -105
   ClientTop       =   585
   ClientWidth     =   11100
   LinkTopic       =   "Form1"
   MaxButton       =   0   'False
   MinButton       =   0   'False
   ScaleHeight     =   4680
   ScaleWidth      =   11100
   ShowInTaskbar   =   0   'False
   StartUpPosition =   1  'CenterOwner
   Begin VB.CommandButton cmdNone
      Caption         =   "None"
      Height          =   255
      Left            =   6120
      TabIndex        =   10
      Top             =   525
      Width           =   735
   End
   Begin VB.CommandButton cmdGoto
      Caption         =   "Go to"
      Height          =   255
      Left            =   8280
      TabIndex        =   9
      Top             =   525
      Width           =   735
   End
   Begin VB.CheckBox chkEntire
      Caption         =   "Match Entire Field Value Contents"
      Height          =   195
      Left            =   1200
      TabIndex        =   8
      Top             =   600
      Width           =   2775
   End
   Begin VB.CommandButton cmdAll
      Caption         =   "All"
      Height          =   255
      Left            =   5280
      TabIndex        =   7
      Top             =   525
      Width           =   735
   End
   Begin VB.CommandButton cmdReplace
      Caption         =   "Replace Selected"
      Height          =   375
      Left            =   9120
      TabIndex        =   6
      Top             =   4200
      Width           =   1815
   End
   Begin VB.TextBox txtReplaceText
      Height          =   375
      Left            =   1200
      TabIndex        =   5
      Top             =   4200
      Width           =   7815
   End
   Begin VB.CommandButton cmdFind
      Caption         =   "Find"
      Default         =   -1  'True
      Height          =   375
      Left            =   9120
      TabIndex        =   2
      Top             =   120
      Width           =   1815
   End
   Begin VB.TextBox txtSearchText
      Height          =   375
      Left            =   1200
      TabIndex        =   1
      Top             =   120
      Width           =   7815
   End
   Begin VB.ListBox lstFindResults
      Height          =   3210
      ItemData        =   "frmSearch.frx":0000
      Left            =   120
      List            =   "frmSearch.frx":0007
      Style           =   1  'Checkbox
      TabIndex        =   0
      Top             =   840
      Width           =   10860
   End
   Begin VB.Label Label2
      Caption         =   "Replace with:"
      Height          =   255
      Left            =   120
      TabIndex        =   4
      Top             =   4320
      Width           =   1095
   End
   Begin VB.Label Label1
      Caption         =   "Find what:"
      Height          =   255
      Left            =   360
      TabIndex        =   3
      Top             =   240
      Width           =   735
   End
End
Attribute VB_Name = "frmSearch"
Attribute VB_GlobalNameSpace = False
Attribute VB_Creatable = False
Attribute VB_PredeclaredId = True
Attribute VB_Exposed = False
Option Explicit

Public curFormData As IDFEdit
Public useAsSearchField As String
Public useAsReplaceField As String

Dim foundIdfValue() As String
Dim foundIdfName() As String
Dim foundIddObj() As Long
Dim foundIddFld() As Long
Dim foundIdfValueIndx() As Long
Dim foundIdfObjIndx() As Long
Dim selectedReplace() As Boolean

Private Sub Form_Load()
lstFindResults.Clear
cmdAll.Enabled = False
cmdNone.Enabled = False
cmdGoto.Enabled = False
cmdReplace.Enabled = False
If useAsSearchField <> "" Then
  txtSearchText.Text = useAsSearchField
End If
If useAsReplaceField <> "" Then
  txtReplaceText.Text = useAsReplaceField
End If
End Sub

Private Sub cmdFind_Click()
Dim useEntire As Boolean
Dim enableButtons As Boolean
Dim i As Long
enableButtons = False
If chkEntire.Value = 1 Then
  useEntire = True
Else
  useEntire = False
End If
Call curFormData.searchFind(txtSearchText.Text, useEntire, foundIdfValue(), foundIdfName(), foundIddObj(), foundIddFld(), foundIdfValueIndx(), foundIdfObjIndx())
lstFindResults.Clear
For i = 1 To UBound(foundIdfValue)
  If foundIdfValue(i) <> "" Then
    If foundIddObj(i) > 0 And foundIddObj(i) <= maxUsedIDDClass Then
      If foundIddFld(i) > 0 And foundIddFld(i) <= maxUsedField Then
        lstFindResults.AddItem foundIdfValue(i) & "  (" & IDDClassDat(foundIddObj(i)).name & " // " & foundIdfName(i) & " // " & IDDField(foundIddFld(i)).name & ")"
        lstFindResults.ItemData(lstFindResults.NewIndex) = i
        enableButtons = True
      End If
    End If
  End If
Next i
cmdAll.Enabled = enableButtons
cmdNone.Enabled = enableButtons
cmdGoto.Enabled = enableButtons
cmdReplace.Enabled = enableButtons
End Sub

Private Sub cmdAll_Click()
Dim i As Long
For i = 0 To lstFindResults.ListCount - 1
  lstFindResults.Selected(i) = True
Next i
lstFindResults.ListIndex = 0
End Sub

Private Sub cmdNone_Click()
Dim i As Long
For i = 0 To lstFindResults.ListCount - 1
  lstFindResults.Selected(i) = False
Next i
lstFindResults.ListIndex = 0
End Sub

Private Sub cmdReplace_Click()
Dim i As Long
Dim cur As Long
Dim useEntire As Boolean
Dim countOfChanges As Long
If chkEntire.Value = 1 Then
  useEntire = True
Else
  useEntire = False
End If
ReDim selectedReplace(UBound(foundIdfValue))
For i = 0 To lstFindResults.ListCount - 1
  If lstFindResults.Selected(i) Then
    cur = lstFindResults.ItemData(i)
    selectedReplace(cur) = True
  End If
Next i
'MsgBox "REPLACECLICK - search:[" & txtSearchText.Text & "]" & vbCrLf & "replace:[" & txtReplaceText.Text & "]"
Call curFormData.replaceFound(txtSearchText.Text, txtReplaceText.Text, useEntire, selectedReplace(), foundIdfValueIndx(), countOfChanges)
MsgBox Str(countOfChanges) & " changes made", vbInformation, "Search and Replace Results"
If countOfChanges >= 1 Then
  Call curFormData.ShowFileAltered
End If
Unload Me
End Sub

Private Sub cmdGoto_Click()
Dim cur As Long
cur = lstFindResults.ItemData(lstFindResults.ListIndex)
'MsgBox IDDClassDat(foundIddObj(cur)).name & " // " & foundIdfName(cur) & " // " & IDDField(foundIddFld(cur)).name
Me.Hide
Call curFormData.jumpToObjectField(foundIddObj(cur), foundIdfObjIndx(cur), foundIddFld(cur))
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
