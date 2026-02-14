VERSION 5.00
Object = "{831FDD16-0C5C-11D2-A9FC-0000F8754DA1}#2.0#0"; "mscomctl.OCX"
Begin VB.Form frmNodeSelect
   BorderStyle     =   3  'Fixed Dialog
   Caption         =   "Edit or Select Node Name"
   ClientHeight    =   8520
   ClientLeft      =   45
   ClientTop       =   150
   ClientWidth     =   19485
   LinkTopic       =   "Form1"
   MaxButton       =   0   'False
   MinButton       =   0   'False
   ScaleHeight     =   8520
   ScaleWidth      =   19485
   ShowInTaskbar   =   0   'False
   StartUpPosition =   1  'CenterOwner
   Begin VB.ListBox lstCurrentClassObjField
      BackColor       =   &H8000000F&
      Height          =   645
      ItemData        =   "frmNodeSelect.frx":0000
      Left            =   2040
      List            =   "frmNodeSelect.frx":000D
      TabIndex        =   17
      Top             =   120
      Width           =   8775
   End
   Begin VB.TextBox txtCurrentNodeName
      Height          =   285
      Left            =   2040
      TabIndex        =   15
      Text            =   "Inlet Node A"
      Top             =   840
      Width           =   8760
   End
   Begin VB.ListBox lstContaining
      Height          =   5715
      ItemData        =   "frmNodeSelect.frx":005C
      Left            =   14280
      List            =   "frmNodeSelect.frx":0087
      Sorted          =   -1  'True
      TabIndex        =   12
      Top             =   1560
      Width           =   4815
   End
   Begin VB.ListBox lstOtherAppearances
      BackColor       =   &H8000000F&
      Height          =   1425
      ItemData        =   "frmNodeSelect.frx":01DB
      Left            =   120
      List            =   "frmNodeSelect.frx":01E8
      TabIndex        =   7
      Top             =   6840
      Width           =   6495
   End
   Begin VB.TextBox txtSearchText
      Height          =   375
      Left            =   8760
      TabIndex        =   6
      Top             =   7440
      Width           =   2655
   End
   Begin VB.CommandButton cmdCancel
      Cancel          =   -1  'True
      Caption         =   "Cancel"
      Height          =   375
      Left            =   9960
      TabIndex        =   3
      Top             =   8040
      Width           =   1500
   End
   Begin VB.CommandButton cmdOK
      Caption         =   "OK"
      Default         =   -1  'True
      Height          =   375
      Left            =   8280
      TabIndex        =   2
      Top             =   8040
      Width           =   1500
   End
   Begin VB.ListBox lstOtherNodeNames
      Height          =   4935
      Left            =   120
      Sorted          =   -1  'True
      TabIndex        =   0
      Top             =   1560
      Width           =   6495
   End
   Begin VB.OptionButton optAll
      Caption         =   "All"
      Height          =   255
      Left            =   2400
      TabIndex        =   4
      Top             =   1320
      Width           =   495
   End
   Begin VB.OptionButton optRecent
      Caption         =   "Recent"
      Height          =   255
      Left            =   3120
      TabIndex        =   5
      Top             =   1320
      Width           =   855
   End
   Begin VB.OptionButton optContaining
      Caption         =   "Containing"
      Height          =   255
      Left            =   4200
      TabIndex        =   10
      Top             =   1320
      Width           =   1095
   End
   Begin VB.OptionButton OptClassOrField
      Caption         =   "Class or Field"
      Height          =   255
      Left            =   5400
      TabIndex        =   11
      Top             =   1320
      Value           =   -1  'True
      Width           =   1335
   End
   Begin MSComctlLib.TreeView treeClassField
      Height          =   5715
      Left            =   8400
      TabIndex        =   18
      Top             =   1560
      Width           =   4815
      _ExtentX        =   8493
      _ExtentY        =   10081
      _Version        =   393217
      LabelEdit       =   1
      LineStyle       =   1
      Sorted          =   -1  'True
      Style           =   6
      SingleSel       =   -1  'True
      Appearance      =   1
   End
   Begin VB.Label Label6
      Caption         =   "Node Name"
      Height          =   255
      Left            =   120
      TabIndex        =   16
      Top             =   840
      Width           =   1575
   End
   Begin VB.Label lblSelection
      Caption         =   "Selection"
      Height          =   255
      Left            =   7080
      TabIndex        =   14
      Top             =   1320
      Width           =   3615
   End
   Begin VB.Label Label2
      Caption         =   "Containing Text"
      Height          =   375
      Left            =   7560
      TabIndex        =   13
      Top             =   7440
      Width           =   1575
   End
   Begin VB.Line Line2
      X1              =   0
      X2              =   10800
      Y1              =   1200
      Y2              =   1200
   End
   Begin VB.Label Label3
      Caption         =   "Class/Object Name/Field"
      Height          =   255
      Left            =   120
      TabIndex        =   9
      Top             =   120
      Width           =   1935
   End
   Begin VB.Label Label7
      Caption         =   "Where Selected Other Node Name Appears in File"
      Height          =   255
      Left            =   120
      TabIndex        =   8
      Top             =   6600
      Width           =   5775
   End
   Begin VB.Label Label1
      Caption         =   "Other Node Names"
      Height          =   255
      Left            =   120
      TabIndex        =   1
      Top             =   1320
      Width           =   1935
   End
