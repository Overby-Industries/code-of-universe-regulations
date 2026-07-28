<#
.SYNOPSIS
    Bump the corpus version across every document in a Rights for All Life or
    Code of Universe Regulations repository.

.DESCRIPTION
    Inter-Bill Relationship Framework section 5.2 rule 4 requires that all bills
    and supporting documents carry the same version number. Applied by hand that
    rule fails quietly: through v1.5.2 the corpus was released as 1.5.2 while
    every document still read 1.5.1, and the document index pointed at filenames
    with old version numbers embedded in them.

    This script performs the mechanical part of a release.

    WHAT IT CHANGES
      - "**Version:** X-Official-Evergreen" header lines, with or without a
        leading list marker
      - The shields.io version badge in README.md
      - Version cells in DOCUMENT_INDEX.md tables

    WHAT IT DELIBERATELY LEAVES ALONE
      - Anything under archive/. Archived documents record what a version
        actually said at the time. Moving their version numbers forward would
        make the archive lie about its own contents.
      - Anything with a status suffix other than -Official-Evergreen. CUR titles
        carry their own -Draft versions that track the drafting state of an
        individual Part and have nothing to do with the corpus release. Bumping
        those would destroy real information.
      - CHANGELOG.md, which is written by hand because it has to be.
      - Prose mentioning a version historically ("superseded in v1.5.2").
        Those references are correct as written and must not move.

.PARAMETER Version
    Target version, without the status suffix. Example: 1.6.0

.PARAMETER Root
    Repository root. Defaults to the parent of the directory holding this script.

.PARAMETER DryRun
    Report what would change without writing anything.

.EXAMPLE
    ./process/bump-version.ps1 -Version 1.6.0 -DryRun
    ./process/bump-version.ps1 -Version 1.6.0
    ./process/bump-version.ps1 -Version 1.6.0 -Root "C:/path/to/code-of-universe-regulations"
#>

param(
    [Parameter(Mandatory = $true)][string]$Version,
    [string]$Root,
    [switch]$DryRun
)

$ErrorActionPreference = 'Stop'

if (-not $Root) { $Root = Split-Path -Parent $PSScriptRoot }
if (-not (Test-Path $Root)) { throw "Root not found: $Root" }

if ($Version -notmatch '^\d+\.\d+\.\d+$') {
    throw "Version must be Major.Minor.Patch, for example 1.6.0. Got: $Version"
}

$status  = 'Official-Evergreen'
$target  = "$Version-$status"
$changed = @()
$skipped = @()

function Write-IfChanged([string]$Path, [string]$Original, [string]$New, [string]$What) {
    if ($Original -eq $New) { return $false }
    if (-not $DryRun) {
        # UTF8 without BOM, LF preserved as-is from the original content.
        [System.IO.File]::WriteAllText($Path, $New, (New-Object System.Text.UTF8Encoding $false))
    }
    $script:changed += [pscustomobject]@{
        File   = (Resolve-Path -Relative $Path)
        Change = $What
    }
    return $true
}

# --- 1. Document version headers ------------------------------------------
# Matches "**Version:** 1.5.1-Official-Evergreen" and the list-item form
# "- **Version:** 1.5.1-Official-Evergreen". Only -Official-Evergreen is
# touched; see the note above on why -Draft is left alone.
$versionPattern = '(?m)^(?<lead>[-*]\s*)?\*\*Version:\*\*\s*(?<ver>\d+\.\d+\.\d+)-Official-Evergreen\s*$'

