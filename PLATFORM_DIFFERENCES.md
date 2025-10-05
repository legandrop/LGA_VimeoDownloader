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
- Checks for `yt-dlp.exe` in the `tools/` subdirectory relative to the application executable
- Uses `QFile::exists()` to verify file presence

### Installation Method
- **Install**: Downloads `yt-dlp.exe` from GitHub releases to `tools/` subdirectory
- **Update**: Downloads latest `yt-dlp.exe` and replaces existing file in `tools/` subdirectory
- Downloads from: `https://github.com/yt-dlp/yt-dlp/releases/latest/download/yt-dlp.exe`
- Automatically creates `tools/` directory if it doesn't exist

### Requirements
- Internet connection for download
- Write permissions in application directory and `tools/` subdirectory
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
| Detection | System PATH | tools/ subdirectory | Not implemented |
| Installation | Homebrew | GitHub download to tools/ | Not implemented |
| Updates | Homebrew | GitHub download to tools/ | Not implemented |
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
   - [ ] Downloaded yt-dlp.exe works with video downloads from tools/ subdirectory
   - [ ] Correct executable path is used for downloads (tools/yt-dlp.exe)

---

## Notes for Developers

- Windows implementation uses Qt's networking classes (`QNetworkAccessManager`)
- File operations use Qt's cross-platform file handling
- Error handling includes both network and file system errors
- Progress reporting keeps user informed during downloads
- All Windows-specific code is wrapped in `#ifdef Q_OS_WIN` blocks
- Tools are stored in `tools/` subdirectory for better organization and portability

## YouTube Support Requirements

### Why ffmpeg is Required for YouTube

YouTube videos often come in separate audio and video streams that need to be merged:
- **Video Stream**: Contains video without audio
- **Audio Stream**: Contains audio without video
- **ffmpeg**: Merges these streams into a single playable file

### Platform-Specific YouTube Support

| Platform | yt-dlp | ffmpeg | YouTube Support |
|----------|--------|--------|-----------------|
| macOS | ✅ Auto | ✅ Auto | ✅ Full |
| Windows | ✅ Auto | ❌ Manual | ⚠️ Partial |
| Linux | ❌ Manual | ❌ Manual | ⚠️ Manual |
