VERSION 5.00
Begin VB.Form frmDuplicateAndChange
   Caption         =   "Duplicate and Change..."
   ClientHeight    =   1695
   ClientLeft      =   60
   ClientTop       =   405
   ClientWidth     =   8220
   LinkTopic       =   "Form1"
   ScaleHeight     =   1695
   ScaleWidth      =   8220
   StartUpPosition =   3  'Windows Default
   Begin VB.CommandButton cmdCancel
      Cancel          =   -1  'True
      Caption         =   "Cancel"
      Height          =   375
      Left            =   6360
      TabIndex        =   5
      Top             =   1200
      Width           =   1575
   End
   Begin VB.CommandButton cmdOk
      Caption         =   "OK"
      Default         =   -1  'True
      Height          =   375
      Left            =   4680
      TabIndex        =   4
      Top             =   1200
      Width           =   1575
   End
   Begin VB.TextBox txtChgInDup
      Height          =   375
      Left            =   2640
      TabIndex        =   3
      Text            =   "Text1"
      Top             =   600
      Width           =   5415
   End
   Begin VB.TextBox txtOriginal
      Height          =   375
      Left            =   2640
      TabIndex        =   0
      Text            =   "Text1"
      Top             =   120
      Width           =   5415
   End
   Begin VB.Label Label2
      Caption         =   "Changed Text in Duplicate"
      Height          =   255
      Left            =   360
      TabIndex        =   2
      Top             =   720
      Width           =   1935
   End
   Begin VB.Label Label1
      Caption         =   "Original Text"
      Height          =   255
      Left            =   360
      TabIndex        =   1
      Top             =   240
      Width           =   1455
   End
End
Attribute VB_Name = "frmDuplicateAndChange"
Attribute VB_GlobalNameSpace = False
Attribute VB_Creatable = False
Attribute VB_PredeclaredId = True
Attribute VB_Exposed = False


Private Sub Form_Load()
txtOriginal.Text = searchTerm
txtChgInDup.Text = searchTerm
End Sub

Private Sub cmdOk_Click()
searchTerm = txtOriginal.Text
replaceTerm = txtChgInDup.Text
Unload Me
End Sub

Private Sub cmdCancel_Click()
replaceTerm = "" ' return a blank if canceled by user
Unload Me
End Sub