End
Attribute VB_Name = "frmNodeSelect"
Attribute VB_GlobalNameSpace = False
Attribute VB_Creatable = False
Attribute VB_PredeclaredId = True
Attribute VB_Exposed = False
Option Explicit

Public currentNodeName As String   'the main result and input to the dialog box
Dim origCurrentNodeName As String  'copy of the original value of the current value

Public currentClassName As String  'to remind user of currently edited cell
Public currentObjectName As String 'to remind user of currently edited cell
Public currentFieldName As String  'to remind user of currently edited cell

Dim otherNodeNamesMode As Integer
Const onnmAll = 1     'show all node names
Const onnmRecent = 2  'show only recent node names that have been editted are in list
Const onnmContain = 3 'show only node names containing text shown in list
Const onnmObjFld = 4  'show only node names that are used in selected object of field

Dim curSelectedClassFieldKey As String

Dim uniqueNames() As String
Dim numUniqueNames As Integer
Dim sizeUniqueNames As Integer


'-----------------------------------------------------------------------------
' When the form first loads, initialize the dialog box
'-----------------------------------------------------------------------------
Private Sub Form_Load()
Dim i As Long
Me.Width = 12060
origCurrentNodeName = currentNodeName
txtCurrentNodeName.Text = currentNodeName
lstCurrentClassObjField.Clear
lstCurrentClassObjField.AddItem currentClassName
lstCurrentClassObjField.AddItem "    " & currentObjectName
lstCurrentClassObjField.AddItem "        " & currentFieldName
lstOtherAppearances.Clear
optAll.Value = True
otherNodeNamesMode = onnmAll
ReDim uniqueNames(1000)
Call initializeListOfRecent
Call initializeWordsContained
Call initializeObjectsFields
Call updateOtherNodeList
End Sub
Private Sub Form_Activate()
txtCurrentNodeName.SetFocus
txtCurrentNodeName.SelStart = 0
txtCurrentNodeName.SelLength = Len(txtCurrentNodeName.Text)
End Sub


'-----------------------------------------------------------------------------
' User has pressed OK so return response to main editor form
'-----------------------------------------------------------------------------
Private Sub cmdOK_Click()
currentNodeName = Trim(txtCurrentNodeName.Text)
Unload Me
End Sub

'-----------------------------------------------------------------------------
' User has pressed CANCEL so return original value to main editor form
'-----------------------------------------------------------------------------
Private Sub cmdCancel_Click()
currentNodeName = origCurrentNodeName
Unload Me
End Sub


'-----------------------------------------------------------------------------
' When the user clicks on the main list of other node names, put the name
' in the active edit field and display where else it is used in the file
'-----------------------------------------------------------------------------
Private Sub lstOtherNodeNames_Click()
Dim curName As String
Dim i As Long
curName = lstOtherNodeNames.List(lstOtherNodeNames.ListIndex)
txtCurrentNodeName.Text = curName
lstOtherAppearances.Clear
For i = 1 To maxNodeNameDialog
  If nodeNameDialog(i).name = curName Then
    If nodeNameDialog(i).clsIndx > 0 Then
      lstOtherAppearances.AddItem IDDClassDat(nodeNameDialog(i).clsIndx).name
    End If
    lstOtherAppearances.AddItem "    " & nodeNameDialog(i).objName
    If nodeNameDialog(i).fldIndx > 0 Then
      lstOtherAppearances.AddItem "        " & IDDField(nodeNameDialog(i).fldIndx).name
    End If
  End If
