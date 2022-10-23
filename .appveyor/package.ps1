$ErrorActionPreference = "Stop"

$ts_part = 'ts'
if ($env:TS -eq '0') {
    $ts_part += 'nts'
}
$commit = $env:APPVEYOR_REPO_COMMIT.substring(0, 8)
$zip_bname = "php_immutable_cache-$commit-$env:PHP_VER-$ts_part-$env:VC-$env:ARCH.zip"

$dir = 'c:\projects\immutable_cache\'
if ($env:ARCH -eq 'x64') {
    $dir += 'x64\'
}
$dir += 'Release'
if ($env:TS -eq '1') {
    $dir += '_TS'
}

Compress-Archive @("$dir\php_immutable_cache.dll", "$dir\php_immutable_cache.pdb", "c:\projects\immutable_cache\LICENSE") "c:\$zip_bname"
Push-AppveyorArtifact "c:\$zip_bname"
