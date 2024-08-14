param (
    [Parameter(Mandatory = $true, HelpMessage = "Source directory path")]
    [string]$SourceDirectory,

    [Parameter(Mandatory = $true, HelpMessage = "Destination directory path")]
    [string]$DestinationDirectory
)

if (-Not (Test-Path -Path $SourceDirectory)) {
    Write-Error "Source directory does not exist: $SourceDirectory"
    exit 1
}

Copy-Item -Path $SourceDirectory -Destination $DestinationDirectory -Recurse -PassThru | Write-Host
