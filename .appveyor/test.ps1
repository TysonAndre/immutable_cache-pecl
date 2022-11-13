$ErrorActionPreference = "Stop"

Set-Location 'c:\projects\immutable_cache'

$env:TEST_PHP_EXECUTABLE = Get-Command 'php' | Select-Object -ExpandProperty 'Definition'
& $env:TEST_PHP_EXECUTABLE 'run-tests.php' --show-diff tests
if (-not $?) {
    throw "tests failed with errorlevel $LastExitCode"
}
& $env:TEST_PHP_EXECUTABLE 'run-tests.php' --show-diff tests -d immutable_cache.protect_memory=1 
if (-not $?) {
    throw "tests with protect_memory failed with errorlevel $LastExitCode"
}
