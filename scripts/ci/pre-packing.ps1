param (
    [string]$DirectoryPath
)

# Check if the directory path is provided and exists
if (-not (Test-Path -Path $DirectoryPath)) {
    exit
}

$LOG_PATH = $DirectoryPath + "/debug"
if ((Test-Path $LOG_PATH) -eq "True") {
	Remove-Item $LOG_PATH -Recurse
}

$REMOVE_PATH = $DirectoryPath + "/*"
Remove-Item $REMOVE_PATH -Include "*.iobj"
Remove-Item $REMOVE_PATH -Include "*.ipdb"
Remove-Item $REMOVE_PATH -Include "*.exp"
Remove-Item $REMOVE_PATH -Include "*.ilk"
Remove-Item $REMOVE_PATH -Include "*.lib"
Remove-Item $REMOVE_PATH -Include "*.a"
