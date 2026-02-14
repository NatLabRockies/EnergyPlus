VERSION 5.00
Begin VB.Form frmErrorCheck
   BorderStyle     =   3  'Fixed Dialog
   Caption         =   "Validity Check"
   ClientHeight    =   4275
   ClientLeft      =   45
   ClientTop       =   435
   ClientWidth     =   11175
   LinkTopic       =   "Form1"
   MaxButton       =   0   'False
   MinButton       =   0   'False
   ScaleHeight     =   4275
   ScaleWidth      =   11175
   ShowInTaskbar   =   0   'False
   StartUpPosition =   3  'Windows Default
   Begin VB.CheckBox chkIgnoreMacro
      Caption         =   "Ignore Macros"
      Height          =   375
      Left            =   4800
      TabIndex        =   8
      ToolTipText     =   "Does not check fields that contains any of the following:  # [ @ $ = "
      Top             =   3840
      Width           =   2055
   End
   Begin VB.CommandButton cmdClose
      Cancel          =   -1  'True
      Caption         =   "Close"
      Default         =   -1  'True
      Height          =   375
      Left            =   9600
      TabIndex        =   7
      Top             =   3840
      Width           =   1455
   End
   Begin VB.CommandButton cmdGoto
      Caption         =   "Go to"
      Height          =   375
      Left            =   7440
      TabIndex        =   6
      Top             =   3840
      Width           =   1455
   End
   Begin VB.CheckBox chkValidCheckOnSave
      Caption         =   "Perform Validity Check When Saving File"
      Height          =   375
      Left            =   120
      TabIndex        =   5
      Top             =   3840
      Width           =   3495
   End
   Begin VB.ListBox lstMsgs
      Height          =   3570
      Left            =   120
      TabIndex        =   4
      Top             =   120
      Width           =   10935
   End
   Begin VB.Frame Frame1
      Caption         =   "Check for Valid"
      Height          =   1455
      Left            =   11640
      TabIndex        =   0
      Top             =   1800
      Width           =   2055
      Begin VB.CheckBox Check3
         Caption         =   "Numeric Ranges"
         Height          =   375
         Left            =   240
         TabIndex        =   3
         Top             =   960
         Width           =   1575
      End
      Begin VB.CheckBox Check2
         Caption         =   "References"
         Height          =   375
         Left            =   240
         TabIndex        =   2
         Top             =   600
         Width           =   1575
      End
      Begin VB.CheckBox Check1
         Caption         =   "Choices"
         Height          =   375
         Left            =   240
         TabIndex        =   1
         Top             =   240
         Width           =   1455
      End
   End
End
Attribute VB_Name = "frmErrorCheck"
Attribute VB_GlobalNameSpace = False
Attribute VB_Creatable = False
Attribute VB_PredeclaredId = True
Attribute VB_Exposed = False
Public curFormData As IDFEdit
Public reportWhenNoneFound As Boolean

Dim returnMsg() As String
Dim returnClass() As Long
Dim returnObj() As Long
Dim returnFld() As Long


Private Sub performCheck()
Dim i As Long
Call curFormData.doValidityCheck(returnMsg(), returnClass(), returnObj(), returnFld())
If returnMsg(1) <> "" Then
  For i = 1 To UBound(returnMsg)
    If returnMsg(i) <> "" Then
      lstMsgs.AddItem returnMsg(i)
      lstMsgs.ItemData(lstMsgs.NewIndex) = i
    End If
  Next i
  cmdGoto.Enabled = True
Else
  lstMsgs.AddItem "No invalid entries found."
  cmdGoto.Enabled = False
End If
If checkRangeOnSave = checkRangeYes Then
  chkValidCheckOnSave.Value = vbChecked
Else
  chkValidCheckOnSave.Value = vbUnchecked
End If
If ignoreValidatingMacros Then
  chkIgnoreMacro.Value = vbChecked
Else
  chkIgnoreMacro.Value = vbUnchecked
End If
End Sub



Private Sub Form_Load()
Call performCheck
End Sub

Private Sub Form_Activate()
If returnMsg(1) = "" Then
  If Not reportWhenNoneFound Then
    Unload Me
  End If
End If
End Sub

Private Sub chkValidCheckOnSave_Click()
If chkValidCheckOnSave.Value = vbChecked Then
  checkRangeOnSave = checkRangeYes
Else
  checkRangeOnSave = checkRangeNo
End If
End Sub

Private Sub chkIgnoreMacro_Click()
If chkIgnoreMacro.Value = vbChecked Then
  ignoreValidatingMacros = True
Else
  ignoreValidatingMacros = False
End If
' repopulate the list with new setting
lstMsgs.Clear
Call performCheck
End Sub


Private Sub cmdGoto_Click()
Dim cur As Long
cur = lstMsgs.ItemData(lstMsgs.ListIndex)
Me.Hide
Call curFormData.jumpToObjectField(returnClass(cur), returnObj(cur), returnFld(cur))
Unload Me
End Sub

Private Sub cmdClose_Click()
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
