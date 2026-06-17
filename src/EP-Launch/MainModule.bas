Attribute VB_Name = "MainModule"
'=======================================================
' Main module for EP-Launch
'
' for now a place to put types and arrays that are
' needed both inside and outside of the main epl-ui.frm
'=======================================================

Public Const numOutputKinds = 64
Public Const numOutputSets = 12
Type outputKindType
  suffix As String 'end of file name plus extension (like "Table.html")
  viewer As Integer 'numerical reference to the viewer programs (see ovp constants)
  outSet(numOutputSets) As Boolean 'flag if included for a specific output set (user defined or predefined)
  containsTabs As Boolean 'flag if the file uses tabs and can be opened with either spreadsheet or text editor
End Type
Public outputKind(numOutputKinds) As outputKindType

'the following can be combined to indicate possible options
Public Const ovpTextEditor = 1     'for most other files
Public Const ovpDrawingViewer = 2  'for DXF
Public Const ovpVRMLviewer = 4     'for WRL
Public Const ovpSpreadsheet = 8    'for CSV and TAB
Public Const ovpDiagramming = 16   'for SVG
Public Const ovpHTMLBrowser = 32   'for HTML
Public Const ovpESOviewer = 64     'for ESO
Public Const ovpXMLviewer = 128     'for XML

' outSet 1 through 8 are user defined but 9, 10, 11 and 12 are hard wired
' to the for buttons on the "select" tab of view resutls
Public Const outSetTextOutputFiles = 9
Public Const outSetDrawingFiles = 10
Public Const outSetSpreadsheets = 11
Public Const outSetHTML = 12

Public Const okTab = 1
Public Const okTxt = 2
Public Const okMeterCsv = 3
Public Const okMeterTab = 4
Public Const okMeterTxt = 5
Public Const okTableCsv = 6
Public Const okTableTab = 7
Public Const okTableTxt = 8
Public Const okTableHtml = 9
Public Const okTableXML = 10
Public Const okRdd = 11
Public Const okMdd = 12
Public Const okEio = 13
Public Const okSvg = 14
Public Const okDxf = 15
Public Const okMtd = 16
Public Const okZszCsv = 17
Public Const okZszTab = 18
Public Const okZszTxt = 19
Public Const okSszCsv = 20
Public Const okSszTab = 21
Public Const okSszTxt = 22
Public Const okDelightIn = 23
Public Const okDelightOut = 24
Public Const okMapCsv = 25
Public Const okMapTab = 26
Public Const okMapTxt = 27
Public Const okDelightEldmp = 28
Public Const okDelightDfdmp = 29
Public Const okScreenCsv = 30
Public Const okExpIdf = 31
Public Const okEmpIdf = 32
Public Const okEmpDet = 33
Public Const okShd = 34
Public Const okWrl = 35
Public Const okAudit = 36
Public Const okBnd = 37
Public Const okDbg = 38
Public Const okSln = 39
Public Const okEdd = 40
Public Const okEso = 41
Public Const okMtr = 42
Public Const okProcCsv = 43
Public Const okEnd = 44
Public Const okSci = 45
Public Const okRvAudit = 46
Public Const okSql = 47
Public Const okLog = 48
Public Const okBsmt = 49
Public Const okBsmtOut = 50
Public Const okBsmtAudit = 51
Public Const okBsmtCSV = 52
Public Const okSlab = 53
Public Const okSlabOut = 54
Public Const okSlabErr = 55
Public Const okPerfLogCSV = 56
Public Const okSpszCsv = 57
Public Const okSpszTab = 58
Public Const okSpszTxt = 59
Public Const okPszCsv = 60
Public Const okPszTab = 61
Public Const okPszTxt = 62
Public Const okErr = 63 'at end so it opens last
Public Const okCsv = 64 'at end so it opens last


'=======================================================
' Set up the initial values of the output sets
'=======================================================
Public Sub initializeOutputKindsAndSets()
Dim iKind As Integer
Dim jSet As Integer
'clear are flags first
For iKind = 1 To numOutputKinds
  For jSet = 1 To numOutputSets
    outputKind(iKind).outSet(jSet) = False
  Next jSet
  outputKind(iKind).containsTabs = False
