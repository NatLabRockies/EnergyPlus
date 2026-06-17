#tag Window
Begin Window Window1
   BackColor       =   &cFFFFFF00
   Backdrop        =   0
   CloseButton     =   True
   Composite       =   False
   Frame           =   0
   FullScreen      =   False
   FullScreenButton=   False
   HasBackColor    =   False
   Height          =   230
   ImplicitInstance=   True
   LiveResize      =   "True"
   MacProcID       =   0
   MaxHeight       =   200
   MaximizeButton  =   False
   MaxWidth        =   32000
   MenuBar         =   0
   MenuBarVisible  =   True
   MinHeight       =   200
   MinimizeButton  =   True
   MinWidth        =   662
   Placement       =   2
   Resizeable      =   True
   Title           =   "IDF Version Updater"
   Visible         =   True
   Width           =   802
   Begin TextField txtFileName
      AcceptTabs      =   False
      Alignment       =   0
      AutoDeactivate  =   True
      AutomaticallyCheckSpelling=   False
      BackColor       =   &cFFFFFF00
      Bold            =   False
      Border          =   True
      CueText         =   ""
      DataField       =   ""
      DataSource      =   ""
      Enabled         =   False
      Format          =   ""
      Height          =   24
      HelpTag         =   "The text field shows the file selected when the CHOOSE FILE TO UPDATE button is pressed and a IDF, IDM or LST file is selected."
      Index           =   -2147483648
      Italic          =   False
      Left            =   20
      LimitText       =   0
      LockBottom      =   False
      LockedInPosition=   False
      LockLeft        =   True
      LockRight       =   True
      LockTop         =   True
      Mask            =   ""
      Password        =   False
      ReadOnly        =   False
      Scope           =   0
      TabIndex        =   0
      TabPanelIndex   =   0
      TabStop         =   True
      Text            =   ""
      TextColor       =   &c00000000
      TextFont        =   "System"
      TextSize        =   0.0
      TextUnit        =   0
      Top             =   47
      Transparent     =   True
      Underline       =   False
      UseFocusRing    =   True
      Visible         =   True
      Width           =   762
   End
   Begin BevelButton cmdChooseFile
      AcceptFocus     =   False
      AutoDeactivate  =   True
      BackColor       =   &c00000000
      Bevel           =   0
      Bold            =   False
      ButtonType      =   0
      Caption         =   "Choose File to Update..."
      CaptionAlign    =   3
      CaptionDelta    =   0
      CaptionPlacement=   1
      Enabled         =   True
      HasBackColor    =   False
      HasMenu         =   0
      Height          =   27
      HelpTag         =   "By pressing this button you can choose an IDF, an IMF, or a LST file. The file that you selected will be shown in the large text box below. When an IDF or IMF file is selected the current version of the file will be shown in the CURRENT VERSION box.  For an LST file, the CURRENT VERSION is not used. The LST file contains a list of IDF or IMF files, one per line, with full paths."
      Icon            =   0
      IconAlign       =   0
      IconDX          =   0
      IconDY          =   0
      Index           =   -2147483648
      InitialParent   =   ""
      Italic          =   False
      Left            =   20
      LockBottom      =   False
      LockedInPosition=   False
      LockLeft        =   True
      LockRight       =   False
      LockTop         =   True
      Scope           =   0
      TabIndex        =   1
      TabPanelIndex   =   0
      TabStop         =   True
      TextColor       =   &c00000000
      TextFont        =   "System"
      TextSize        =   0.0
      TextUnit        =   0
      Top             =   14
      Transparent     =   True
      Underline       =   False
      Value           =   False
      Visible         =   True
      Width           =   283
   End
   Begin BevelButton cmdConvert
      AcceptFocus     =   False
      AutoDeactivate  =   True
      BackColor       =   &c00000000
      Bevel           =   0
      Bold            =   False
      ButtonType      =   0
      Caption         =   "Update File..."
      CaptionAlign    =   3
      CaptionDelta    =   0
      CaptionPlacement=   1
      Enabled         =   False
      HasBackColor    =   False
      HasMenu         =   0
      Height          =   27
      HelpTag         =   "Press this button to update the selected file shown above to the version shown in NEW VERSION. The file can be selected by pressing the CHOOSE FILE TO UPDATE button. After this button is pressed, the PROCESSING message is shown. It may take up to a minute to update the file depending on the CURRENT VERSION of the file. \r\n\r\nThe original file is renamed with the word _ORIGINAL at the end of the name if DELETE ORIGINAL FILES is not checked.\r\n\r\nIf CREATE INTERMEDIATE VERSION FILES is checked then files compatible with versions between the CURRENT VERSION and NEW VERSION are kept and named ending with  _Vx-x-x in the same directory as the original file."
      Icon            =   0
      IconAlign       =   0
      IconDX          =   0
      IconDY          =   0
      Index           =   -2147483648
      InitialParent   =   ""
      Italic          =   False
      Left            =   20
      LockBottom      =   True
      LockedInPosition=   False
      LockLeft        =   True
      LockRight       =   False
      LockTop         =   False
      Scope           =   0
      TabIndex        =   2
      TabPanelIndex   =   0
      TabStop         =   True
      TextColor       =   &c00000000
      TextFont        =   "System"
      TextSize        =   0.0
      TextUnit        =   0
      Top             =   183
      Transparent     =   True
      Underline       =   False
      Value           =   False
      Visible         =   True
      Width           =   283
   End
   Begin BevelButton cmdExit
      AcceptFocus     =   False
      AutoDeactivate  =   True
      BackColor       =   &c00000000
      Bevel           =   0
      Bold            =   False
      ButtonType      =   0
      Caption         =   "Close"
      CaptionAlign    =   3
      CaptionDelta    =   0
      CaptionPlacement=   1
      Enabled         =   True
      HasBackColor    =   False
      HasMenu         =   0
      Height          =   27
      HelpTag         =   "Press this button to exit the program."
      Icon            =   0
      IconAlign       =   0
      IconDX          =   0
      IconDY          =   0
      Index           =   -2147483648
      InitialParent   =   ""
      Italic          =   False
      Left            =   639
      LockBottom      =   True
      LockedInPosition=   False
      LockLeft        =   False
      LockRight       =   True
      LockTop         =   False
      Scope           =   0
      TabIndex        =   3
      TabPanelIndex   =   0
      TabStop         =   True
      TextColor       =   &c00000000
      TextFont        =   "System"
      TextSize        =   0.0
      TextUnit        =   0
      Top             =   183
      Transparent     =   True
      Underline       =   False
      Value           =   False
      Visible         =   True
      Width           =   143
   End
   Begin CheckBox chkIntermediate
      AutoDeactivate  =   True
      Bold            =   False
      Caption         =   "Create Intermediate Version Files"
      DataField       =   ""
      DataSource      =   ""
      Enabled         =   True
      Height          =   20
      HelpTag         =   "By checking this box, intermediate versions of the file will be saved after you press the UPDATE FILE button. This is only applicable when you are updating a file from a version of EnergyPlus that is at least two versions old. The intermediate files will be in the same directory as the updated file. The names of the intermediate files will have names that end with _Vx-x-x.idf."
      Index           =   -2147483648
      InitialParent   =   ""
      Italic          =   False
      Left            =   468
      LockBottom      =   False
      LockedInPosition=   False
      LockLeft        =   False
      LockRight       =   True
      LockTop         =   True
      Scope           =   0
      State           =   0
      TabIndex        =   9
      TabPanelIndex   =   0
      TabStop         =   True
      TextFont        =   "System"
      TextSize        =   0.0
      TextUnit        =   0
      Top             =   84
      Transparent     =   True
      Underline       =   False
      Value           =   False
      Visible         =   True
      Width           =   314
   End
   Begin Label Label2
      AutoDeactivate  =   True
      Bold            =   False
      DataField       =   ""
      DataSource      =   ""
      Enabled         =   True
      Height          =   20
      HelpTag         =   ""
      Index           =   -2147483648
      InitialParent   =   ""
      Italic          =   False
      Left            =   188
      LockBottom      =   False
      LockedInPosition=   False
      LockLeft        =   True
      LockRight       =   False
      LockTop         =   True
      Multiline       =   False
      Scope           =   0
      Selectable      =   False
      TabIndex        =   10
      TabPanelIndex   =   0
      TabStop         =   True
      Text            =   "New Version"
      TextAlign       =   0
      TextColor       =   &c00000000
      TextFont        =   "System"
      TextSize        =   0.0
      TextUnit        =   0
      Top             =   83
      Transparent     =   False
      Underline       =   False
      Visible         =   True
      Width           =   144
   End
   Begin Label Label3
      AutoDeactivate  =   True
      Bold            =   True
      DataField       =   ""
      DataSource      =   ""
      Enabled         =   True
      Height          =   24
      HelpTag         =   ""
      Index           =   -2147483648
      InitialParent   =   ""
      Italic          =   False
      Left            =   127
      LockBottom      =   False
      LockedInPosition=   False
      LockLeft        =   True
      LockRight       =   False
      LockTop         =   True
      Multiline       =   False
      Scope           =   0
      Selectable      =   False
      TabIndex        =   12
      TabPanelIndex   =   0
      TabStop         =   True
      Text            =   "-->"
      TextAlign       =   1
      TextColor       =   &c00000000
      TextFont        =   "System"
      TextSize        =   20.0
      TextUnit        =   0
      Top             =   104
      Transparent     =   False
      Underline       =   False
      Visible         =   True
      Width           =   40
   End
   Begin TextField txtCurrentVersion
      AcceptTabs      =   False
      Alignment       =   0
      AutoDeactivate  =   True
      AutomaticallyCheckSpelling=   False
      BackColor       =   &cFFFFFF00
      Bold            =   False
      Border          =   True
      CueText         =   ""
      DataField       =   ""
      DataSource      =   ""
      Enabled         =   False
      Format          =   ""
      Height          =   25
      HelpTag         =   "This field shows the current version of the file selected and shown above."
      Index           =   -2147483648
      InitialParent   =   ""
      Italic          =   False
      Left            =   29
      LimitText       =   0
      LockBottom      =   False
      LockedInPosition=   False
      LockLeft        =   True
      LockRight       =   False
      LockTop         =   True
      Mask            =   ""
      Password        =   False
      ReadOnly        =   True
      Scope           =   0
      TabIndex        =   13
      TabPanelIndex   =   0
      TabStop         =   True
      Text            =   ""
      TextColor       =   &c00000000
      TextFont        =   "System"
      TextSize        =   0.0
      TextUnit        =   0
      Top             =   105
      Transparent     =   True
      Underline       =   False
      UseFocusRing    =   True
      Visible         =   True
      Width           =   76
   End
   Begin Label Label1
      AutoDeactivate  =   True
      Bold            =   False
      DataField       =   ""
      DataSource      =   ""
      Enabled         =   True
      Height          =   20
      HelpTag         =   ""
      Index           =   -2147483648
      InitialParent   =   ""
      Italic          =   False
      Left            =   29
      LockBottom      =   False
      LockedInPosition=   False
      LockLeft        =   True
      LockRight       =   False
      LockTop         =   True
      Multiline       =   False
      Scope           =   0
      Selectable      =   False
      TabIndex        =   14
      TabPanelIndex   =   0
      TabStop         =   True
      Text            =   "Current Version"
      TextAlign       =   0
      TextColor       =   &c00000000
      TextFont        =   "System"
      TextSize        =   0.0
      TextUnit        =   0
      Top             =   84
      Transparent     =   False
      Underline       =   False
      Visible         =   True
      Width           =   134
   End
   Begin BevelButton cmdViewAudit
      AcceptFocus     =   False
      AutoDeactivate  =   True
      BackColor       =   &c00000000
      Bevel           =   0
      Bold            =   False
      ButtonType      =   0
      Caption         =   "View Audit File..."
      CaptionAlign    =   3
      CaptionDelta    =   0
      CaptionPlacement=   1
      Enabled         =   False
      HasBackColor    =   False
      HasMenu         =   0
      Height          =   27
      HelpTag         =   "By pressing this button, the .audit file is shown. The .audit file shows a log of the file updating process including any problems encountered during the process."
      Icon            =   0
      IconAlign       =   0
      IconDX          =   0
      IconDY          =   0
      Index           =   -2147483648
      InitialParent   =   ""
      Italic          =   False
      Left            =   468
      LockBottom      =   True
      LockedInPosition=   False
      LockLeft        =   False
      LockRight       =   True
      LockTop         =   False
      Scope           =   0
      TabIndex        =   15
      TabPanelIndex   =   0
      TabStop         =   True
      TextColor       =   &c00000000
      TextFont        =   "System"
      TextSize        =   0.0
      TextUnit        =   0
      Top             =   183
      Transparent     =   True
      Underline       =   False
      Value           =   False
      Visible         =   True
      Width           =   146
   End
   Begin CheckBox chkDeleteOrig
      AutoDeactivate  =   True
      Bold            =   False
      Caption         =   "Delete Original Files"
      DataField       =   ""
      DataSource      =   ""
      Enabled         =   True
      Height          =   18
      HelpTag         =   "By checking this box when the UPDATE FILE is pressed the original file is not saved but instead is deleted after the updating is complete."
      Index           =   -2147483648
      InitialParent   =   ""
      Italic          =   False
      Left            =   468
      LockBottom      =   False
      LockedInPosition=   False
      LockLeft        =   False
      LockRight       =   True
      LockTop         =   True
      Scope           =   0
      State           =   0
      TabIndex        =   16
      TabPanelIndex   =   0
      TabStop         =   True
      TextFont        =   "System"
      TextSize        =   0.0
      TextUnit        =   0
      Top             =   110
      Transparent     =   True
      Underline       =   False
      Value           =   False
      Visible         =   True
      Width           =   307
   End
   Begin PopupMenu pmnuNewVersion
      AutoDeactivate  =   True
      Bold            =   False
      DataField       =   ""
      DataSource      =   ""
      Enabled         =   True
      Height          =   23
      HelpTag         =   "You can select the version of EnergyPlus that you want the file updated to. By default you the most recent version of EnergyPlus is used but you can select any version after the current version. If an LST file has been selected, only the most recent version of EnergyPlus can be selected."
      Index           =   -2147483648
      InitialParent   =   ""
      InitialValue    =   ""
      Italic          =   False
      Left            =   188
      ListIndex       =   0
      LockBottom      =   False
      LockedInPosition=   False
      LockLeft        =   True
      LockRight       =   False
      LockTop         =   True
      Scope           =   0
      TabIndex        =   17
      TabPanelIndex   =   0
      TabStop         =   True
      TextFont        =   "System"
      TextSize        =   0.0
      TextUnit        =   0
      Top             =   105
      Transparent     =   True
      Underline       =   False
      Visible         =   True
      Width           =   115
   End
   Begin Label lblProcessing
      AutoDeactivate  =   True
      Bold            =   False
      DataField       =   ""
      DataSource      =   ""
      Enabled         =   True
      Height          =   39
      HelpTag         =   ""
      Index           =   -2147483648
      InitialParent   =   ""
      Italic          =   False
      Left            =   20
      LockBottom      =   False
      LockedInPosition=   False
      LockLeft        =   True
      LockRight       =   False
      LockTop         =   True
      Multiline       =   False
      Scope           =   0
      Selectable      =   False
      TabIndex        =   18
      TabPanelIndex   =   0
      TabStop         =   True
      Text            =   "Processing"
      TextAlign       =   0
      TextColor       =   &c0000FF00
      TextFont        =   "System"
      TextSize        =   0.0
      TextUnit        =   0
      Top             =   140
      Transparent     =   False
      Underline       =   False
      Visible         =   False
      Width           =   762
   End
   Begin BevelButton cmdAbout
      AcceptFocus     =   False
      AutoDeactivate  =   True
      BackColor       =   &c00000000
      Bevel           =   0
      Bold            =   False
      ButtonType      =   0
      Caption         =   "About"
      CaptionAlign    =   3
      CaptionDelta    =   0
      CaptionPlacement=   1
      Enabled         =   True
      HasBackColor    =   False
      HasMenu         =   0
      Height          =   22
      HelpTag         =   ""
      Icon            =   0
      IconAlign       =   0
      IconDX          =   0
      IconDY          =   0
      Index           =   -2147483648
      InitialParent   =   ""
      Italic          =   False
      Left            =   732
      LockBottom      =   False
      LockedInPosition=   False
      LockLeft        =   False
      LockRight       =   True
      LockTop         =   True
      Scope           =   0
      TabIndex        =   19
      TabPanelIndex   =   0
      TabStop         =   True
      TextColor       =   &c00000000
      TextFont        =   "System"
      TextSize        =   0.0
      TextUnit        =   0
      Top             =   7
      Transparent     =   True
      Underline       =   False
      Value           =   False
      Visible         =   True
      Width           =   50
   End
