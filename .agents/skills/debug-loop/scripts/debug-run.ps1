param(
    [ValidateSet('TD', 'RA', 'Both')]
    [string]$Game = 'Both',

    # The CMake tree that holds the standalone game build.
    [string]$BuildTree = 'build/release',

    [ValidateSet('RelWithDebInfo', 'Debug')]
    [string]$Config = 'Debug',

    # Machine specific defaults. Override them when the toolchain or the game data move.
    [string]$Sdl2Root = 'C:\Users\franc\dev\deps\vanilla-conquer\SDL2-2.0.12',
    [string]$OpenalRoot = 'C:\Users\franc\dev\deps\vanilla-conquer\openal-soft-1.21.0-bin',
    [string]$Toolchain = 'C:/Users/franc/dev/vcpkg/scripts/buildsystems/vcpkg.cmake',

    # Where the played release keeps its settings. The values are copied into the debug
    # config, the release folder itself is never written to.
    [string]$SettingsRoot = 'C:\BoomerGames\KOMKOMMER',

    # Read only game data for the debug build.
    [string]$TdDataPath = 'D:\Games\VANILLATD',
    [string]$RaDataPath = 'D:\Games\VANILLARA',

    # Remove the debug binary and config of the selected games before building.
    [switch]$Clean,

    # Recompile everything before building.
    [switch]$Rebuild,

    # Keep savegame.* and hallfame.dat in the debug folder.
    [switch]$KeepSaves,

    # Keep the debug config that is already there instead of importing the release settings.
    [switch]$KeepSettings,

    # Start the game after the config is staged.
    [switch]$Launch
)

$ErrorActionPreference = 'Stop'

function Get-VcVars {
    $vswhere = Join-Path ${env:ProgramFiles(x86)} 'Microsoft Visual Studio\Installer\vswhere.exe'
    if (-not (Test-Path $vswhere)) {
        throw "vswhere.exe not found. Install the Visual Studio 2022 C++ build tools."
    }
    $vsRoot = (& $vswhere -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath).Trim()
    if (-not $vsRoot) {
        throw "No Visual Studio installation with the C++ tools was found."
    }
    $vcvars = Join-Path $vsRoot 'VC\Auxiliary\Build\vcvarsall.bat'
    if (-not (Test-Path $vcvars)) {
        throw "vcvarsall.bat not found under $vsRoot"
    }
    return $vcvars
}

function Invoke-Build {
    param([string]$Vcvars, [string]$Tree, [string]$Target, [string]$BuildConfig)

    $command = "call `"$Vcvars`" x64 >nul && cmake --build `"$Tree`" --config $BuildConfig --target $Target"
    cmd /c $command
    if ($LASTEXITCODE -ne 0) {
        throw "Debug build of $Target failed with exit code $LASTEXITCODE"
    }
}

# Reads an ini file and returns its lines with the [Paths] section replaced.
# The body comes from $SourceIni, or from the target file itself when $KeepBody is set.
function Set-DebugConfig {
    param(
        [string]$TargetIni,
        [string]$SourceIni,
        [string]$DataPath,
        [switch]$KeepBody
    )

    $body = @()
    if ($KeepBody -and (Test-Path $TargetIni)) {
        $body = @(Get-Content -LiteralPath $TargetIni)
    } elseif ($SourceIni -and (Test-Path $SourceIni)) {
        $body = @(Get-Content -LiteralPath $SourceIni)
    }

    $out = New-Object System.Collections.Generic.List[string]
    $out.Add('[Paths]')
    $out.Add("DataPath=$DataPath")
    $out.Add('')

    $skip = $false
    foreach ($line in $body) {
        if ($line -match '^\s*\[(.+)\]\s*$') {
            $skip = ($Matches[1] -ieq 'Paths')
            if (-not $skip) {
                $out.Add($line)
            }
            continue
        }
        if (-not $skip) {
            $out.Add($line)
        }
    }

    [System.IO.File]::WriteAllLines($TargetIni, $out, (New-Object System.Text.UTF8Encoding($false)))
}

$repo = (& git -C $PSScriptRoot rev-parse --show-toplevel).Trim()
if (-not $repo) {
    throw "Could not resolve the git root from $PSScriptRoot"
}

$tree = Join-Path $repo $BuildTree
$outDir = Join-Path $tree $Config