Next iKind
'----UPPER LEFT BLOCK----
'variable
outputKind(okCsv).suffix = ".csv"
outputKind(okCsv).viewer = ovpSpreadsheet
outputKind(okCsv).outSet(outSetSpreadsheets) = True
outputKind(okTab).suffix = ".tab"
outputKind(okTab).viewer = ovpSpreadsheet + ovpTextEditor
outputKind(okTab).outSet(outSetTextOutputFiles) = True
outputKind(okTab).containsTabs = True
outputKind(okTxt).suffix = ".txt"
outputKind(okTxt).viewer = ovpTextEditor
outputKind(okTxt).outSet(outSetTextOutputFiles) = True
'meter
outputKind(okMeterCsv).suffix = "Meter.csv"
outputKind(okMeterCsv).viewer = ovpSpreadsheet
outputKind(okMeterCsv).outSet(outSetSpreadsheets) = True
outputKind(okMeterTab).suffix = "Meter.tab"
outputKind(okMeterTab).viewer = ovpSpreadsheet + ovpTextEditor
outputKind(okMeterTab).outSet(outSetTextOutputFiles) = True
outputKind(okMeterTab).outSet(outSetSpreadsheets) = True
outputKind(okMeterTab).containsTabs = True
outputKind(okMeterTxt).suffix = "Meter.txt"
outputKind(okMeterTxt).viewer = ovpTextEditor
outputKind(okMeterTxt).outSet(outSetTextOutputFiles) = True
'table
outputKind(okTableCsv).suffix = "Table.csv"
outputKind(okTableCsv).viewer = ovpSpreadsheet
outputKind(okTableCsv).outSet(outSetSpreadsheets) = True
outputKind(okTableTab).suffix = "Table.tab"
outputKind(okTableTab).viewer = ovpSpreadsheet + ovpTextEditor
outputKind(okTableTab).outSet(outSetTextOutputFiles) = True
outputKind(okTableTab).outSet(outSetSpreadsheets) = True
outputKind(okTableTab).containsTabs = True
outputKind(okTableTxt).suffix = "Table.txt"
outputKind(okTableTxt).viewer = ovpTextEditor
outputKind(okTableTxt).outSet(outSetTextOutputFiles) = True
outputKind(okTableHtml).suffix = "Table.html"
outputKind(okTableHtml).viewer = ovpHTMLBrowser
outputKind(okTableHtml).outSet(outSetHTML) = True
'err
outputKind(okErr).suffix = ".err"
outputKind(okErr).viewer = ovpTextEditor
outputKind(okErr).outSet(outSetTextOutputFiles) = True
'rdd
outputKind(okRdd).suffix = ".rdd"
outputKind(okRdd).viewer = ovpTextEditor
outputKind(okRdd).outSet(outSetTextOutputFiles) = True
'mdd
outputKind(okMdd).suffix = ".mdd"
outputKind(okMdd).viewer = ovpTextEditor
outputKind(okMdd).outSet(outSetTextOutputFiles) = True
'----LOWER LEFT BLOCK----
'eio
outputKind(okEio).suffix = ".eio"
outputKind(okEio).viewer = ovpTextEditor
outputKind(okEio).outSet(outSetTextOutputFiles) = True
'svg
outputKind(okSvg).suffix = ".svg"
outputKind(okSvg).viewer = ovpDiagramming
outputKind(okSvg).outSet(outSetDrawingFiles) = True
'dxf
outputKind(okDxf).suffix = ".dxf"
outputKind(okDxf).viewer = outSetDrawingFiles
outputKind(okDxf).outSet(outSetDrawingFiles) = True
'mtd
outputKind(okMtd).suffix = ".mtd"
outputKind(okMtd).viewer = ovpTextEditor
outputKind(okMtd).outSet(outSetTextOutputFiles) = True
'zsz
outputKind(okZszCsv).suffix = "Zsz.csv"
outputKind(okZszCsv).viewer = ovpSpreadsheet
outputKind(okZszCsv).outSet(outSetSpreadsheets) = True
outputKind(okZszTab).suffix = "Zsz.tab"
outputKind(okZszTab).viewer = ovpSpreadsheet + ovpTextEditor
outputKind(okZszTab).outSet(outSetTextOutputFiles) = True
outputKind(okZszTab).outSet(outSetSpreadsheets) = True
outputKind(okZszTab).containsTabs = True
outputKind(okZszTxt).suffix = "Zsz.txt"
outputKind(okZszTxt).viewer = ovpTextEditor
outputKind(okZszTxt).outSet(outSetTextOutputFiles) = True
'ssz
outputKind(okSszCsv).suffix = "Ssz.csv"
outputKind(okSszCsv).viewer = ovpSpreadsheet
outputKind(okSszCsv).outSet(outSetSpreadsheets) = True
outputKind(okSszTab).suffix = "Ssz.tab"
outputKind(okSszTab).viewer = ovpSpreadsheet + ovpTextEditor
outputKind(okSszTab).outSet(outSetTextOutputFiles) = True
outputKind(okSszTab).outSet(outSetSpreadsheets) = True
outputKind(okSszTab).containsTabs = True
outputKind(okSszTxt).suffix = "Ssz.txt"
outputKind(okSszTxt).viewer = ovpTextEditor
outputKind(okSszTxt).outSet(outSetTextOutputFiles) = True
'----UPPER MIDDLE BLOCK----
'DE IN
outputKind(okDelightIn).suffix = "DElight.in"
outputKind(okDelightIn).viewer = ovpTextEditor
outputKind(okDelightIn).outSet(outSetTextOutputFiles) = True
'DE OUT
outputKind(okDelightOut).suffix = "DElight.out"
outputKind(okDelightOut).viewer = ovpTextEditor
outputKind(okDelightOut).outSet(outSetTextOutputFiles) = True
'Map
outputKind(okMapCsv).suffix = "Map.csv"
outputKind(okMapCsv).viewer = ovpSpreadsheet
outputKind(okMapCsv).outSet(outSetSpreadsheets) = True
outputKind(okMapTab).suffix = "Map.tab"
outputKind(okMapTab).viewer = ovpSpreadsheet + ovpTextEditor
outputKind(okMapTab).outSet(outSetTextOutputFiles) = True
outputKind(okMapTab).outSet(outSetSpreadsheets) = True
outputKind(okMapTab).containsTabs = True
outputKind(okMapTxt).suffix = "Map.txt"
outputKind(okMapTxt).viewer = ovpTextEditor
outputKind(okMapTxt).outSet(outSetTextOutputFiles) = True
'ELDMP
outputKind(okDelightEldmp).suffix = "DElight.eldmp"
outputKind(okDelightEldmp).viewer = ovpTextEditor
outputKind(okDelightEldmp).outSet(outSetTextOutputFiles) = True
'DFDMP
outputKind(okDelightDfdmp).suffix = "DElight.dfdmp"
outputKind(okDelightDfdmp).viewer = ovpTextEditor
outputKind(okDelightDfdmp).outSet(outSetTextOutputFiles) = True
'Screen
outputKind(okScreenCsv).suffix = "Screen.csv"
outputKind(okScreenCsv).viewer = ovpSpreadsheet
outputKind(okScreenCsv).outSet(outSetSpreadsheets) = True
'----LOWER MIDDLE BLOCK----
'EXPIDF
outputKind(okExpIdf).suffix = ".expidf"
outputKind(okExpIdf).viewer = ovpTextEditor
outputKind(okExpIdf).outSet(outSetTextOutputFiles) = True
'EPMIDF
outputKind(okEmpIdf).suffix = ".epmidf"
outputKind(okEmpIdf).viewer = ovpTextEditor
outputKind(okEmpIdf).outSet(outSetTextOutputFiles) = True
'EPMDET
outputKind(okEmpDet).suffix = ".epmdet"
outputKind(okEmpDet).viewer = ovpTextEditor
outputKind(okEmpDet).outSet(outSetTextOutputFiles) = True
'SHD
outputKind(okShd).suffix = ".shd"
outputKind(okShd).viewer = ovpTextEditor
outputKind(okShd).outSet(outSetTextOutputFiles) = True
'VRML
outputKind(okWrl).suffix = ".wrl"
outputKind(okWrl).viewer = ovpVRMLviewer
outputKind(okWrl).outSet(outSetDrawingFiles) = True
'Audit
outputKind(okAudit).suffix = ".audit"
outputKind(okAudit).viewer = ovpTextEditor
outputKind(okAudit).outSet(outSetTextOutputFiles) = True
'----UPPER RIGHT BLOCK----
'BND
outputKind(okBnd).suffix = ".bnd"
outputKind(okBnd).viewer = ovpTextEditor
outputKind(okBnd).outSet(outSetTextOutputFiles) = True
'DBG
outputKind(okDbg).suffix = ".dbg"
outputKind(okDbg).viewer = ovpTextEditor
outputKind(okDbg).outSet(outSetTextOutputFiles) = True
'SLN
outputKind(okSln).suffix = ".sln"
outputKind(okSln).viewer = ovpTextEditor
outputKind(okSln).outSet(outSetTextOutputFiles) = True
'EDD
outputKind(okEdd).suffix = ".edd"
outputKind(okEdd).viewer = ovpTextEditor
'Bsmt Out
outputKind(okBsmtOut).suffix = "_bsmt.out"
outputKind(okBsmtOut).viewer = ovpTextEditor
'Bsmt
outputKind(okBsmt).suffix = ".bsmt"
outputKind(okBsmt).viewer = ovpTextEditor
'Bsmt Audit
outputKind(okBsmtAudit).suffix = "_bsmt.audit"
outputKind(okBsmtAudit).viewer = ovpTextEditor
'Bsmt CSV
outputKind(okBsmtCSV).suffix = "_bsmt.csv"
outputKind(okBsmtCSV).viewer = ovpSpreadsheet
'Table XML
outputKind(okTableXML).suffix = "Table.xml"
outputKind(okTableXML).viewer = ovpXMLviewer

