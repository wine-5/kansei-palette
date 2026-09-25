# Web ビルド用のヘルパー(Windows / PowerShell)。VS Code の F5 からも呼ばれる。
#
#   powershell -ExecutionPolicy Bypass -File tools/web.ps1 build     # build-web/web/index.html を生成
#   powershell -ExecutionPolicy Bypass -File tools/web.ps1 serve     # http://localhost:8080 で配信
#   powershell -ExecutionPolicy Bypass -File tools/web.ps1 package   # itch.io 提出用 zip を生成
#
# emsdk の場所は環境変数 EMSDK、無ければ C:\emsdk を使う。
param(
    [Parameter(Mandatory = $true)][ValidateSet('build', 'serve', 'package')][string]$Action,
    [int]$Port = 8080
)
# 注意: $ErrorActionPreference = 'Stop' にすると emsdk や emcc の標準エラー出力(警告)で止まるので使わない
$root = Split-Path -Parent $PSScriptRoot
$buildDir = Join-Path $root 'build-web'

# --- emsdk の環境変数(emcc の PATH、EMSDK_PYTHON など)を読み込む ---
$emsdk = if ($env:EMSDK) { $env:EMSDK } else { 'C:\emsdk' }
if (-not (Test-Path "$emsdk\emsdk_env.ps1")) {
    throw "emsdk が見つかりません: $emsdk (環境変数 EMSDK に emsdk のフォルダを設定してください)"
}
$env:EMSDK_QUIET = '1'
& "$emsdk\emsdk_env.ps1" | Out-Null

if ($Action -eq 'serve') {
    & $env:EMSDK_PYTHON -u (Join-Path $PSScriptRoot 'serve_web.py') (Join-Path $buildDir 'web') $Port
    exit $LASTEXITCODE
}

# --- Ninja を探す(無ければ Visual Studio 同梱のものを使う) ---
if (-not (Get-Command ninja -ErrorAction SilentlyContinue)) {
    $vswhere = "${env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer\vswhere.exe"
    $vsPath = if (Test-Path $vswhere) { & $vswhere -latest -property installationPath } else { $null }
    $vsNinja = "$vsPath\Common7\IDE\CommonExtensions\Microsoft\CMake\Ninja"
    if ($vsPath -and (Test-Path "$vsNinja\ninja.exe")) {
        $env:PATH = "$vsNinja;$env:PATH"
    } else {
        throw 'ninja が見つかりません。winget install Ninja-build.Ninja でインストールしてください'
    }
}

if (-not (Test-Path (Join-Path $buildDir 'CMakeCache.txt'))) {
    emcmake cmake -S $root -B $buildDir -G Ninja -DCMAKE_BUILD_TYPE=Release
    if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }
}

$target = if ($Action -eq 'package') { 'package_web' } else { 'all' }
cmake --build $buildDir --target $target
exit $LASTEXITCODE
