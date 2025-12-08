#tag Window
Begin Window Window1
   BackColor       =   16777215
   Backdrop        =   ""
   CloseButton     =   True
   Composite       =   False
   Frame           =   1
   FullScreen      =   False
   HasBackColor    =   False
   Height          =   352
   ImplicitInstance=   True
   LiveResize      =   True
   MacProcID       =   0
   MaxHeight       =   32000
   MaximizeButton  =   False
   MaxWidth        =   32000
   MenuBar         =   1789399039
   MenuBarVisible  =   True
   MinHeight       =   64
   MinimizeButton  =   False
   MinWidth        =   64
   Placement       =   2
   Resizeable      =   False
   Title           =   "EPDrawGUI"
   Visible         =   True
   Width           =   302
   Begin TabPanel tabMain
      AutoDeactivate  =   True
      Bold            =   ""
      Enabled         =   True
      Height          =   281
      HelpTag         =   ""
      Index           =   -2147483648
      InitialParent   =   ""
      Italic          =   ""
      Left            =   22
      LockBottom      =   ""
      LockedInPosition=   False
      LockLeft        =   True
      LockRight       =   ""
      LockTop         =   True
      Panels          =   ""
      Scope           =   0
      SmallTabs       =   ""
      TabDefinition   =   "Main\rOptions"
      TabIndex        =   0
      TabPanelIndex   =   0
      TabStop         =   True
      TextFont        =   "System"
      TextSize        =   0
      TextUnit        =   0
      Top             =   14
      Underline       =   ""
      Value           =   0
      Visible         =   True
      Width           =   260
      Begin BevelButton cmdExpressDXF
         AcceptFocus     =   False
         AutoDeactivate  =   True
         BackColor       =   0
         Bevel           =   0
         Bold            =   False
         ButtonType      =   0
         Caption         =   "Select IDF and Create DXF..."
         CaptionAlign    =   3
         CaptionDelta    =   0
         CaptionPlacement=   1
         Enabled         =   True
         HasBackColor    =   False
         HasMenu         =   0
         Height          =   28
         HelpTag         =   "By pressing this button, you select an EnergyPlus input file with an IDF extension and it is used to create a visual represetation of the file in a DXF file format. Depending on the option selected, the file is then displayed using the DXF file viewer."
         Icon            =   ""
         IconAlign       =   0
         IconDX          =   0
         IconDY          =   0
         Index           =   -2147483648
         InitialParent   =   "tabMain"
         Italic          =   False
         Left            =   59
         LockBottom      =   ""
         LockedInPosition=   False
         LockLeft        =   True
         LockRight       =   ""
         LockTop         =   True
         MenuValue       =   0
         Scope           =   0
         TabIndex        =   0
         TabPanelIndex   =   1
         TabStop         =   True
         TextColor       =   0
         TextFont        =   "System"
         TextSize        =   ""
         TextUnit        =   0
         Top             =   101
         Underline       =   False
         Value           =   False
         Visible         =   True
         Width           =   184
      End
      Begin CheckBox chkDisplayDXF
         AutoDeactivate  =   True
         Bold            =   ""
         Caption         =   "Show DXF File After Created"
         DataField       =   ""
         DataSource      =   ""
         Enabled         =   True
         Height          =   18
         HelpTag         =   "When this option is checked after the CREATE DXF FROM IDF button is pressed, the DXF file will be displayed using the selected viewer.\r\n\r\nIf the DXF file is not shown, go to the OPTIONS tab and use SELECT DXF FILE VIEWER to select a file viewer for DXF files."
         Index           =   -2147483648
         InitialParent   =   "tabMain"
         Italic          =   ""
         Left            =   40
         LockBottom      =   ""
         LockedInPosition=   False
         LockLeft        =   True
         LockRight       =   ""
         LockTop         =   True
         Scope           =   0
         State           =   1
         TabIndex        =   1
         TabPanelIndex   =   1
         TabStop         =   True
         TextFont        =   "System"
         TextSize        =   0
         TextUnit        =   0
         Top             =   248
         Underline       =   ""
         Value           =   True
         Visible         =   True
         Width           =   222
      End
      Begin GroupBox GroupBox1
         AutoDeactivate  =   True
         Bold            =   ""
         Caption         =   "Polygons with 5+ Sides"
         Enabled         =   True
         Height          =   156
         HelpTag         =   ""
         Index           =   -2147483648
         InitialParent   =   "tabMain"
         Italic          =   ""
         Left            =   40
         LockBottom      =   ""
         LockedInPosition=   False
         LockLeft        =   True
         LockRight       =   ""
         LockTop         =   True
         Scope           =   0
         TabIndex        =   1
         TabPanelIndex   =   2
         TextFont        =   "System"
         TextSize        =   0
         TextUnit        =   0
         Top             =   86
         Underline       =   ""
         Visible         =   True
         Width           =   226
         Begin RadioButton optTriangle
            AutoDeactivate  =   True
            Bold            =   ""
            Caption         =   "Attempt Triangulation"
            Enabled         =   True
            Height          =   19
            HelpTag         =   "This option attempts simple triangulation for the polygon (>4 sides) surfaces.  This triangulation will show in the wireframe views but will appear as a solid face in 3D views. This triangulation is only for drawing purposes and does not affect the simulations in any way. The triangle algorithm is not perfect and warnings do result when the software cannot triangulate a surface. If unable to triangulate simply, a warning error is generated to the .EPDerr file."
            Index           =   -2147483648
            InitialParent   =   "GroupBox1"
            Italic          =   ""
            Left            =   59
            LockBottom      =   ""
            LockedInPosition=   False
            LockLeft        =   True
            LockRight       =   ""
            LockTop         =   True
            Scope           =   0
            TabIndex        =   0
            TabPanelIndex   =   2
            TabStop         =   True
            TextFont        =   "System"
            TextSize        =   0
            TextUnit        =   0
            Top             =   102
            Underline       =   ""
            Value           =   True
            Visible         =   True
            Width           =   163
         End
         Begin RadioButton optThickPoly
            AutoDeactivate  =   True
            Bold            =   ""
            Caption         =   "Thick Polyline"
            Enabled         =   True
            Height          =   21
            HelpTag         =   "This option creates a 'thick' line at the border of the polygon (>4 sides) surfaces. It will look like a hole in the drawing with a thicker edge. This thick border shows in wireframe as well as 3D views and can be confusing, due to overlap with other surfaces."
            Index           =   -2147483648
            InitialParent   =   "GroupBox1"
            Italic          =   ""
            Left            =   59
            LockBottom      =   ""
            LockedInPosition=   False
            LockLeft        =   True
            LockRight       =   ""
            LockTop         =   True
            Scope           =   0
            TabIndex        =   1
            TabPanelIndex   =   2
            TabStop         =   True
            TextFont        =   "System"
            TextSize        =   0
            TextUnit        =   0
            Top             =   133
            Underline       =   ""
            Value           =   ""
            Visible         =   True
            Width           =   163
         End
         Begin RadioButton optRegPoly
            AutoDeactivate  =   True
            Bold            =   ""
            Caption         =   "Regular Polyline"
            Enabled         =   True
            Height          =   23
            HelpTag         =   "This option creates a 'regular' polyline for all polygon (>4 sides) surfaces. It will look like a hole in the drawing.  Also, it will look the same in both wireframe and 3D views."
            Index           =   -2147483648
            InitialParent   =   "GroupBox1"
            Italic          =   ""
            Left            =   59
            LockBottom      =   ""
            LockedInPosition=   False
            LockLeft        =   True
            LockRight       =   ""
            LockTop         =   True
            Scope           =   0
            TabIndex        =   2
            TabPanelIndex   =   2
            TabStop         =   True
            TextFont        =   "System"
            TextSize        =   0
            TextUnit        =   0
            Top             =   166
            Underline       =   ""
            Value           =   ""
            Visible         =   True
            Width           =   163
         End
         Begin RadioButton optWireframe
            AutoDeactivate  =   True
            Bold            =   ""
            Caption         =   "Wireframe"
            Enabled         =   True
            Height          =   27
            HelpTag         =   "This option creates a wireframe drawing (all lines) for all surfaces.  All surfaces will appear as lines in both wireframe and 3D views."
            Index           =   -2147483648
            InitialParent   =   "GroupBox1"
            Italic          =   ""
            Left            =   59
            LockBottom      =   ""
            LockedInPosition=   False
            LockLeft        =   True
            LockRight       =   ""
            LockTop         =   True
            Scope           =   0
            TabIndex        =   3
            TabPanelIndex   =   2
            TabStop         =   True
            TextFont        =   "System"
            TextSize        =   0
            TextUnit        =   0
            Top             =   201
            Underline       =   ""
            Value           =   ""
            Visible         =   True
            Width           =   163
         End
      End
      Begin BevelButton cmdViewDXF
         AcceptFocus     =   False
         AutoDeactivate  =   True
         BackColor       =   0
         Bevel           =   0
         Bold            =   False
         ButtonType      =   0
         Caption         =   "View DXF File"
         CaptionAlign    =   3
         CaptionDelta    =   0
         CaptionPlacement=   1
         Enabled         =   True
         HasBackColor    =   False
         HasMenu         =   0
         Height          =   26
         HelpTag         =   "By pressing this button, you select a DXF file that is already on the computer and display it with the selected DXF viewer."
         Icon            =   ""
         IconAlign       =   0
         IconDX          =   0
         IconDY          =   0
         Index           =   -2147483648
         InitialParent   =   "tabMain"
         Italic          =   False
         Left            =   40
         LockBottom      =   ""
         LockedInPosition=   False
         LockLeft        =   True
         LockRight       =   ""
         LockTop         =   True
         MenuValue       =   0
         Scope           =   0
         TabIndex        =   2
         TabPanelIndex   =   2
         TabStop         =   True
         TextColor       =   0
         TextFont        =   "System"
         TextSize        =   ""
         TextUnit        =   0
         Top             =   48
         Underline       =   False
         Value           =   False
         Visible         =   True
         Width           =   144
      End
      Begin BevelButton cmdViewer
         AcceptFocus     =   False
         AutoDeactivate  =   True
         BackColor       =   0
         Bevel           =   0
         Bold            =   False
         ButtonType      =   0
         Caption         =   "Select DXF Viewer"
         CaptionAlign    =   3
         CaptionDelta    =   0
         CaptionPlacement=   1
         Enabled         =   True
         HasBackColor    =   False
         HasMenu         =   0
         Height          =   26
         HelpTag         =   "By using this button, you can select the application that displays DXF files."
         Icon            =   ""
         IconAlign       =   0
         IconDX          =   0
         IconDY          =   0
         Index           =   -2147483648
         InitialParent   =   "tabMain"
         Italic          =   False
         Left            =   40
         LockBottom      =   ""
         LockedInPosition=   False
         LockLeft        =   True
         LockRight       =   ""
         LockTop         =   True
         MenuValue       =   0
         Scope           =   0
         TabIndex        =   3
         TabPanelIndex   =   2
         TabStop         =   True
         TextColor       =   0
         TextFont        =   "System"
         TextSize        =   ""
         TextUnit        =   0
         Top             =   254
         Underline       =   False
         Value           =   False
         Visible         =   True
         Width           =   144
      End
   End
   Begin BevelButton cmdClose
      AcceptFocus     =   False
      AutoDeactivate  =   True
      BackColor       =   0
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
      Height          =   25
      HelpTag         =   "Exit the program"
      Icon            =   ""
      IconAlign       =   0
      IconDX          =   0
      IconDY          =   0
      Index           =   -2147483648
      InitialParent   =   ""
      Italic          =   False
      Left            =   196
      LockBottom      =   ""
      LockedInPosition=   False
      LockLeft        =   True
      LockRight       =   ""
      LockTop         =   True
      MenuValue       =   0
      Scope           =   0
      TabIndex        =   1
      TabPanelIndex   =   0
      TabStop         =   True
      TextColor       =   0
      TextFont        =   "System"
      TextSize        =   ""
      TextUnit        =   0
      Top             =   307
      Underline       =   False
      Value           =   False
      Visible         =   True
      Width           =   86
   End
   Begin BevelButton cmdAbout
      AcceptFocus     =   False
      AutoDeactivate  =   True
      BackColor       =   "&c000000"
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
      Height          =   25
      HelpTag         =   ""
      Icon            =   ""
      IconAlign       =   0
      IconDX          =   0
      IconDY          =   0
      Index           =   -2147483648
      InitialParent   =   ""
      Italic          =   False
      Left            =   22
      LockBottom      =   ""
      LockedInPosition=   False
      LockLeft        =   True
      LockRight       =   ""
      LockTop         =   True
      MenuValue       =   0
      Scope           =   0
      TabIndex        =   3
      TabPanelIndex   =   0
      TabStop         =   True
      TextColor       =   "&c000000"
      TextFont        =   "System"
      TextSize        =   ""
      TextUnit        =   0
      Top             =   307
      Underline       =   False
      Value           =   False
      Visible         =   True
      Width           =   51
   End
