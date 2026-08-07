VERSION 5.00
Begin VB.Form About
   BorderStyle     =   3  'Fixed Dialog
   ClientHeight    =   5850
   ClientLeft      =   255
   ClientTop       =   1410
   ClientWidth     =   5280
   ClipControls    =   0   'False
   ControlBox      =   0   'False
   Icon            =   "about.frx":0000
   KeyPreview      =   -1  'True
   LinkTopic       =   "Form2"
   MaxButton       =   0   'False
   MinButton       =   0   'False
   Picture         =   "about.frx":000C
   ScaleHeight     =   5850
   ScaleWidth      =   5280
   ShowInTaskbar   =   0   'False
   StartUpPosition =   1  'CenterOwner
   Begin VB.TextBox txtNotice
      BeginProperty Font
         Name            =   "Courier New"
         Size            =   8.25
         Charset         =   0
         Weight          =   400
         Underline       =   0   'False
         Italic          =   0   'False
         Strikethrough   =   0   'False
      EndProperty
      Height          =   3615
      Left            =   120
      MultiLine       =   -1  'True
      ScrollBars      =   2  'Vertical
      TabIndex        =   5
      Text            =   "about.frx":08D6
      Top             =   1560
      Width           =   5055
   End
   Begin VB.CommandButton cmdSplashClose
      Caption         =   "Close"
      Height          =   375
      Left            =   4200
      TabIndex        =   3
      Top             =   5400
      Width           =   975
   End
   Begin VB.Frame Frame1
      Height          =   1380
      Left            =   120
      TabIndex        =   0
      Top             =   0
      Width           =   5025
      Begin VB.PictureBox Picture1
         AutoSize        =   -1  'True
         Height          =   540
         Left            =   585
         Picture         =   "about.frx":08DC
         ScaleHeight     =   480
         ScaleWidth      =   480
         TabIndex        =   6
         Top             =   465
         Width           =   540
      End
      Begin VB.Label lblEnableDebugMode
         AutoSize        =   -1  'True
         BeginProperty Font
            Name            =   "Arial"
            Size            =   32.25
            Charset         =   0
            Weight          =   700
            Underline       =   0   'False
            Italic          =   0   'False
            Strikethrough   =   0   'False
         EndProperty
         Height          =   255
         Left            =   3720
         TabIndex        =   4
         Top             =   1080
         Width           =   1275
      End
      Begin VB.Label lblVersion
         Alignment       =   1  'Right Justify
         AutoSize        =   -1  'True
         Caption         =   "Version 1.56"
         BeginProperty Font
            Name            =   "Arial"
            Size            =   12
            Charset         =   0
            Weight          =   700
            Underline       =   0   'False
            Italic          =   0   'False
            Strikethrough   =   0   'False
         EndProperty
         Height          =   285
         Left            =   1680
         TabIndex        =   1
         Top             =   960
         Width           =   1410
      End
      Begin VB.Label lblProductName
         AutoSize        =   -1  'True
         Caption         =   "IDF Editor"
         BeginProperty Font
            Name            =   "Arial"
            Size            =   32.25
            Charset         =   0
            Weight          =   700
            Underline       =   0   'False
            Italic          =   0   'False
            Strikethrough   =   0   'False
         EndProperty
         Height          =   765
         Left            =   1200
         TabIndex        =   2
         Top             =   360
         Width           =   3075
      End
   End
   Begin VB.Label Label1
      Height          =   615
      Left            =   0
      TabIndex        =   8
      Top             =   0
      Width           =   615
   End
   Begin VB.Label lblAboutEPVer
      Caption         =   "Label1"
      Height          =   255
      Left            =   120
      TabIndex        =   7
      Top             =   5400
      Width           =   3735
   End
End
Attribute VB_Name = "About"
Attribute VB_GlobalNameSpace = False
Attribute VB_Creatable = False
Attribute VB_PredeclaredId = True
Attribute VB_Exposed = False

Option Explicit

Private Sub cmdSplashClose_Click()
  Unload Me
End Sub


Private Sub Form_KeyPress(KeyAscii As Integer)
  Unload Me
End Sub

