Attribute VB_Name = "IDFMain"
'-----------------------------------------------------------------------------
'do list
' run energyPlus
' make installer
'
' associate .IDF with IDFEditor in installation program
' include comment changes in Editor
' unsupported objects (in IDF but not IDD)
' add file change alert (like ultraedit)
' IP units in comments
' IP units shown
' weather directory sticky
' manage comments in IDF as multiple entries in an array
' comments from front of file
' resizable window and panes and shrink boxes
' object counters in object list on screen
' support \required-field, \ip-units, \unique-object, \required-object
' display status of main arrays usages
' add "comments for.." and "explaination of keyword for..."
' fix ORG backup and BAK backup of files and smooth out saving
'
'done
' insert column, copy column, delete column
' new file/make empty IDF file (initialize/reinitialize pointers)
' respond to edits/selections in combobox
' add file name to heading
' respond to clicks on grid
' add toolbar
' number of objects displayed in list
' save IDF file
'-----------------------------------------------------------------------------
'
' command line argument /idd:iddfilename
'

Public Const ver = "1.55" 'current version of IDFEditor - less than 1 is a beta
Option Explicit
Option Base 1

'define the variable to hold the main parent form
Public frmParent As parentMDI

'variable that says if IDD should be selected or not
Public useSpecialIDD As Boolean
Public specialIDDName As String
Public useIDDhardPath As Boolean

' Define all array sizes

' now that this is dynamic, don't need the following
'Public Const maxNumObjects = 10000
'Public Const maxNumValues = 1500000
'Public Const maxNumComments = 300000
'Public Const maxNumFields = 40000
'Public Const maxNumChoice = 10000
'Public Const maxNumClassGroup = 200
'Public Const maxNumObjListItem = 300
'Public Const maxNumObjListName = 200
'Public Const maxNumConvUnits = 300
'Public Const maxNumClassNames = 2000


' Main array map
'
'
'   IDDClassGroup
'      |
'      |----- IDDClassDat
'      |         |
'      |         |-------------- IDDField
'      |
'      |----- IDDClassObjPt
'                |
'                |---- IDFObject
'                          |
'                          |---- IDFValue
'



' ==================== Variables for IDD ==========================
' Define arrays for holding IDD information

Type IDDClassGroupRecord
  name As String
  classStart As Long
End Type

Public IDDClassGroup() As IDDClassGroupRecord
Public maxUsedClassGroup As Long
Public sizeClassGroup As Long

' the IDDClassDat and IDDClassObjPt were one array and type prior to the MDI version
' of the software - one pointer is used for both arrays. The IDDClassObjPt just
' holds the pointers to the object start
Type IDDClassRecord
  name As String ' contains "Location", etc
  fieldStart As Long  ' pointer to the first field
  fieldEnd As Long    ' pointer to the last field
  memo As String         ' contains the text that is the documentation for the object
  minFields As Long 'minimum number of fields that should be put in IDF
  format As Integer
  specialMultiUnit As Boolean 'true if special object handling for multiple unit fields (certain schedule objects)
  uniqueObject As Boolean 'if \unique-object is specified
End Type
Public Const formatStandard = 0
Public Const formatSingleLine = 1
Public Const formatVertices = 2
Public Const formatCompactSch = 3
Public Const formatFluidProperty = 4
Public Const formatViewFactor = 5
Public Const formatSpectral = 6

Public IDDClassDat() As IDDClassRecord
Public maxUsedIDDClass As Long
Public curIDDClass As Long
Public sizeClassDat As Long

Type IDDClassObjPtType
  objectStart As Long ' pointer to the first IDF object (objects are in linked list)
  objectCount As Long ' the number of objects in the class
  lstObjIndx As Long   'pointer to the list of objects
End Type

Type IDDFieldRecord
  id As String 'contains "A1", "N2" etc
  AN As Byte '1 if A, 2 if N
'The following arrays are the extra data from the \key value items in the IDD
  name As String
  type As Byte  '1=real, 2=integer, 3=alpha, 4=generic choice, 5=reference-list choice, 6=node
  Units As String 'value shown
  IPUnits As String 'the IP-Units if entered
  autosizable As Boolean  'if autosizable then true
  defaultAutosize As Boolean  'if autosizable is a default
  autocalculatable As Boolean
  defaultAutoCalc As Boolean
  unitsIndex As Long   'pointer to list of units
  minimum As Double
  maximum As Double
  minSpecified As Boolean 'if minimum specified than true
  maxSpecified As Boolean 'if maximum specified than true
  exclusiveMin As Boolean 'if exclusive minimum than true
  exclusiveMax As Boolean 'if exclusive maximum than true
  defSpecified As Boolean 'if default is specified
  defaultValue As Double
  defaultChoice As Long  'pointer to choice list
  note As String
  choiceStart As Long  'pointer to choice list
  choiceEnd As Long  'pointer to last choice in choice list
  ' no longer use as of Nov 2010 - use next two fields instead - objListIndex As Long ' pointer to the object list name
  listOfObjListStart As Long 'pointer to the first item in the list of object lists
  listOfObjListEnd As Long 'pointer to the last item in the list of object lists
  autoObjList As Integer
  deprecated As Boolean 'if the field is no longer used by EnergyPlus
  required As Boolean 'if \required-field is specified
  preserveIndent As Boolean 'preserve the users leading spaces if \preserveIndent is specified
  unitsBasedOnFieldString As String 'field ID
  unitsBasedOnFieldIndex As Long 'pointer to the field index for the units
  usedForUnitsBasedOn As Boolean 'this field is used by other fields to determine the units
End Type
Const fieldA = 1
Const fieldN = 2
Public Const autoObjListKindVar = 1
Public Const autoObjListKindMeter = 2
Public Const autoObjListKindVarMeter = 3

Public IDDField() As IDDFieldRecord
Public maxUsedField As Long
Public sizeField As Long

Public IDDChoice() As String
Public maxUsedChoice As Long
Public sizeChoice As Long

' This array holds the name of the object-list and points to the
' array that holds the list of where that object-list is referenced
' i.e. IDDobjListItems()
Type objListNameRecord
  name As String
  objListItemStart As Long
  usedInObjectList As Boolean
  usedInReference As Boolean
End Type
Public IDDObjListName() As objListNameRecord
Public maxUsedObjListName As Long
Public sizeObjListName As Long

' This array holds lists of pointers for object-list so that the
' the class and field of each reference to an object-list can be
' scanned dynamically so that the references are up to date.
Type objListItemRecord
  classWithRef As Long
  fieldWithRef As Long
  nextObjListItem As Long
End Type
Public IDDObjListItem() As objListItemRecord
Public maxUsedObjListItem As Long
Public sizeObjListItem As Long
'special case of fieldWithRef to indicate using class name instead of field
Public Const fwrClassName = -999

' In Nov 2010 added the ability to have multiple \object-list in
' a single field. To do this, the fields no longer point to an object
' list array directly but first to the following array that contains
' all of the possible object lists for that field. The values of this
' array point to the IDDObjListName array.
Public ListOfObjList() As Long
Public maxListOfObjList As Long
Public sizeListOfObjList As Long

Type conversionUnitsRecord
  siName As String   'metric name of units
  ipName As String   'english name of units
  mult As Double     'conversion multiplier english/metric
  offset As Double   'conversion offset english = (metric * multi) + offset
  alt As Boolean     'indicates if an alternative or if a default
  multiUnitName As String 'unit used to process fields with multiple possible units
End Type
Public convUnits() As conversionUnitsRecord
Public maxUsedConvUnits As Long
Public Const noUnitsSpecified = -1
Public Const unitsVaryByObject = -2
Public unitsDimensionless As Long

'classes that need special handling related to fields that can have multiple units related to schedules
Public classScheduleTypeLimits As Long
Public classScheduleDayHourly As Long
Public classScheduleDayList As Long
Public classScheduleDayInterval As Long
Public classScheduleCompact As Long
Public classScheduleConstant As Long

' ==================== Variables for IDF ==========================
' Define arrys for holding IDF information
Type objectRecord
  classType As Long  'pointer to the IDDClass array
  'ValueStart is a pointer to the value array
  'number of values determined by number of IDDClass.fieldstart and .fieldend
  valueStart As Long
  'nextObjectInClass is a linked list structure where each object refers to the next object
  'in the same class.  If zero, it is the last object.
  'If isDeleted (-999) then the object has been deleted
  nextObjectInClass As Long
End Type
Public Const isDeleted = -999

' IDFValue is the main array that holds the displayed values in the grid
' of the main screen.  It is pointed to by the IDFObject valueStart pointer
' and the number of items for that object is based on the number of possible
' items for that entire class (see IDDClass).  It is a string array because
' everything is assumed to be a string and the fact that they are values is
' only computed at input time.
Type IDFValueRecord
  entry As String
  outstr As String
  commentStart As Long
  commentEnd As Long
  rangeTestStatus As Boolean
  isExpression As Boolean
  leadingSpaces As Integer 'number of leading spaces, only useful if preserveIndent is true
End Type

Public IDDFileName As String  'the name and path for the active IDD file
Public postprocessorFileName As String
Public executableDirectory As String
Public IDDVersion As String   'holds the version identifier for the IDD file

Public acrobatReaderFileName As String 'path and file to the Acrobat reader program
Public documentationPath As String     'path to the energyplus documentation directory
Public dataSetPath As String           'path to the datasets direcotry

Public showVersionWarnings As Boolean

Public errCnt As Long

Public Const outOfRangeColor = &HC0E0FF

Public formLayoutOption As Integer
Public Const floShortShort = 1
Public Const floShortTall = 2
Public Const floTallShort = 3
Public Const floTallTall = 4

Public saveOrderOptDefault As Integer
Public Const saveOrderSorted = 0
Public Const saveOrderOrigTop = 1
Public Const saveOrderOrigBot = 2

Public specialFormatOptDefault As Integer
Public Const specFormYes = 0
Public Const specFormNo = 1

'this is a setting across files
Public checkRangeOnSave As Integer
Public Const checkRangeYes = 1
Public Const checkRangeNo = 2
Public ignoreValidatingMacros As Boolean

Public useWordWrap As Boolean

Public previousVersion As String 'saved version of IDF Editor for displaying whats new dialog

Public searchTerm As String
Public replaceTerm As String

Type recentFilesType
  nameOnly As String
  nameWithPath As String
End Type
Public Const maxRecentFiles = 6  'Maximum number of recent files in list - make sure list on forms include this many
Public recentFiles(maxRecentFiles) As recentFilesType
Public numRecentFiles As Integer 'number of active recent files

Type nodeNameDialogType
  name As String
  clsIndx As Long
  objName As String
  fldIndx As Long
  isUnique As Boolean
  isRecent As Boolean
End Type
Public nodeNameDialog() As nodeNameDialogType
Public maxNodeNameDialog As Long
Public sizeNodeNameDialog As Long

Public htmlViewFileName As String


Private Declare Function FindExecutable Lib "shell32.dll" Alias "FindExecutableA" (ByVal lpFile As String, ByVal lpDirectory As String, ByVal lpresults As String) As Long
Private Declare Function GetShortPathName Lib "kernel32.dll" Alias "GetShortPathNameA" (ByVal lpszLongPath As String, ByVal lpszShortPath As String, ByVal cchBuffer As Long) As Long


' ==================== Variables and Types for Getting Shelled Process ==========================

   Private Type STARTUPINFO
      cb As Long
      lpReserved As String
      lpDesktop As String
      lpTitle As String
      dwX As Long
      dwY As Long
      dwXSize As Long
      dwYSize As Long
      dwXCountChars As Long
      dwYCountChars As Long
      dwFillAttribute As Long
      dwFlags As Long
      wShowWindow As Integer
      cbReserved2 As Integer
      lpReserved2 As Long
      hStdInput As Long
      hStdOutput As Long
      hStdError As Long
   End Type
   Private Type PROCESS_INFORMATION
      hProcess As Long
      hThread As Long
      dwProcessID As Long
      dwThreadID As Long
   End Type
   Private Declare Function WaitForSingleObject Lib "kernel32" (ByVal _
      hHandle As Long, ByVal dwMilliseconds As Long) As Long
   Private Declare Function CreateProcessA Lib "kernel32" (ByVal _
      lpApplicationName As String, ByVal lpCommandLine As String, ByVal _
      lpProcessAttributes As Long, ByVal lpThreadAttributes As Long, _
      ByVal bInheritHandles As Long, ByVal dwCreationFlags As Long, _
      ByVal lpEnvironment As Long, ByVal lpCurrentDirectory As String, _
      lpStartupInfo As STARTUPINFO, lpProcessInformation As _
      PROCESS_INFORMATION) As Long
   Private Declare Function CloseHandle Lib "kernel32" _
      (ByVal hObject As Long) As Long
   Private Declare Function GetExitCodeProcess Lib "kernel32" _
      (ByVal hProcess As Long, lpExitCode As Long) As Long
   Private Const NORMAL_PRIORITY_CLASS = &H20&
   Private Const INFINITE = -1&
