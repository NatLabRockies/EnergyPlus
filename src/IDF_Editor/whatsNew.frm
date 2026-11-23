VERSION 5.00
Begin VB.Form frmWhatsNew
   BorderStyle     =   3  'Fixed Dialog
   Caption         =   "New Features of IDF Editor"
   ClientHeight    =   8280
   ClientLeft      =   45
   ClientTop       =   435
   ClientWidth     =   10905
   BeginProperty Font
      Name            =   "MS Sans Serif"
      Size            =   9.75
      Charset         =   0
      Weight          =   700
      Underline       =   0   'False
      Italic          =   0   'False
      Strikethrough   =   0   'False
   EndProperty
   LinkTopic       =   "Form1"
   MaxButton       =   0   'False
   MinButton       =   0   'False
   ScaleHeight     =   8280
   ScaleWidth      =   10905
   ShowInTaskbar   =   0   'False
   StartUpPosition =   1  'CenterOwner
   Begin VB.TextBox txtWhatsNew
      BeginProperty Font
         Name            =   "MS Sans Serif"
         Size            =   9.75
         Charset         =   0
         Weight          =   400
         Underline       =   0   'False
         Italic          =   0   'False
         Strikethrough   =   0   'False
      EndProperty
      Height          =   7575
      Left            =   120
      MultiLine       =   -1  'True
      ScrollBars      =   2  'Vertical
      TabIndex        =   1
      Text            =   "whatsNew.frx":0000
      Top             =   120
      Width           =   10695
   End
   Begin VB.CommandButton cmdClose
      Caption         =   "Close"
      BeginProperty Font
         Name            =   "MS Sans Serif"
         Size            =   8.25
         Charset         =   0
         Weight          =   400
         Underline       =   0   'False
         Italic          =   0   'False
         Strikethrough   =   0   'False
      EndProperty
      Height          =   375
      Left            =   9360
      TabIndex        =   0
      Top             =   7800
      Width           =   1455
   End
   Begin VB.Label Label1
      Caption         =   "This dialog box is shown at start up one time and then can be found under HELP .. WHATS NEW"
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
      TabIndex        =   2
      Top             =   7800
      Width           =   8775
   End
End
Attribute VB_Name = "frmWhatsNew"
Attribute VB_GlobalNameSpace = False
Attribute VB_Creatable = False
Attribute VB_PredeclaredId = True
Attribute VB_Exposed = False
Option Explicit

Private Sub cmdClose_Click()
Unload Me
End Sub

Private Sub Form_Load()
Dim t As String
t = ""
t = t & "New Features of IDF Editor Version 1.56" & vbCrLf
t = t & "-------------------------------------------------------------------------------" & vbCrLf
t = t & "Added VIEW .. FONT SIZE to change the size of the text on the main window. " & vbCrLf & vbCrLf

t = t & "New Features of IDF Editor Version 1.50" & vbCrLf
t = t & "-------------------------------------------------------------------------------" & vbCrLf
t = t & "New 'Duplicate Object and Change' menu item under Edit and 'Dup Obj + Chg' toolbar item. This will duplicate the currently selected object and for " _
      & "each field in the object replace any text with new text. The original suggested text for change is based on the name of the object. This is " _
      & "especially useful for HVAC related objects when the node names are based on some of the same text as the name of the object." & vbCrLf & vbCrLf

t = t & "New Features of IDF Editor Version 1.49" & vbCrLf
t = t & "-------------------------------------------------------------------------------" & vbCrLf
t = t & "In EDIT .. VALIDITY CHECK now have an option to IGNORE MACROS which will not perform validity checking on fields that contain characters " _
      & "typically used for macros:  # [ @ $ = " & vbCrLf & vbCrLf

t = t & "Allow multiple objects to be duplicated or deleted at the same time when multiple columns are selected." & vbCrLf & vbCrLf

t = t & "New Features of IDF Editor Version 1.48" & vbCrLf
t = t & "-------------------------------------------------------------------------------" & vbCrLf
t = t & "Added warning when trying to open file with Unix file line endings." & vbCrLf & vbCrLf

t = t & "New Features of IDF Editor Version 1.44" & vbCrLf
t = t & "-------------------------------------------------------------------------------" & vbCrLf
t = t & "The new JUMP menu allows you to easily navigate between fields in different objects that are using the same name. For example, it can be " _
      & "used to jump from construction to the surfaces that use it or between places a node name is used. It can be used to jump between anywhere " _
      & "the same name is used in multiple objects. To use select a cell that has a name used somewhere and select the JUMP menu. " _
      & "Only the first forty possible jumps are shown in the menu. " & vbCrLf & vbCrLf
