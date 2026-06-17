VERSION 5.00
Begin VB.Form saveOption
   BorderStyle     =   3  'Fixed Dialog
   Caption         =   "Save Options..."
   ClientHeight    =   8415
   ClientLeft      =   45
   ClientTop       =   435
   ClientWidth     =   9030
   ControlBox      =   0   'False
   LinkTopic       =   "Form1"
   MaxButton       =   0   'False
   MinButton       =   0   'False
   ScaleHeight     =   8415
   ScaleWidth      =   9030
   ShowInTaskbar   =   0   'False
   StartUpPosition =   1  'CenterOwner
   Begin VB.TextBox txtHelp
      BackColor       =   &H8000000F&
      BeginProperty Font
         Name            =   "MS Sans Serif"
         Size            =   9.75
         Charset         =   0
         Weight          =   400
         Underline       =   0   'False
         Italic          =   0   'False
         Strikethrough   =   0   'False
      EndProperty
      Height          =   6255
      Left            =   120
      Locked          =   -1  'True
      MultiLine       =   -1  'True
      TabIndex        =   8
      Text            =   "saveOption.frx":0000
      Top             =   2040
      Width           =   8775
   End
   Begin VB.CheckBox chkSetDefault
      Caption         =   "Set as Default"
      Height          =   255
      Left            =   3600
      TabIndex        =   7
      Top             =   1080
      Width           =   1455
   End
   Begin VB.CheckBox chkViewHelp
      Caption         =   "Help"
      Height          =   375
      Left            =   120
      Style           =   1  'Graphical
      TabIndex        =   6
      Top             =   1440
      Width           =   735
   End
   Begin VB.ComboBox cmbSpecialFormat
      Height          =   315
      Left            =   2640
      Style           =   2  'Dropdown List
      TabIndex        =   5
      Top             =   540
      Width           =   2400
   End
   Begin VB.CommandButton cmdCancel
      Cancel          =   -1  'True
      Caption         =   "Cancel"
      Height          =   375
      Left            =   3840
      TabIndex        =   3
      Top             =   1440
      Width           =   1215
   End
   Begin VB.CommandButton cmdOK
      Caption         =   "OK"
      Default         =   -1  'True
      Height          =   375
      Left            =   2520
      TabIndex        =   2
      Top             =   1440
      Width           =   1215
   End
   Begin VB.ComboBox cmbSaveOrder
      Height          =   315
      Left            =   2640
      Style           =   2  'Dropdown List
      TabIndex        =   1
      Top             =   60
      Width           =   2400
   End
   Begin VB.Label Label4
      Caption         =   "Special Format for Some Objects"
      Height          =   255
      Left            =   120
      TabIndex        =   4
      Top             =   600
      Width           =   2415
   End
   Begin VB.Label Label1
      Caption         =   "Saved Order of Objects"
      Height          =   255
      Left            =   120
      TabIndex        =   0
      Top             =   120
      Width           =   1815
   End
End
Attribute VB_Name = "saveOption"
Attribute VB_GlobalNameSpace = False
Attribute VB_Creatable = False
Attribute VB_PredeclaredId = True
Attribute VB_Exposed = False
Option Explicit
Public activeSaveOrderOpt As Integer
Public activeSpecialFormatOpt As Integer

Private Sub cmdCancel_Click()
Me.Hide
End Sub

Private Sub cmdOK_Click()
activeSaveOrderOpt = cmbSaveOrder.ListIndex
activeSpecialFormatOpt = cmbSpecialFormat.ListIndex
If chkSetDefault.Value = vbChecked Then
  saveOrderOptDefault = cmbSaveOrder.ListIndex
  specialFormatOptDefault = cmbSpecialFormat.ListIndex
End If
Me.Hide
End Sub

Private Sub Form_Activate()
chkSetDefault.Value = vbUnchecked
cmbSaveOrder.ListIndex = activeSaveOrderOpt
cmbSpecialFormat.ListIndex = activeSpecialFormatOpt
End Sub

Private Sub Form_Load()
Dim a As String
saveOption.Height = 2415
saveOption.Width = 5265
cmbSaveOrder.AddItem "Sorted"
cmbSaveOrder.AddItem "Original with New at Top"
cmbSaveOrder.AddItem "Original with New at Bottom"
cmbSpecialFormat.AddItem "Yes"
cmbSpecialFormat.AddItem "No"
a = "The save options are related to the layout of the IDF file after it is saved. These options are not "
a = a & "important if you never edit the IDF file with a text editor.  " & vbCrLf & vbCrLf
a = a & "The sorted order of saving objects is the traditional way the IDF Editor sorts objects within files. "
a = a & "Each type of object is presented in groups in the order they appear in the Energy+.IDD. The other "
a = a & "options preserve the original order of the objects from the file but each object will be still "
a = a & "be reformatted. By preserving the order, the objects are not rearranged so you can group them "
a = a & "using a text editor and they will stay in that order.  New objects are placed either near the top "
a = a & "of the file or near the bottom of the file so that they can be easily found when using a text "
a = a & "editor.  " & vbCrLf & vbCrLf
a = a & "You can also choose to specially format some objects. This affects how individual "
a = a & "fields in objects are arranged when saved. "
a = a & "Selecting this option will format the following objects on a single line: "
a = a & "Report, Report Meter, Report Variable, "
a = a & "Version, Timestep in Hour, Inside Convection Algorithm, Outside Convection Algorithm, Solution "
a = a & "Algorithm, Shadowing Calculations, Ground Reflectances, and GroundTemperatures:Deep.  In addition, "
a = a & "Schedule:Compact objects will be formatted to have two field for some lines. With this option, objects "
a = a & "with geometric "
a = a & "vertices are formatted to have the X, Y, and Z values on the same line. Those objects include: "
a = a & "Surface:HeatTransfer, Surface:HeatTransfer:Sub, Surface:Shading:Detached:Fixed, "
a = a & "Surface:Shading:Detached:Building "
a = a & "and Surface:Shading:Attached." & vbCrLf & vbCrLf
a = a & "These options are saved for each file.  If a file has not been saved with IDF Editor yet, the default  but if a file does not specify the default values for these "
a = a & "can also be set by using the set as default option. "
a = a & "The saved file keeps these options by using the !-option line with SortedOrder, OriginalOrderTop, "
a = a & "OriginalOrderBottom, and UseSpecialFormat."
txtHelp.Text = a
End Sub

Private Sub chkViewHelp_Click()
If chkViewHelp.Value = vbChecked Then
  saveOption.Height = 8895
  saveOption.Width = 9120
Else
  saveOption.Height = 2415
  saveOption.Width = 5265
End If
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