'-----------------------------------------------------------------------------
' CEPTChange: Added for autocomplete and autodropdown functionality on combobox
'-----------------------------------------------------------------------------
Private Declare Function SendMessage Lib "user32" Alias "SendMessageA" ( _
        ByVal hwnd As Long, _
        ByVal wMsg As Long, _
        ByVal wParam As Long, _
        lParam As Any _
        ) As Long                           'internal windows messaging API

Private Const CB_ERR = -1
Private Const CB_SELECTSTRING = &H14D
Private Const CBN_SELENDOK = 9
Private Const CB_SHOWDROPDOWN = &H14F

' to store Show Quick select combos or not
Public ShowQuickSelectCombos As Boolean
'------------CEPTChange over----------------------------------------------------

   Public Function ExecCmd(cmdline$)
      Dim proc As PROCESS_INFORMATION
      Dim Start As STARTUPINFO
      Dim Ret&
      ' Initialize the STARTUPINFO structure:
      Start.cb = Len(Start)
      ' Start the shelled application:
      Ret& = CreateProcessA(vbNullString, cmdline$, 0&, 0&, 1&, _
         NORMAL_PRIORITY_CLASS, 0&, vbNullString, Start, proc)
      ' Wait for the shelled application to finish:
         Ret& = WaitForSingleObject(proc.hProcess, INFINITE)
         Call GetExitCodeProcess(proc.hProcess, Ret&)
         Call CloseHandle(proc.hThread)
         Call CloseHandle(proc.hProcess)
         ExecCmd = Ret&
   End Function


'***************************************************************************
' Converts Long FileName to Short FileName
'***************************************************************************
Public Function ShortName(LongPath As String) As String
    Dim ShortPath As String
    Dim Ret As Long
    Const MAX_PATH = 260
    If LongPath = "" Then
        Exit Function
    End If
    ShortPath = Space$(MAX_PATH)
    Ret = GetShortPathName(LongPath, ShortPath, MAX_PATH)
    If Ret Then
        ShortName = Left$(ShortPath, Ret)
    End If
End Function


'-----------------------------------------------------------------------------
' Main starting point for the entire program
'-----------------------------------------------------------------------------
Sub Main()
Dim locSlashIDD As Integer
Dim commandLine As String
showVersionWarnings = True
commandLine = Command
useIDDhardPath = False
Call createDynamicArrays
locSlashIDD = InStr(commandLine, "/idd:")
If locSlashIDD > 0 Then
  useSpecialIDD = True
  specialIDDName = Trim(Mid(commandLine, locSlashIDD + 5))
  If locSlashIDD > 1 Then
    commandLine = Left(commandLine, locSlashIDD - 1)
  Else
    commandLine = ""
  End If
Else
  useSpecialIDD = False
End If
Set frmParent = New parentMDI
frmParent.Show
'to handle double clicked documents (show up on the command line)
If commandLine <> "" Then
  Call frmParent.doDoubleClickDocument(commandLine)
Else
  frmParent.loadBlankDocument
End If
Load saveOption
If previousVersion <> ver Then frmWhatsNew.Show
End Sub

'-----------------------------------------------------------------------------
' Initialize the Dynamic Arrays
'-----------------------------------------------------------------------------
Sub createDynamicArrays()
' field
sizeField = 12000
ReDim IDDField(sizeField)
Call resizeFieldArray(1)
'choice
sizeChoice = 600
ReDim IDDChoice(sizeChoice)
Call resizeChoiceArray(1)
'class group
sizeClassGroup = 20
ReDim IDDClassGroup(sizeClassGroup)
Call resizeClassGroupArray(1)
'ObjListItem
sizeObjListItem = 20
ReDim IDDObjListItem(sizeObjListItem)
Call resizeObjListItemArray(1)
'ObjListName
sizeObjListName = 20
ReDim IDDObjListName(sizeObjListName)
Call resizeObjListNameArray(1)
'IDDClassDat
sizeClassDat = 80
ReDim IDDClassDat(sizeClassDat)
Call resizeClassDatArray(1)
'ListOfObjList
sizeListOfObjList = 1500
ReDim ListOfObjList(sizeListOfObjList)
Call resizeListOfObjListArray(1)
'NodeNameDialog
sizeNodeNameDialog = 500
ReDim nodeNameDialog(sizeNodeNameDialog)
Call resizeNodeNameDialogArray(1)
End Sub

'-----------------------------------------------------------------------------
' Increases the size of the field array if more space is needed
'-----------------------------------------------------------------------------
Sub resizeFieldArray(addedSpace As Long)
Dim orgSize As Long
orgSize = sizeField
Do While maxUsedField + addedSpace > sizeField
  sizeField = sizeField * 2
Loop
If sizeField > orgSize Then
  ReDim Preserve IDDField(sizeField)
End If
'Debug.Print "Field array resized to: "; sizeField
End Sub

'-----------------------------------------------------------------------------
' Increases the size of the choice array if more space is needed
'-----------------------------------------------------------------------------
Sub resizeChoiceArray(addedSpace As Long)
Dim orgSize As Long
orgSize = sizeChoice
Do While maxUsedChoice + addedSpace > sizeChoice
  sizeChoice = sizeChoice * 2
Loop
If sizeChoice > orgSize Then
  ReDim Preserve IDDChoice(sizeChoice)
End If
'Debug.Print "Choice array resized to: "; sizeChoice
End Sub

'-----------------------------------------------------------------------------
' Increases the size of the choice array if more space is needed
'-----------------------------------------------------------------------------
Sub resizeClassGroupArray(addedSpace As Long)
Dim orgSize As Long
orgSize = sizeClassGroup
Do While maxUsedClassGroup + addedSpace > sizeClassGroup
  sizeClassGroup = sizeClassGroup * 2
Loop
If sizeClassGroup > orgSize Then
  ReDim Preserve IDDClassGroup(sizeClassGroup)
End If
'Debug.Print "ClassGroup array resized to: "; sizeClassGroup
End Sub

'-----------------------------------------------------------------------------
' Increases the size of the choice array if more space is needed
'-----------------------------------------------------------------------------
Sub resizeObjListItemArray(addedSpace As Long)
Dim orgSize As Long
orgSize = sizeObjListItem
Do While maxUsedObjListItem + addedSpace > sizeObjListItem
  sizeObjListItem = sizeObjListItem * 2
Loop
If sizeObjListItem > orgSize Then
  ReDim Preserve IDDObjListItem(sizeObjListItem)
End If
'Debug.Print "ClassGroup array resized to: "; sizeClassGroup
End Sub

'-----------------------------------------------------------------------------
' Increases the size of the choice array if more space is needed
'-----------------------------------------------------------------------------
Sub resizeObjListNameArray(addedSpace As Long)
Dim orgSize As Long
orgSize = sizeObjListName
Do While maxUsedObjListName + addedSpace > sizeObjListName
  sizeObjListName = sizeObjListName * 2
Loop
If sizeObjListName > orgSize Then
  ReDim Preserve IDDObjListName(sizeObjListName)
End If
'Debug.Print "ClassGroup array resized to: "; sizeClassGroup
End Sub

'-----------------------------------------------------------------------------
' Increases the size of the IDDClassDat array if more space is needed
'-----------------------------------------------------------------------------
Sub resizeClassDatArray(addedSpace As Long)
Dim orgSize As Long
orgSize = sizeClassDat
Do While maxUsedIDDClass + addedSpace > sizeClassDat
  sizeClassDat = sizeClassDat * 2
Loop
If sizeClassDat > orgSize Then
  ReDim Preserve IDDClassDat(sizeClassDat)
End If
'Debug.Print "ClassDat array resized to: "; sizeClassDat
End Sub

'-----------------------------------------------------------------------------
' Increases the size of the listOfObjList array if more space is needed
'-----------------------------------------------------------------------------
Sub resizeListOfObjListArray(addedSpace As Long)
Dim orgSize As Long
orgSize = sizeListOfObjList
Do While maxListOfObjList + addedSpace > sizeListOfObjList
  sizeListOfObjList = sizeListOfObjList * 2
Loop
If sizeListOfObjList > orgSize Then
  ReDim Preserve ListOfObjList(sizeListOfObjList)
End If
'Debug.Print "listOfObjList array resized to: "; sizeListOfObjList
End Sub

'-----------------------------------------------------------------------------
' Increases the size of the nodeNameDialog array if more space is needed
'-----------------------------------------------------------------------------
Sub resizeNodeNameDialogArray(addedSpace As Long)
Dim orgSize As Long
orgSize = sizeNodeNameDialog
Do While maxNodeNameDialog + addedSpace > sizeNodeNameDialog
  sizeNodeNameDialog = sizeNodeNameDialog * 2
Loop
If sizeNodeNameDialog > orgSize Then
  ReDim Preserve nodeNameDialog(sizeNodeNameDialog)
End If
End Sub