Next i
End Sub

'-----------------------------------------------------------------------------
' Option buttons for other node list
'-----------------------------------------------------------------------------
Private Sub optAll_Click()
otherNodeNamesMode = onnmAll
Call updateOtherNodeList
End Sub
Private Sub optRecent_Click()
otherNodeNamesMode = onnmRecent
Call updateOtherNodeList
End Sub
Private Sub optContaining_Click()
otherNodeNamesMode = onnmContain
Call updateOtherNodeList
End Sub
Private Sub OptClassOrField_Click()
otherNodeNamesMode = onnmObjFld
Call updateOtherNodeList
End Sub

'-----------------------------------------------------------------------------
' Based on the selections update the Tree Select
'-----------------------------------------------------------------------------
Sub updateTreeSelect()
End Sub

'-----------------------------------------------------------------------------
' Based on the selections update the Other Node List
' view or hide the list containing other words or the tree of objects and fields
'-----------------------------------------------------------------------------
Sub updateOtherNodeList()
Dim searchText As String
Dim searchWord As String
Dim useItem() As Boolean
Dim curClass As Long
Dim curField As Long
Dim words() As String
Dim found As Long
Dim i As Long
ReDim useItem(maxNodeNameDialog)
'first set all flags to display items to true
For i = 1 To maxNodeNameDialog
  useItem(i) = True
Next i
'second set flag for items that should be used that are unique
If otherNodeNamesMode <> onnmObjFld Then
  For i = 1 To maxNodeNameDialog
    If Not nodeNameDialog(i).isUnique Then
      useItem(i) = False
    End If
  Next i
End If
Select Case otherNodeNamesMode
  Case onnmAll
    treeClassField.Left = 20000
    lstContaining.Left = 20000
    lblSelection.Visible = False
  Case onnmRecent
    treeClassField.Left = 20000
    lstContaining.Left = 20000
    lblSelection.Visible = False
    For i = 1 To maxNodeNameDialog
      If Not nodeNameDialog(i).isRecent Then
        useItem(i) = False
      End If
    Next i
  Case onnmContain
    treeClassField.Left = 20000
    lstContaining.Left = 7080
    lblSelection.Caption = "Filter by Contents"
    lblSelection.Visible = True
    searchWord = UCase(lstContaining.List(lstContaining.ListIndex))
    If Len(searchWord) > 0 Then
      For i = 1 To maxNodeNameDialog
        If useItem(i) Then
          'if search text edit box has contents then only include
          'those that match the contents
          If InStr(UCase(nodeNameDialog(i).name), searchWord) = 0 Then
            useItem(i) = False
          End If
        End If
      Next i
    End If
  Case onnmObjFld
    treeClassField.Left = 7080
    lstContaining.Left = 20000
    lblSelection.Caption = "Filter by Object or Field"
    lblSelection.Visible = True
    If curSelectedClassFieldKey <> "" Then
      words = Split(curSelectedClassFieldKey, "-")
      If UBound(words) = 2 Then 'key contains class and field
        curField = Val(words(2))
        If curField > 0 Then
          For i = 1 To maxNodeNameDialog
            If nodeNameDialog(i).fldIndx <> curField Then
              useItem(i) = False
            End If
          Next i
        End If
      ElseIf UBound(words) = 1 Then 'key contains class only
        curClass = Val(words(1))
        If curClass > 0 Then
          For i = 1 To maxNodeNameDialog
            If nodeNameDialog(i).clsIndx <> curClass Then
              useItem(i) = False
            End If
          Next i
        End If
      End If
    End If
End Select
'eliminate those that do not match the search term
searchText = Trim(UCase(txtSearchText.Text))
If Len(searchText) > 0 Then
  For i = 1 To maxNodeNameDialog
    If useItem(i) Then
      'if search text edit box has contents then only include
      'those that match the contents
      If InStr(UCase(nodeNameDialog(i).name), searchText) = 0 Then
        useItem(i) = False
      End If
    End If
  Next i
