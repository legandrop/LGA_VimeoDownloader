# Platform Differences - VimeoDownloader

This document explains the differences in yt-dlp installation and management between different operating systems.

## macOS Implementation ✅ (Tested)

### Detection Method
- Uses system PATH to detect yt-dlp installation
- Runs `yt-dlp --version` to verify installation

### Installation Method
- **Install**: `brew install yt-dlp`
- **Update**: `brew upgrade yt-dlp`
- Requires Homebrew to be installed

### Requirements
- Homebrew package manager
- Internet connection for installation/updates

### Status
- ✅ Fully implemented and tested
- ✅ Detection working
- ✅ Installation working
- ✅ Updates working

---

## Windows Implementation ⚠️ (Not Tested Yet)

### Detection Method
- Checks for `yt-dlp.exe` in the same directory as the application executable
- Uses `QFile::exists()` to verify file presence

### Installation Method
- **Install**: Downloads `yt-dlp.exe` from GitHub releases
- **Update**: Downloads latest `yt-dlp.exe` and replaces existing file
- Downloads from: `https://github.com/yt-dlp/yt-dlp/releases/latest/download/yt-dlp.exe`

### Requirements
- Internet connection for download
- Write permissions in application directory
- No external dependencies (self-contained)

### Status
- ⚠️ Implemented but **NOT TESTED**
- ❓ Detection logic implemented
- ❓ Download logic implemented
- ❓ File replacement logic implemented

### Testing Needed
- [ ] Verify yt-dlp.exe detection works correctly
- [ ] Test download functionality
- [ ] Test file replacement for updates
- [ ] Verify downloaded executable works
- [ ] Test error handling for network issues
- [ ] Test error handling for permission issues

---

## Linux/Other Platforms ❌ (Not Implemented)

### Status
- ❌ Not implemented
- Shows placeholder message
- Suggests manual installation

### Future Implementation
Could use similar approaches to macOS:
- Package managers (apt, yum, pacman, etc.)
- pip installation
- Direct binary download

---

## Key Differences Summary

| Feature | macOS | Windows | Linux |
|---------|-------|---------|-------|
| Detection | System PATH | Local exe file | Not implemented |
| Installation | Homebrew | GitHub download | Not implemented |
| Updates | Homebrew | GitHub download | Not implemented |
| Dependencies | Homebrew | None | Not implemented |
| Status | ✅ Tested | ⚠️ Needs testing | ❌ Not implemented |

---

## Testing Checklist for Windows

When testing on Windows, verify:

1. **Initial Detection**
   - [ ] App correctly detects missing yt-dlp.exe
   - [ ] Install button appears in red
   - [ ] Correct log messages appear

2. **Installation Process**
   - [ ] Download starts successfully
   - [ ] Progress messages appear in log
   - [ ] File is saved to correct location
   - [ ] Permissions are set correctly
   - [ ] Detection updates after installation

3. **Update Process**
   - [ ] Update button appears when yt-dlp.exe exists
   - [ ] Old file is replaced successfully
   - [ ] No permission errors occur

4. **Error Handling**
   - [ ] Network errors are handled gracefully
   - [ ] Permission errors show helpful messages
   - [ ] UI remains responsive during download

5. **Integration**
   - [ ] Downloaded yt-dlp.exe works with video downloads
   - [ ] Correct executable path is used for downloads

---

## Notes for Developers

- Windows implementation uses Qt's networking classes (`QNetworkAccessManager`)
- File operations use Qt's cross-platform file handling
- Error handling includes both network and file system errors
- Progress reporting keeps user informed during downloads
- All Windows-specific code is wrapped in `#ifdef Q_OS_WIN` blocks
