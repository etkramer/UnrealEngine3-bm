Dim clientSpec,clientRoot,startTime

Function GetP4Info() 
	''starts a dos box and gets the output of p4 info
	Dim strCommand, objShell, objExec 
	strCommand = "p4 info"
	Set objShell = CreateObject("WScript.Shell") 
	Set objExec = objShell.Exec(strCommand) 
	GetP4Info = objExec.StdOut.ReadAll 
End Function 

Function StringToArray(input) 
	''converts the multiline output to an array so that it can be parsed line by line 
	StringToArray = Split(input,vbCrLf) 
End Function 

Function ParseP4Output()
	Dim outputLines
	Dim outputLine

	outputLines = StringToArray(GetP4Info())

	For Each outputLine In outputLines
		If (InStr(outputLine,"Client name: ") <> 0) Then
			clientSpec = (Split(outputLine,"Client name: "))(1)
		ElseIf (InStr(outputLine,"Client root: ") <> 0) Then 
			clientRoot = (Split(outputLine,"Client root: "))(1)
		End If
	Next
End Function

Sub CreateProjectJob()
	const HKEY_CURRENT_USER = &H80000001
	strComputer = "."
	Set StdOut = WScript.StdOut
 
	Set oReg=GetObject("winmgmts:{impersonationLevel=impersonate}!\\" &_ 
		strComputer & "\root\default:StdRegProv")
 
	strKeyPath = "Software\Epic Games\UnrealSync"
	strKeyPath = strKeyPath & "\" & "Gears 2"

	batchFilePath = clientRoot & "\UnrealEngine3\GearGame\Build\geargame-artist-sync.bat"
	Randomize()
	startTime = Int(2*Rnd+6) & ":" & Int(50*Rnd+10) & " AM"

	oReg.CreateKey HKEY_CURRENT_USER,strKeyPath
	oReg.SetStringValue HKEY_CURRENT_USER,strKeyPath,"BatchFilePath",batchFilePath
	oReg.SetStringValue HKEY_CURRENT_USER,strKeyPath,"Enabled","True"
	oReg.SetStringValue HKEY_CURRENT_USER,strKeyPath,"GameProcessName","GearGame"
	oReg.SetStringValue HKEY_CURRENT_USER,strKeyPath,"Label",""
	oReg.SetStringValue HKEY_CURRENT_USER,strKeyPath,"PerforceClientSpec",clientSpec
	oReg.SetStringValue HKEY_CURRENT_USER,strKeyPath,"PostBatchPath",""
	oReg.SetStringValue HKEY_CURRENT_USER,strKeyPath,"StartTime",startTime
End Sub

ParseP4Output()
CreateProjectJob()
MsgBox("Sync job installed... Your sync job will start at " & startTime)