'-----------------------------------------------------------------------------
' Set the unit conversion values for all units that appear in the EnergyPlus
' IDD file
'
' Note ALTERNATIVE units always come after the DEFAULT unit combination
'-----------------------------------------------------------------------------
Sub setUnits()
Dim i As Long
maxUsedConvUnits = 156
unitsDimensionless = 56
ReDim convUnits(maxUsedConvUnits)
' the following is pasted in from the unitsIPandSI.xls file
convUnits(1).siName = "m"
convUnits(2).siName = "m"
convUnits(3).siName = "W"
convUnits(4).siName = "W"
convUnits(5).siName = "m3/s"
convUnits(6).siName = "m3/s"
convUnits(7).siName = "C"
convUnits(8).siName = "kg/J"
convUnits(9).siName = "Pa"
convUnits(10).siName = "Pa"
convUnits(11).siName = "Pa"
convUnits(12).siName = "Pa"
convUnits(13).siName = "W/m-K"
convUnits(14).siName = "W/K"
convUnits(15).siName = "deltaC"
convUnits(16).siName = "m2"
convUnits(17).siName = "K"
convUnits(18).siName = "(kg/s)/W"
convUnits(19).siName = "J/kg"
convUnits(20).siName = "kgWater/kgDryAir"
convUnits(21).siName = "kJ/kg"
convUnits(22).siName = "lux"
convUnits(23).siName = "kg/m3"
convUnits(24).siName = "kg/s"
convUnits(25).siName = "kg/s-m"
convUnits(26).siName = "m3"
convUnits(27).siName = "m3"
convUnits(28).siName = "W/m2-K"
convUnits(29).siName = "1/m"
convUnits(30).siName = "J/kg-K"
convUnits(31).siName = "J/m3-K"
convUnits(32).siName = "m/s"
convUnits(33).siName = "m/s"
convUnits(34).siName = "m2-K/W"
convUnits(35).siName = "W/m2"
convUnits(36).siName = "A/K"
convUnits(37).siName = "g/kg"
convUnits(38).siName = "g/m-s"
convUnits(39).siName = "g/m-s-K"
convUnits(40).siName = "J/K"
convUnits(41).siName = "J/kg-K2"
convUnits(42).siName = "J/m3"
convUnits(43).siName = "kg/kg-K"
convUnits(44).siName = "kPa"
convUnits(45).siName = "kPa"
convUnits(46).siName = "m2/s"
convUnits(47).siName = "m3/kg"
convUnits(48).siName = "m3/m3"
convUnits(49).siName = "N-s/m2"
convUnits(50).siName = "V/K"
convUnits(51).siName = "W/m-K2"
convUnits(52).siName = "m3/s-m"
convUnits(53).siName = "deg"
convUnits(54).siName = "hr"
convUnits(55).siName = "A"
convUnits(56).siName = "dimensionless"
convUnits(57).siName = "V"
convUnits(58).siName = "A/V"
convUnits(59).siName = "eV"
convUnits(60).siName = "percent"
convUnits(61).siName = "percentage (as a real decimal)"
convUnits(62).siName = "s"
convUnits(63).siName = "W/m2 or deg C"
convUnits(64).siName = "W/m2, W or deg C"
convUnits(65).siName = "1/K"
convUnits(66).siName = "J/m2-K"
convUnits(67).siName = "ohms"
convUnits(68).siName = "cycles/hr"
convUnits(69).siName = "kg/kg"
convUnits(70).siName = "J/J"
convUnits(71).siName = "g/GJ"
convUnits(72).siName = "L/GJ"
convUnits(73).siName = "m3/GJ"
convUnits(74).siName = "m3/s-m2"
convUnits(75).siName = "m3/s-person"
convUnits(76).siName = "W/m2-K2"
convUnits(77).siName = "g/MJ"
convUnits(78).siName = "L/MJ"
convUnits(79).siName = "m3/MJ"
convUnits(80).siName = "W/W"
convUnits(81).siName = "$/m2"
convUnits(82).siName = "$"
convUnits(83).siName = "$/kW"
convUnits(84).siName = "$/m3"
convUnits(85).siName = "years"
convUnits(86).siName = "$/(W/K)"
convUnits(87).siName = "$/(m3/s)"
convUnits(88).siName = "W/m"
convUnits(89).siName = "minutes"
convUnits(90).siName = "cm"
convUnits(91).siName = "K/m"
convUnits(92).siName = "W/s"
convUnits(93).siName = "kmol"
convUnits(94).siName = "J"
convUnits(95).siName = "GJ"
convUnits(96).siName = "days"
convUnits(97).siName = "kg/m2"
convUnits(98).siName = "kg"
convUnits(99).siName = "kmol/s"
convUnits(100).siName = "percent/K"
convUnits(101).siName = "kg/s2"
convUnits(102).siName = "g/mol"
convUnits(103).siName = "deltaJ/kg"
convUnits(104).siName = "person/m2"
convUnits(105).siName = "m2/person"
convUnits(106).siName = "W/person"
convUnits(107).siName = "W/person"
convUnits(108).siName = "W/m2"
convUnits(109).siName = "m3/person"
convUnits(110).siName = "m3/hr-person"
convUnits(111).siName = "m3/m2"
convUnits(112).siName = "m3/hr-m2"
convUnits(113).siName = "m3/hr"
convUnits(114).siName = "s/m"
convUnits(115).siName = "W/m2"
convUnits(116).siName = "m2/m"
convUnits(117).siName = "L/day"
convUnits(118).siName = "L/kWh"
convUnits(119).siName = "kg/Pa-s-m2"
convUnits(120).siName = "m/hr"
convUnits(121).siName = "Mode"
convUnits(122).siName = "Control"
convUnits(123).siName = "Availability"
convUnits(124).siName = "rev/min"
convUnits(125).siName = "W/(m3/s)"
convUnits(126).siName = "W/m-K"
convUnits(127).siName = "VA"
convUnits(128).siName = "N-m"
convUnits(129).siName = "m3/s-W"
convUnits(130).siName = "cm2"
convUnits(131).siName = "kg/m"
convUnits(132).siName = "Pa"
convUnits(133).siName = "m/yr"
convUnits(134).siName = "1/hr"
convUnits(135).siName = "ppm"
convUnits(136).siName = "W/m-K3"
convUnits(137).siName = "kg/m-s"
convUnits(138).siName = "kg/m-s-K"
convUnits(139).siName = "kg/m-s-K2"
convUnits(140).siName = "J/kg-K3"
convUnits(141).siName = "ms"
convUnits(142).siName = "Ah"
convUnits(143).siName = "deltaC/hr"
convUnits(144).siName = "micron"
convUnits(145).siName = "W/(m3/s)"
convUnits(146).siName = "W/((m3/s)-Pa)"
convUnits(147).siName = "m3/s-m"
convUnits(148).siName = "m3/s-W"
convUnits(149).siName = "m3/person"
convUnits(150).siName = "m3/hr-person"
convUnits(151).siName = "m3/m2"
convUnits(152).siName = "m3/hr-m2"
convUnits(153).siName = "m3/hr"
convUnits(154).siName = "W/((m3/s)-Pa)"
convUnits(155).siName = "W/m2, deg C or cd/m2"
convUnits(156).siName = "umol/m2-s"

convUnits(1).ipName = "ft"
convUnits(2).ipName = "in"
convUnits(3).ipName = "Btu/h"
convUnits(4).ipName = "W"
convUnits(5).ipName = "ft3/min"
convUnits(6).ipName = "gal/min"
convUnits(7).ipName = "F"
convUnits(8).ipName = "lb/Btu"
convUnits(9).ipName = "psi"
convUnits(10).ipName = "inHg"
convUnits(11).ipName = "inH2O"
convUnits(12).ipName = "ftH2O"
convUnits(13).ipName = "Btu-in/h-ft2-F"
convUnits(14).ipName = "Btu/h-F"
convUnits(15).ipName = "deltaF"
convUnits(16).ipName = "ft2"
convUnits(17).ipName = "R"
convUnits(18).ipName = "(lbm/sec)/(Btu/hr)"
convUnits(19).ipName = "Btu/lb"
convUnits(20).ipName = "lbWater/lbDryAir"
convUnits(21).ipName = "Btu/lb"
convUnits(22).ipName = "foot-candles"
convUnits(23).ipName = "lb/ft3"
convUnits(24).ipName = "lb/s"
convUnits(25).ipName = "lb/s-ft"
convUnits(26).ipName = "ft3"
convUnits(27).ipName = "gal"
convUnits(28).ipName = "Btu/h-ft2-F"
convUnits(29).ipName = "1/ft"
convUnits(30).ipName = "Btu/lb-F"
convUnits(31).ipName = "Btu/ft3-F"
convUnits(32).ipName = "ft/min"
convUnits(33).ipName = "miles/hr"
convUnits(34).ipName = "ft2-F-hr/Btu"
convUnits(35).ipName = "Btu/h-ft2"
convUnits(36).ipName = "A/F"
convUnits(37).ipName = "grains/lb"
convUnits(38).ipName = "lb/ft-s"
convUnits(39).ipName = "lb/ft-s-F"
convUnits(40).ipName = "Btu/F"
convUnits(41).ipName = "Btu/lb-F2"
convUnits(42).ipName = "Btu/ft3"
convUnits(43).ipName = "lb/lb-F"
convUnits(44).ipName = "psi"
convUnits(45).ipName = "inHg"
convUnits(46).ipName = "ft2/s"
convUnits(47).ipName = "ft3/lb"
convUnits(48).ipName = "ft3/ft3"
convUnits(49).ipName = "lbf-s/ft2"
convUnits(50).ipName = "V/F"
convUnits(51).ipName = "Btu/h-F2-ft"
convUnits(52).ipName = "ft3/min-ft"
convUnits(53).ipName = "deg"
convUnits(54).ipName = "hr"
convUnits(55).ipName = "A"
convUnits(56).ipName = "dimensionless"
convUnits(57).ipName = "V"
convUnits(58).ipName = "A/V"
convUnits(59).ipName = "eV"
convUnits(60).ipName = "percent"
convUnits(61).ipName = "percentage (as a real decimal)"
convUnits(62).ipName = "s"
convUnits(63).ipName = "unknown"
convUnits(64).ipName = "unknown"
convUnits(65).ipName = "1/F"
convUnits(66).ipName = "Btu/ft2-F"
convUnits(67).ipName = "ohms"
convUnits(68).ipName = "cycles/hr"
convUnits(69).ipName = "lb/lb"
convUnits(70).ipName = "Btu/Btu"
convUnits(71).ipName = "lb/MWh"
convUnits(72).ipName = "gal/kWh"
convUnits(73).ipName = "ft3/MWh"
convUnits(74).ipName = "ft3/min-ft2"
convUnits(75).ipName = "ft3/min-person"
convUnits(76).ipName = "Btu/h-ft2-F2"
convUnits(77).ipName = "lb/MWh"
convUnits(78).ipName = "gal/kWh"
convUnits(79).ipName = "ft3/kWh"
convUnits(80).ipName = "Btuh/Btuh"
convUnits(81).ipName = "$/ft2"
convUnits(82).ipName = "$"
convUnits(83).ipName = "$/(kBtuh/h)"
convUnits(84).ipName = "$/ft3"
convUnits(85).ipName = "years"
convUnits(86).ipName = "$/(Btu/h-F)"
convUnits(87).ipName = "$/(ft3/min)"
convUnits(88).ipName = "Btu/h-ft"
convUnits(89).ipName = "minutes"
convUnits(90).ipName = "in"
convUnits(91).ipName = "F/ft"
convUnits(92).ipName = "W/s"
convUnits(93).ipName = "kmol"
convUnits(94).ipName = "Wh"
convUnits(95).ipName = "ton-hrs"
convUnits(96).ipName = "days"
convUnits(97).ipName = "lb/ft2"
convUnits(98).ipName = "lb"
convUnits(99).ipName = "kmol/s"
convUnits(100).ipName = "percent/F"
convUnits(101).ipName = "lb/s2"
convUnits(102).ipName = "lb/mol"
convUnits(103).ipName = "deltaBtu/lb"
convUnits(104).ipName = "person/ft2"
convUnits(105).ipName = "ft2/person"
convUnits(106).ipName = "Btu/h-person"
convUnits(107).ipName = "W/person"
convUnits(108).ipName = "W/m2"
convUnits(109).ipName = "ft3/person"
convUnits(110).ipName = "ft3/hr-person"
convUnits(111).ipName = "ft3/ft2"
convUnits(112).ipName = "ft3/hr-ft2"
convUnits(113).ipName = "ft3/hr"
convUnits(114).ipName = "s/ft"
convUnits(115).ipName = "W/ft2"
convUnits(116).ipName = "ft2/ft"
convUnits(117).ipName = "pint/day"
convUnits(118).ipName = "pint/kWh"
convUnits(119).ipName = "lb/psi-s-ft2"
convUnits(120).ipName = "ft/hr"
convUnits(121).ipName = "Mode"
convUnits(122).ipName = "Control"
convUnits(123).ipName = "Availability"
convUnits(124).ipName = "rev/min"
convUnits(125).ipName = "W/(ft3/min)"
convUnits(126).ipName = "Btu/h-ft-F"
convUnits(127).ipName = "VA"
convUnits(128).ipName = "lbf-in"
convUnits(129).ipName = "(ft3/min)/(Btu/h)"
convUnits(130).ipName = "inch2"
convUnits(131).ipName = "lb/ft"
convUnits(132).ipName = "Pa"
convUnits(133).ipName = "inch/yr"
convUnits(134).ipName = "1/hr"
convUnits(135).ipName = "ppm"
convUnits(136).ipName = "Btu/h-F3-ft"
convUnits(137).ipName = "kg/m-s"
convUnits(138).ipName = "kg/m-s-F"
convUnits(139).ipName = "kg/m-s-F2"
convUnits(140).ipName = "J/kg-K3"
convUnits(141).ipName = "ms"
convUnits(142).ipName = "Ah"
convUnits(143).ipName = "deltaF/hr"
convUnits(144).ipName = "micron"
convUnits(145).ipName = "W/(gal/min)"
convUnits(146).ipName = "W/((gal/min)-ftH20)"
convUnits(147).ipName = "gal/min-ft"
convUnits(148).ipName = "(gal/min)/(Btu/h)"
convUnits(149).ipName = "gal/person"
convUnits(150).ipName = "gal/hr-person"
convUnits(151).ipName = "gal/ft2"
convUnits(152).ipName = "gal/hr-ft2"
convUnits(153).ipName = "gal/hr"
convUnits(154).ipName = "W/((ft3/min)-inH2O)"
convUnits(155).ipName = "unknown"
convUnits(156).ipName = "umol/ft2-s"