End
#tag EndWindow

#tag WindowCode
	#tag Event
		Sub Close()
		  call saveAllSettings

		End Sub
	#tag EndEvent

	#tag Event
		Sub Open()
		  call GetAllSettings
		  call GetIDDFileName
		  call ProcessCommandLine

		End Sub
	#tag EndEvent


	#tag Method, Flags = &h0
		Function ChangeExtension(origFileName as String, newExtension as String) As String
		  'return a string when a string is supplied
		  dim parts(-1) as string
		  dim newFileName as String
		  parts = origFileName.split(".") 'split at all periods
		  if parts.Ubound>=1 then
		    parts(parts.Ubound) = newExtension
		    newFileName = join(parts,".")
		  else
		    newFileName = origFileName
		  end if
		  return newFileName



		End Function
	#tag EndMethod

	#tag Method, Flags = &h0
		Sub CreateDXFfromIDFfile(IDFfileName as String, supressDXFviewing as Boolean)
		  Soft Declare Sub CreateDXFFile Lib "EPlusDrw.dll" (fileIDD As cString, fileIDDLen As Int32, fileName As cString, ileNameLen As Int32, option As cString, OptionLen As Int32, strError As cString, errstrLen As Int32, byRef ErrorFlag As Int16)

		  dim cIDFfileName as String
		  dim cIDDFileName as String
		  dim returnedError as String
		  dim polygonActionString as String
		  dim dxfFileName as String
		  dim fDXF as FolderItem
		  dim fErr as FolderItem
		  dim flagOfError as Int16 'should be Boolean but the fortran is asking for two byte logical - try using Short instead
		  dim msg as Integer
		  dim SourceStream as TextInputStream
		  dim errorText as String
		  me.MouseCursor = system.Cursors.Wait
		  ' convert to C style string
		  cIDFfileName = IDFfileName
		  cIDDFileName = IDDFileName
		  'create file name with DXF extension
		  dxfFileName = ChangeExtension(IDFfileName,"dxf")
		  fDXF = new FolderItem(dxfFileName)
		  'delete the file if it exists
		  if fDXF.Exists then
		    fDXF.Delete
		  end if
		  if IDFfileName="" then
		    msg=MsgBox("Input file has not been selected.",0 + 16,"Create DXF from IDF Error")
		  elseif IDDFileName = "" then
		    msg=MsgBox("Energy+.IDD file not located.",0 + 16,"Create DXF from IDF Error")
		  elseif not system.IsFunctionAvailable("CreateDXFFile","EPlusDrw.dll") then
		    msg=MsgBox("Library for EPlusDraw missing.",0 + 16,"Create DXF from IDF Error")
		  else
		    returnedError = RepeatSpaces(255)
		    'MsgBox "[" + returnedError + "]" + str(len(returnedError))
		    select case PolyOption
		    case 1
		      polygonActionString = "TRIANGULATE 3DFACE"
		    case 2
		      polygonActionString = "THICK POLYLINE"
		    case 3
		      polygonActionString = "REGULAR POLYLINE"
		    case 4
		      polygonActionString = "WIREFRAME"
		    end Select
		    'msgbox "IDDFileName:" + EndOfLine + cIDDFileName + EndOfLine + "IDFfileName:" + EndOfLine + IDFfileName + EndOfLine + "polygonActionString:" + EndOfLine + polygonActionString
		    call CreateDXFFile(cIDDFileName,len(cIDDFileName),cIDFfileName,len(cIDFfileName),polygonActionString,len(polygonActionString),returnedError,len(returnedError),flagOfError)
		    if flagOfError <> 0 or not fDXF.Exists then 'True is passed as -1, False is passed as 0.
		      fErr = new FolderItem(ChangeExtension(IDFfileName,"EPDerr"))
		      if fErr.Exists then
		        errorText = ""
		        SourceStream = fErr.OpenAsTextFile
		        errorText = SourceStream.ReadAll
		      end if
		      msg=MsgBox("Error when creating DXF file from IDF file: " + EndOfLine + EndOfLine + returnedError + EndOfLine + errorText,0 + 16,"Create DXF from IDF Error")
		    else
		      if not supressDXFviewing then
		        msg=MsgBox("File created: " + EndOfLine + EndOfLine + dxfFileName,0 + 64,"Create DXF from IDF")
		        if not supressDXFviewing then
		          call viewdxffile(fdxf)
		        end if
		      end if
		    end if
		  end if
		  me.MouseCursor = system.Cursors.StandardPointer
		exception err
		  MsgBox err.message


		End Sub
	#tag EndMethod

	#tag Method, Flags = &h0
		Sub ExperimentToGetDLLcallWorking(IDFfileName as String)
		  'VB6: Private Declare Sub CreateDXFFile Lib "EPlusDrw.dll" (ByVal strPathToIDD As String, ByVal pathToIDDLen As Long, ByVal strFileName As String, ByVal FileNameLen As Long, ByVal strOption As String, ByVal OptionLen As Long, ByVal strError As String, ByVal errstrLen As Long, ErrorFlag As Boolean)
		  'Soft Declare Sub CreateDXFFile Lib "EPlusDrw.dll" (byVal fileIDD As cString,byVal fileIDDLen As Int32, byVal fileName As cString, byVal fileNameLen As Int32, byVal option As cString, byVal OptionLen As Int32, byVal strError As cString, byVal errstrLen As Int32, byVal ErrorFlag As short)
		  soft Declare Sub CreateDXFFile Lib "EPlusDrw.dll" (byVal fileIDD As cString,byVal fileIDDLen As Int32, byVal fileName As cString, byVal fileNameLen As Int32, byVal option As cString, byVal OptionLen As Int32, byVal strError As cString, byVal errstrLen As Int32, byVal ErrorFlag As short)
		  'Declare Sub CreateDXFFile Lib "EPlusDrw.dll" (byVal fileIDD As cString,byVal fileIDDLen As UInt32, byVal fileName As cString, byVal fileNameLen As UInt32, byVal option As cString, byVal OptionLen As UInt32, byVal strError As cString, byVal errstrLen As UInt32, byVal ErrorFlag As short)
		  'Declare Sub CreateDXFFile Lib "EPlusDrw.dll" (byVal fileIDD As cString, byVal fileName As cString, byVal option As cString,  byVal strError As cString,  byVal ErrorFlag As short)
		  'Declare Sub CreateDXFFile Lib "EPlusDrw.dll" (fileIDD As cString, fileName As cString, option As cString,  strError As cString, ErrorFlag As Boolean)
		  'Declare Sub CreateDXFFile Lib "EPlusDrw" (fileIDD As cString, fileName As cString, option As cString,  strError As cString, ErrorFlag As Short)
		  Soft Declare Sub TestIt Lib "TestDLL.dll" (byref years as Integer)
		  Soft Declare Sub TestBlank Lib "TestDLL.dll"
		  Soft Declare Sub TestString Lib "TestDLL.dll" (stringToTest as CString, lenOfStringToTest as Int32)
		  'Soft Declare Sub TestMultiString Lib "TestDLL.dll" (fileIDD as CString, lenOfFileIDD as Int32, fileName as CString, lenOfFileName as Int32, optionStr as CString, lenOfOpt as Int32,  errorStr as CString, lenOfError as Int32)
		  Soft Declare Sub TestMultiString Lib "TestDLL.dll" (fileIDD as CString, fileName as CString, optionStr as CString, errorStr as CString, lenOfFileIDD as Int32, lenOfFileName as Int32, lenOfOpt as Int32,  lenOfError as Int32)
		  'Soft Declare Sub TestMultiStringBool Lib "TestDLL.dll" (fileIDD as CString, fileName as CString, optionStr as CString,  errorStr as CString, lenOfFileIDD as Int32, lenOfFileName as Int32, lenOfOpt as Int32,  lenOfError as Int32,flgErr as Int16)
		  Soft Declare Sub TestMultiStringBool Lib "TestDLL.dll" (flgErr as Integer,fileIDD as CString, fileName as CString, optionStr as CString,  errorStr as CString, lenOfFileIDD as Int32, lenOfFileName as Int32, lenOfOpt as Int32,  lenOfError as Int32)
		  Soft Declare Sub TestMultiStringInt Lib "TestDLL.dll" (fileIDD as CString, fileName as CString, optionStr as CString,  errorStr as CString, byref flgErr as Int16,lenOfFileIDD as Int32, lenOfFileName as Int32, lenOfOpt as Int32,  lenOfError as Int32)

		  dim testInt as Int16
		  dim cIDFfileName as String
		  dim cIDDFileName as String
		  dim returnedError as String
		  dim polygonActionString as String
		  dim flagOfError as Int16 'should be Boolean but the fortran is asking for two byte logical - try using Short instead
		  ' convert to C style string
		  cIDFfileName = IDFfileName
		  cIDDFileName = IDDFileName
		  if IDFfileName="" then
		    MsgBox "Input file has not been selected."
		  elseif IDDFileName = "" then
		    MsgBox "Energy+.IDD file not located."
		  elseif not system.IsFunctionAvailable("CreateDXFFile","EPlusDrw.dll") then
		    MsgBox "Library for EPlusDraw missing."
		  else
		    returnedError = RepeatSpaces(255)
		    'MsgBox "[" + returnedError + "]" + str(len(returnedError))
		    select case PolyOption
		    case 1
		      polygonActionString = "TRIANGULATE 3DFACE"
		    case 2
		      polygonActionString = "THICK POLYLINE"
		    case 3
		      polygonActionString = "REGULAR POLYLINE"
		    case 4
		      polygonActionString = "WIREFRAME"
		    end Select
		    'try
		    call testBlank
		    msgbox "testblank done"

		    'testInt = 267
		    'call testIt(testInt)
		    'MsgBox str(testInt)

		    'call CreateDXFFile(cIDDFileName,len(cIDDFileName),cIDFfileName,len(cIDFfileName),polygonActionString,len(polygonActionString),returnedError,len(returnedError),flagOfError)
		    'call CreateDXFFile(cIDDFileName,cIDFfileName,polygonActionString,returnedError,flagOfError)
		    'catch err as
		    'MsgBox "Error calling DLL." err.
		    'end try
		    'cIDFfileName = "Goodbye"
		    'call testString(cIDFfileName,len(cIDFfileName))
		    'call TestMultiString(cIDDFileName,len(cIDDFileName),cIDFfileName,len(cIDFfileName),polygonActionString,len(polygonActionString),returnedError,len(returnedError))
		    'call TestMultiString(cIDDFileName,cIDFfileName,polygonActionString,returnedError,len(cIDDFileName),len(cIDFfileName),len(polygonActionString),len(returnedError))
		    'flagOfError = 1
		    'call TestMultiStringBool(testInt,cIDDFileName,cIDFfileName,polygonActionString,returnedError,len(cIDDFileName),len(cIDFfileName),len(polygonActionString),len(returnedError))
		    call TestMultiStringInt(cIDDFileName,cIDFfileName,polygonActionString,returnedError,testInt,len(cIDDFileName),len(cIDFfileName),len(polygonActionString),len(returnedError))
		    MsgBox str(testInt)
		    MsgBox returnedError
		  end if
		exception err
		  MsgBox err.message


		End Sub
	#tag EndMethod

	#tag Method, Flags = &h0
		Sub GetAllSettings()
		  dim SourceStream as TextInputStream
		  dim curLine as string = ""
		  dim listFolder as FolderItem
		  dim listFile as FolderItem
		  dim equalLoc as Integer
		  dim kind as string
		  dim value as string
		  dim valueAsInt as Integer
		  'set default options if no file is present or item is not found
		  PolyOption = 1
		  optTriangle.Value=True
		  try
		    listFolder = SpecialFolder.ApplicationData
		    listFile = listFolder.child("EPDrawGUIsettings.txt")
		  catch
		    msgbox "Cannot open EPDrawGUIsettings.txt file in special application data folder"
		  end try
		  IF listFile.Exists then
		    SourceStream = listFile.OpenAsTextFile
		    While Not SourceStream.EOF
		      curLine = SourceStream.ReadLine
		      equalLoc = curLine.InStr("=")
		      if equalLoc > 0 then
		        kind = curline.left(equalLoc)
		        value = curline.mid(equalLoc + 1)
		        select case kind.Uppercase
		        case "GUITOP="
		          valueAsInt = value.Val
		          if valueAsInt>0 and valueAsInt<screen(0).Height then
		            self.top = valueAsInt
		          end if
		        case "GUILEFT="
		          valueAsInt = value.Val
		          if valueAsInt>0 and valueAsInt<Screen(0).Width then
		            self.Left = valueAsInt
		          end if
		        case "DXFVIEWER="
		          if value.len>0 then
		            DXFViewerApp = trim(value)
		          end if
		        case "POLYGONOPTION="
		          value = value.Uppercase
		          if value.InStr("TRI")>0 then
		            PolyOption = 1
		            optTriangle.Value=True
		          elseif value.instr("THICK")>0 then
		            PolyOption = 2
		            optThickPoly.Value=True
		          elseif value.instr("REG")>0 then
		            PolyOption = 3
		            optRegPoly.Value=True
		          elseif value.instr("WIRE")>0 then
		            PolyOption = 4
		            optWireframe.Value=True
		          else
		            PolyOption = 1
		            optTriangle.Value=True
		          end if
		        case "LASTDIRECTORY="
		          lastInputDirectory = trim(value)
		        case "SHOWDXF="
		          IF value.InStr("F")>0 then
		            chkDisplayDXF.Value=False
		          else
		            chkDisplayDXF.Value=True
		          end if
		        end select
		      end if
		    wend
		    SourceStream.Close
		  end if

		End Sub
	#tag EndMethod

	#tag Method, Flags = &h0
		Sub GetIDDFileName()
		  ' set the IDDFileName variable by searching first for the ENERGY+.INI file and
		  ' if that is not found then searching directly for the ENERGY+.IDD file. If the
		  ' INI file is found, the IDD file location is confirmed.
		  dim appPath as FolderItem
		  dim curPath as FolderItem
		  dim ini as FolderItem
		  dim idd as FolderItem
		  dim found as Boolean
		  dim SourceStream as TextInputStream
		  dim curLine as string = ""
		  dim currentErrorMessage as String = ""
		  'look for ini file
		  try
		    appPath = app.ExecutableFile.Parent 'this returns the directory that the current application is located
		  catch
		    MsgBox "the parent to the application path cannot be found"
		  end try
		  curPath = appPath
		  found = False
		  IDDFileName = ""
		  do until curPath=Nil 'if found root exit loop
		    try
		      ini =  curPath.child("Energy+.ini")
		    catch
		      MsgBox "In searching for Energy+.ini cannot locate child of current path (1)"
		    end try
		    if ini.Exists then
		      found = True
		      exit
		    end if
		    try
		      curPath = curPath.Parent 'keep looking up the directory tree
		    catch
		      MsgBox "Error in searching for Energy+.ini when looking up the directory tree"
		    end try
		  loop
		  if found then
		    'msgbox "Found ini file:" + ini.AbsolutePath
		    SourceStream = ini.OpenAsTextFile
		    While Not SourceStream.EOF
		      curLine = SourceStream.ReadLine
		      if curLine.Lowercase = "[program]" then
		        'found the correct group. The next line contains the directory
		        curLine = SourceStream.ReadLine
		        if curline.Left(4).Lowercase = "dir=" then
		          if trim(curline.mid(5))<>"" then
		            try
		              idd = new FolderItem(curline.mid(5))
		            catch
		              currentErrorMessage = "Establishing the IDD file for invalid location(1): " + curline.mid(5)
		              found = False 'not yet found.
		            end try
		            if idd.exists then
		              try
		                idd = idd.child("Energy+.idd") 'add the name of the idd file to the directory
		              catch
		                currentErrorMessage = "Establishing the IDD file for invalid location(2): " + curline.mid(5) + " Energy+.idd"
		                found = False 'not yet found.
		              end try
		            else
		              currentErrorMessage = "IDD directory not found. May be an issue with Energy+.ini DIR= line"
		              found = false 'not yet found
		            end if
		            if idd.Exists then
		              IDDFileName = idd.AbsolutePath
		            else
		              currentErrorMessage = "IDD file not found. "
		              found = false 'not yet found
		            end if
		            exit
		          else
		            currentErrorMessage = "The Energy+.ini does not point to a specific IDD file. Please edit it so that it does."
		            found = False 'not yet found.
		          end if
		        end if
		      end if
		    wend
		  end if
		  'if the idd was not successfully found
		  if not found then
		    curPath = appPath
		    do until curPath = Nil 'if root is found then end loop
		      try
		        idd = curpath.child("Energy+.idd")
		      catch
		        MsgBox "In searching for Energy+.ini cannot locate child of current path (2)"
		        found = False 'flag to show error later.
		      end try
		      if idd.exists then
		        found = True
		        exit
		      end if
		      try
		        curPath = curPath.Parent 'keep looking up the directory tree
		      catch
		        msgbox "In searching for Energy+.ini cannot locate parent of current path"
		        found = False 'flag to show error later.
		      end try
		    loop
		    if found then
		      IDDFileName = idd.AbsolutePath
		    end if
		  end if
		  if not found then
		    MsgBox "No Energy+.idd file found. The EPDrawGUI program should be located in the same directory as the Energy+.ini or Energy+.idd file." + EndOfLine + currentErrorMessage
		  end if
		  'MsgBox "IDDFileName:" + IDDFileName
		exception
		  MsgBox "Unhandled exception in GetIDDFileName routine." + EndOfLine + currentErrorMessage
		End Sub
	#tag EndMethod

	#tag Method, Flags = &h0
		Sub ProcessCommandLine()
		  'process the command line
		  dim cl as String
		  dim clArg as String
		  dim loc as Integer
		  dim idfFile as FolderItem
		  cl = system.CommandLine
		  'we only want the part after the name of the exe
		  loc = cl.InStr("EPDrawGUI.exe")
		  clArg = cl.mid(loc+14) 'skip the name of the exe plus one for a quote
		  clArg = clArg.Trim
		  if clarg.Len > 0 then
		    'remove double quotes at front and back
		    if clarg.right(1) = Chr(34) then
		      clArg = clArg.left(clArg.len - 1)
		    end if
		    if clArg.left(1) = chr(34) then
		      clArg = clArg.mid(2)
		    end if
		    idfFile = new FolderItem(clArg)
		    if idfFile.Exists then
		      call CreateDXFfromIDFfile(idfFile.AbsolutePath, True)
		    else
		      msgBox "File not found: " + EndOfLine + idfFile.AbsolutePath
		    end if
		    Quit 'if using command line do not open the GUI instead just exit the program
		  end if


		End Sub
	#tag EndMethod

	#tag Method, Flags = &h0
		Function RepeatSpaces(numberOfSpaces as Integer) As String
		  ' create a string of the length provided of all spaces
		  dim s as String
		  dim i as Integer
		  s = ""
		  for i = 1 to numberOfSpaces
		    s = s + " "
		  next i
		  Return s

		End Function
	#tag EndMethod

	#tag Method, Flags = &h0
		Sub SaveAllSettings()
		  dim outStream as TextOutputStream
		  dim curLine as string = ""
		  dim listFolder as FolderItem
		  dim listFile as FolderItem

		  listFolder = SpecialFolder.ApplicationData
		  listFile = listFolder.child("EPDrawGUIsettings.txt")
		  outStream = listFile.CreateTextFile
		  curLine = "GUItop=" + str(self.Top)
		  outStream.WriteLine curLine
		  curLine = "GUIleft=" + str(self.Left)
		  outStream.WriteLine curLine
		  curLine = "DXFviewer=" + trim(DXFViewerApp)
		  outStream.WriteLine curLine
		  select case PolyOption
		  case 1
		    curLine = "PolygonOption=Triangulation"
		  case 2
		    curLine = "PolygonOption=ThickPolyLine"
		  case 3
		    curLine = "PolygonOption=RegularPolyLine"
		  case 4
		    curLine = "PolygonOption=Wireframe"
		  case else
		    curline = "PolygonOption="
		  end select
		  outStream.WriteLine curLine
		  curline = "LastDirectory=" + trim(lastInputDirectory)
		  outStream.WriteLine curLine
		  if chkDisplayDXF.Value then
		    curLine = "ShowDXF=True"
		  else
		    curLine = "ShowDXF=False"
		  end if
		  outStream.WriteLine curLine
		  outStream.Close

		End Sub
	#tag EndMethod

	#tag Method, Flags = &h0
		Sub ViewDXFfile(inFile as FolderItem)
		  dim dxfv as FolderItem

		  if inFile <>nil then
		    if DXFViewerApp<>"" then
		      try
		        ' use the selected dxf viewer program to launch passing the
		        ' name of the selected file as an argument.
		        dxfv = new FolderItem(DXFViewerApp,folderitem.PathTypeAbsolute)
		        dxfv.launch(inFile.AbsolutePath)
		      catch
		        ' if that doesn't work just launch the file using what ever
		        ' might be associated with it.
		        inFile.Launch
		      end  try
		    else
		      inFile.Launch
		    end if
		  end if

		End Sub
	#tag EndMethod


	#tag Property, Flags = &h0
		DXFViewerApp As String
	#tag EndProperty

	#tag Property, Flags = &h0
		IDDFileName As String
	#tag EndProperty

	#tag Property, Flags = &h0
		lastInputDirectory As String
	#tag EndProperty

	#tag Property, Flags = &h0
		#tag Note
			1=attempt triangulation
			2=thick polyline
			3=regular polyline
			4=wireframe
		#tag EndNote
		PolyOption As Integer
	#tag EndProperty