End If
'for what ever items are left (useItem is true) put in list
lstOtherNodeNames.Clear
For i = 1 To maxNodeNameDialog
  If useItem(i) Then
    lstOtherNodeNames.AddItem nodeNameDialog(i).name
  End If
Next i
'select the item on the list that matches in the input field or
'else the first item on the list
found = -1
For i = 0 To lstOtherNodeNames.ListCount - 1
  If lstOtherNodeNames.List(i) = currentNodeName Then
    found = i
    Exit For
  End If
Next i
If lstOtherNodeNames.ListCount >= 1 Then
  If found >= 0 Then
    lstOtherNodeNames.ListIndex = i
  Else
    lstOtherNodeNames.ListIndex = 0
  End If
End If
End Sub



'-----------------------------------------------------------------------------
' User has changed the contents of the search text box
'-----------------------------------------------------------------------------
Private Sub txtSearchText_Change()
Call updateOtherNodeList
End Sub

'-----------------------------------------------------------------------------
' Initialize the list of words containing in existing node names
'-----------------------------------------------------------------------------
Sub initializeWordsContained()
Dim words() As String
Dim found As Boolean
Dim i As Long
Dim j As Long
Dim k As Long
lstContaining.Clear
For i = 1 To maxNodeNameDialog
  If nodeNameDialog(i).isUnique Then
    'now parse each one for separate words
    words = Split(nodeNameDialog(i).name, " ")
    For j = 0 To UBound(words)
      found = False
      For k = 0 To lstContaining.ListCount - 1
        If lstContaining.List(k) = Trim(words(j)) Then
          found = True
          Exit For
        End If
      Next k
      If Not found Then
        lstContaining.AddItem Trim(words(j))
      End If
    Next j
  End If
Next i
If lstContaining.ListCount > 0 Then
  lstContaining.ListIndex = 0
End If
End Sub

'-----------------------------------------------------------------------------
' When user clicks on the list of words contained
'-----------------------------------------------------------------------------
Private Sub lstContaining_Click()
Call updateOtherNodeList
End Sub

'-----------------------------------------------------------------------------
' Initialize the objects and fields list
'-----------------------------------------------------------------------------
Sub initializeObjectsFields()
Dim i As Long
Dim j As Long
Dim k As Long
Dim uniqueClass() As Boolean
Dim uniqueField() As Boolean
Dim curClass As Long
treeClassField.Nodes.Clear
ReDim uniqueClass(maxNodeNameDialog)
ReDim uniqueField(maxNodeNameDialog)
'put in all unique class names
For i = 1 To maxNodeNameDialog
  uniqueClass(i) = True
  For j = 1 To i - 1
    If nodeNameDialog(j).clsIndx = nodeNameDialog(i).clsIndx Then
      uniqueClass(i) = False
      Exit For
    End If
  Next j
Next i
For i = 1 To maxNodeNameDialog
  If uniqueClass(i) Then
    curClass = nodeNameDialog(i).clsIndx
    If curClass > 0 Then
      treeClassField.Nodes.Add , , "C-" & Trim(Val(curClass)), IDDClassDat(curClass).name
      For k = IDDClassDat(curClass).fieldStart To IDDClassDat(curClass).fieldEnd
        If IDDField(k).type = 6 Then 'node
          treeClassField.Nodes.Add "C-" & Trim(Val(curClass)), tvwChild, "C-" & Trim(Val(curClass)) & "-" & Trim(Val(k)), IDDField(k).name
        End If
      Next k
    End If
  End If
Next i
'treeClassField.Nodes.Item(1).Root.Child.Selected = True
End Sub

'-----------------------------------------------------------------------------
' When user clicks on the heirarchical list of classes and fields
'-----------------------------------------------------------------------------
Private Sub treeClassField_NodeClick(ByVal Node As MSComctlLib.Node)
Debug.Print "-----------NODECLICK-----"
Debug.Print Node.Text
Debug.Print Node.Key
curSelectedClassFieldKey = Node.Key
Call updateOtherNodeList
End Sub

'-----------------------------------------------------------------------------
' Initialize the list of recently edited nodes by adding the current node
'-----------------------------------------------------------------------------
Sub initializeListOfRecent()


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
