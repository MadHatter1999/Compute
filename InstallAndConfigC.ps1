# Define variables for download
$Msys2RepoUrl = "https://github.com/msys2/msys2-installer.git"
$ClonePath = "$env:TEMP\msys2-installer"

# Ensure Git is installed
if (-Not (Get-Command git -ErrorAction SilentlyContinue)) {
    Write-Host "Git is not installed. Please install Git to proceed." -ForegroundColor Red
    exit 1
}

# Clone the MSYS2 installer repository
Write-Host "Cloning the MSYS2 installer repository..." -ForegroundColor Green
if (Test-Path $ClonePath) {
    Remove-Item -Recurse -Force $ClonePath
}

git clone $Msys2RepoUrl $ClonePath

# Check if the repository was cloned successfully
if (-Not (Test-Path $ClonePath)) {
    Write-Host "Failed to clone the MSYS2 repository. Exiting." -ForegroundColor Red
    exit 1
}

Write-Host "Running MSYS2 installer..." -ForegroundColor Green
Start-Process -FilePath "$ClonePath\msys2.exe" -ArgumentList "/S" -Wait

# Path to the MSYS2 root directory (default installation location)
$Msys2Root = "C:\msys64"

# Check if MSYS2 was installed
if (-Not (Test-Path $Msys2Root)) {
    Write-Host "MSYS2 installation failed. Please check the installer log." -ForegroundColor Red
    exit 1
}

# Update MSYS2 and install GCC
Write-Host "Updating MSYS2 and installing GCC..." -ForegroundColor Green
$Msys2Shell = "$Msys2Root\usr\bin\bash.exe"

# Run commands in MSYS2 shell
& $Msys2Shell --login -c "pacman -Syuu --noconfirm && pacman -S --noconfirm base-devel gcc"

# Add MSYS2 to the system PATH
$EnvPath = [System.Environment]::GetEnvironmentVariable("Path", [System.EnvironmentVariableTarget]::Machine)
if (-Not ($EnvPath -match [regex]::Escape("$Msys2Root\\usr\\bin"))) {
    [System.Environment]::SetEnvironmentVariable("Path", "$EnvPath;$Msys2Root\\usr\\bin", [System.EnvironmentVariableTarget]::Machine)
    Write-Host "Added MSYS2 to system PATH." -ForegroundColor Green
}

Write-Host "GCC installation completed. Open a new terminal to use GCC." -ForegroundColor Green