#tag EndWindowCode

#tag Events cmdExpressDXF
	#tag Event
		Sub Action()
		  dim idfType as new FileType
		  dim f as FolderItem
		  dim dlg as new OpenDialog
		  idfType.Name = "EnergyPlus Input File"
		  idfType.MacType = "IDF"
		  idfType.Extensions = "idf"
		  dlg.Title = "Select EnergyPlus IDF File to Convert to DXF"
		  dlg.filter = idfType
		  if lastInputDirectory<>"" then
		    dlg.InitialDirectory = new folderitem(lastInputDirectory)
		  end if
		  f = dlg.ShowModal()
		  if f<>nil then
		    lastInputDirectory = f.Parent.AbsolutePath
		    call CreateDXFfromIDFfile(f.AbsolutePath, false)
		    'msgbox lastInputDirectory
		    'MsgBox app.ExecutableFile.AbsolutePath
		  end if



		End Sub
	#tag EndEvent
#tag EndEvents
#tag Events optTriangle
	#tag Event
		Sub Action()
		  PolyOption=1
		End Sub
	#tag EndEvent
#tag EndEvents
#tag Events optThickPoly
	#tag Event
		Sub Action()
		  PolyOption=2
		End Sub
	#tag EndEvent
#tag EndEvents
#tag Events optRegPoly
	#tag Event
		Sub Action()
		  PolyOption = 3
		End Sub
	#tag EndEvent