End
#tag EndWindow

#tag WindowCode
	#tag Event
		Sub Open()
		  dim msgResult as Integer
		  call gatherAllPossibleVersions
		  lblProcessing.Visible = False
		  if AllNewVersions.Ubound < 0 then
		    msgResult = MsgBox("No TransitionV-x-x-to-V-x-x.exe files found. The directory that IDFVersionUpdater is located should contain TransitionV-x-x-to-V-x-x files.",16,"IDF Version Updater")
		    quit
		  end if
		End Sub
	#tag EndEvent


	#tag Method, Flags = &h0
		Sub ConvertFile(inFile as string, inOrigVersion as String, inFinalVersion as String, showSummary as Boolean)
		  dim inFileNoExt as String
		  dim origFile as FolderItem
		  dim origVerIndx as Integer
		  dim finalVerIndx as Integer
		  dim iVer as Integer
		  dim cmdLine as String
		  dim dirOfTransApp as FolderItem
		  dim s as Shell
		  dim origFileOrigDate as FolderItem
		  dim origFileInter as FolderItem
		  dim origFileWithNewName as String
		  dim auditSource as FolderItem
		  dim auditText as String = ""
		  dim SourceStream as TextInputStream
		  dim OutStream as TextOutputStream
		  dim auditOutput as FolderItem
		  dim inFileExt as String
		  dim rviOrigFile as FolderItem
		  dim mviOrigFile as FolderItem
		  dim rviOrigDate as Date
		  dim mviOrigDate As Date
		  dim shortVersion as String =  ""
		  dim origShortVersion as String =""

		  const notFound = -1 'special meaning because used with IndexOf

		  s = new Shell
		  s.TimeOut = -1  'number of millisecond until shell is automatically terminated - a negative one indicates never to time out
		  inFileNoExt = fileNameWithoutExtension(inFile)
		  inFileExt = ExtensionOnly(inFile)

		  if inFile<>"" then
		    origFile = new FolderItem(inFile)
		    origVerIndx = AllOldVersions.IndexOf(inOrigVersion)
		    origShortVersion = "_V" + inOrigVersion.ReplaceAll(".","") ' make the version identifier for the end of a file name "_V320" if the version is 3.2.0
		    finalVerIndx = AllNewVersions.IndexOf(inFinalVersion)
		    rviOrigFile = new FolderItem(inFileNoExt + ".rvi")
		    rviOrigDate = rviOrigFile.ModificationDate
		    mviOrigFile = new FolderItem(inFileNoExt + ".mvi")
		    mviOrigDate = mviOrigFile.ModificationDate
		    if origFile.Exists and origVerIndx<>notFound and finalVerIndx<>notFound and origVerIndx <= finalVerIndx then
		      'indicate the dialog box is waiting
		      lblProcessing.Visible = True
		      lblProcessing.Text = "Processing: " + origFile.Name
		      dirOfTransApp = transitionapps(iVer).Parent
		      'msgbox origFile.AbsolutePath + EndOfLine + origVer + EndOfLine + finalVer
		      'if original file should be retained then make copy with name ending with "_original.idf"
		      if not chkDeleteOrig.Value then
		        'IDF/IMF file handling
		        origFileWithNewName =  inFileNoExt  + origShortVersion +"." + inFileExt
		        origFileOrigDate = new FolderItem(origFileWithNewName)
		        if origFileOrigDate.Exists then
		          origFileOrigDate.Delete
		        end if
		        try
		          origFile.CopyFileTo(origFileOrigDate) 'make copy of original file with new "_Vxxx.idf" ending with original version
		        exception
		          MsgBox "Cannot create original .idf file for: " + inFile
		        end try
		        'RVI file - make original version with same file date and time
		        if rviOrigFile.exists then
		          origFileWithNewName =  inFileNoExt + origShortVersion + ".rvi"
		          origFileOrigDate = new FolderItem(origFileWithNewName)
		          if origFileOrigDate.Exists then
		            origFileOrigDate.Delete
		          end if
		          try
		            rviOrigFile.CopyFileTo(origFileOrigDate) 'make copy of original file with new "_Vxxx.rvi" ending
		          exception
		            MsgBox "Cannot create  original .rvi file for: " + inFileNoExt + ".rvi"
		          end try
		        end if
		        'MVI file handling - make original version with same file date and time
		        if mviOrigFile.exists then
		          origFileWithNewName =  inFileNoExt + origShortVersion + ".mvi"
		          origFileOrigDate = new FolderItem(origFileWithNewName)
		          if origFileOrigDate.Exists then
		            origFileOrigDate.Delete
		          end if
		          try
		            mviOrigFile.CopyFileTo(origFileOrigDate) 'make copy of original file with new "_Vxxx.mvi" ending with original version
		          exception
		            MsgBox "Cannot create original .mvi file for: " + inFileNoExt + ".mvi"
		          end try
		        end if
		      end if
		      'delete the Transition.audit file
		      auditSource = new FolderItem(dirOfTransApp.NativePath + "Transition.audit")
		      if auditSource.Exists then
		        auditSource.Delete
		      end if
		      for iVer = origVerIndx to finalVerIndx
		        shortVersion = "_V" + AllNewVersions(iVer).ReplaceAll(".","") ' Convert "3.1.0" format to "_V310"
		        lblProcessing.Text = "Processing: " + origFile.Name +  "  Into: " + AllNewVersions(iVer)
		        Window1.Refresh
		        'msgbox dirOfTransApp.AbsolutePath + EndOfLine + TransitionApps(iVer).AbsolutePath + EndOfLine + origFile.AbsolutePath

		        'for windows and linux
		        #if TargetWindows or TargetLinux
		          cmdLine = TransitionApps(iVer).NativePath + " " + chr(34) + origFile.NativePath + chr(34)
		        #else
		          'for MacOS assuming using Bash shell
		          cmdLine = "cd " ' this changes the directory and drive - must end with a space
		          cmdLine = cmdLine  + dirOfTransApp.ShellPath 'change to the directory of the transition app because needs to find IDD files in that location - use shellpath to eliminate spaces
		          cmdLine = cmdLine + " ; ./" 'this is DOS function that joins to commands on the same line and only does the second if the first was successful - need spaces before and after
		          cmdLine = cmdLine + TransitionApps(iVer).name 'this is the name of the Transition program
		          cmdLine = cmdLine + " " ' leave a space before the argument
		          cmdLine = cmdLine +  origFile.ShellPath  'pass the name of the file to be transitioned as an argument
		        #endif


		        ' for debugging
		        'msgbox cmdLine
		        'dim clip as new Clipboard
		        'clip.Text = cmdLine

		        s.Execute cmdLine
		        'MsgBox "transition" + EndOfLine + s.ReadAll

		        if s.ErrorCode <>0 then
		          MsgBox "Error starting Transition-Vx-x-x Program: " + str(s.ErrorCode) + EndOfLine + EndOfLine + s.Result
		        end if

		        'collect audit text
		        if auditSource.Exists then
		          SourceStream = TextInputStream.Open(auditSource)
		          auditText = auditText + EndOfLine + SourceStream.ReadAll 'append the latest transition.audit file text
		          SourceStream.Close
		          auditSource.Delete 'delete the audit file
		        end if

		        'save intermediate file names
		        if chkIntermediate.Value and iVer<finalVerIndx then
		          origFileWithNewName = inFileNoExt +shortVersion + "." + inFileExt
		          origFileInter = new FolderItem(origFileWithNewName)
		          if origFileInter.Exists then
		            origFileInter.Delete
		          end if
		          origFile = new FolderItem(inFile ) 'may not be needed but file handle may be different after Transition is called
		          origFile.CopyFileTo(origFileInter)
		        end if
		      next iVer

		      'delete .idfnew  .idfold  .imfnew .imfold .VCpErr
		      call DeleteFileByName(inFileNoExt + "." + inFileExt + "new")
		      call DeleteFileByName(inFileNoExt + "." + inFileExt + "old")
		      call DeleteFileByName(inFileNoExt + ".VCpErr")
		      call DeleteFileByName(inFileNoExt + ".rvinew")
		      call DeleteFileByName(inFileNoExt + ".rviold")
		      call DeleteFileByName(inFileNoExt + ".mvinew")
		      call DeleteFileByName(inFileNoExt + ".mviold")

		      'if RVI or MVI file was not touched delete _original version
		      if rviOrigFile.ModificationDate.TotalSeconds = rviOrigDate.TotalSeconds then
		        call DeleteFileByName(inFileNoExt + origShortVersion + ".rvi")
		      end if
		      if mviOrigFile.ModificationDate.TotalSeconds = mviOrigDate.TotalSeconds then
		        call DeleteFileByName(inFileNoExt + origShortVersion + ".mvi")
		      end if


		      'create audit file
		      auditOutput = new FolderItem(inFileNoExt + "_transition.audit")
		      if auditOutput.Exists then
		        auditOutput.Delete
		      end if
		      OutStream = TextOutputStream.Create(auditOutput)
		      OutStream.Write(auditText)
		      OutStream.Close

		      'dialog done waiting
		      lblProcessing.Text = "Processing: "
		      lblProcessing.Visible = False
		      cmdConvert.Enabled = False

		      'show summary message
		      if showSummary then
		        MsgBox "Completed transition of:" + EndOfLine + EndOfLine + origFile.NativePath + EndOfLine + EndOfLine + "From " + inOrigVersion + " to " + inFinalVersion
		      end if
		    elseif origFile.Exists and origVerIndx=notFound then
		      ' the file has no VERSION object at all.
		      'make note in audit file
		      auditOutput = new FolderItem(inFileNoExt + "_transition.audit")
		      if auditOutput.Exists then
		        auditOutput.Delete
		      end if
		      OutStream = TextOutputStream.Create(auditOutput)
		      OutStream.WriteLine"No VERSION object found in the file. Please add a VERSION object with the correct version number of the original version of the file and try again."
		      OutStream.WriteLine"  " + inFile
		      OutStream.Close
		    end if
		  end if
		  s.Close

		End Sub
	#tag EndMethod

	#tag Method, Flags = &h0
		Sub CopyIDDandCSV(inFile as string, inOrigVersion as String, inFinalVersion as String)
		  ' for Linux and MacOS copy the IDD and report CSV files to the directory with the active file.
		  dim origFile as FolderItem
		  dim origFileDirectory as FolderItem
		  dim origVerIndx as Integer
		  dim finalVerIndx as Integer
		  dim iVer as Integer
		  dim dashVersion as String
		  dim dirOfTransApp as FolderItem
		  dim iddName as String
		  dim iddFileInTrans as FolderItem
		  dim iddFileInOrig as FolderItem
		  dim nextDashVersion as String
		  dim repcsvName as String
		  dim repcsvFileInTrans as FolderItem
		  dim repcsvFileInOrig as FolderItem

		  origVerIndx = AllNewVersions.IndexOf(inOrigVersion)
		  if origVerIndx = -1 then origVerIndx = AllNewVersions.FirstIndex 'if not found just start at the beginning

		  finalVerIndx = AllNewVersions.IndexOf(inFinalVersion)
		  if finalVerIndx = -1 then finalVerIndx = AllNewVersions.LastIndex 'if not found go until the last

		  if inFile<>"" then
		    origFile = new FolderItem(inFile)
		    origFileDirectory = origFile.Parent
		    for iVer = origVerIndx to finalVerIndx
		      dashVersion = AllNewVersions(iVer).ReplaceAll(".","-") ' Convert "3.1.0" format to "3-1-0"
		      dirOfTransApp = TransitionApps(iVer).Parent
		      ' copy the VXX-X-X-Energy+.idd files
		      iddName = "V" + dashVersion + "-Energy+.idd"
		      iddFileInTrans = dirOfTransApp.Child(iddName)
		      iddFileInOrig = origFileDirectory.Child(iddName)
		      if iddFileInTrans.exists then
		        try
		          iddFileInTrans.CopyFileTo(iddFileInOrig)
		        exception
		          MsgBox "Cannot copy IDD file to local directory: " + iddFileInOrig.NativePath
		        end try
		      end if
		      ' copy the Report Variables XX-X-X to XX-X-X.csv
		      ' don't need to do last iteration
		      if iVer <> finalVerIndx then
		        nextDashVersion = AllNewVersions(iVer + 1).ReplaceAll(".","-")
		        repcsvName = "Report Variables "+ dashVersion + " to " + nextDashVersion + ".csv"
		        repcsvFileInTrans = dirOfTransApp.Child(repcsvName)
		        repcsvFileInOrig = origFileDirectory.Child(repcsvName)
		        if repcsvFileInTrans.exists then
		          try
		            repcsvFileInTrans.CopyFileTo(repcsvFileInOrig)
		          exception
		            MsgBox "Cannot copy report csv file to local directory: " + repcsvFileInOrig.NativePath
		          end try
		        end if
		      end if
		    next iVer
		  end if

		End Sub
	#tag EndMethod

	#tag Method, Flags = &h0
		Sub DeleteCopiedIDDandCSV(inFile as string, inOrigVersion as String, inFinalVersion as String)
		  ' for Linux and MacOS delete the IDD and report CSV files previously copied
		  dim origFile as FolderItem
		  dim origFileDirectory as FolderItem
		  dim origVerIndx as Integer
		  dim finalVerIndx as Integer
		  dim iVer as Integer
		  dim dashVersion as String
		  dim iddName as String
		  dim iddFileInOrig as FolderItem
		  dim nextDashVersion as String
		  dim repcsvName as String
		  dim repcsvFileInOrig as FolderItem
		  dim dirOfTransApp as FolderItem

		  origVerIndx = AllNewVersions.IndexOf(inOrigVersion)
		  if origVerIndx = -1 then origVerIndx = AllNewVersions.FirstIndex 'if not found just start at the beginning

		  finalVerIndx = AllNewVersions.IndexOf(inFinalVersion)
		  if finalVerIndx = -1 then finalVerIndx = AllNewVersions.LastIndex 'if not found go until the last

		  if inFile<>"" then
		    origFile = new FolderItem(inFile)
		    origFileDirectory = origFile.Parent
		    for iVer = origVerIndx to finalVerIndx
		      dirOfTransApp = TransitionApps(iVer).Parent
		      dashVersion = AllNewVersions(iVer).ReplaceAll(".","-") ' Convert "3.1.0" format to "3-1-0"
		      ' delete previously copied VXX-X-X-Energy+.idd files
		      iddName = "V" + dashVersion + "-Energy+.idd"
		      iddFileInOrig = origFileDirectory.Child(iddName)
		      if origFileDirectory.NativePath <> dirOfTransApp.NativePath then
		        if iddFileInOrig.exists then
		          try
		            iddFileInOrig.Remove()
		          exception
		            MsgBox "Cannot remove copied IDD file from local directory: " + iddFileInOrig.NativePath
		          end try
		        end if
		        ' delete previously copied Report Variables XX-X-X to XX-X-X.csv files
		        ' don't need to do last iteration
		        if iVer <> finalVerIndx then
		          nextDashVersion = AllNewVersions(iVer + 1).ReplaceAll(".","-")
		          repcsvName = "Report Variables "+ dashVersion + " to " + nextDashVersion + ".csv"
		          repcsvFileInOrig = origFileDirectory.Child(repcsvName)
		          if repcsvFileInOrig.exists then
		            try
		              repcsvFileInOrig.Remove()
		            exception
		              MsgBox "Cannot remove copied report csv file to local directory: " + repcsvFileInOrig.NativePath
		            end try
		          end if
		        end if
		      end if
		    next iVer
		  end if

		End Sub
	#tag EndMethod

	#tag Method, Flags = &h0
		Sub DeleteFileByName(fileNameIn as String)
		  dim f as FolderItem
		  f = new FolderItem(fileNameIn)
		  if f.Exists then
		    f.Delete
		  end if
		End Sub
	#tag EndMethod

	#tag Method, Flags = &h0
		Function ExtensionOnly(inName as String) As String
		  'returns just a file extension made uppercase
		  'no period is returned just the letters.
		  dim i as Integer = 0
		  dim dotPos as Integer = 0
		  for i = inName.Len DownTo 1
		    if inName.Mid(i,1)= "." then
		      dotPos = i
		      Exit
		    end if
		  next i
		  if dotPos > 0 then
		    return Lowercase(inName.mid(dotPos + 1))
		  end if
		End Function
	#tag EndMethod

	#tag Method, Flags = &h0
		Function fileNameWithoutExtension(inName as String) As String
		  dim i as Integer = 0
		  dim dotPos as Integer = 0
		  for i = inName.Len DownTo 1
		    if inName.Mid(i,1)= "." then
		      dotPos = i
		      Exit
		    end if
		  next i
		  if dotPos > 0 then
		    return inName.left(dotPos - 1)
		  end if
		End Function
	#tag EndMethod

	#tag Method, Flags = &h0
		Sub FillLaterVersions(currentVersion as String)
		  ' this routine fills the new version drop down list with all the versions past the current version
		  dim i as Integer = 0
		  dim foundOld as Integer
		  dim foundNew as Integer
		  const notFound = -1 'special meaning because used with IndexOf

		  'clear the list
		  pmnuNewVersion.DeleteAllRows
		  'first find the version in the list
		  foundOld = AllOldVersions.IndexOf(currentVersion)
		  foundNew = AllNewVersions.IndexOf(currentVersion) 'should usually be one value less than foundOld since transition file names have both the old and new version numbers
		  if foundNew = AllNewVersions.Ubound then
		    ' version number is the same as the latest version
		    pmnuNewVersion.AddRow currentVersion
		    pmnuNewVersion.ListIndex = 0
		    cmdConvert.Enabled = False
		    MsgBox "The file does not need to be updated."
		  elseif foundOld = notFound then
		    'no Transition exe found that can update the file
		    pmnuNewVersion.AddRow currentVersion
		    pmnuNewVersion.ListIndex = 0
		    cmdConvert.Enabled = False
		    MsgBox "No appropriate TransitionV-x-x-to-V-x-x.exe files found. They may need to be downloaded from " + EndOfLine + EndOfLine + _
		    " https://energyplushelp.freshdesk.com/" + EndOfLine + EndOfLine + "click on IDF Version Updater under Downloads."
		  elseif foundOld <= AllOldVersions.Ubound then
		    'this is the case when it has been found and a transition should be performed
		    for i = foundOld to AllOldVersions.Ubound 'loop through the old version numbers but add the new version numbers
		      pmnuNewVersion.AddRow AllNewVersions(i)
		    next i
		    'select last item in the list
		    if pmnuNewVersion.ListCount > 0 then
		      pmnuNewVersion.ListIndex = pmnuNewVersion.ListCount - 1
		    end if
		    cmdConvert.Enabled = True
		  else
		    pmnuNewVersion.AddRow currentVersion
		    pmnuNewVersion.ListIndex = 0
		    cmdConvert.Enabled = False
		    MsgBox "Cannot determine if the file needs to be updated."
		  end if

		End Sub
	#tag EndMethod

	#tag Method, Flags = &h0
		Sub gatherAllPossibleVersions()
		  dim transFolder as FolderItem
		  dim f as FolderItem
		  dim numFiles as Integer  = 0
		  dim i as integer = 0
		  dim fileName as String
		  'dim s as String =""
		  'dim t as String =""
		  dim u as String = ""
		  'dim v as String = ""
		  dim newVersionWithDashes as String
		  dim oldVersionWithDashes as String
		  Var strParts() as String
		  dim sortVersion as String
		  Var allSortVersions() as String

		  #if TargetMacOS
		    transFolder = app.ExecutableFile.Parent.Parent.Parent.Parent
		  #else
		    transFolder = app.ExecutableFile.Parent
		  #endif
		  #if DebugBuild then
		    transFolder = transFolder.Parent
		  #endif
		  'MsgBox transFolder.AbsolutePath
		  if transFolder.Directory then
		    numFiles = transFolder.Count
		    for i = 1 to numFiles
		      f = transFolder.Item(i)
		      fileName = f.Name
		      u = u + "["+ fileName + "]" + EndOfLine
		      'Transition-V5-0-0-to-V6-0-0.exe
		      'Transition-V6-0-0-to-V7-0-0.exe
		      '1234567890123456789012345678901
		      ' this is the original line that did not work well with Linux because of use of the extension
		      'if fileName.Left(12).Lowercase = "transition-v" and filename.right(4).Lowercase = ".exe" then

		      ' to fix #9193 which is the lack of support for version numbers in the form 22.1.0
		      'Transition-V9-6-0-to-V22-1-0.exe

		      if fileName.Left(12).Lowercase = "transition-v" then
		        strParts = fileName.Split("V")
		        oldVersionWithDashes = strParts(1)
		        oldVersionWithDashes = oldVersionWithDashes.Replace("-to-","")
		        newVersionWithDashes = strParts(2)
		        newVersionWithDashes = newVersionWithDashes.Replace(".exe","")
		        ' old way of doing this
		        ' oldVersionWithDashes = fileName.mid(13,5)
		        ' newVersionWithDashes = fileName.mid(23,5)
		        sortVersion = newVersionWithDashes
		        if sortVersion.mid(2,1) = "-" then
		          sortVersion = "0" + sortVersion
		        end if
		        if newVersionWithDashes <> "" then
		          AllOldVersions.Append  oldVersionWithDashes.ReplaceAll("-",".")
		          AllNewVersions.Append  newVersionWithDashes.ReplaceAll("-",".")
		          TransitionApps.Append f
		          allSortVersions.Append sortVersion
		        end if
		      end if
		    next i
		  end if
		  'sort the arrays (usually sorted already but just in case)
		  'AllNewVersions.SortWith(TransitionApps,AllOldVersions)
		  allSortVersions.SortWith(AllNewVersions,TransitionApps,AllOldVersions)

		  ' THE FOLLOWING IS TO HELP DEBUG THE LIST OF TRANSITION EXE FILES
		  'display the list
		  'for i = 0 to AllNewVersions.Ubound
		  's = s + AllNewVersions(i) + EndOfLine
		  'next i
		  'for i = 0 to TransitionApps.Ubound
		  't = t + TransitionApps(i).name + EndOfLine
		  'next i
		  'for i = 0 to TransitionApps.Ubound
		  'v = v + allSortVersions(i) + EndOfLine
		  'next i
		  'MsgBox "Application program location:" + EndOfLine + "  " + transFolder.NativePath + EndOfLine + EndOfLine + "Number of files: "  _
		  '+ str(numFiles) + EndOfLine + EndOfLine + "Versions found: " + EndOfLine + s +  EndOfLine + "Transition Programs:" _
		  '+ EndOfLine + "AllSortVersions" + EndOfLine + v + EndOfLine
		  '+ EndOfLine + t + EndOfline + "All files: " + EndOfLine + u + EndOfLine _





		End Sub
	#tag EndMethod

	#tag Method, Flags = &h0
		Function getCurrentFileVersion(InFile as FolderItem) As String
		  dim SourceStream as TextInputStream
		  dim curLine as string = ""
		  dim nextLine as string = ""
		  dim fullObj as string = ""
		  dim commaPos as Integer = 0
		  dim semiPos as Integer = 0
		  dim versionFound as String = ""
		  Var strParts() as String
		  dim returnVersion as String = ""
		  IF InFile.Exists then
		    SourceStream = TextInputStream.Open(InFile)
		    While Not SourceStream.EOF
		      curLine = Uppercase(LTrim(SourceStream.ReadLine))
		      If curLine.Len>0 and  curLine.Left(1) <>"!" then 'skip lines with comments and blank lines
		        if curLine.Left(8) = "VERSION," Then
		          'get the next line that is not a comment or blank
		          While Not SourceStream.EOF
		            nextLine = Uppercase(LTrim(SourceStream.ReadLine))
		            If nextLine.len>0 and nextLine.Left(1) <>"!" then 'skip lines with comments and blank lines
		              fullObj = trimOffComment(curLine) + trimOffComment(nextLine)
		              Exit
		            end if
		          wend
		          if fullObj.Len >0 then
		            exit
		          end if
		        end if
		      end if
		    wend
		    if fullobj.Len>0 then
		      commaPos = fullObj.instr(",")
		      semiPos = fullObj.instr(";")
		      If semiPos > commaPos Then
		        versionFound = fullObj.mid(commaPos + 1, (semiPos - commaPos) - 1)
		        versionFound = versionFound.Trim
		        strParts = versionFound.split(".")
		        if strParts.lastIndex = 1 then
		          returnVersion = strParts(0) + "." + strParts(1) + ".0"
		        elseif strParts.LastIndex >= 2 then
		          returnVersion = strParts(0) + "." + strParts(1) + "." + strParts(2)
		        Else
		          returnVersion = ""
		        End If
		      end if
		      SourceStream.Close
		    end if
		  end if
		  return returnVersion

		End Function
	#tag EndMethod

	#tag Method, Flags = &h0
		Function trimOffComment(stringWithComment as String) As String
		  Dim explPtPos As Integer
		  explPtPos = stringWithComment.instr("!")
		  If explPtPos > 1 Then
		    Return stringWithComment.left(explPtPos - 1)
		  ElseIf explPtPos = 1 Then
		    Return ""
		  Else
		    Return stringWithComment
		  End If
		End Function
	#tag EndMethod


	#tag Property, Flags = &h0
		#tag Note
			'holds the version numbers as string of the new version numbers
			'old version means the y-y-y version listed in each TransitionVx-x-x-to-Vy-y-y.exe
			' 1.0.1
			' 2.1.0
			' etc...
		#tag EndNote
		AllNewVersions() As String
	#tag EndProperty

	#tag Property, Flags = &h0
		#tag Note
			'holds the version numbers as string of the old version numbers
			'old version means the x-x-x version listed in each TransitionVx-x-x-to-Vy-y-y.exe
			' 1.0.1
			' 2.1.0
			' etc...
		#tag EndNote
		AllOldVersions() As String
	#tag EndProperty

	#tag Property, Flags = &h0
		#tag Note
			Holds the names of the transition programs like
			Transition-V5-0-0-to-V6-0-0.exe
		#tag EndNote
		TransitionApps() As FolderItem
	#tag EndProperty