convUnits(1).mult = 3.28083989501312
convUnits(2).mult = 39.3700787401575
convUnits(3).mult = 3.4121412858518
convUnits(4).mult = 1
convUnits(5).mult = 2118.88000328931
convUnits(6).mult = 15850.3222370511
convUnits(7).mult = 1.8
convUnits(8).mult = 2325.83774250441
convUnits(9).mult = 1.45037743897283E-04
convUnits(10).mult = 0.00029613
convUnits(11).mult = 0.00401463
convUnits(12).mult = 0.00033455
convUnits(13).mult = 6.93481276005548
convUnits(14).mult = 1.89563404769544
convUnits(15).mult = 1.8
convUnits(16).mult = 10.7639104167097
convUnits(17).mult = 1.8
convUnits(18).mult = 0.646078115385742
convUnits(19).mult = 0.00042986
convUnits(20).mult = 1
convUnits(21).mult = 0.429925
convUnits(22).mult = 0.092902267
convUnits(23).mult = 0.062428
convUnits(24).mult = 2.20462247603796
convUnits(25).mult = 0.67196893069637
convUnits(26).mult = 35.3146667214886
convUnits(27).mult = 264.172037284185
convUnits(28).mult = 0.176110194261872
convUnits(29).mult = 0.3048
convUnits(30).mult = 2.39005736137667E-04
convUnits(31).mult = 1.49237004739337E-05
convUnits(32).mult = 196.850393700787
convUnits(33).mult = 2.2369362920544
convUnits(34).mult = 5.678263
convUnits(35).mult = 0.316957210776545
convUnits(36).mult = 0.555555555555556
convUnits(37).mult = 7
convUnits(38).mult = 0.000671968949659
convUnits(39).mult = 3.73574867724868E-04
convUnits(40).mult = 526.565
convUnits(41).mult = 1.32889924714692E-04
convUnits(42).mult = 2.68096514745308E-05
convUnits(43).mult = 0.555555555555556
convUnits(44).mult = 0.145038
convUnits(45).mult = 0.29523
convUnits(46).mult = 10.7639104167097
convUnits(47).mult = 16.018
convUnits(48).mult = 1
convUnits(49).mult = 2.08857913669065E-02
convUnits(50).mult = 0.555555555555556
convUnits(51).mult = 0.321418310071648
convUnits(52).mult = 645.89
convUnits(53).mult = 1
convUnits(54).mult = 1
convUnits(55).mult = 1
convUnits(56).mult = 1
convUnits(57).mult = 1
convUnits(58).mult = 1
convUnits(59).mult = 1
convUnits(60).mult = 1
convUnits(61).mult = 1
convUnits(62).mult = 1
convUnits(63).mult = 1
convUnits(64).mult = 1
convUnits(65).mult = 0.555555555555556
convUnits(66).mult = 4.89224766847393E-05
convUnits(67).mult = 1
convUnits(68).mult = 1
convUnits(69).mult = 1
convUnits(70).mult = 1
convUnits(71).mult = 7.93664091373665E-03
convUnits(72).mult = 9.51022349025202E-04
convUnits(73).mult = 127.13292
convUnits(74).mult = 196.85
convUnits(75).mult = 2118.6438
convUnits(76).mult = 0.097826
convUnits(77).mult = 7.93664091373665
convUnits(78).mult = 0.951022349025202
convUnits(79).mult = 127.13292
convUnits(80).mult = 1
convUnits(81).mult = 9.28939733269818E-02
convUnits(82).mult = 1
convUnits(83).mult = 0.293083235638921
convUnits(84).mult = 2.83127014102352E-02
convUnits(85).mult = 1
convUnits(86).mult = 0.52667614683731
convUnits(87).mult = 4.72000059660808E-04
convUnits(88).mult = 1.04072
convUnits(89).mult = 1
convUnits(90).mult = 0.3937
convUnits(91).mult = 0.54861322767449
convUnits(92).mult = 1
convUnits(93).mult = 1
convUnits(94).mult = 2.77777777777778E-04
convUnits(95).mult = 78.9889415481832
convUnits(96).mult = 1
convUnits(97).mult = 0.204794053596664
convUnits(98).mult = 2.2046
convUnits(99).mult = 1
convUnits(100).mult = 0.555555555555556
convUnits(101).mult = 2.2046
convUnits(102).mult = 0.0022046
convUnits(103).mult = 0.0004299
convUnits(104).mult = 9.28939733269818E-02
convUnits(105).mult = 10.764961
convUnits(106).mult = 3.4121412858518
convUnits(107).mult = 1
convUnits(108).mult = 1
convUnits(109).mult = 35.3146667214886
convUnits(110).mult = 35.3146667214886
convUnits(111).mult = 3.28083989501312
convUnits(112).mult = 3.28083989501312
convUnits(113).mult = 35.3146667214886
convUnits(114).mult = 0.3048
convUnits(115).mult = 0.09290304
convUnits(116).mult = 3.28083989501312
convUnits(117).mult = 2.11337629827348
convUnits(118).mult = 2.11337629827348
convUnits(119).mult = 1412.00523459398
convUnits(120).mult = 3.28083989501312
convUnits(121).mult = 1
convUnits(122).mult = 1
convUnits(123).mult = 1
convUnits(124).mult = 1
convUnits(125).mult = 0.0004719475
convUnits(126).mult = 0.577796066000163
convUnits(127).mult = 1
convUnits(128).mult = 8.85074900525547
convUnits(129).mult = 621.099127332943
convUnits(130).mult = 0.15500031000062
convUnits(131).mult = 0.67196893069637
convUnits(132).mult = 1
convUnits(133).mult = 39.3700787401575
convUnits(134).mult = 1
convUnits(135).mult = 1
convUnits(136).mult = 0.178565727817582
convUnits(137).mult = 0.67196893069637
convUnits(138).mult = 0.373316072609094
convUnits(139).mult = 0.207397818116164
convUnits(140).mult = 7.38277359526066E-05
convUnits(141).mult = 1
convUnits(142).mult = 1
convUnits(143).mult = 1.8
convUnits(144).mult = 1
convUnits(145).mult = 0.0000630902
convUnits(146).mult = 0.188582274697355
convUnits(147).mult = 4831.17821785317
convUnits(148).mult = 4645.27137336702
convUnits(149).mult = 264.172037284185
convUnits(150).mult = 264.172037284185
convUnits(151).mult = 24.5423853466941
convUnits(152).mult = 24.5423853466941
convUnits(153).mult = 264.172037284185
convUnits(154).mult = 0.117556910599482
convUnits(155).mult = 1
convUnits(156).mult = 0.09290304

convUnits(7).offset = 32
convUnits(19).offset = 7.686
convUnits(2).alt = True
convUnits(4).alt = True
convUnits(6).alt = True
convUnits(10).alt = True
convUnits(11).alt = True
convUnits(12).alt = True
convUnits(45).alt = True
convUnits(107).alt = True
convUnits(108).alt = True
convUnits(126).alt = True
convUnits(132).alt = True
convUnits(145).alt = True
convUnits(147).alt = True
convUnits(148).alt = True
convUnits(149).alt = True
convUnits(150).alt = True
convUnits(151).alt = True
convUnits(152).alt = True
convUnits(153).alt = True
convUnits(154).alt = True

convUnits(1).multiUnitName = "Distance"
convUnits(3).multiUnitName = "Capacity"
convUnits(4).multiUnitName = "Power"
convUnits(5).multiUnitName = "VolumetricFlow"
convUnits(7).multiUnitName = "Temperature"
convUnits(9).multiUnitName = "Pressure"
convUnits(13).multiUnitName = "Conductivity"
convUnits(15).multiUnitName = "DeltaTemperature"
convUnits(19).multiUnitName = "Enthalpy"
convUnits(23).multiUnitName = "Density"
convUnits(24).multiUnitName = "MassFlow"
convUnits(28).multiUnitName = "ConvectionCoefficient"
convUnits(30).multiUnitName = "SpecificHeat"
convUnits(32).multiUnitName = "Velocity"
convUnits(49).multiUnitName = "Viscosity"
convUnits(53).multiUnitName = "Angle"
convUnits(56).multiUnitName = "Dimensionless"
convUnits(60).multiUnitName = "Percent"
convUnits(94).multiUnitName = "Energy"
convUnits(107).multiUnitName = "ActivityLevel"
convUnits(120).multiUnitName = "PrecipitationRate"
convUnits(121).multiUnitName = "Mode"
convUnits(122).multiUnitName = "Control"
convUnits(123).multiUnitName = "Availability"
convUnits(129).multiUnitName = "VolumetricFlowPerPower"
convUnits(144).multiUnitName = "Wavelength"
End Sub

'-----------------------------------------------------------------------------
' Read the Energy+.ini file to see where the idd file is located
' only for special IDDs
'-----------------------------------------------------------------------------
Sub ReadINI()
Dim lineIn As String
Dim specialWithBrackets As String
Dim dotLocation As Integer
Dim p As String
Dim found As Boolean
On Error Resume Next
'find the Energy+.ini file by searching staring in the
'program directory and getting lower and lower in the directory
'structure.
'assume it is found
found = True
p = App.Path
Open p & "\energy+.ini" For Input As 1
If Err.Number > 0 Then
  p = upDirectory(p)
  Err.Clear
  Open p & "\energy+.ini" For Input As 1
  If Err.Number > 0 Then
    Err.Clear
    p = upDirectory(p)
    Open p & "\energy+.ini" For Input As 1
    If Err.Number > 0 Then
      p = upDirectory(p)
      Err.Clear
      Open p & "\energy+.ini" For Input As 1
      If Err.Number > 0 Then
        p = upDirectory(p)
        Err.Clear
        Open p & "\energy+.ini" For Input As 1
        If Err.Number > 0 Then
          found = False
          Err.Clear
        End If
      End If
    End If
  End If
End If
dotLocation = InStrRev(specialIDDName, ".")
specialWithBrackets = "[" & Trim(Left(specialIDDName, dotLocation - 1)) & "]"
'if it was found then
If found Then
  Do While Not EOF(1)
    Line Input #1, lineIn
    If Trim(UCase(lineIn)) = UCase(specialWithBrackets) Then
      Line Input #1, lineIn
      If UCase(Left(Trim(lineIn), 4)) = "DIR=" Then
        IDDFileName = Mid(Trim(lineIn), 5) & "\" & specialIDDName
        If InStr(IDDFileName, ":") = 0 Then
          If InStr(IDDFileName, "\\") = 0 Then
            'make sure leads with slash
            If Left(IDDFileName, 1) <> "\" Then
              IDDFileName = "\" & IDDFileName
            End If
            IDDFileName = p & IDDFileName
          End If
        End If
        useIDDhardPath = True
      Else
        IDDFileName = specialIDDName
      End If
      Exit Do
    End If
  Loop
  Close 1
End If
If IDDFileName = "" Then 'if the ini file was never found then just set it
  'App.Path should be in \EnergyPlusV-x-x-x\Preprocess\IDFEditor
  IDDFileName = upDirectory(App.Path) & "\GrndTempCalc\" & specialIDDName
  useIDDhardPath = True
End If
'MsgBox "IDD file name after energy+.ini searched: " & IDDFileName, vbInformation
End Sub

'-----------------------------------------------------------------------------
' Read the IDD file - usually this is the EnergyPlus.idd
'-----------------------------------------------------------------------------
Sub ReadIDD()
Dim t As String, p As String
Dim explptLoc As Long
Dim sqBrkStart As Long, sqBrkEnd As Long
Dim curLoc As Long, tLength As Long
Dim curChar As String, curCharAsc As Long
Dim afterSlash As String, slashKey As String, slashVal As String, spaceLoc As Long, colonLoc As Long
Dim lastWord As String
Dim invalidSlashCodesFound As Boolean
Dim invalidSlashCodeText As String
Dim curObjListName As String
Dim i As Long
Dim j As Long
Dim k As Long
Dim curStringToFind As String
Dim found As Long
'parseMode indicates the current mode of parsing
' = 1 not in class
' = 2 look for N and A
' = 3 found N
' = 4 found A
Dim parseMode As Integer
'Dim refListNumber As Integer, refPt As Integer, lastRefPt As Integer
Dim fndObjList As Long, lastObjListItem As Long, curObjListItem As Long
Dim curClassName As String

