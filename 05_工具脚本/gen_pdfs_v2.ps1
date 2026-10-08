$ErrorActionPreference = 'Stop'
$base = "C:\天津华铁科为车载式铁路周边环境巡检"
$edge = "${env:ProgramFiles(x86)}\Microsoft\Edge\Application\msedge.exe"

if (-not (Test-Path $edge)) {
    $edge = "${env:ProgramFiles}\Microsoft\Edge\Application\msedge.exe"
}
if (-not (Test-Path $edge)) {
    Write-Error "Edge not found"
    exit 1
}

$jobs = @(
    @{ name = "项目研发规划.pdf";                   src = "项目研发规划.html" },
    @{ name = "项目研发规划_V6.pdf";                src = "项目研发规划_V6.html" },
    @{ name = "项目研发规划_V6_printable.pdf";     src = "项目研发规划_V6_printable.html" },
    @{ name = "项目研发规划_V5.pdf";                src = "项目研发规划_V5.html" },
    @{ name = "项目研发规划_V5_printable.pdf";     src = "项目研发规划_V5_printable.html" },
    @{ name = "项目研发规划_V4.1.pdf";              src = "项目研发规划_V4.1.html" },
    @{ name = "项目研发规划_V4.1_printable.pdf";   src = "项目研发规划_V4.1_printable.html" },
    @{ name = "项目研发规划_V4.00.pdf";             src = "项目研发规划_V4.00.html" },
    @{ name = "项目研发规划_V4.00_printable.pdf";  src = "项目研发规划_V4.00_printable.html" },
    @{ name = "项目研发规划_V1.00.pdf";             src = "项目研发规划_V1.00_printable.html" },
    @{ name = "预算汇总_v3.00.pdf";                 src = "预算汇总_v3.00.html" },
    @{ name = "招聘要求_V6.pdf";                    src = "招聘要求_V6.html" }
)

foreach ($j in $jobs) {
    $subDir = Join-Path $base $j.subdir
    $srcPath = Join-Path $subDir $j.src
    $pdfPath = Join-Path $subDir $j.name
    if (-not (Test-Path $srcPath)) {
        Write-Warning "Missing source: $srcPath"
        continue
    }
    $url = "file:///$($srcPath -replace '\\','/')"
    Write-Host "Generating $($j.name) from $($j.src) ..."
    if (Test-Path $pdfPath) { Remove-Item $pdfPath -Force }
    $proc = Start-Process -FilePath $edge `
        -ArgumentList "--headless","--disable-gpu","--no-sandbox","--no-pdf-header-footer","--print-to-pdf=`"$pdfPath`"",$url `
        -PassThru -Wait
    if (Test-Path $pdfPath) {
        $size = (Get-Item $pdfPath).Length
        Write-Host "  OK $($j.name)  generated  size=$size bytes"
    } else {
        Write-Error "  FAIL $($j.name)  no output file"
    }
}

Write-Host "Done."