#tag EndWindowCode

#tag Events cmdChooseFile
	#tag Event
		Sub Action()
		  dim f as FolderItem
		  dim curVersion as string = ""
		  dim filepath as string = ""
		  dim auditFile as FolderItem
		  'dlg.Title = "Select Old EnergyPlus File to Update"
		  f = FolderItem.ShowOpenFileDialog(ReadFileTypeGroup.all)
		  if f<>nil then
		    filepath = f.NativePath
		    #if TargetMacOS then
		      if ExtensionOnly(filepath) = "txt" then
		        filepath = filepath.left(filepath.length - 4)
		      end if
		    #EndIf
		    txtFileName.Text = filepath
		    'msgbox "cmdChooseFile-Action-1: " + filepath + " extension " + ExtensionOnly(filepath)
		    txtFileName.Enabled = True
		    if extensionOnly(filepath) <> "lst" then 'IDF or IDM files are processed
		      curVersion =  getCurrentFileVersion(f)
		      if curVersion<>"" then
		        txtCurrentVersion.Text = curVersion
		        txtCurrentVersion.Enabled = True
		        pmnuNewVersion.Enabled = True
		        call FillLaterVersions(curVersion)
		        'enable the View Audit button if exists.
		        auditFile = new FolderItem(fileNameWithoutExtension(f.NativePath) + "_transition.audit" )
		        if auditFile.Exists then
		          cmdViewAudit.Enabled = True
		        else
		          cmdViewAudit.Enabled = False
		        end if
		      else
		        MsgBox "No VERSION object found in the file. Please add a VERSION object with the correct version number of the original version of the file and select the file again."
		      end if
		    else
		      'just allow the latest version
		      pmnuNewVersion.DeleteAllRows
		      pmnuNewVersion.AddRow AllNewVersions(AllNewVersions.Ubound )
		      pmnuNewVersion.ListIndex = pmnuNewVersion.ListCount - 1
		      cmdConvert.Enabled =True
		    end if
		  end if

		End Sub
	#tag EndEvent