Dim lastFldPt As Long, fldPt As Long
Dim iddLineCount As Long
Dim extraObjListWarning As String
Dim classListWarning As String
Const NotInClass = 1
Const Look4NA = 2
Const FoundN = 3
Const FoundA = 4
On Error Resume Next    'this is the only way to trap for errors
parseMode = 1
extraObjListWarning = ""
'if a hard path is used for the idd just use it directly
'but this is usually only the case when a special idd file
'is specified
If useIDDhardPath Then
  Open IDDFileName For Input As 1
  If Err.Number > 0 Then
    MsgBox "Could not find using hard path " & IDDFileName, vbOKOnly, "IDD Error"
    End
  End If
  documentationPath = upDirectory(IDDFileName) & "\Documentation\"
  dataSetPath = upDirectory(IDDFileName) & "\DataSets\"
Else
  ' first find the energy+.idd file
  '  look first in the application directory.  If it doesn't find
  '  it there it will look up three directory levels
  p = App.Path
  Open p & "\" & IDDFileName For Input As 1
  If Err.Number > 0 Then
    p = upDirectory(p)
    Err.Clear
    Open p & "\" & IDDFileName For Input As 1
    If Err.Number > 0 Then
      Err.Clear
      p = upDirectory(p)
      Open p & "\" & IDDFileName For Input As 1
      If Err.Number > 0 Then
        p = upDirectory(p)
        Err.Clear
        Open p & "\" & IDDFileName For Input As 1
        If Err.Number > 0 Then
          p = upDirectory(p)
          Err.Clear
          Open p & "\" & IDDFileName For Input As 1
          If Err.Number > 0 Then
            MsgBox "Could not find " & IDDFileName, vbOKOnly, "IDD Error"
            End
          End If
        End If
      End If
    End If
  End If
  documentationPath = p & "\Documentation\"
  dataSetPath = p & "\DataSets\"
