param(
    [string]$JdkPath = ''
)

# Helper script to set JAVA_HOME (temporarily) and run the Android Gradle wrapper to clean+build.
# Usage:
#   .\build.ps1 -JdkPath 'C:\Program Files\Java\jdk-17.0.8'
# or just
#   .\build.ps1

function Fail([string]$msg) {
    Write-Error $msg
    exit 1
}

# If a JDK path was provided, validate and set it for this session
if ($JdkPath -ne '') {
    if (-not (Test-Path $JdkPath)) { Fail "Provided JDK path not found: $JdkPath" }
    $env:JAVA_HOME = $JdkPath
    $env:Path = "$($env:JAVA_HOME)\bin;$env:Path"
    Write-Host "Set JAVA_HOME=$env:JAVA_HOME for this session"
} else {
    # If java is already available, keep it
    try {
        java -version 2>$null
        Write-Host "java found on PATH"
    } catch {
        # Try a few common paths (Android Studio JBR, typical JDK locations)
        $candidates = @(
            'C:\Program Files\Android\Android Studio\jbr',
            'C:\Program Files\Java\jdk-17',
            'C:\Program Files\Java\jdk-17.0.8',
            'C:\Program Files\Java\jdk-11'
        )
        $found = $candidates | Where-Object { Test-Path $_ } | Select-Object -First 1
        if ($found) {
            $env:JAVA_HOME = $found
            $env:Path = "$($env:JAVA_HOME)\bin;$env:Path"
            Write-Host "Auto-set JAVA_HOME=$env:JAVA_HOME for this session"
        } else {
            Fail "No java executable found. Install a JDK or re-run this script with -JdkPath <path>"
        }
    }
}

$gradlew = Join-Path $PSScriptRoot 'gradlew.bat'
if (-not (Test-Path $gradlew)) { Fail "gradlew.bat not found at $gradlew" }

# Stop Gradle daemons
& $gradlew --stop

# Remove native and build intermediates to force a fresh configure
Remove-Item -Recurse -Force (Join-Path $PSScriptRoot 'app\.cxx') -ErrorAction SilentlyContinue
Remove-Item -Recurse -Force (Join-Path $PSScriptRoot 'app\build') -ErrorAction SilentlyContinue

# Run the build with AndroidX flags enforced
$buildArgs = @('clean', 'assembleDebug', '-Pandroid.useAndroidX=true', '-Pandroid.enableJetifier=true', '--no-daemon', '--refresh-dependencies', '--stacktrace')
Write-Host "Running: $gradlew $($buildArgs -join ' ') from directory: $PSScriptRoot"

# Change current directory to the android project folder so Gradle finds settings.gradle there
Push-Location $PSScriptRoot
try {
    # Execute the gradlew with splatted arguments so PowerShell handles quoting correctly
    & $gradlew @buildArgs
    if ($LASTEXITCODE -ne 0) { Fail "Gradle build failed (exit $LASTEXITCODE)" }
    Write-Host "Build finished successfully."
} finally {
    Pop-Location
}