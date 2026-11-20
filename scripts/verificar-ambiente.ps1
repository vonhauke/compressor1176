# verificar-ambiente.ps1
# Verifica o que ja esta instalado para compilar o plugin no Windows.
# NAO instala nada. Rode no PowerShell:
#   powershell -ExecutionPolicy Bypass -File .\scripts\verificar-ambiente.ps1
# e mande a saida inteira de volta.

$ErrorActionPreference = 'SilentlyContinue'
$faltando = @()

function Linha($nome, $ok, $detalhe) {
    if ($ok) { $marca = '[OK]    ' } else { $marca = '[FALTA] ' }
    Write-Host ($marca + $nome.PadRight(34) + $detalhe)
}

Write-Host '=== Sistema ==='
$os = Get-CimInstance Win32_OperatingSystem
Write-Host ("Windows: " + $os.Caption + " build " + $os.BuildNumber + " (" + $env:PROCESSOR_ARCHITECTURE + ")")
Write-Host ("PowerShell: " + $PSVersionTable.PSVersion.ToString())
$cpu = Get-CimInstance Win32_Processor | Select-Object -First 1
Write-Host ("CPU: " + $cpu.Name.Trim() + ", nucleos: " + $cpu.NumberOfCores)
Write-Host ''

Write-Host '=== Ferramentas ==='

# winget
$winget = Get-Command winget
Linha 'winget' ($null -ne $winget) ($(if ($winget) { (winget --version) } else { '' }))

# Git
$git = Get-Command git
if ($git) { Linha 'Git' $true (git --version) } else { Linha 'Git' $false ''; $faltando += 'Git' }

# Visual Studio Build Tools 2022 (via vswhere)
$vswhere = Join-Path ${env:ProgramFiles(x86)} 'Microsoft Visual Studio\Installer\vswhere.exe'
$vsPath = $null
if (Test-Path $vswhere) {
    $vsPath = & $vswhere -products * -version '[17.0,18.0)' -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath | Select-Object -First 1
    $vsTodos = & $vswhere -products * -all -format value -property displayName
    Write-Host ('        Instalacoes do Visual Studio encontradas: ' + (($vsTodos | Select-Object -Unique) -join '; '))
}
if ($vsPath) {
    $msvcDir = Join-Path $vsPath 'VC\Tools\MSVC'
    $msvc = (Get-ChildItem $msvcDir -Directory | Sort-Object Name -Descending | Select-Object -First 1).Name
    Linha 'VS 2022 com compilador C++ (MSVC)' $true ($vsPath + '  MSVC ' + $msvc)
} else {
    Linha 'VS 2022 com compilador C++ (MSVC)' $false 'Build Tools 2022 sem o componente "Desenvolvimento para desktop com C++"?'
    $faltando += 'Visual Studio Build Tools 2022 + carga "Desktop development with C++"'
}

# Windows SDK
$sdkInc = Join-Path ${env:ProgramFiles(x86)} 'Windows Kits\10\Include'
$sdks = @()
if (Test-Path $sdkInc) { $sdks = Get-ChildItem $sdkInc -Directory | Where-Object { $_.Name -like '10.*' } | Sort-Object Name -Descending }
if ($sdks.Count -gt 0) { Linha 'Windows 10/11 SDK' $true (($sdks | Select-Object -First 3 | ForEach-Object { $_.Name }) -join ', ') }
else { Linha 'Windows 10/11 SDK' $false ''; $faltando += 'Windows SDK (vem com a carga C++ do Build Tools)' }

# CMake (no PATH ou o que vem junto com o VS)
$cmake = Get-Command cmake
$cmakeExe = $null
if ($cmake) { $cmakeExe = $cmake.Source }
elseif ($vsPath) {
    $c = Join-Path $vsPath 'Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe'
    if (Test-Path $c) { $cmakeExe = $c }
}
if ($cmakeExe) {
    $verTxt = (& $cmakeExe --version | Select-Object -First 1)
    $ok = $false
    if ($verTxt -match '(\d+\.\d+(\.\d+)?)') { $ok = ([version]$matches[1]) -ge [version]'3.22' }
    $onde = $(if ($cmake) { 'no PATH' } else { 'so dentro do VS (fora do PATH)' })
    Linha 'CMake >= 3.22' $ok ($verTxt + '  ' + $onde)
    if (-not $ok) { $faltando += 'CMake 3.22 ou mais novo' }
} else { Linha 'CMake >= 3.22' $false ''; $faltando += 'CMake 3.22 ou mais novo' }

# Ninja (opcional)
$ninja = Get-Command ninja
Linha 'Ninja (opcional)' ($null -ne $ninja) ($(if ($ninja) { 'versao ' + (ninja --version) } else { 'nao e obrigatorio' }))

# Python (opcional, para scripts de medicao locais)
$py = Get-Command python
$pyVer = $null
if ($py) { $pyVer = (python --version 2>&1) }
$pyReal = ($py -and ($pyVer -match 'Python 3'))
Linha 'Python 3 (opcional)' $pyReal ($(if ($pyReal) { $pyVer } else { 'nao e obrigatorio; as medicoes podem rodar do meu lado' }))

# pluginval
$pv = Get-Command pluginval
Linha 'pluginval' ($null -ne $pv) ($(if ($pv) { $pv.Source } else { 'pode ser baixado depois (binario pronto no GitHub)' }))
if (-not $pv) { $faltando += 'pluginval (so na etapa a)' }

Write-Host ''
Write-Host '=== Audio ==='

$vst3 = Join-Path $env:CommonProgramFiles 'VST3'
Linha 'Pasta VST3 do sistema' (Test-Path $vst3) $vst3
$vst3User = Join-Path $env:LOCALAPPDATA 'Programs\Common\VST3'
Linha 'Pasta VST3 do usuario (opcional)' (Test-Path $vst3User) $vst3User

$ableton = @()
foreach ($base in @($env:ProgramData, $env:ProgramFiles)) {
    if ($base) { $ableton += Get-ChildItem (Join-Path $base 'Ableton') -Directory | Where-Object { $_.Name -like 'Live*' } }
}
if ($ableton.Count -gt 0) { Linha 'Ableton Live' $true (($ableton | ForEach-Object { $_.Name } | Select-Object -Unique) -join ', ') }
else { Linha 'Ableton Live' $false 'nao achei em ProgramData/Program Files (pode estar em outro lugar)' }

Write-Host ''
Write-Host '=== Resumo ==='
if ($faltando.Count -eq 0) { Write-Host 'Nada faltando para a etapa (a).' }
else {
    Write-Host 'Faltando:'
    $faltando | Select-Object -Unique | ForEach-Object { Write-Host ('  - ' + $_) }
}
Write-Host 'Nenhuma instalacao foi feita.'
