param (
  [string]$SourceDir,
  [string]$AppName,
  [string]$Manufacturer,
  [string]$Version,
  [string]$OutputMsi
)

$wxsFile = "$env:TEMP\$AppName-heat.wxs"
$mainWxs = "$env:TEMP\$AppName-main.wxs"
$upgradeCode = [guid]::NewGuid().ToString()
$targetDirId = "$AppName`Dir"

$absSourceDir = (Resolve-Path $SourceDir).Path

& "C:\Program Files\WiX Toolset v6.0\bin\x64\heat.exe" dir $absSourceDir `
  -cg AppFiles `
  -dr INSTALLDIR `
  -gg `
  -g1 `
  -srd `
  -sfrag `
  -out $wxsFile

$wxsContent = @"
<?xml version=`"1.0`" encoding=`"UTF-8`"?>
<Wix xmlns="http://wixtoolset.org/schemas/v4/wxs">
  <Package Name="$AppName"
           Manufacturer="$Manufacturer"
           Version="$Version"
           Language="1033"
           UpgradeCode="$upgradeCode"
           InstallerVersion="500"
           Compressed="yes">
    <Media Id="1" Cabinet="media1.cab" EmbedCab="yes" />
    <StandardDirectory Id="ProgramFilesFolder">
      <Directory Id="$targetDirId" Name="$AppName">
        <Directory Id="INSTALLDIR" Name="." />
      </Directory>
    </Directory>
    <Feature Id="DefaultFeature" Title="$AppName Feature" Level="1">
      <ComponentGroupRef Id="AppFiles" />
    </Feature>
  </Package>
</Wix>
"@

$wxsContent | Set-Content $mainWxs -Encoding UTF8
Write-Output "[DEBUG] heat wxs content here:"
Get-Content $wxsFile
Write-Output "[DEBUG] main wxs content here:"
Get-Content $mainWxs
& "C:\Program Files\WiX Toolset v6.0\bin\wix.exe" build `
  -arch x64 `
  -out $OutputMsi `
  $mainWxs $wxsFile
