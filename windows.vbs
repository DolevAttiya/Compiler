Dim objFso: Set objFso = CreateObject("Scripting.FileSystemObject")
Dim objShell: Set objShell = WScript.CreateObject( "WScript.Shell" )
Dim objStartFolder: objStartFolder = objFso.GetAbsolutePathName(".")
Dim objFolder: Set objFolder = objFSO.GetFolder(objStartFolder)
Dim colFiles: Set colFiles = objFolder.Files
Dim FileName: FileName = ""

For Each objFile in colFiles
	If UCase(objFSO.GetExtensionName(objFile.name)) = "LEX" Then
		FileName = 	objFile.Name
		Exit For
	End If
Next

FileName = InputBox("Enter lex file name:", "HiT Script", FileName)

If (FileName <> "") then	
	objShell.Run("cmd /c """"" & objStartFolder & "\win_flex.exe"" --outfile=""win.lex.yy.c"" --wincompat " & FileName & " > output.txt 2>&1""")

	WScript.Sleep 1500

	if objFso.GetFile(objStartFolder & "\output.txt").size <> 0 then
		Set file = objFso.OpenTextFile(objStartFolder & "\output.txt", 1)
		text = file.ReadAll
		file.Close
		MsgBox text, vbExclamation, "HiT Script"
    else
		MsgBox "OK", vbinformation, "HiT Script"
	end if
		
End If

Set objShell = Nothing
Set objFso = Nothing