'----LOWER RIGHT BLOCK----
'ESO
outputKind(okEso).suffix = ".eso"
outputKind(okEso).viewer = ovpESOviewer + ovpTextEditor
outputKind(okEso).outSet(outSetTextOutputFiles) = True
'MTR
outputKind(okMtr).suffix = ".mtr"
outputKind(okMtr).viewer = ovpTextEditor
outputKind(okMtr).outSet(outSetTextOutputFiles) = True
'Proc CSV
outputKind(okProcCsv).suffix = "-Proc.csv"
outputKind(okProcCsv).viewer = ovpSpreadsheet
outputKind(okProcCsv).outSet(outSetSpreadsheets) = True
'Slab Out
outputKind(okSlabOut).suffix = "_slab.out"
outputKind(okSlabOut).viewer = ovpTextEditor
'Slab
outputKind(okSlab).suffix = ".slab"
outputKind(okSlab).viewer = ovpTextEditor
'Slab Err
outputKind(okSlabErr).suffix = "_slab.ger"
outputKind(okSlabErr).viewer = ovpTextEditor
'Perflog.csv
outputKind(okPerfLogCSV).suffix = "_perflog.csv"
outputKind(okPerfLogCSV).viewer = ovpSpreadsheet
outputKind(okPerfLogCSV).outSet(outSetSpreadsheets) = True
'Spsz
outputKind(okSpszCsv).suffix = "Spsz.csv"
outputKind(okSpszCsv).viewer = ovpSpreadsheet
outputKind(okSpszCsv).outSet(outSetSpreadsheets) = True
outputKind(okSpszTab).suffix = "Spsz.tab"
outputKind(okSpszTab).viewer = ovpSpreadsheet + ovpTextEditor
outputKind(okSpszTab).outSet(outSetTextOutputFiles) = True
outputKind(okSpszTab).outSet(outSetSpreadsheets) = True
outputKind(okSpszTab).containsTabs = True
outputKind(okSpszTxt).suffix = "Spsz.txt"
outputKind(okSpszTxt).viewer = ovpTextEditor
outputKind(okSpszTxt).outSet(outSetTextOutputFiles) = True
'Psz
outputKind(okPszCsv).suffix = "Psz.csv"
outputKind(okPszCsv).viewer = ovpSpreadsheet
outputKind(okPszCsv).outSet(outSetSpreadsheets) = True
outputKind(okPszTab).suffix = "Psz.tab"
outputKind(okPszTab).viewer = ovpSpreadsheet + ovpTextEditor
outputKind(okPszTab).outSet(outSetTextOutputFiles) = True
outputKind(okPszTab).outSet(outSetSpreadsheets) = True
outputKind(okPszTab).containsTabs = True
outputKind(okPszTxt).suffix = "Psz.txt"
outputKind(okPszTxt).viewer = ovpTextEditor
outputKind(okPszTxt).outSet(outSetTextOutputFiles) = True