t = t & "Also, when multiple columns are selected, the new menu item EDIT .. FILL RIGHT copies the left most selected cell to the selected cells to the " _
      & "right of it. This is similar to the Fill Right function found in many spreadsheet programs. Multiple rows and columns can be selected at the " _
      & "same time. You can use the menu or Control-D as the short cut key (D is short for dextral which means right handed)." & vbCrLf & vbCrLf

t = t & "New Features of IDF Editor Version 1.42" & vbCrLf
t = t & "-------------------------------------------------------------------------------" & vbCrLf
t = t & "Added new menu item VIEW .. USE NODE NAME EDITOR to enable or disable the Edit or Select Node Name dialog box. When it is " _
      & "disabled the node name can be edited directly in the cell." & vbCrLf & vbCrLf

t = t & "New Features of IDF Editor Version 1.40" & vbCrLf
t = t & "-------------------------------------------------------------------------------" & vbCrLf
t = t & "Added the new EDIT .. SEARCH AND REPLACE dialog to search and replace values in a file. This function is especially useful when changing " _
      & "the names of objects if used after editing the name in the current cell." & vbCrLf & vbCrLf
t = t & "When editing node names, the new Edit or Select Node Name dialog can help select node names used in other parts of the file. The node names " _
      & "can be selected based on words that appear in the names, by object, or by field." & vbCrLf & vbCrLf
t = t & "Use VIEW .. VALIDITY CHECK to check for invalid references, out-of-range numeric values, and invalid choices. " _
      & "This expands on the old Check Out-Of-Range function. " _
      & "Cells with invalid references or invalid choices are now highlighted. " & vbCrLf & vbCrLf

t = t & "New Features of IDF Editor Version 1.37" & vbCrLf
t = t & "-------------------------------------------------------------------------------" & vbCrLf
t = t & "Added support for SI to Inch-Pound unit conversions for fields that can be multiple units including "
t = t & "schedules, fluid properties, and curves. For curves, only the minimum and maximum are converted not the coefficients. " & vbCrLf & vbCrLf

t = t & "New Features of IDF Editor Version 1.34" & vbCrLf
t = t & "-------------------------------------------------------------------------------" & vbCrLf
t = t & "The dataset files may now be accessed directly using the FILE ... OPEN DATASET option." & vbCrLf & vbCrLf

t = t & "New Features of IDF Editor Version 1.33" & vbCrLf
t = t & "-------------------------------------------------------------------------------" & vbCrLf
t = t & "Blue fields indicate that the field is considered a required field in EnergyPlus." & vbCrLf & vbCrLf
t = t & "The text that appear in fields and field values now can wrap on to multiple lines by chosing VIEW .. WORD WRAP. This allows " _
      & "for long values and field names to be completely shown. " & vbCrLf & vbCrLf
t = t & "Recently used files can be quickly opened. They are listed under the file menu. Up to six recent files will be shown." & vbCrLf & vbCrLf
t = t & "New features are shown in this Whats New dialog box which is available under the HELP .. WHATS NEW." & vbCrLf & vbCrLf

t = t & "Other Recent New Features" & vbCrLf
t = t & "-------------------------------------------------------------------------------" & vbCrLf
t = t & "Objects that contain values that are above the maximum value or below the minimum value allowed in EnergyPlus can be found by " _
      & "using VIEW .. CHECK OUT-OF-RANGE.  Also this check can be done every time the file is saved by checking " _
      & "VIEW .. OUT-OF-RANGE CHECK ON SAVE." & vbCrLf & vbCrLf
t = t & "An alternative method to quickly navigate the CLASS LIST is to use VIEW .. SHOW QUICK SELECT DROP DOWNS. This option allows " _
      & "dropdown boxes above the CLASS LIST to find the group and class quickly. Typing the first few letters finds the class even " _
      & "faster." & vbCrLf & vbCrLf
t = t & "To change the order that objects are saved in the IDF file choose FILE .. SAVE OPTIONS. This option allows " _
      & "the original order of objects in a file to be preserved." & vbCrLf & vbCrLf




txtWhatsNew.Text = t
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