#tag EndEvents
#tag Events optWireframe
	#tag Event
		Sub Action()
		  PolyOption=4
		End Sub
	#tag EndEvent
#tag EndEvents
#tag Events cmdViewDXF
	#tag Event
		Sub Action()
		  dim dxfType as new FileType
		  dim f as FolderItem
		  dim s as new Shell
		  dxfType.Name = "Drawing Exchange Format"
		  dxfType.MacType = "dxf"
		  dxfType.Extensions = "dxf"
		  f = GetOpenFolderItem(dxfType)
		  call viewdxffile(f)


		End Sub
	#tag EndEvent
#tag EndEvents
#tag Events cmdViewer
	#tag Event
		Sub Action()
		  dim AppType as new FileType
		  dim f as FolderItem
		  AppType.Name = "Application"
		  AppType.MacType = "EXE"
		  AppType.Extensions = "exe"
		  f = GetOpenFolderItem(AppType)
		  if f<>nil then
		    DXFViewerApp = f.AbsolutePath
		  end if


		End Sub
	#tag EndEvent
#tag EndEvents
#tag Events cmdClose
	#tag Event
		Sub Action()
		  close()
		End Sub
	#tag EndEvent
#tag EndEvents
#tag Events cmdAbout
	#tag Event
		Sub Action()
		  dim t as String
		  t = "EPDrawGUI - Version 0.10" + EndOfLine+ EndOfLine
		  t = t + "Copyright (c) 2011 GARD Analytics, All rights reserved." + EndOfLine+ EndOfLine
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
