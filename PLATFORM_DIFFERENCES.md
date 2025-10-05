# Platform Differences - VimeoDownloader

This document explains the differences in yt-dlp and ffmpeg installation and management between different operating systems.

## macOS Implementation ✅ (Fully Implemented)

### Detection Method
- **Primary**: Checks for binaries in `toolsmac/` subdirectory within the application bundle
- **Fallback**: Checks common Homebrew locations (`/opt/homebrew/bin/`, `/usr/local/bin/`)
- **Final Fallback**: Uses system PATH

### Installation Method
- **Install**: Downloads binaries directly from GitHub/evermeet.cx to `toolsmac/` subdirectory
- **Update**: Downloads latest yt-dlp and replaces existing file (ffmpeg not updated)
- **yt-dlp**: `https://github.com/yt-dlp/yt-dlp/releases/latest/download/yt-dlp_macos`
- **ffmpeg**: `https://evermeet.cx/ffmpeg/getrelease/zip` (extracted automatically)

### Requirements
- Internet connection for download
- Write permissions in application bundle
- No external dependencies (self-contained)

### Status
- ✅ Fully implemented and tested
- ✅ Detection working (local binaries prioritized)
- ✅ Installation working (direct download)
- ✅ Updates working (yt-dlp only)
- ✅ Completely portable (no Homebrew required)

---

## Windows Implementation ✅ (Fully Implemented)

### Detection Method
- Checks for `yt-dlp.exe` and `ffmpeg.exe` in the `tools/` subdirectory relative to the application executable
- Uses `QFile::exists()` to verify file presence

### Installation Method
- **Install**: Downloads `yt-dlp.exe` from GitHub releases to `tools/` subdirectory
- **Update**: Downloads latest `yt-dlp.exe` and replaces existing file (ffmpeg not updated)
- **yt-dlp**: `https://github.com/yt-dlp/yt-dlp/releases/latest/download/yt-dlp.exe`
- **ffmpeg**: Manual placement in `tools/` directory (not auto-downloaded)
- Automatically creates `tools/` directory if it doesn't exist

### Requirements
- Internet connection for yt-dlp download
- Write permissions in application directory and `tools/` subdirectory
- Manual ffmpeg placement for full YouTube support
- No external dependencies (self-contained)

### Status
- ✅ Fully implemented
- ✅ Detection logic working
- ✅ Download logic working
- ✅ File replacement logic working
- ✅ Completely portable

---

## Linux/Other Platforms ❌ (Not Implemented)

### Status
- ❌ Not implemented
- Shows placeholder message
- Suggests manual installation via package managers

### Future Implementation
Could use similar approaches to macOS/Windows:
- Package managers (apt, yum, pacman, etc.)
- Direct binary download
- AppImage or Flatpak distribution

---

## Key Differences Summary

| Feature | macOS | Windows | Linux |
|---------|-------|---------|-------|
| Detection | toolsmac/ → Homebrew → PATH | tools/ subdirectory | Not implemented |
| Installation | GitHub/evermeet.cx → toolsmac/ | GitHub → tools/ | Not implemented |
| Updates | yt-dlp auto, ffmpeg manual | yt-dlp auto, ffmpeg manual | Not implemented |
| Dependencies | None (self-contained) | None (self-contained) | Not implemented |
| Status | ✅ Fully working | ✅ Fully working | ❌ Not implemented |

---

## Binary Locations

### macOS
```
VimeoDownloader.app/
└── Contents/
    └── MacOS/
        ├── VimeoDownloader
        └── toolsmac/
            ├── yt-dlp
            └── ffmpeg
```

### Windows
```
VimeoDownloader/
├── VimeoDownloader.exe
└── tools/
    ├── yt-dlp.exe
    ├── ffmpeg.exe
    └── [ffmpeg DLLs]
```

---

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
| Windows | ✅ Auto | ⚠️ Manual | ✅ Full (with manual ffmpeg) |
| Linux | ❌ Manual | ❌ Manual | ⚠️ Manual |

---

## Installation Process Flow

### macOS
1. Check `toolsmac/yt-dlp` and `toolsmac/ffmpeg`
2. If missing, download from GitHub/evermeet.cx
3. Extract and set executable permissions
4. Ready to use (completely self-contained)

### Windows
1. Check `tools/yt-dlp.exe` and `tools/ffmpeg.exe`
2. If yt-dlp missing, download from GitHub
3. If ffmpeg missing, show manual installation message
4. Ready to use when both present

### Linux
1. Check system PATH for yt-dlp and ffmpeg
2. Show manual installation instructions
3. User must install via package manager

---

## Update Process

### Both macOS and Windows
- **yt-dlp**: Automatically downloaded and replaced when "Update Tools" is clicked
- **ffmpeg**: Not automatically updated (stable, less frequent updates needed)
- **Process**: Download → Replace → Verify → Ready

---

## Notes for Developers

- All platform-specific code is wrapped in appropriate `#ifdef` blocks
- Qt's networking classes (`QNetworkAccessManager`) handle downloads
- File operations use Qt's cross-platform file handling
- Error handling includes both network and file system errors
- Progress reporting keeps user informed during downloads
- Tools are stored in platform-specific subdirectories for organization
- Both platforms now support completely portable, self-contained operation