Private Sub Form_Load()
lblAboutEPVer.Caption = "EnergyPlus and IDD version number: " & IDDVersion
txtNotice.Text = "Copyright (c) 2000-2026 GARD Analytics, " & vbCrLf
txtNotice.Text = txtNotice.Text & "Inc.  All rights reserved." & vbCrLf
txtNotice.Text = txtNotice.Text & "                                        " & vbCrLf
txtNotice.Text = txtNotice.Text & "The code for quick select dropdowns     " & vbCrLf
txtNotice.Text = txtNotice.Text & "contributed by Rajan Rawal and Tejpal   " & vbCrLf
txtNotice.Text = txtNotice.Text & "Mehta of CEPT University.               " & vbCrLf
txtNotice.Text = txtNotice.Text & "                                        " & vbCrLf
txtNotice.Text = txtNotice.Text & "NOTICE: The U.S. Government is granted  " & vbCrLf
txtNotice.Text = txtNotice.Text & "for itself and others acting on its     " & vbCrLf
txtNotice.Text = txtNotice.Text & "behalf a paid-up, nonexclusive,         " & vbCrLf
txtNotice.Text = txtNotice.Text & "irrevocable, worldwide license in this  " & vbCrLf
txtNotice.Text = txtNotice.Text & "data to reproduce, prepare derivative   " & vbCrLf
txtNotice.Text = txtNotice.Text & "works, and perform publicly and display " & vbCrLf
txtNotice.Text = txtNotice.Text & "publicly. Beginning five (5) years after" & vbCrLf
txtNotice.Text = txtNotice.Text & "permission to assert copyright is       " & vbCrLf
txtNotice.Text = txtNotice.Text & "granted, subject to two possible five   " & vbCrLf
txtNotice.Text = txtNotice.Text & "year renewals, the U.S. Government is   " & vbCrLf
txtNotice.Text = txtNotice.Text & "granted for itself and others acting on " & vbCrLf
txtNotice.Text = txtNotice.Text & "its behalf a paid-up, non-exclusive,    " & vbCrLf
txtNotice.Text = txtNotice.Text & "irrevocable worldwide license in this   " & vbCrLf
txtNotice.Text = txtNotice.Text & "data to reproduce, prepare derivative   " & vbCrLf
txtNotice.Text = txtNotice.Text & "works, distribute copies to the public, " & vbCrLf
txtNotice.Text = txtNotice.Text & "perform publicly and display publicly,  " & vbCrLf
txtNotice.Text = txtNotice.Text & "and to permit others to do so.          " & vbCrLf
txtNotice.Text = txtNotice.Text & "                                        " & vbCrLf
txtNotice.Text = txtNotice.Text & "TRADEMARKS: EnergyPlus, DOE-2.1E, DOE-2," & vbCrLf
txtNotice.Text = txtNotice.Text & "and DOE are trademarks of the US        " & vbCrLf
txtNotice.Text = txtNotice.Text & "Department of Energy.                   " & vbCrLf
txtNotice.Text = txtNotice.Text & "                                        " & vbCrLf
txtNotice.Text = txtNotice.Text & "DISCLAIMER OF WARRANTY AND LIMITATION OF" & vbCrLf
txtNotice.Text = txtNotice.Text & "LIABILITY: THIS SOFTWARE IS PROVIDED 'AS" & vbCrLf
txtNotice.Text = txtNotice.Text & "IS' WITHOUT WARRANTY OF ANY KIND.       " & vbCrLf
txtNotice.Text = txtNotice.Text & "NEITHER GARD ANALYTICS, THE DEPARTMENT  " & vbCrLf
txtNotice.Text = txtNotice.Text & "OF ENERGY, THE US GOVERNMENT, THEIR     " & vbCrLf
txtNotice.Text = txtNotice.Text & "LICENSORS, OR ANY PERSON OR ORGANIZATION" & vbCrLf
txtNotice.Text = txtNotice.Text & "ACTING ON BEHALF OF ANY OF THEM:        " & vbCrLf
txtNotice.Text = txtNotice.Text & "                                        " & vbCrLf
txtNotice.Text = txtNotice.Text & "A.  MAKE ANY WARRANTY OR REPRESENTATION " & vbCrLf
txtNotice.Text = txtNotice.Text & "WHATSOEVER, EXPRESS OR IMPLIED, WITH    " & vbCrLf
txtNotice.Text = txtNotice.Text & "RESPECT TO ENERGYPLUS OR ANY DERIVATIVE " & vbCrLf
txtNotice.Text = txtNotice.Text & "WORKS THEREOF, INCLUDING WITHOUT        " & vbCrLf
txtNotice.Text = txtNotice.Text & "LIMITATION WARRANTIES OF                " & vbCrLf
txtNotice.Text = txtNotice.Text & "MERCHANTABILITY, WARRANTIES OF FITNESS  " & vbCrLf
txtNotice.Text = txtNotice.Text & "FOR A PARTICULAR PURPOSE, OR WARRANTIES " & vbCrLf
txtNotice.Text = txtNotice.Text & "OR REPRESENTATIONS REGARDING THE USE, OR" & vbCrLf
txtNotice.Text = txtNotice.Text & "THE RESULTS OF THE USE OF ENERGYPLUS OR " & vbCrLf
txtNotice.Text = txtNotice.Text & "DERIVATIVE WORKS THEREOF IN TERMS OF    " & vbCrLf
txtNotice.Text = txtNotice.Text & "CORRECTNESS, ACCURACY, RELIABILITY,     " & vbCrLf
txtNotice.Text = txtNotice.Text & "CURRENTNESS, OR OTHERWISE. THE ENTIRE   " & vbCrLf
txtNotice.Text = txtNotice.Text & "RISK AS TO THE RESULTS AND PERFORMANCE  " & vbCrLf
txtNotice.Text = txtNotice.Text & "OF THE LICENSED SOFTWARE IS ASSUMED BY  " & vbCrLf
txtNotice.Text = txtNotice.Text & "THE LICENSEE.                           " & vbCrLf
txtNotice.Text = txtNotice.Text & "                                        " & vbCrLf
txtNotice.Text = txtNotice.Text & "B.  MAKE ANY REPRESENTATION OR WARRANTY " & vbCrLf
txtNotice.Text = txtNotice.Text & "THAT ENERGYPLUS OR DERIVATIVE WORKS     " & vbCrLf
txtNotice.Text = txtNotice.Text & "THEREOF WILL NOT INFRINGE ANY COPYRIGHT " & vbCrLf
txtNotice.Text = txtNotice.Text & "OR OTHER PROPRIETARY RIGHT.             " & vbCrLf
txtNotice.Text = txtNotice.Text & "                                        " & vbCrLf
txtNotice.Text = txtNotice.Text & "C.  ASSUME ANY LIABILITY WHATSOEVER WITH" & vbCrLf
txtNotice.Text = txtNotice.Text & "RESPECT TO ANY USE OF ENERGYPLUS,       " & vbCrLf
txtNotice.Text = txtNotice.Text & "DERIVATIVE WORKS THEREOF, OR ANY PORTION" & vbCrLf
txtNotice.Text = txtNotice.Text & "THEREOF OR WITH RESPECT TO ANY DAMAGES  " & vbCrLf
txtNotice.Text = txtNotice.Text & "WHICH MAY RESULT FROM SUCH USE.         " & vbCrLf
txtNotice.Text = txtNotice.Text & "                                        " & vbCrLf
txtNotice.Text = txtNotice.Text & "DISCLAIMER OF ENDORSEMENT: Reference    " & vbCrLf
txtNotice.Text = txtNotice.Text & "herein to any specific commercial       " & vbCrLf
txtNotice.Text = txtNotice.Text & "products, process, or service by trade  " & vbCrLf
txtNotice.Text = txtNotice.Text & "name, trademark, manufacturer, or       " & vbCrLf
txtNotice.Text = txtNotice.Text & "otherwise, does not necessarily         " & vbCrLf
txtNotice.Text = txtNotice.Text & "constitute or imply its endorsement,    " & vbCrLf
txtNotice.Text = txtNotice.Text & "recommendation, or favoring by the      " & vbCrLf
txtNotice.Text = txtNotice.Text & "United States Government or GARD        " & vbCrLf
txtNotice.Text = txtNotice.Text & "Analytics.                              " & vbCrLf

End Sub

Private Sub lblEnableDebugMode_Click()
  IDFEdit.mnuCreateRangeTestFiles.Visible = True
  MsgBox "You found debug mode.  Please exit the application now if you are not an EnergyPlus developer or tester.", vbExclamation, "Debug Mode"
  Unload About
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
