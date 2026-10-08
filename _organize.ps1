$base = "C:\天津华铁科为车载式铁路周边环境巡检"

$plan = [ordered]@{
    "02_项目研发规划" = @(
        "项目研发规划.html",
        "项目研发规划.pdf",
        "项目研发规划_V1.00.md",
        "项目研发规划_V1.00.pdf",
        "项目研发规划_V1.00_printable.html",
        "项目研发规划_V4.00.html",
        "项目研发规划_V4.00.md",
        "项目研发规划_V4.00.pdf",
        "项目研发规划_V4.00_printable.html",
        "项目研发规划_V4.00_printable.pdf",
        "项目研发规划_V4.1.html",
        "项目研发规划_V4.1.md",
        "项目研发规划_V4.1.pdf",
        "项目研发规划_V4.1_printable.html",
        "项目研发规划_V4.1_printable.pdf",
        "项目研发规划_V5.html",
        "项目研发规划_V5.md",
        "项目研发规划_V5.pdf",
        "项目研发规划_V5_printable.html",
        "项目研发规划_V5_printable.pdf",
        "项目研发规划_V6.html",
        "项目研发规划_V6.md",
        "项目研发规划_V6.pdf",
        "项目研发规划_V6_printable.html",
        "项目研发规划_V6_printable.pdf"
    )
    "03_预算汇总" = @(
        "预算汇总.html",
        "预算汇总_v2.00.html",
        "预算汇总_v3.00.html",
        "预算汇总_v3.00.pdf"
    )
    "04_招聘公告" = @(
        "招聘要求_V6.html",
        "招聘要求_V6.pdf"
    )
    "05_工具脚本" = @(
        "gen_pdfs_v2.ps1",
        "_render_arch_v6.py"
    )
    "06_参考资源" = @(
        "BOM清单.md",
        "价格清单.md"
    )
    "07_历史归档" = @(
        "对话记录_2026-09-08_152.3万预算版.jsonl",
        "对话记录_2026-09-08_152.3万预算版_摘要.md"
    )
    "08_系统临时" = @(
        "bash.exe.stackdump"
    )
}

foreach ($d in $plan.Keys) {
    $dirPath = Join-Path $base $d
    if (-not (Test-Path $dirPath)) {
        New-Item -Path $dirPath -ItemType Directory -Force | Out-Null
        Write-Host "mkdir: $d/"
    }
    foreach ($f in $plan[$d]) {
        $src = Join-Path $base $f
        if (Test-Path $src) {
            Move-Item -LiteralPath $src -Destination $dirPath -Force
            Write-Host ("moved: " + $f + " -> " + $d + "/")
        } else {
            Write-Host ("skip (not found): " + $f)
        }
    }
}

$imagesSrc = Join-Path $base "images"
$imagesDst = Join-Path $base "06_参考资源\images"
if (Test-Path $imagesSrc) {
    Move-Item -LiteralPath $imagesSrc -Destination $imagesDst -Force
    Write-Host "moved: images/ -> 06_参考资源/images/"
}

$existingEmpty = Join-Path $base "整理"
if (Test-Path $existingEmpty) {
    $items = Get-ChildItem -LiteralPath $existingEmpty -ErrorAction SilentlyContinue
    if ($items.Count -eq 0) {
        Write-Host "kept: 整理/ (empty, kept)"
    }
}

Write-Host ""
Write-Host "Done."