'NOT SHOWN ON ALL TAB BUT USED
outputKind(okEnd).suffix = ".end"
outputKind(okEnd).viewer = ovpTextEditor
outputKind(okSci).suffix = ".sci"
outputKind(okSci).viewer = ovpTextEditor
outputKind(okRvAudit).suffix = ".rvaudit"
outputKind(okRvAudit).viewer = ovpTextEditor
outputKind(okSql).suffix = ".sql"
outputKind(okSql).viewer = 0  'no typical viewer can open SQL file
outputKind(okLog).suffix = ".log"
outputKind(okLog).viewer = ovpTextEditor


'NOT SHOWN ON ALL TAB AND NOT USED (see June 2, 2010 email to LKL)
'outputKind(1).suffix = ".det"
'outputKind(1).viewer = 0
'outputKind(1).suffix = ".zsz"
'outputKind(1).viewer = 0
'outputKind(1).suffix = ".ssz"
'outputKind(1).viewer = 0
'outputKind(1).suffix = "Spark.log"
'outputKind(1).viewer = 0
End Sub


'     NOTICE
'
'     The contents of this file are subject to the EnergyPlus Open Source License
'     Version 1.0 (the "License"); you may not use this file except in compliance
'     with the License. You may obtain a copy of the License at
'
'     https://energyplus.net/licensing
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
