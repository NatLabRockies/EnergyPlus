VERSION 5.00
Begin VB.Form LayoutOption
   BorderStyle     =   3  'Fixed Dialog
   Caption         =   "Layout Options"
   ClientHeight    =   5550
   ClientLeft      =   45
   ClientTop       =   330
   ClientWidth     =   5745
   ControlBox      =   0   'False
   LinkTopic       =   "Form1"
   MaxButton       =   0   'False
   MinButton       =   0   'False
   ScaleHeight     =   5550
   ScaleWidth      =   5745
   ShowInTaskbar   =   0   'False
   StartUpPosition =   1  'CenterOwner
   Begin VB.PictureBox picTallTall
      BackColor       =   &H80000004&
      Height          =   2295
      Left            =   3000
      ScaleHeight     =   2235
      ScaleWidth      =   2595
      TabIndex        =   4
      Top             =   2640
      Width           =   2655
      Begin VB.Shape Shape16
         Height          =   1935
         Left            =   120
         Top             =   120
         Width           =   1095
      End
      Begin VB.Shape Shape15
         Height          =   855
         Left            =   1320
         Top             =   120
         Width           =   495
      End
      Begin VB.Shape Shape14
         Height          =   855
         Left            =   1920
         Top             =   120
         Width           =   495
      End
      Begin VB.Shape Shape13
         Height          =   975
         Left            =   1320
         Top             =   1080
         Width           =   1095
      End
   End
   Begin VB.PictureBox picTallShort
      BackColor       =   &H80000004&
      Height          =   2295
      Left            =   3000
      ScaleHeight     =   2235
      ScaleWidth      =   2595
      TabIndex        =   3
      Top             =   120
      Width           =   2655
      Begin VB.Shape Shape12
         Height          =   1935
         Left            =   120
         Top             =   120
         Width           =   1095
      End
      Begin VB.Shape Shape11
         Height          =   375
         Left            =   1320
         Top             =   120
         Width           =   1095
      End
      Begin VB.Shape Shape10
         Height          =   375
         Left            =   1320
         Top             =   600
         Width           =   1095
      End
      Begin VB.Shape Shape9
         Height          =   975
         Left            =   1320
         Top             =   1080
         Width           =   1095
      End
   End
   Begin VB.PictureBox picShortTall
      BackColor       =   &H80000004&
      Height          =   2295
      Left            =   120
      ScaleHeight     =   2235
      ScaleWidth      =   2595
      TabIndex        =   2
      Top             =   2640
      Width           =   2655
      Begin VB.Shape Shape8
         Height          =   855
         Left            =   120
         Top             =   120
         Width           =   1095
      End
      Begin VB.Shape Shape7
         Height          =   855
         Left            =   1320
         Top             =   120
         Width           =   495
      End
      Begin VB.Shape Shape6
         Height          =   855
         Left            =   1920
         Top             =   120
         Width           =   495
      End
      Begin VB.Shape Shape5
         Height          =   975
         Left            =   120
         Top             =   1080
         Width           =   2295
      End
   End
   Begin VB.PictureBox picShortShort
      BackColor       =   &H00E0E0E0&
      Height          =   2295
      Left            =   120
      ScaleHeight     =   2235
      ScaleWidth      =   2595
      TabIndex        =   1
      Top             =   120
      Width           =   2655
      Begin VB.Shape Shape4
         Height          =   975
         Left            =   120
         Top             =   1080
         Width           =   2295
      End
      Begin VB.Shape Shape3
         Height          =   375
         Left            =   1320
         Top             =   600
         Width           =   1095
      End
      Begin VB.Shape Shape2
         Height          =   375
         Left            =   1320
         Top             =   120
         Width           =   1095
      End
      Begin VB.Shape Shape1
         Height          =   855
         Left            =   120
         Top             =   120
         Width           =   1095
      End
   End
   Begin VB.CommandButton cmdOK
      Caption         =   "OK"
      Default         =   -1  'True
      Height          =   375
      Left            =   4440
      TabIndex        =   0
      Top             =   5040
      Width           =   1215
   End
End
Attribute VB_Name = "LayoutOption"
Attribute VB_GlobalNameSpace = False
Attribute VB_Creatable = False
Attribute VB_PredeclaredId = True
Attribute VB_Exposed = False
Option Explicit

Private Sub cmdOK_Click()
Unload Me
End Sub

Private Sub Form_Load()
Select Case formLayoutOption
  Case floShortShort
    Call picShortShort_Click
  Case floShortTall
    Call picShortTall_Click
  Case floTallShort
    Call picTallShort_Click
  Case floTallTall
    Call picTallTall_Click
End Select
End Sub

Private Sub picShortShort_Click()
Call resetAllButtons
picShortShort.Appearance = 1
picShortShort.BackColor = &HE0E0E0
formLayoutOption = floShortShort
End Sub

Private Sub picShortShort_DblClick()
Call resetAllButtons
picShortShort.Appearance = 1
picShortShort.BackColor = &HE0E0E0
formLayoutOption = floShortShort
Unload Me
End Sub


Private Sub picTallShort_Click()
Call resetAllButtons
picTallShort.Appearance = 1
picTallShort.BackColor = &HE0E0E0
formLayoutOption = floTallShort
End Sub

Private Sub picTallShort_DblClick()
Call resetAllButtons
picTallShort.Appearance = 1
picTallShort.BackColor = &HE0E0E0
formLayoutOption = floTallShort
Unload Me
End Sub

Private Sub picTallTall_Click()
Call resetAllButtons
picTallTall.Appearance = 1
picTallTall.BackColor = &HE0E0E0
formLayoutOption = floTallTall
End Sub

Private Sub picTallTall_DblClick()
Call resetAllButtons
picTallTall.Appearance = 1
picTallTall.BackColor = &HE0E0E0
formLayoutOption = floTallTall
Unload Me
End Sub

Private Sub picShortTall_Click()
Call resetAllButtons
picShortTall.Appearance = 1
picShortTall.BackColor = &HE0E0E0
formLayoutOption = floShortTall
End Sub

Private Sub picShortTall_DblClick()
Call resetAllButtons
picShortTall.Appearance = 1
picShortTall.BackColor = &HE0E0E0
formLayoutOption = floShortTall
Unload Me
End Sub

Sub resetAllButtons()
picShortShort.Appearance = 0
picShortShort.BackColor = &H80000004
picShortTall.Appearance = 0
picShortTall.BackColor = &H80000004
picTallShort.Appearance = 0
picTallShort.BackColor = &H80000004
picTallTall.Appearance = 0
picTallTall.BackColor = &H80000004
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