Get-ChildItem -Path $Root -Filter *.md -Recurse -File |
    Where-Object { $_.FullName -notmatch '[\\/](\.git|node_modules|build|archive)[\\/]' } |
    ForEach-Object {
        $path = $_.FullName
        $text = [System.IO.File]::ReadAllText($path)

        if ($text -notmatch $versionPattern) {
            # Record documents that carry a version we are choosing not to move,
            # so a release never silently leaves something behind.
            if ($text -match '(?m)^([-*]\s*)?\*\*Version:\*\*\s*(?<v>\S+)\s*$') {
                $script:skipped += [pscustomobject]@{
                    File    = (Resolve-Path -Relative $path)
                    Version = $Matches['v']
                }
            }
            return
        }

        $new = [regex]::Replace($text, $versionPattern, {
            param($m)
            $lead = $m.Groups['lead'].Value
            "$lead**Version:** $target"
        })

        [void](Write-IfChanged -Path $path -Original $text -New $new -What "version header -> $target")
    }

# --- 2. README badge -------------------------------------------------------
# shields.io escapes a literal hyphen as "--", so 1.5.2-Official-Evergreen is
# written version-1.5.2--Official--Evergreen.
#
# Matched case-insensitively, and the label's own case is preserved through the
# capture group. The two repositories spell it differently — RFAL's README uses
# "version-", CUR's uses "Version-" — and a case-sensitive pattern updated one
# and silently skipped the other. That is exactly the failure this script exists
# to prevent, so it is matched loosely and rewritten faithfully.
$readme = Join-Path $Root 'README.md'
if (Test-Path $readme) {
    $text = [System.IO.File]::ReadAllText($readme)
    $badge = '${1}' + $Version + '--' + ($status -replace '-', '--')
    $new = [regex]::Replace($text,
        '(?i)(version-)\d+\.\d+\.\d+--Official--Evergreen', $badge)
    [void](Write-IfChanged -Path $readme -Original $text -New $new -What "README badge -> $Version")
}

# --- 3. VERSION.md corpus line ---------------------------------------------
# CUR's VERSION.md opens by stating the current corpus version in prose. It is
# the authoritative statement of that number, so leaving it behind would mean
# the file documenting the versioning convention was the one document out of
# date. Harmless where the line does not exist.
$versionFile = Join-Path $Root 'VERSION.md'
if (Test-Path $versionFile) {
    $text = [System.IO.File]::ReadAllText($versionFile)
    $new = [regex]::Replace($text,
        '(?im)^(\*\*Current corpus version:\s*)\d+\.\d+\.\d+-\S+?(\*\*)\s*$',
        ('${1}' + $target + '${2}'))
    [void](Write-IfChanged -Path $versionFile -Original $text -New $new -What "corpus version line -> $target")
}

# --- 4. DOCUMENT_INDEX version column -------------------------------------
# Table cells holding a bare version, e.g. "| 1.5.1 |".
Get-ChildItem -Path $Root -Filter 'DOCUMENT_INDEX.md' -Recurse -File | ForEach-Object {
    $path = $_.FullName
    $text = [System.IO.File]::ReadAllText($path)
    $new = [regex]::Replace($text, '(?m)(\|\s*)\d+\.\d+\.\d+(\s*\|)', ('${1}' + $Version + '${2}'))
    [void](Write-IfChanged -Path $path -Original $text -New $new -What "index version column -> $Version")
}

# --- Report ----------------------------------------------------------------
$mode = if ($DryRun) { 'DRY RUN - nothing written' } else { 'applied' }
Write-Output ""
Write-Output "Target: $target   ($mode)"
Write-Output "Root:   $Root"
Write-Output ""

if ($changed.Count -eq 0) {
    Write-Output "No changes. Every document already reads $target."
} else {
    Write-Output "Updated $($changed.Count) file(s):"
    $changed | ForEach-Object { Write-Output ("  {0,-62} {1}" -f $_.File, $_.Change) }
}

if ($skipped.Count -gt 0) {
    Write-Output ""
    Write-Output "Left alone ($($skipped.Count)) - non-Evergreen status, by design:"
    $skipped | ForEach-Object { Write-Output ("  {0,-62} {1}" -f $_.File, $_.Version) }
}

Write-Output ""
Write-Output "CHANGELOG.md is not touched. Write the entry by hand."