# The settings that came from the release build. The debug build keeps its own copy,
# so the played release folder is only ever read.
$games = @()
if ($Game -ne 'RA') {
    $games += @{
        Name = 'TD'
        Target = 'KomkommerTD'
        Exe = 'komkommertd.exe'
        Ini = 'conquer.ini'
        Source = (Join-Path $SettingsRoot 'TD\conquer.ini')
        DataPath = $TdDataPath
    }
}
if ($Game -ne 'TD') {
    $games += @{
        Name = 'RA'
        Target = 'KomkommerRA'
        Exe = 'komkommerra.exe'
        Ini = 'redalert.ini'
        Source = (Join-Path $SettingsRoot 'RA\redalert.ini')
        DataPath = $RaDataPath
    }
}

if (-not (Test-Path (Join-Path $tree 'CMakeCache.txt'))) {
    Write-Host "Configuring $BuildTree"
    $vcvars = Get-VcVars
    $configure = "call `"$vcvars`" x64 >nul && cd /d `"$repo`" && cmake --preset $BuildTree -DSDL2_ROOT_DIR=`"$Sdl2Root`" -DOPENAL_ROOT=`"$OpenalRoot`" -DCMAKE_TOOLCHAIN_FILE=`"$Toolchain`""
    cmd /c $configure
    if ($LASTEXITCODE -ne 0) {
        throw "Configuring $BuildTree failed with exit code $LASTEXITCODE"
    }
}

$vcvars = Get-VcVars

if ($Rebuild) {
    Write-Host "Recompiling $BuildTree ($Config)"
    $rebuild = "call `"$vcvars`" x64 >nul && cmake --build `"$tree`" --config $Config --target clean"
    cmd /c $rebuild
    if ($LASTEXITCODE -ne 0) {
        throw "Cleaning $BuildTree failed with exit code $LASTEXITCODE"
    }
}

if ($Clean) {
    foreach ($entry in $games) {
        foreach ($file in @($entry.Exe, $entry.Ini, ([System.IO.Path]::GetFileNameWithoutExtension($entry.Exe) + '.pdb'))) {
            $stale = Join-Path $outDir $file
            if (Test-Path $stale) {
                Write-Host "Removing $stale"
                Remove-Item $stale -Force
            }
        }
    }
}

foreach ($entry in $games) {
    Write-Host "Building $($entry.Target) ($Config)"
    Invoke-Build -Vcvars $vcvars -Tree $tree -Target $entry.Target -BuildConfig $Config
}

foreach ($entry in $games) {
    $exe = Join-Path $outDir $entry.Exe
    if (-not (Test-Path $exe)) {
        throw "The build did not produce $exe"
    }

    $targetIni = Join-Path $outDir $entry.Ini
    if (-not (Test-Path $entry.Source) -and -not $KeepSettings) {
        Write-Warning "No release settings at $($entry.Source). The debug config keeps only the paths."
    }
    Set-DebugConfig -TargetIni $targetIni -SourceIni $entry.Source -DataPath $entry.DataPath -KeepBody:$KeepSettings

    if (-not (Test-Path $entry.DataPath)) {
        Write-Warning "The game data folder $($entry.DataPath) does not exist."
    }

    if (-not $KeepSaves) {
        Get-ChildItem -Path $outDir -Filter 'savegame.*' -ErrorAction SilentlyContinue | Remove-Item -Force
        $hallfame = Join-Path $outDir 'hallfame.dat'
        if (Test-Path $hallfame) {
            Remove-Item $hallfame -Force
        }
    }

    foreach ($dll in @('SDL2.dll', 'OpenAL32.dll')) {
        if (-not (Test-Path (Join-Path $outDir $dll))) {
            $fallback = Join-Path $SettingsRoot $dll
            if (Test-Path $fallback) {
                Copy-Item $fallback (Join-Path $outDir $dll) -Force
            } else {
                Write-Warning "$dll is missing next to $($entry.Exe)."
            }
        }
    }

    $info = Get-Item $exe
    Write-Host ("{0}: {1} ({2} bytes)" -f $entry.Name, $exe, $info.Length)
    Write-Host ("     config {0}" -f $targetIni)
    Write-Host ("     data   {0}" -f $entry.DataPath)
    Write-Host ("     saved from {0}" -f $entry.Source)

    if ($Launch) {
        Start-Process -FilePath $exe -WorkingDirectory $outDir
        Write-Host "     started $entry.Name"
    }
}
