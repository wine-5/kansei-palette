# Web ビルド用のヘルパー(Windows / PowerShell)。VS Code の F5 からも呼ばれる。
#
#   powershell -ExecutionPolicy Bypass -File tools/web.ps1 build     # build-web/web/index.html を生成
#   powershell -ExecutionPolicy Bypass -File tools/web.ps1 serve     # http://localhost:8080 で配信(Ctrl+C で停止)
#   powershell -ExecutionPolicy Bypass -File tools/web.ps1 start     # 配信を裏で起動してすぐ戻る(F5 用)
#   powershell -ExecutionPolicy Bypass -File tools/web.ps1 stop      # 裏で起動した配信を止める
#   powershell -ExecutionPolicy Bypass -File tools/web.ps1 package   # itch.io 提出用 zip を生成
#
# emsdk の場所は環境変数 EMSDK、無ければ C:\emsdk を使う。
param(
    [Parameter(Mandatory = $true)][ValidateSet('build', 'serve', 'start', 'stop', 'package')][string]$Action,
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

$serverArgs = @('-u', (Join-Path $PSScriptRoot 'serve_web.py'), (Join-Path $buildDir 'web'), $Port)

function Test-ServerUp {
    # Windows は閉じたポートへの接続失敗に約 2 秒かかるため、200ms で打ち切る
    $client = New-Object Net.Sockets.TcpClient
    try { return $client.ConnectAsync('127.0.0.1', $Port).Wait(200) } catch { return $false } finally { $client.Close() }
}

if ($Action -eq 'serve') {
    & $env:EMSDK_PYTHON @serverArgs
    exit $LASTEXITCODE
}

if ($Action -eq 'start') {
    # サーバーを裏で起動してすぐ戻る(F5 用。起動済みなら何もしない)
    if (-not (Test-ServerUp)) {
        Start-Process -FilePath $env:EMSDK_PYTHON -ArgumentList $serverArgs -WindowStyle Hidden
        for ($i = 0; $i -lt 50 -and -not (Test-ServerUp); $i++) { Start-Sleep -Milliseconds 100 }
        if (-not (Test-ServerUp)) { throw "サーバーを起動できませんでした (port $Port)" }
    }
    Write-Host "Server running: http://localhost:$Port/"
    exit 0
}

if ($Action -eq 'stop') {
    Get-CimInstance Win32_Process -Filter "name='python.exe'" |
        Where-Object { $_.CommandLine -match 'serve_web\.py' } |
        ForEach-Object { Stop-Process -Id $_.ProcessId -Force; Write-Host "Stopped server (pid $($_.ProcessId))" }
    exit 0
}

# cmake が見つからないと、以降のコマンドが失敗しても終了コード 0 のまま終わってしまうので先に確かめる
if (-not (Get-Command cmake -ErrorAction SilentlyContinue)) {
    throw 'cmake が見つかりません。CMake をインストールし、PATH に追加してください'
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