#tag EndEvents
#tag Events cmdConvert
	#tag Event
		Sub Action()
		  dim SourceStream as TextInputStream
		  dim curLine as string = ""
		  dim curFile as FolderItem
		  dim curVersion as String =""
		  dim activeFile as FolderItem
		  dim activeFileName as String = ""

		  'Called when the CONVERT button is pressed
		  me.MouseCursor = system.Cursors.Wait
		  me.Refresh
		  ' for Windows and Linux copy the IDD and CSV to current file location
		  #if TargetWindows or TargetLinux then
		    call CopyIDDandCSV(txtFileName.Text, txtCurrentVersion.Text, pmnuNewVersion.Text)
		  #EndIf

		  activeFileName = txtFileName.Text
		  activeFile = new FolderItem(activeFileName)
		  'on MacOS the file name sometime has an extra extension of .txt
		  #if TargetMacOS then
		    if not activeFile.Exists then
		      activeFileName = activeFileName + ".txt"
		      activeFile = new FolderItem(activeFileName)
		    end if
		  #endif
		  'MsgBox "cmdConvert-Action-2: " + activeFileName + " extension " + ExtensionOnly(activeFileName)

		  if ExtensionOnly(txtFileName.Text) = "lst" then
		    IF activeFile.Exists then
		      SourceStream = TextInputStream.Open(activeFile)
		      While Not SourceStream.EOF
		        curLine = LTrim(SourceStream.ReadLine)
		        If curLine.Len>0 and  curLine.Left(1) <>"!" then 'skip lines with comments and blank lines
		          try
		            curFile = new FolderItem(curLine)
		            if curFile.Exists then
		              curVersion = getCurrentFileVersion(curFile)
		              call ConvertFile(curLine, curVersion, pmnuNewVersion.Text,False)
		            else
		              MsgBox "File not found (a): " + curLine
		            end if
		          catch err as UnsupportedFormatException
		            MsgBox "File not found (b): " + curLine
		          catch err
		            MsgBox "File not working: " + curLine
		          end try
		        end if
		      wend
		      MsgBox "All conversions complete"
		    else
		      MsgBox "Could not file file: " + txtFileName.Text + " or " + txtFileName.Text + ".txt"
		    end if
		  else
		    call ConvertFile(activeFileName, txtCurrentVersion.Text, pmnuNewVersion.Text,True)
		    cmdViewAudit.Enabled = True
		  end if
		  ' for Windows and Linux copy the IDD and CSV to current file location
		  #if TargetWindows or TargetLinux then
		    call DeleteCopiedIDDandCSV(txtFileName.Text, txtCurrentVersion.Text, pmnuNewVersion.Text)
		  #EndIf



		  me.MouseCursor = system.Cursors.StandardPointer

		  Exception
		    MsgBox "File not found (c): " + curLine
		End Sub
	#tag EndEvent