End If
'On Error Resume Next
'Open "c:\temp\IDDparse.tmp" For Output As 2
'If Err.Number > 0 Then
'  Open "IDDparse.tmp" For Output As 2
'  If Err.Number > 0 Then
'    MsgBox "Cannot create temporary IDDparse.tmp file", vbExclamation, "IDD Error"
'    End
'  End If
'  Err.Clear
'End If
On Error GoTo 0 'start up normal error handling
parseMode = NotInClass
invalidSlashCodesFound = False
invalidSlashCodeText = ""
iddLineCount = 0
Do While Not EOF(1)
  iddLineCount = iddLineCount + 1
  Line Input #1, t
  'Debug.Print "((("; t; ")))"
  'get rid of comments starting with explanation point
  If Left(t, 12) = "!IDD_Version" Then
    IDDVersion = Mid(t, 14)
  End If
  explptLoc = InStr(1, t, "!")
  If explptLoc > 1 Then
    t = Left$(t, explptLoc - 1) 'trucate to characters left of !
  ElseIf explptLoc = 1 Then
    t = ""
  End If
  'get rid of comments in square brackets
  Do  'need to loop because some lines have multiple bracketed comments
    sqBrkStart = InStr(1, t, "[")
    sqBrkEnd = InStr(1, t, "]")
    If sqBrkStart > 0 And sqBrkEnd > 0 And sqBrkEnd > sqBrkStart Then
      If sqBrkEnd < Len(t) Then
        t = Left(t, sqBrkStart - 1) & Mid(t, sqBrkEnd + 1)
      Else  'last character on line is ]
        t = Left(t, sqBrkStart - 1)
      End If
    Else
      Exit Do
    End If
  Loop
  t = LTrim(RTrim(t)) 'get rid of leading and trailing spaces
  ' The following section does the majority of the
  ' parsing task.  Previous sections just got rid of
  ' unparsable items from the file
  If t <> "" Then  'if not empty
 '   Print #2, "{"; t; "}" 'output to IDDparse.tmp
    tLength = Len(t)
    curLoc = 1
    lastWord = ""
    Do While curLoc <= tLength
      curChar = Mid$(t, curLoc, 1)
      curCharAsc = Asc(curChar)
      'Debug.Print curChar; "-"; parseMode; "-"; curCharAsc
      Select Case curCharAsc
        Case 44 ' , comma
          Select Case parseMode
            Case NotInClass
              Call resizeClassDatArray(1)
              maxUsedIDDClass = maxUsedIDDClass + 1
              curClassName = Trim(lastWord)
              IDDClassDat(maxUsedIDDClass).name = curClassName
              'special filtering for handling certain schedule objects that provide multi-unit field support
              Select Case UCase(curClassName)
                Case "SCHEDULETYPELIMITS"
                  classScheduleTypeLimits = maxUsedIDDClass
                  IDDClassDat(maxUsedIDDClass).specialMultiUnit = True
                Case "SCHEDULE:DAY:HOURLY"
                  classScheduleDayHourly = maxUsedIDDClass
                  IDDClassDat(maxUsedIDDClass).specialMultiUnit = True
                Case "SCHEDULE:DAY:INTERVAL"
                  classScheduleDayInterval = maxUsedIDDClass
                  IDDClassDat(maxUsedIDDClass).specialMultiUnit = True
                Case "SCHEDULE:DAY:LIST"
                  classScheduleDayList = maxUsedIDDClass
                  IDDClassDat(maxUsedIDDClass).specialMultiUnit = True
                Case "SCHEDULE:COMPACT"
                  classScheduleCompact = maxUsedIDDClass
                  IDDClassDat(maxUsedIDDClass).specialMultiUnit = True
                Case "SCHEDULE:CONSTANT"
                  classScheduleConstant = maxUsedIDDClass
                  IDDClassDat(maxUsedIDDClass).specialMultiUnit = True
                Case Else
                  IDDClassDat(maxUsedIDDClass).specialMultiUnit = False
              End Select
              lastWord = ""
              parseMode = Look4NA
            Case Look4NA
              errDisplay "No N or A found before , in line:" & t, "IDD Parsing ERROR"
            Case FoundN
              'process numeric
              Call resizeFieldArray(1)
              maxUsedField = maxUsedField + 1
              IDDClassDat(maxUsedIDDClass).fieldEnd = maxUsedField
              If IDDClassDat(maxUsedIDDClass).fieldStart = 0 Then
                IDDClassDat(maxUsedIDDClass).fieldStart = maxUsedField
              End If
              IDDField(maxUsedField).AN = fieldN
              IDDField(maxUsedField).id = lastWord
              If IDDField(maxUsedField).name = "" Then
                IDDField(maxUsedField).name = lastWord
              End If
              lastWord = ""
              parseMode = Look4NA
            Case FoundA
              'process alpha
              Call resizeFieldArray(1)
              maxUsedField = maxUsedField + 1
              IDDClassDat(maxUsedIDDClass).fieldEnd = maxUsedField
              If IDDClassDat(maxUsedIDDClass).fieldStart = 0 Then
                IDDClassDat(maxUsedIDDClass).fieldStart = maxUsedField
              End If
              IDDField(maxUsedField).AN = fieldA
              IDDField(maxUsedField).id = lastWord
              IDDField(maxUsedField).type = 3  'set the default value to alpha
              If IDDField(maxUsedField).name = "" Then
                IDDField(maxUsedField).name = lastWord
              End If
              lastWord = ""
              parseMode = Look4NA
          End Select
        Case 59 ' ; semicolon
          Select Case parseMode
            Case NotInClass 'this must be a "section"
              'do something with section????
              lastWord = ""
            Case Look4NA
              errDisplay "No N or A found before ; in line:" & t, "IDD Parsing ERROR"
            Case FoundN
              'process numeric
              Call resizeFieldArray(1)
              maxUsedField = maxUsedField + 1
              IDDClassDat(maxUsedIDDClass).fieldEnd = maxUsedField
              If IDDClassDat(maxUsedIDDClass).fieldStart = 0 Then
                IDDClassDat(maxUsedIDDClass).fieldStart = maxUsedField
              End If
              IDDField(maxUsedField).AN = fieldN
              IDDField(maxUsedField).id = lastWord
              If IDDField(maxUsedField).name = "" Then
                IDDField(maxUsedField).name = lastWord
              End If
              lastWord = ""
              parseMode = NotInClass
            Case FoundA
              'process alpha
              Call resizeFieldArray(1)
              maxUsedField = maxUsedField + 1
              IDDClassDat(maxUsedIDDClass).fieldEnd = maxUsedField
              If IDDClassDat(maxUsedIDDClass).fieldStart = 0 Then
                IDDClassDat(maxUsedIDDClass).fieldStart = maxUsedField
              End If
              IDDField(maxUsedField).AN = fieldA
              IDDField(maxUsedField).id = lastWord
              IDDField(maxUsedField).type = 3  'set the default value to alpha
              If IDDField(maxUsedField).name = "" Then
                IDDField(maxUsedField).name = lastWord
              End If
              lastWord = ""
              parseMode = NotInClass
          End Select
        Case 65, 97 ' A, a
          Select Case parseMode
            Case Look4NA
              parseMode = FoundA
            Case FoundA, FoundN
              errDisplay "Missing comma in previous line:" & t & " as part of " & IDDClassDat(maxUsedIDDClass).name, "IDD Parsing ERROR"
          End Select
          lastWord = lastWord & curChar
        Case 78, 110 ' N, n
          Select Case parseMode
            Case Look4NA
              parseMode = FoundN
            Case FoundA, FoundN
              errDisplay "Missing comma in previous line:" & t & " as part of " & IDDClassDat(maxUsedIDDClass).name, "IDD Parsing ERROR"
          End Select
          lastWord = lastWord & curChar
        Case 92 ' \ slash extra field information
          If curLoc <> tLength Then 'if not last character on line
            afterSlash = RTrim(LTrim(Mid(t, curLoc + 1)))
            spaceLoc = InStr(afterSlash, " ")
            ' kludge to support colon as delimiter
            colonLoc = InStr(afterSlash, ":")
            If (colonLoc > 0 And spaceLoc > 0 And colonLoc < spaceLoc) Or (spaceLoc = 0 And colonLoc > 0) Then spaceLoc = colonLoc
            If spaceLoc > 0 Then
              slashKey = UCase(Left(afterSlash, spaceLoc - 1)) 'make upper case to make select easier
              slashVal = Trim(Mid(afterSlash, spaceLoc + 1))
            Else
              slashKey = UCase(afterSlash)
            End If
            slashKey = RemoveTrailingTabs(slashKey)
            Select Case slashKey
              Case "FIELD"
                IDDField(maxUsedField).name = slashVal
                IDDField(maxUsedField).autosizable = False  'default for all fields is not autosizable
                IDDField(maxUsedField).autocalculatable = False
                IDDField(maxUsedField).deprecated = False 'default is not deprecated
              Case "UNITS"
                IDDField(maxUsedField).Units = slashVal
              Case "KEY"
                Call resizeChoiceArray(1)
                maxUsedChoice = maxUsedChoice + 1
                IDDField(maxUsedField).choiceEnd = maxUsedChoice
                If IDDField(maxUsedField).choiceStart = 0 Then
                  IDDField(maxUsedField).choiceStart = maxUsedChoice
                End If
                IDDChoice(maxUsedChoice) = slashVal
              Case "MINIMUM"
                IDDField(maxUsedField).minimum = Val(slashVal)
                IDDField(maxUsedField).exclusiveMin = False
                IDDField(maxUsedField).minSpecified = True
              Case "MINIMUM>"
                IDDField(maxUsedField).minimum = Val(slashVal)
                IDDField(maxUsedField).exclusiveMin = True
                IDDField(maxUsedField).minSpecified = True
              Case "MAXIMUM"
                IDDField(maxUsedField).maximum = Val(slashVal)
                IDDField(maxUsedField).exclusiveMax = False
                IDDField(maxUsedField).maxSpecified = True
              Case "MAXIMUM<"
                IDDField(maxUsedField).maximum = Val(slashVal)
                IDDField(maxUsedField).exclusiveMax = True
                IDDField(maxUsedField).maxSpecified = True
              Case "DEFAULT"
                If IsNumeric(slashVal) Then
                  IDDField(maxUsedField).defaultValue = Val(slashVal)
                ElseIf UCase(slashVal) = "AUTOSIZE" Then
                  IDDField(maxUsedField).defaultAutosize = True
                ElseIf UCase(slashVal) = "AUTOCALCULATE" Then
                  IDDField(maxUsedField).defaultAutoCalc = True
                Else
                  Call resizeChoiceArray(1)
                  maxUsedChoice = maxUsedChoice + 1
                  IDDField(maxUsedField).choiceEnd = maxUsedChoice
                  If IDDField(maxUsedField).choiceStart = 0 Then
                    IDDField(maxUsedField).choiceStart = maxUsedChoice
                  End If
                  IDDChoice(maxUsedChoice) = slashVal
                  IDDField(maxUsedField).defaultChoice = maxUsedChoice
                End If
                IDDField(maxUsedField).defSpecified = True
              Case "NOTE"
                IDDField(maxUsedField).note = IDDField(maxUsedField).note & " " & slashVal
              Case "TYPE"
                slashVal = RemoveTrailingTabs(slashVal)
                Select Case UCase(slashVal)
                  Case "REAL"
                    IDDField(maxUsedField).type = 1
                  Case "INTEGER"
                    IDDField(maxUsedField).type = 2
                  Case "ALPHA"
                    IDDField(maxUsedField).type = 3
                  Case "CHOICE"
                    IDDField(maxUsedField).type = 4
                  Case "OBJECT-LIST", "EXTERNAL-LIST"
                    IDDField(maxUsedField).type = 5
                  Case "NODE"
                    IDDField(maxUsedField).type = 6
                  Case Else
                    errDisplay "Invalid \type in line: " & t & vbCrLf & "   line number: " & Str(iddLineCount), "IDD Parsing ERROR"
                End Select
              Case "GROUP"  'this is a command to group classes
                Call resizeClassGroupArray(1)
                maxUsedClassGroup = maxUsedClassGroup + 1
                IDDClassGroup(maxUsedClassGroup).name = slashVal
                IDDClassGroup(maxUsedClassGroup).classStart = maxUsedIDDClass + 1
                'Debug.Print "CLASS GROUP:"; slashVal
              Case "MEMO" 'this is a comment for an entire object
                If IDDClassDat(maxUsedIDDClass).memo = "" Then
                  IDDClassDat(maxUsedIDDClass).memo = slashVal 'if empty don't bother with vbcrlf
                Else
                  IDDClassDat(maxUsedIDDClass).memo = IDDClassDat(maxUsedIDDClass).memo & " " & vbCrLf & slashVal
                End If
              Case "OBJECT-LIST", "EXTERNAL-LIST" 'this is a type 5, pointer to defining a new list of referencable items
                'note that \external-list is just a synonym for \object-list at this point but at some future time
                'it would probably make sense to handle them separately
                fndObjList = 0
                curObjListName = UCase(slashVal)
                'check if the list is a special code word for lists pulled automatically out of the
                'RDD file to show the names of variables and meters available
                IDDField(maxUsedField).autoObjList = 0  'the default is not an automatic list
                If curObjListName = "AUTORDDVARIABLE" Then IDDField(maxUsedField).autoObjList = autoObjListKindVar
                If curObjListName = "AUTORDDMETER" Then IDDField(maxUsedField).autoObjList = autoObjListKindMeter
                If curObjListName = "AUTORDDVARIABLEMETER" Then IDDField(maxUsedField).autoObjList = autoObjListKindVarMeter
                For i = 1 To maxUsedObjListName
                  If curObjListName = IDDObjListName(i).name Then 'found existing object list
                    fndObjList = i
                    Exit For
                  End If
                Next i
                If fndObjList > 0 Then 'use existing list
                  'add to the list of object lists for the field
                  Call resizeListOfObjListArray(1)
                  maxListOfObjList = maxListOfObjList + 1
                  If IDDField(maxUsedField).listOfObjListStart = 0 Then
                    IDDField(maxUsedField).listOfObjListStart = maxListOfObjList
                  End If
                  IDDField(maxUsedField).listOfObjListEnd = maxListOfObjList
                  ListOfObjList(maxListOfObjList) = fndObjList
                  'Debug.Print "OL Existing Object List Name", IDDObjListName(fndObjList).name; " from "; iddclassdat(maxUsedIDDClass).name
                  IDDObjListName(fndObjList).usedInObjectList = True
                Else  'must be a new object list name
                  ' create a new obj-list name
                  Call resizeObjListNameArray(1)
                  maxUsedObjListName = maxUsedObjListName + 1
                  IDDObjListName(maxUsedObjListName).name = curObjListName
                  ' now add it to the list of object lists for the field
                  Call resizeListOfObjListArray(1)
                  maxListOfObjList = maxListOfObjList + 1
                  If IDDField(maxUsedField).listOfObjListStart = 0 Then
                    IDDField(maxUsedField).listOfObjListStart = maxListOfObjList
                  End If
                  IDDField(maxUsedField).listOfObjListEnd = maxListOfObjList
                  ListOfObjList(maxListOfObjList) = maxUsedObjListName
                  ' create new obj-list item
                  IDDObjListName(maxUsedObjListName).objListItemStart = 0 'set to zero to show that none exist
                  'Debug.Print "OL New Object List Defined", IDDObjListName(maxUsedObjListName).name; " from "; IDDClassDat(maxUsedIDDClass).name
                  IDDObjListName(maxUsedObjListName).usedInObjectList = True
                End If
              Case "REFERENCE"
                fndObjList = 0
                lastObjListItem = 0
                For i = 1 To maxUsedObjListName 'look through to see if name for object-list has been used
                  If UCase(slashVal) = IDDObjListName(i).name Then 'found existing object list
                    fndObjList = i
                    Exit For
                  End If
                Next i
                If fndObjList > 0 Then 'use existing list
                  'since it has been used go through list of references until the end
                  curObjListItem = IDDObjListName(fndObjList).objListItemStart
                  Do While curObjListItem > 0 'scan through list of references and find next one
                    lastObjListItem = curObjListItem
                    curObjListItem = IDDObjListItem(lastObjListItem).nextObjListItem
                  Loop
                  ' create a new obj-list item
                  Call resizeObjListItemArray(1)
                  maxUsedObjListItem = maxUsedObjListItem + 1
                  'NOT SURE WHAT THE FOLLOWING LINE WAS FOR PRIOR TO NOV 2010 UPDATE
                  'IDDField(maxUsedField).objListIndex = maxUsedObjListName
                  IDDObjListItem(maxUsedObjListItem).classWithRef = maxUsedIDDClass
                  IDDObjListItem(maxUsedObjListItem).fieldWithRef = maxUsedField
                  If lastObjListItem > 0 Then 'will be positive unless Object-List occured before reference
                    IDDObjListItem(lastObjListItem).nextObjListItem = maxUsedObjListItem
                  Else  'even though it was found before it wasn't
                    IDDObjListName(fndObjList).objListItemStart = maxUsedObjListItem
                  End If
                  'Debug.Print "Ref Existing Object List Defined", IDDObjListName(fndObjList).name; " from "; iddclassdat(maxUsedIDDClass).name
                  IDDObjListName(fndObjList).usedInReference = True
                Else  'must be a new object list name
                  ' create a new obj-list name
                  Call resizeObjListNameArray(1)
                  maxUsedObjListName = maxUsedObjListName + 1
                  IDDObjListName(maxUsedObjListName).name = UCase(slashVal)
                  'NOT SURE WHAT THE FOLLOWING LINE WAS FOR PRIOR TO NOV 2010 UPDATE
                  'IDDField(maxUsedField).objListIndex = maxUsedObjListName
                  ' create a new obj-list item
                  Call resizeObjListItemArray(1)
                  maxUsedObjListItem = maxUsedObjListItem + 1
                  IDDObjListName(maxUsedObjListName).objListItemStart = maxUsedObjListItem
                  IDDObjListItem(maxUsedObjListItem).classWithRef = maxUsedIDDClass
                  IDDObjListItem(maxUsedObjListItem).fieldWithRef = maxUsedField
                  'Debug.Print "Ref New Object List Defined", IDDObjListName(maxUsedObjListName).name; " from "; IDDClassDat(maxUsedIDDClass).name
                  IDDObjListName(maxUsedObjListName).usedInReference = True
                End If
              Case "REFERENCE-CLASS-NAME"
                fndObjList = 0
                lastObjListItem = 0
                For i = 1 To maxUsedObjListName 'look through to see if name for object-list has been used
                  If UCase(slashVal) = IDDObjListName(i).name Then 'found existing object list
                    fndObjList = i
                    Exit For
                  End If
                Next i
                If fndObjList > 0 Then 'use existing list
                  'since it has been used go through list of references until the end
                  curObjListItem = IDDObjListName(fndObjList).objListItemStart
                  Do While curObjListItem > 0 'scan through list of references and find next one
                    lastObjListItem = curObjListItem
                    curObjListItem = IDDObjListItem(lastObjListItem).nextObjListItem
                  Loop
                  ' create a new obj-list item
                  Call resizeObjListItemArray(1)
                  maxUsedObjListItem = maxUsedObjListItem + 1
                  IDDObjListItem(maxUsedObjListItem).classWithRef = maxUsedIDDClass
                  IDDObjListItem(maxUsedObjListItem).fieldWithRef = fwrClassName
                  If lastObjListItem > 0 Then 'will be positive unless Object-List occured before reference
                    IDDObjListItem(lastObjListItem).nextObjListItem = maxUsedObjListItem
                  Else  'even though it was found before it wasn't
                    IDDObjListName(fndObjList).objListItemStart = maxUsedObjListItem
                  End If
                  'Debug.Print "Ref Existing Object List Defined", IDDObjListName(fndObjList).name; " from "; iddclassdat(maxUsedIDDClass).name
                  IDDObjListName(fndObjList).usedInReference = True
                Else  'must be a new object list name
                  ' create a new obj-list name
                  Call resizeObjListNameArray(1)
                  maxUsedObjListName = maxUsedObjListName + 1
                  IDDObjListName(maxUsedObjListName).name = UCase(slashVal)
                  ' create a new obj-list item
                  Call resizeObjListItemArray(1)
                  maxUsedObjListItem = maxUsedObjListItem + 1
                  IDDObjListName(maxUsedObjListName).objListItemStart = maxUsedObjListItem
                  IDDObjListItem(maxUsedObjListItem).classWithRef = maxUsedIDDClass
                  IDDObjListItem(maxUsedObjListItem).fieldWithRef = fwrClassName
                  Debug.Print "Ref New Object List Defined", IDDObjListName(maxUsedObjListName).name; " from "; IDDClassDat(maxUsedIDDClass).name
                  IDDObjListName(maxUsedObjListName).usedInReference = True
                End If
              Case "MIN-FIELDS"
                IDDClassDat(maxUsedIDDClass).minFields = Val(slashVal)
              Case "IP-UNITS"
                IDDField(maxUsedField).IPUnits = slashVal
              Case "FORMAT"
                Select Case UCase(slashVal)
                  Case "SINGLELINE"
                    IDDClassDat(maxUsedIDDClass).format = formatSingleLine
                  Case "VERTICES"
                    IDDClassDat(maxUsedIDDClass).format = formatVertices
                  Case "COMPACTSCHEDULE"
                    IDDClassDat(maxUsedIDDClass).format = formatCompactSch
                  Case "FLUIDPROPERTY"
                    IDDClassDat(maxUsedIDDClass).format = formatFluidProperty
                  Case "VIEWFACTOR"
                    IDDClassDat(maxUsedIDDClass).format = formatViewFactor
                  Case "SPECTRAL"
                    IDDClassDat(maxUsedIDDClass).format = formatSpectral
                End Select
              Case "DEPRECATED"
                IDDField(maxUsedField).deprecated = True
              Case "REQUIRED-FIELD"
                IDDField(maxUsedField).required = True
              Case "UNIQUE-OBJECT"
                IDDClassDat(maxUsedIDDClass).uniqueObject = True
              Case "REQUIRED-OBJECT"
              Case "AUTOSIZABLE" '\autosizable
                IDDField(maxUsedField).autosizable = True
              Case "AUTOCALCULATABLE"
                IDDField(maxUsedField).autocalculatable = True
              Case "PRESERVEINDENT"
                IDDField(maxUsedField).preserveIndent = True
              Case "UNITSBASEDONFIELD"
                IDDField(maxUsedField).unitsBasedOnFieldString = slashVal
              ' do nothing just for compatibility
              Case "EXTENSIBLE"  '\extensible
              Case "OBSOLETE"    '\obsolete
              Case "RETAINCASE"  '\retaincase
              Case "BEGIN-EXTENSIBLE" '\begin-extensible
              ' the following are used for the Simergy IDD
              Case "FIELD-INDEX" '\field-index
              Case "SEQUENCE-ID" '\sequence-id
              Case "SEQUENCE-INDEX" '\sequence-index
              Case "GROUP-INDEX" '\group-index
              Case "HIDEINLIBRARYUI" '\HideInLibraryUI
              Case "EMSUNIQUECOMPONENT" '\EmsUniqueComponent
              Case "HIDEINALLUI" '\HideInAllUI
              Case "SIMERGYONLY" '\SimergyOnly
              Case "NO-SEQUENCE" '\no-sequence
              Case "SURROGATE-NAME-FOR-SEQUENCE" '\surrogate-name-for-sequence
              Case Else
                invalidSlashCodesFound = True
                invalidSlashCodeText = invalidSlashCodeText & vbCrLf & t
            End Select
            curLoc = tLength 'ignore the rest of line and go to next line
          End If
        Case 48 To 57, 66 To 90, 98 To 122 '0-9, B-Z, b-z
          lastWord = lastWord & curChar   'append letter to last word
        Case 34 To 43, 45 To 47, 58, 60 To 64, 91, 93 To 96, 123 To 126 '"#$%&')(*+-./:<=>?@[]^_`{|}~
          lastWord = lastWord & curChar   'append letter to last word
        Case 32 'space
          If lastWord <> "" Then lastWord = lastWord & curChar 'append letter to last word
        Case 9 'tab
          'ignore
        Case Else
          errDisplay "Invalid character found in line:" & t & "ascii" & curCharAsc, "IDD Parsing ERROR"
      End Select
      curLoc = curLoc + 1
    Loop
  End If
Loop
Close 1
Close 2
For i = 1 To maxUsedField
  ' go through the fields and make sure that if it is a N field
  ' that the type is set to something and if it isn't then set it
  ' to real (type=1)
  If IDDField(i).AN = fieldN Then
    If IDDField(i).type = 0 Then IDDField(i).type = 1
  End If
  'go through the fields and make sure that the object list
  'slash code is set to the correct type
  If IDDField(i).listOfObjListStart > 0 Then
    If IDDField(maxUsedField).type = 0 Then
      IDDField(maxUsedField).type = 5
    End If
  End If
  'trim trailing spaces from ID
  IDDField(i).id = Trim(IDDField(i).id)
Next i
'check if a list is used by \object-list but not by \reference
classListWarning = ""
For i = 1 To maxUsedObjListName
  ' THE ORIGINAL CHECK:
  ' look through the object list referneces and make sure that
  ' each one is used in an \object-list and in either a \reference
  ' or a \reference-class-name
  'If Not (IDDObjListName(i).usedInObjectList And IDDObjListName(i).usedInReference) Then
  '  If IDDObjListName(i).name <> "AUTORDDVARIABLE" And IDDObjListName(i).name <> "AUTORDDMETER" And IDDObjListName(i).name <> "AUTORDDVARIABLEMETER" Then
  '    classListWarning = classListWarning & vbCrLf & IDDObjListName(i).name
  '  End If
  'End If
  If IDDObjListName(i).usedInObjectList And Not IDDObjListName(i).usedInReference Then
    If IDDObjListName(i).name <> "AUTORDDVARIABLE" And IDDObjListName(i).name <> "AUTORDDMETER" And IDDObjListName(i).name <> "AUTORDDVARIABLEMETER" Then
      classListWarning = classListWarning & vbCrLf & IDDObjListName(i).name
    End If
  End If
Next i
If classListWarning <> "" Then
  MsgBox "The following lists appear in the IDD for an \object-list but not in a \reference or \reference-class-name:" & vbCrLf & classListWarning, vbOKOnly, "Unmatched Object Lists Found"
End If



' if no groups have been defined add a new one called "default"
If maxUsedClassGroup = 0 Then
  Call resizeClassGroupArray(1)
  maxUsedClassGroup = 1
  IDDClassGroup(maxUsedClassGroup).name = "default"
  IDDClassGroup(maxUsedClassGroup).classStart = 1
End If
If invalidSlashCodesFound Then
  MsgBox "The following invalid slash codes were found during parsing the Energy+.IDD file:" & vbCrLf & vbCrLf & invalidSlashCodeText, vbOKOnly, "Invalid Slash Codes Found"
End If
'the following line just completes the last pointer to the end of the class array
Call resizeClassGroupArray(2)
IDDClassGroup(maxUsedClassGroup + 1).classStart = maxUsedIDDClass + 1
'Stop
Call associateUnits
Call checkMinFields
If extraObjListWarning <> "" Then
  MsgBox "The following fields have more than one \object-list slash codes. Only one is allowed per field." & vbCrLf & vbCrLf & extraObjListWarning, vbInformation, "Reading IDD"
End If
End Sub


'-----------------------------------------------------------------------------
' Scan though the items with units and associate the index for the conversion
'-----------------------------------------------------------------------------
Sub associateUnits()
Dim fldUnits As String, found As Long, errMessage As String, foundKey As Long
Dim i As Long, j As Long, k As Long, l As Long, m As Long
Dim curStringToFind As String
Dim curBasedOn As Long
errMessage = ""
For i = 1 To maxUsedField
  If IDDField(i).AN = fieldN Then
    IDDField(i).unitsIndex = 0
    If IDDField(i).IPUnits = "" Then 'no IP units specified - the most common case
      fldUnits = LCase(IDDField(i).Units)
      If fldUnits <> "" Then 'some SI units specified
        found = 0
        For j = 1 To maxUsedConvUnits
          If LCase(convUnits(j).siName) = fldUnits Then
            found = j
            Exit For
          End If
        Next j
        If found > 0 Then
          IDDField(i).unitsIndex = found
        Else
          Debug.Print "Unit not defined: "; IDDField(i).Units; " from field "; IDDField(i).name
        End If
      Else
        IDDField(i).unitsIndex = noUnitsSpecified 'flag value - no units specified so no conversion
      End If
    Else  'IP units are specified
      If IDDField(i).Units <> "" Then 'both IP and SI specified - this should always be the case
        fldUnits = LCase(IDDField(i).Units) & ">>>" & LCase(IDDField(i).IPUnits)  'make combined search string
        found = 0
        For j = 1 To maxUsedConvUnits
          If LCase(convUnits(j).siName) & ">>>" & LCase(convUnits(j).ipName) = fldUnits Then
            found = j
            Exit For
          End If
        Next j
        If found > 0 Then
          IDDField(i).unitsIndex = found
        Else
          Debug.Print "IP Unit not defined: "; IDDField(i).IPUnits; " from field "; IDDField(i).name
        End If
      Else 'IP units specified but no SI specified - should not happend but if it does check through list
        'since few IP units are specified that are not automatically converted and they
        'are all unique, just search through the list of IP units and then check the si unit
        'to make sure it is consistent
        fldUnits = LCase(IDDField(i).IPUnits)
        found = 0
        For j = 1 To maxUsedConvUnits
          If LCase(convUnits(j).ipName) = fldUnits Then
            found = j
            Exit For
          End If
        Next j
        If found > 0 Then
          IDDField(i).unitsIndex = found
        Else
          Debug.Print "IP Unit not defined: "; IDDField(i).IPUnits; " from field "; IDDField(i).name
        End If
      End If
    End If
    If IDDField(i).unitsIndex = 0 Then
      errMessage = errMessage & vbCrLf & IDDField(i).name & ":  " & IDDField(i).Units & " into " & IDDField(i).IPUnits
      IDDField(i).unitsIndex = noUnitsSpecified 'flag value - no units specified so no conversion
    End If
  Else ' an alpha field has no units
    IDDField(i).unitsIndex = noUnitsSpecified 'flag value - no units specified so no conversion
  End If
Next i
' for unit conversions that depend on other fields
'associate an index with each UnitsBasedOnField slashcode found
For j = 1 To maxUsedIDDClass
  For i = IDDClassDat(j).fieldStart To IDDClassDat(j).fieldEnd
    If IDDField(i).unitsBasedOnFieldString <> "" Then
      'search through field in selected object to match the
      curStringToFind = UCase(IDDField(i).unitsBasedOnFieldString)
      found = 0
      For k = IDDClassDat(j).fieldStart To IDDClassDat(j).fieldEnd
        If UCase(IDDField(k).id) = curStringToFind Then
          found = k
          Exit For
        End If
      Next k
      If found > 0 Then
        IDDField(i).unitsBasedOnFieldIndex = found
        IDDField(i).unitsIndex = unitsVaryByObject
        IDDField(found).usedForUnitsBasedOn = True
      End If
    End If
  Next i
Next j
'now check the keys in that filed and make sure they make sense
For j = 1 To maxUsedIDDClass
  For i = IDDClassDat(j).fieldStart To IDDClassDat(j).fieldEnd
    If IDDField(i).usedForUnitsBasedOn Then
      For l = IDDField(i).choiceStart To IDDField(i).choiceEnd
        fldUnits = LCase(IDDChoice(l))
        foundKey = 0
        For m = 1 To maxUsedConvUnits
          If LCase(convUnits(m).multiUnitName) = fldUnits Then
            foundKey = m
            Exit For
          End If
        Next m
        If foundKey = 0 Then
          errMessage = errMessage & vbCrLf & "Key:" & IDDChoice(l) & " invalid unit in field: " & IDDField(i).name & " in object: " & IDDClassDat(j).name
        End If
      Next l
    End If
  Next i
Next j
If errMessage <> "" Then
  Call MsgBox("Unit conversions not understood: " & vbCrLf & vbCrLf & errMessage, vbInformation, "Warning")
End If
End Sub


'-----------------------------------------------------------------------------
' Make sure the IDD \min-fields is less than or equal to the number of
' fields in each object
'-----------------------------------------------------------------------------
Sub checkMinFields()
Dim numfields As Integer
Dim errMessage As String
Dim i As Integer
errMessage = ""
For i = 1 To maxUsedIDDClass
    numfields = 1 + IDDClassDat(i).fieldEnd - IDDClassDat(i).fieldStart
    If IDDClassDat(i).minFields > numfields Then
        errMessage = errMessage & vbCrLf & IDDClassDat(i).name
    End If
Next i
If errMessage <> "" Then
    Call MsgBox("\min-fields is greater than the number of fields in: " & vbCrLf & errMessage, vbInformation, "Warning")
End If
End Sub


'-----------------------------------------------------------------------------
' Removes the last slash and what ever is right of it
' This effectively is like moving up one directory for a string
' that does not end in a slash.  If it does end in a slash it simply
' removes it.  If no slash is found than it returns the original string.
'-----------------------------------------------------------------------------
Function upDirectory(inpath As String) As String
Dim p As String
Dim i As Long, lastSlash As Long
p = inpath
For i = 1 To Len(p)
  If Mid(p, i, 1) = "\" Then lastSlash = i
Next i
If lastSlash > 2 Then
  upDirectory = Left(p, lastSlash - 1)
Else
  upDirectory = p
End If
End Function


'-----------------------------------------------------------------------------
'This displays errors in a pop up dialog box
'-----------------------------------------------------------------------------
Sub errDisplay(msg As String, section As String)
errCnt = errCnt + 1
If errCnt > 10 Then End
MsgBox msg, vbQuestion, section
End Sub

'-----------------------------------------------------------------------------
' Converts to international string representation
' of the number for example if the string says
' "1.1" than it returns "1,1"
'
' Note: internally all values are represented as
' u.s. numbers in strings but for display only
' they are converted to international format.
'
' This routine also makes numbers that are the correct width for the display in
' the column. The numDigitsShown is altered when the column width is altered.
' If a string conversion of the number is longer than that it is converted to
' scientific notation with (numberDigistsShown - 6) of zeros right of the decimal.

'-----------------------------------------------------------------------------
Function toIntl(numString As String, showNumDigits As Long) As String
Dim valNum As Double
Dim nDigits As Long
' original routine
'   toIntl = CStr(Val(numString))
' end of original routine
'On Error Resume Next
nDigits = showNumDigits
If nDigits < 7 Then nDigits = 7 'this protects against initialization problem when called with zero
valNum = Val(numString)
If Len(CStr(valNum)) > nDigits Then
  toIntl = format(valNum, "0." & String(nDigits - 6, "0") & "E+00")
  If Err.Number <> 0 Then toIntl = CStr(valNum)
Else
  toIntl = CStr(valNum)
End If
'Debug.Print "tointl", numString, valNum, toIntl, Len(CStr(valNum))
End Function

'-----------------------------------------------------------------------------
' Converts from the international string representation
' of a number to the u.s. number system for example
' "1,1" returns "1.1"
'
' Note: internally all values are represented as
' u.s. numbers in strings but for display only
' they are converted to international format.
'-----------------------------------------------------------------------------
Function fromIntl(numString As String) As String
Dim bareString As String
On Error Resume Next
If numString = "" Then bareString = "0"
bareString = stripNonNumeric(numString)
fromIntl = Str(CDbl(bareString))
End Function

'=======================================================
' Start Adobe Acrobat Reader With a File
'=======================================================
Sub startAcrobat(s)
Dim shellString As String
On Error Resume Next
Debug.Print acrobatReaderFileName
Debug.Print App.Path
shellString = ShortName(acrobatReaderFileName) & " " & ShortName(documentationPath & s)
Shell shellString, vbNormalFocus
If Err.Number <> 0 Then
  MsgBox "Error initiating acrobat to view documentation file", vbExclamation, "Help Cancelled"
  Exit Sub
End If
End Sub

'=======================================================
' Find the acrobat reader program
'=======================================================
Sub findAcrobat()
Dim arFN As String
Debug.Print "DocumentationPath["; documentationPath; "]"
arFN = String(250, 0)
Call FindExecutable(documentationPath & "InputOutputReference.pdf", vbNullString, arFN)
acrobatReaderFileName = Left(arFN, InStr(arFN, vbNullChar) - 1) 'clean up
Debug.Print "Acrobat["; acrobatReaderFileName; "]"
End Sub


'=======================================================
' Find the web browser program
'=======================================================
Sub findWebBrowser()
Dim arFN As String
Debug.Print "DocumentationPath["; documentationPath; "]"
arFN = String(250, 0)
Call FindExecutable(documentationPath & "index.html", vbNullString, arFN)
htmlViewFileName = Left(arFN, InStr(arFN, vbNullChar) - 1) 'clean up
Debug.Print "Webbrowser["; htmlViewFileName; "]"
End Sub


'-----------------------------------------------------------------------------
' Parse a Line with Comma Separation using SPLIT
'-----------------------------------------------------------------------------
'Public Sub commaParse(inString As String, outString() As String, numItems As Integer)
'Dim j As Integer
'Dim interString(0 To 1000) As String
'interString = Split(inString, ",", -1)
'numItems = UBound(outString) + 1
''shift array up so it is 1 based
'For j = numItems To 1 Step -1
'  outString(j) = interString(j - 1)
'Next j
'End Sub

'-----------------------------------------------------------------------------
' Parse a Line with Comma Separation
'-----------------------------------------------------------------------------
Public Sub commaParse(i$, o$(), n As Long)
Dim m(100) As Long, ci As Long, cio As Long, c As Long, wl As Long
Dim mi As Long, delim As String, l As Long, flg As Long
Dim cpi As String
n = 0
ci = 0  'character index
cio = 0 'old character index
mi = 0   'comma index
delim$ = ","
l = Len(i$)
flg = 0
Erase o$
'                              locate commas
While flg = 0
  If cio + 1 <= l Then
    ci = InStr(cio + 1, i$, delim$)
'                PRINT ci
    If ci > 0 Then
      mi = mi + 1
      m(mi) = ci
      cio = ci
'                ELSEIF cio > 0 THEN
 '                       mi = mi + 1
  '                      flg = 1
    Else
      flg = 1
    End If
  Else
    flg = 1
  End If
Wend
If mi > 0 Then
'               parse the first field
  If m(1) = 1 Then
    o$(1) = ""
  Else
    o$(1) = Left$(i$, m(1) - 1)
  End If
'               parse the last field
  If m(mi) = l Then
    o$(mi + 1) = ""
  Else
    o$(mi + 1) = Mid$(i$, m(mi) + 1)
  End If
'               parse middle fields
  If mi > 2 Then
    For c = 2 To mi
      wl = m(c) - m(c - 1) - 1
      If wl > 0 Then
        o$(c) = Mid$(i$, m(c - 1) + 1, wl)
      Else
        o$(c) = ""
      End If
    Next c
  End If
  n = mi + 1
ElseIf l > 0 Then
  o$(1) = i$
  n = 1
End If
For c = 1 To n
  o(c) = Trim(o(c))
Next c
End Sub


'-----------------------------------------------------------------------------
' Take all characters that aren't numeric and removes them from the string
'-----------------------------------------------------------------------------
Function stripNonNumeric(inString As String) As String
Dim curChar As String
Dim newString As String
Dim i As Long
For i = 1 To Len(inString)
  curChar = Mid(inString, i, 1)
  Select Case Asc(curChar)
    Case 48 To 57
      newString = newString & curChar
    Case 69  ' E
      newString = newString & curChar
    Case 46  ' .
      newString = newString & curChar
    Case 44  ' ,
      newString = newString & curChar
    Case 43  ' +
      newString = newString & curChar
    Case 45  ' -
      newString = newString & curChar
  End Select
Next i
stripNonNumeric = newString
End Function

'=======================================================
' Returns only the file name and extension and strips out
' the path. A file with path should be provided.
'=======================================================
Function fileWithExt(pathWithFile As String) As String
Dim slpt As Integer
slpt = InStrRev(pathWithFile, "\") 'finds last slash
If slpt > 1 Then
  fileWithExt = Mid(pathWithFile, slpt + 1)
Else
  fileWithExt = ""
End If
End Function

'=======================================================
' Returns only the path for the file and path provided
'=======================================================
Function pathOnly(pathWithFile As String) As String
Dim slpt As Integer
slpt = InStrRev(pathWithFile, "\") 'finds last slash
If slpt > 1 Then
  pathOnly = Left(pathWithFile, slpt)
Else
  pathOnly = "c:\"
End If
End Function

'=======================================================
' Returns the number of leading spaces
'=======================================================
Function numberOfLeadingSpaces(inString As String) As Integer
Dim i As Integer
numberOfLeadingSpaces = 0
For i = 1 To Len(inString)
  If Mid(inString, i, 1) <> " " Then
    numberOfLeadingSpaces = i - 1
    Exit For
  End If
Next i
End Function

'-----------------------------------------------------------------------------
' CEPTChange: Added Find Index from ItemData for a listbox or combobox
'-----------------------------------------------------------------------------
Public Function ItemDataToIndex(lo As Object, ItemData As Long, Optional Start As Long = 0, Optional Finish As Long = -1) As Long

On Error Resume Next

    Dim i As Integer
    Dim lCount As Long
    Dim iStep As Integer

    ItemDataToIndex = -1

    If Not (TypeOf lo Is ListBox Or TypeOf lo Is ComboBox) Then
        ItemDataToIndex = -1
        Exit Function
    End If

    lCount = lo.ListCount

    If lCount = 0 Then
        ItemDataToIndex = -1
        Exit Function
    End If

    iStep = 1

    If Start >= lCount Then
        If Finish < Start And Finish <= lCount Then
            iStep = -1
        Else
            ItemDataToIndex = -1
            Exit Function
        End If
    Else
        If Finish = -1 Then Finish = lCount
        'If Finish < Start < lCount Then iStep = -1
        If Finish < Start Then iStep = -1 Else iStep = 1
    End If

    For i = Start To Finish Step iStep
        If lo.ItemData(i) = ItemData Then
            ItemDataToIndex = i
            Exit Function
        End If
    Next i
End Function


'-----------------------------------------------------------------------------
' CEPTChange: Added To add Autocomplete feature to comboboxes in IDFEdit form
'-----------------------------------------------------------------------------
Public Function AutoCompleteCombo(ByRef cbBox As ComboBox, ByVal KeyAscii As Integer) As Integer


    Dim strFindThis As String, bContinueSearch As Boolean
    Dim lResult As Long, lStart As Long, lLength As Long
    AutoCompleteCombo = 0 ' block cbBox since we handle everything
    bContinueSearch = True
    lStart = cbBox.SelStart
    lLength = cbBox.SelLength

    On Error GoTo ErrHandle

    If KeyAscii < 32 Then 'control char
        bContinueSearch = False
        cbBox.SelLength = 0 'select nothing since we will delete/enter
        If KeyAscii = Asc(vbBack) Then 'take care BackSpace and Delete first
            If lLength = 0 Then 'delete last char
                If Len(cbBox) > 0 Then ' in case user delete empty cbBox
                    cbBox.Text = Left(cbBox.Text, Len(cbBox) - 1)
                End If
            Else 'leave unselected char(s) and delete rest of text
                cbBox.Text = Left(cbBox.Text, lStart)
            End If
            cbBox.SelStart = Len(cbBox) 'set insertion position @ the end of string
        ElseIf KeyAscii = vbKeyReturn Then  'user select this string
            cbBox.SelStart = Len(cbBox)
            lResult = SendMessage(cbBox.hwnd, CB_SELECTSTRING, -1, ByVal cbBox.Text)
            AutoCompleteCombo = KeyAscii 'let caller a chance to handle "Enter"
        ElseIf KeyAscii = vbKeyEscape Then ' if Esc pressed clear text in combobox (Req. by Rajan)
            cbBox.Text = ""
            AutoCompleteCombo = KeyAscii
        End If
    Else 'generate searching string
        If lLength = 0 Then
            strFindThis = cbBox.Text & Chr(KeyAscii) 'No selection, append it
        Else
            strFindThis = Left(cbBox.Text, lStart) & Chr(KeyAscii)
        End If
    End If

    If bContinueSearch Then 'need to search
        Call AutoDropDownComboBox(cbBox)  'open dropdown list
        lResult = SendMessage(cbBox.hwnd, CB_SELECTSTRING, -1, ByVal strFindThis)
        If lResult = CB_ERR Then 'not found
            cbBox.Text = strFindThis 'set cbBox as whatever it is
            cbBox.SelLength = 0 'no selected char(s) since not found
            cbBox.SelStart = Len(cbBox) 'set insertion position @ the end of string
        Else
            'found string, highlight rest of string for user
            cbBox.SelStart = Len(strFindThis)
            cbBox.SelLength = Len(cbBox) - cbBox.SelStart
        End If
    End If
    On Error GoTo 0
    Exit Function

ErrHandle:
    'got problem, simply return whatever pass in
    Debug.Print "Failed: AutoCompleteComboBox due to : " & Err.Description
    Debug.Assert False
    AutoCompleteCombo = KeyAscii
    On Error GoTo 0
End Function

'open dorpdown list
Public Sub AutoDropDownComboBox(ByRef cbBox As ComboBox)
    Call SendMessage(cbBox.hwnd, CB_SHOWDROPDOWN, Abs(True), 0)
End Sub

Function RemoveTrailingTabs(inString As String) As String
Dim i As Integer
Dim lastTab As Integer
Dim outString As String
outString = Trim(inString)
For i = Len(outString) To 1 Step -1
  If Mid(outString, i, 1) = vbTab Then
    Mid(outString, i, 1) = " "  'replace tabs with spaces
  Else
    Exit For
  End If
Next i
RemoveTrailingTabs = Trim(outString) 'remove spaces that were tabs
End Function

'=======================================================
' View a web page given the full URL as the input parameter
'=======================================================
Sub viewWebPage(page As String)
Dim q As String
Dim cl As String
q = Chr(34)
If htmlViewFileName = "" Then
  Exit Sub
End If
On Error Resume Next
Err.Clear
If Err.Number = 0 Then
    cl = htmlViewFileName & " " & q & page & q
    Debug.Print cl
    Shell cl, vbNormalFocus
    If Err.Number <> 0 Then
        MsgBox "Error initiating HTML browser. This is likely due to the HTML browser program being deleted or moved. Please go to VIEW OPTIONS to select a new HTML BROWSER program.", vbExclamation, "Edit Cancelled"
        Exit Sub
    End If
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