#tag EndEvents
#tag Events cmdExit
	#tag Event
		Sub Action()
		  close()

		End Sub
	#tag EndEvent
#tag EndEvents
#tag Events cmdViewAudit
	#tag Event
		Sub Action()
		  dim SourceStream as TextInputStream
		  dim curFile as FolderItem

		  curFile = new FolderItem(fileNameWithoutExtension(txtFileName.Text) + "_transition.audit" )
		  if curFile.Exists then
		    SourceStream = TextInputStream.Open(curFile)
		    DisplayText.TextArea1.Text = SourceStream.ReadAll
		    SourceStream.Close
		    displaytext.Title = "Audit File"
		    DisplayText.ShowModal
		  end if



		End Sub
	#tag EndEvent
#tag EndEvents
#tag Events cmdAbout
	#tag Event
		Sub Action()
		  dim t as String
		  t = "IDF Version Updater - Version 0.17" + EndOfLine+ EndOfLine
		  t = t + "Copyright (c) 2011-2024 GARD Analytics, All rights reserved." + EndOfLine+ EndOfLine
		  t = t + "NOTICE: The U.S. Government is granted for itself and others acting on its behalf a paid-up, nonexclusive, irrevocable, worldwide license in this data to reproduce, prepare derivativeworks, and perform publicly and display publicly. Beginning five (5) years after permission to assert copyright is granted, subject to two possible five year renewals, the U.S. Government is granted for itself and others acting on its behalf a paid-up, non-exclusive,irrevocable worldwide license in this data to reproduce, prepare derivative works, distribute copies to the public,perform publicly and display publicly,and to permit others to do so." + EndOfLine+ EndOfLine
		  t = t + "TRADEMARKS: EnergyPlus, DOE-2.1E, DOE-2, and DOE are trademarks of the US Department of Energy." + EndOfLine+ EndOfLine
		  t = t + "DISCLAIMER OF WARRANTY AND LIMITATION OF LIABILITY: THIS SOFTWARE IS PROVIDED 'AS IS' WITHOUT WARRANTY OF ANY KIND. NEITHER GARD ANALYTICS, THE DEPARTMENT OF ENERGY, THE US GOVERNMENT, THEIR LICENSORS, OR ANY PERSON OR ORGANIZATION ACTING ON BEHALF OF ANY OF THEM:" + EndOfLine+ EndOfLine
		  t = t + "A.  MAKE ANY WARRANTY OR REPRESENTATION WHATSOEVER, EXPRESS OR IMPLIED, WITH RESPECT TO ENERGYPLUS OR ANY DERIVATIVE WORKS THEREOF, INCLUDING WITHOUT LIMITATION WARRANTIES OF MERCHANTABILITY, WARRANTIES OF FITNESS FOR A PARTICULAR PURPOSE, OR WARRANTIES OR REPRESENTATIONS REGARDING THE USE, OR THE RESULTS OF THE USE OF ENERGYPLUS OR DERIVATIVE WORKS THEREOF IN TERMS OF CORRECTNESS, ACCURACY, RELIABILITY, CURRENTNESS, OR OTHERWISE. THE ENTIRE RISK AS TO THE RESULTS AND PERFORMANCE OF THE LICENSED SOFTWARE IS ASSUMED BY THE LICENSEE." + EndOfLine+ EndOfLine
		  t = t + "B.  MAKE ANY REPRESENTATION OR WARRANTY THAT ENERGYPLUS OR DERIVATIVE WORKS THEREOF WILL NOT INFRINGE ANY  COPYRIGHT OR OTHER PROPRIETARY RIGHT." + EndOfLine+ EndOfLine
		  t = t + "C.  ASSUME ANY LIABILITY WHATSOEVER WITH RESPECT TO ANY USE OF ENERGYPLUS, DERIVATIVE WORKS THEREOF, OR ANY PORTION THEREOF OR WITH RESPECT TO ANY DAMAGES WHICH MAY RESULT FROM SUCH USE." + EndOfLine+ EndOfLine
		  t = t + "DISCLAIMER OF ENDORSEMENT: Reference herein to any specific commercial products, process, or service by trade name, trademark, manufacturer, or otherwise, does not necessarily constitute or imply its endorsement,recommendation, or favoring by the United States Government or GARD Analytics." + EndOfLine+ EndOfLine
		  DisplayText.TextArea1.Text = t
		  displaytext.Title = "About.."
		  DisplayText.ShowModal

		End Sub
	#tag EndEvent
#tag EndEvents
