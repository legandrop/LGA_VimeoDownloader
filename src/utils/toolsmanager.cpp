#include "vimeodownloader/toolsmanager.h"

#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QNetworkRequest>
#include <QStandardPaths>
#include <QStyle>
#include <QTimer>

ToolsManager::ToolsManager(QTextEdit *logOutput, QPushButton *toolsButton, QObject *parent)
    : QObject(parent)
    , m_logOutput(logOutput)
    , m_toolsButton(toolsButton)
    , m_ytDlpInstalled(false)
    , m_ffmpegInstalled(false)
    , m_checkingTools(false)
    , m_networkManager(nullptr)
    , m_pendingProcesses(0)
{
    // Connect button signal
    connect(m_toolsButton, &QPushButton::clicked, this, &ToolsManager::onInstallUpdateClicked);
    
    // Initialize network manager
    m_networkManager = new QNetworkAccessManager(this);
}

ToolsManager::~ToolsManager()
{
    // Qt handles cleanup automatically
}

void ToolsManager::checkToolsInstallation()
{
    if (m_checkingTools) {
        return; // Already checking
    }
    
    m_checkingTools = true;
    m_pendingProcesses = 0;
    
    logMessage("Checking tools installation...");
    
    // Check both tools
    checkYtDlpInstallation();
    checkFfmpegInstallation();
}

void ToolsManager::checkYtDlpInstallation()
{
#ifdef Q_OS_WIN
    // Windows: Check if yt-dlp.exe exists in the tools subdirectory
    QString appDir = QCoreApplication::applicationDirPath();
    QString ytDlpPath = appDir + "/tools/yt-dlp.exe";
    
    if (QFile::exists(ytDlpPath)) {
        m_ytDlpInstalled = true;
        logMessage("✓ yt-dlp.exe found in tools directory");
    } else {
        m_ytDlpInstalled = false;
        logMessage("✗ yt-dlp.exe not found in tools directory");
    }
    
    // Check ffmpeg after yt-dlp check is done
    if (m_pendingProcesses == 0) {
        QTimer::singleShot(100, this, &ToolsManager::updateButtonState);
    }
    return;
#endif
    
#ifdef Q_OS_MAC
    // macOS: Check if yt-dlp exists in the toolsmac subdirectory first, then fallback to system
    QString appDir = QCoreApplication::applicationDirPath();
    QString ytDlpPath = appDir + "/toolsmac/yt-dlp";
    
    if (QFile::exists(ytDlpPath)) {
        m_ytDlpInstalled = true;
        logMessage("✓ yt-dlp found in toolsmac directory");
        
        // Check ffmpeg after yt-dlp check is done
        if (m_pendingProcesses == 0) {
            QTimer::singleShot(100, this, &ToolsManager::updateButtonState);
        }
        return;
    }
    
    // Fallback: Check in common Homebrew locations first, then PATH
    QStringList possiblePaths = {
        "/opt/homebrew/bin/yt-dlp",  // Apple Silicon Homebrew
        "/usr/local/bin/yt-dlp",    // Intel Homebrew
        "yt-dlp"                    // System PATH (fallback)
    };
    
    QString foundPath;
    for (const QString &path : possiblePaths) {
        if (path == "yt-dlp") {
            // Try PATH version
            break;
        } else if (QFile::exists(path)) {
            foundPath = path;
            break;
        }
    }
    
    if (!foundPath.isEmpty()) {
        // Found in Homebrew location, verify it works
        m_pendingProcesses++;
        QProcess *process = new QProcess(this);
        
        connect(process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
                [this, process, foundPath](int exitCode, QProcess::ExitStatus exitStatus) {
            
            if (exitStatus == QProcess::NormalExit && exitCode == 0) {
                m_ytDlpInstalled = true;
                logMessage(QString("✓ yt-dlp found at: %1").arg(foundPath));
            } else {
                m_ytDlpInstalled = false;
                logMessage("✗ yt-dlp found but not working properly");
            }
            
            m_pendingProcesses--;
            if (m_pendingProcesses == 0) {
                updateButtonState();
            }
            
            process->deleteLater();
        });
        
        process->start(foundPath, QStringList() << "--version");
        
        if (!process->waitForStarted(3000)) {
            m_ytDlpInstalled = false;
            logMessage("✗ yt-dlp found but failed to start");
            m_pendingProcesses--;
            if (m_pendingProcesses == 0) {
                updateButtonState();
            }
            process->deleteLater();
        }
    } else {
        // Fallback to PATH check
        m_pendingProcesses++;
        QProcess *process = new QProcess(this);
        
        connect(process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
                [this, process](int exitCode, QProcess::ExitStatus exitStatus) {
            
            if (exitStatus == QProcess::NormalExit && exitCode == 0) {
                m_ytDlpInstalled = true;
                logMessage("✓ yt-dlp is installed and available");
            } else {
                m_ytDlpInstalled = false;
                logMessage("✗ yt-dlp is not installed");
            }
            
            m_pendingProcesses--;
            if (m_pendingProcesses == 0) {
                updateButtonState();
            }
            
            process->deleteLater();
        });
        
        process->start("yt-dlp", QStringList() << "--version");
        
        if (!process->waitForStarted(3000)) {
            m_ytDlpInstalled = false;
            logMessage("✗ yt-dlp is not installed");
            m_pendingProcesses--;
            if (m_pendingProcesses == 0) {
                updateButtonState();
            }
            process->deleteLater();
        }
    }
#else
    // Linux: Check via PATH
    m_pendingProcesses++;
    QProcess *process = new QProcess(this);
    
    connect(process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            [this, process](int exitCode, QProcess::ExitStatus exitStatus) {
        
        if (exitStatus == QProcess::NormalExit && exitCode == 0) {
            m_ytDlpInstalled = true;
            logMessage("✓ yt-dlp is installed and available");
        } else {
            m_ytDlpInstalled = false;
            logMessage("✗ yt-dlp is not installed");
        }
        
        m_pendingProcesses--;
        if (m_pendingProcesses == 0) {
            updateButtonState();
        }
        
        process->deleteLater();
    });
    
    // Check if yt-dlp is available
    process->start("yt-dlp", QStringList() << "--version");
    
    if (!process->waitForStarted(3000)) {
        m_ytDlpInstalled = false;
        logMessage("✗ yt-dlp is not installed");
        m_pendingProcesses--;
        if (m_pendingProcesses == 0) {
            updateButtonState();
        }
        process->deleteLater();
    }
#endif
}

void ToolsManager::checkFfmpegInstallation()
{
#ifdef Q_OS_WIN
    // Windows: Check if ffmpeg.exe exists in the tools subdirectory
    QString appDir = QCoreApplication::applicationDirPath();
    QString ffmpegPath = appDir + "/tools/ffmpeg.exe";
    
    if (QFile::exists(ffmpegPath)) {
        m_ffmpegInstalled = true;
        logMessage("✓ ffmpeg.exe found in tools directory");
    } else {
        m_ffmpegInstalled = false;
        logMessage("✗ ffmpeg.exe not found in tools directory");
    }
    
    // Update button state after both checks are done
    if (m_pendingProcesses == 0) {
        QTimer::singleShot(100, this, &ToolsManager::updateButtonState);
    }
    return;
#endif
    
#ifdef Q_OS_MAC
    // macOS: Check if ffmpeg exists in the toolsmac subdirectory first, then fallback to system
    QString appDir = QCoreApplication::applicationDirPath();
    QString ffmpegPath = appDir + "/toolsmac/ffmpeg";
    
    if (QFile::exists(ffmpegPath)) {
        m_ffmpegInstalled = true;
        logMessage("✓ ffmpeg found in toolsmac directory");
        
        // Update button state after both checks are done
        if (m_pendingProcesses == 0) {
            QTimer::singleShot(100, this, &ToolsManager::updateButtonState);
        }
        return;
    }
    
    // Fallback: Check in common Homebrew locations first, then PATH
    QStringList possiblePaths = {
        "/opt/homebrew/bin/ffmpeg",  // Apple Silicon Homebrew
        "/usr/local/bin/ffmpeg",    // Intel Homebrew
        "ffmpeg"                    // System PATH (fallback)
    };
    
    QString foundPath;
    for (const QString &path : possiblePaths) {
        if (path == "ffmpeg") {
            // Try PATH version
            break;
        } else if (QFile::exists(path)) {
            foundPath = path;
            break;
        }
    }
    
    if (!foundPath.isEmpty()) {
        // Found in Homebrew location, verify it works
        m_pendingProcesses++;
        QProcess *process = new QProcess(this);
        
        connect(process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
                [this, process, foundPath](int exitCode, QProcess::ExitStatus exitStatus) {
            
            if (exitStatus == QProcess::NormalExit && exitCode == 0) {
                m_ffmpegInstalled = true;
                logMessage(QString("✓ ffmpeg found at: %1").arg(foundPath));
            } else {
                m_ffmpegInstalled = false;
                logMessage("✗ ffmpeg found but not working properly");
            }
            
            m_pendingProcesses--;
            if (m_pendingProcesses == 0) {
                updateButtonState();
            }
            
            process->deleteLater();
        });
        
        process->start(foundPath, QStringList() << "-version");
        
        if (!process->waitForStarted(3000)) {
            m_ffmpegInstalled = false;
            logMessage("✗ ffmpeg found but failed to start");
            m_pendingProcesses--;
            if (m_pendingProcesses == 0) {
                updateButtonState();
            }
            process->deleteLater();
        }
    } else {
        // Fallback to PATH check
        m_pendingProcesses++;
        QProcess *process = new QProcess(this);
        
        connect(process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
                [this, process](int exitCode, QProcess::ExitStatus exitStatus) {
            
            if (exitStatus == QProcess::NormalExit && exitCode == 0) {
                m_ffmpegInstalled = true;
                logMessage("✓ ffmpeg is installed and available");
            } else {
                m_ffmpegInstalled = false;
                logMessage("✗ ffmpeg is not installed");
            }
            
            m_pendingProcesses--;
            if (m_pendingProcesses == 0) {
                updateButtonState();
            }
            
            process->deleteLater();
        });
        
        process->start("ffmpeg", QStringList() << "-version");
        
        if (!process->waitForStarted(3000)) {
            m_ffmpegInstalled = false;
            logMessage("✗ ffmpeg is not installed");
            m_pendingProcesses--;
            if (m_pendingProcesses == 0) {
                updateButtonState();
            }
            process->deleteLater();
        }
    }
#else
    // Linux: Check via PATH
    m_pendingProcesses++;
    QProcess *process = new QProcess(this);
    
    connect(process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            [this, process](int exitCode, QProcess::ExitStatus exitStatus) {
        
        if (exitStatus == QProcess::NormalExit && exitCode == 0) {
            m_ffmpegInstalled = true;
            logMessage("✓ ffmpeg is installed and available");
        } else {
            m_ffmpegInstalled = false;
            logMessage("✗ ffmpeg is not installed");
        }
        
        m_pendingProcesses--;
        if (m_pendingProcesses == 0) {
            updateButtonState();
        }
        
        process->deleteLater();
    });
    
    // Check if ffmpeg is available
    process->start("ffmpeg", QStringList() << "-version");
    
    if (!process->waitForStarted(3000)) {
        m_ffmpegInstalled = false;
        logMessage("✗ ffmpeg is not installed");
        m_pendingProcesses--;
        if (m_pendingProcesses == 0) {
            updateButtonState();
        }
        process->deleteLater();
    }
#endif
}

void ToolsManager::updateButtonState()
{
    m_checkingTools = false;
    
    bool allInstalled = m_ytDlpInstalled && m_ffmpegInstalled;
    
    if (allInstalled) {
        setButtonText("Update Tools");
        setButtonStyle("");
    } else {
        setButtonText("Install Tools");
        setButtonStyle("danger");
    }
    
    setButtonEnabled(true);
    emit toolsStatusChanged(allInstalled);
}

bool ToolsManager::areToolsInstalled() const
{
    return m_ytDlpInstalled && m_ffmpegInstalled;
}

void ToolsManager::installOrUpdateTools()
{
    onInstallUpdateClicked();
}

void ToolsManager::onInstallUpdateClicked()
{
    setButtonEnabled(false);
    
    bool allInstalled = m_ytDlpInstalled && m_ffmpegInstalled;
    
#ifdef Q_OS_WIN
    // Windows: Download executables from GitHub
    if (allInstalled) {
        logMessage("=== Updating Tools ===");
        logMessage("Downloading latest yt-dlp.exe from GitHub...");
        // Note: ffmpeg is not updated on Windows - only downloaded once
    } else {
        logMessage("=== Installing Tools ===");
        if (!m_ytDlpInstalled) {
            logMessage("Downloading yt-dlp.exe from GitHub...");
        }
        if (!m_ffmpegInstalled) {
            logMessage("Downloading ffmpeg.exe from GitHub...");
        }
    }
    
    // Start downloads
    if (!m_ytDlpInstalled || allInstalled) {
        downloadYtDlpWindows();
    }
    if (!m_ffmpegInstalled) {
        downloadFfmpegWindows();
    }
    return;
#endif
    
#ifdef Q_OS_MAC
    // macOS: Download binaries to toolsmac directory
    if (allInstalled) {
        logMessage("=== Updating Tools ===");
        logMessage("Downloading latest yt-dlp from GitHub...");
        // Note: ffmpeg is not updated on macOS - only downloaded once
    } else {
        logMessage("=== Installing Tools ===");
        if (!m_ytDlpInstalled) {
            logMessage("Downloading yt-dlp from GitHub...");
        }
        if (!m_ffmpegInstalled) {
            logMessage("Downloading ffmpeg from evermeet.cx...");
        }
    }
    
    // Start downloads
    if (!m_ytDlpInstalled || allInstalled) {
        downloadYtDlpMac();
    }
    if (!m_ffmpegInstalled) {
        downloadFfmpegMac();
    }
    return;
#endif
    
#ifdef Q_OS_LINUX
    logMessage("=== Linux Platform ===");
    logMessage("Automatic installation not implemented for Linux yet.");
    logMessage("Please install manually:");
    logMessage("  sudo apt install yt-dlp ffmpeg  # Ubuntu/Debian");
    logMessage("  sudo yum install yt-dlp ffmpeg  # CentOS/RHEL");
    logMessage("  sudo pacman -S yt-dlp ffmpeg    # Arch Linux");
    setButtonEnabled(true);
#endif
}

// macOS Download Methods
void ToolsManager::downloadYtDlpMac()
{
#ifdef Q_OS_MAC
    // GitHub URL for latest yt-dlp for macOS
    QString url = "https://github.com/yt-dlp/yt-dlp/releases/latest/download/yt-dlp_macos";
    QNetworkRequest request(url);
    
    // Set user agent
    request.setRawHeader("User-Agent", "VimeoDownloader/1.0");
    
    logMessage(QString("Downloading yt-dlp from: %1").arg(url));
    
    // Start download
    QNetworkReply *reply = m_networkManager->get(request);
    
    connect(reply, &QNetworkReply::downloadProgress, [this](qint64 received, qint64 total) {
        if (total > 0) {
            int percentage = (received * 100) / total;
            logMessage(QString("yt-dlp download progress: %1% (%2 / %3 bytes)")
                       .arg(percentage)
                       .arg(received)
                       .arg(total));
        }
    });
    
    connect(reply, &QNetworkReply::finished, [this, reply]() {
        if (reply->error() == QNetworkReply::NoError) {
            // Save the downloaded file
            QString appDir = QCoreApplication::applicationDirPath();
            QString toolsDir = appDir + "/toolsmac";
            
            // Create toolsmac directory if it doesn't exist
            QDir dir;
            if (!dir.exists(toolsDir)) {
                if (!dir.mkpath(toolsDir)) {
                    logMessage("ERROR: Could not create toolsmac directory");
                    setButtonEnabled(true);
                    reply->deleteLater();
                    return;
                }
            }
            
            QString ytDlpPath = toolsDir + "/yt-dlp";
            
            QFile file(ytDlpPath);
            if (file.open(QIODevice::WriteOnly)) {
                file.write(reply->readAll());
                file.close();
                
                // Make executable
                file.setPermissions(QFile::ReadOwner | QFile::WriteOwner | QFile::ExeOwner |
                                   QFile::ReadGroup | QFile::ExeGroup |
                                   QFile::ReadOther | QFile::ExeOther);
                
                logMessage("=== yt-dlp downloaded successfully ===");
                logMessage(QString("Saved to: %1").arg(ytDlpPath));
                
                // Check installation after download
                QTimer::singleShot(500, [this]() {
                    checkToolsInstallation();
                });
            } else {
                logMessage("ERROR: Could not save yt-dlp");
                logMessage("Check write permissions in application directory");
                setButtonEnabled(true);
            }
        } else {
            logMessage("ERROR: Failed to download yt-dlp");
            logMessage(QString("Error: %1").arg(reply->errorString()));
            logMessage("Please check your internet connection");
            setButtonEnabled(true);
        }
        
        reply->deleteLater();
    });
#endif
}

void ToolsManager::updateYtDlpMac()
{
#ifdef Q_OS_MAC
    // For updates, just download the latest version (same as install)
    downloadYtDlpMac();
#endif
}

void ToolsManager::downloadFfmpegMac()
{
#ifdef Q_OS_MAC
    // evermeet.cx URL for latest ffmpeg for macOS
    QString url = "https://evermeet.cx/ffmpeg/getrelease/zip";
    QNetworkRequest request(url);
    
    // Set user agent
    request.setRawHeader("User-Agent", "VimeoDownloader/1.0");
    
    logMessage(QString("Downloading ffmpeg from: %1").arg(url));
    
    // Start download
    QNetworkReply *reply = m_networkManager->get(request);
    
    connect(reply, &QNetworkReply::downloadProgress, [this](qint64 received, qint64 total) {
        if (total > 0) {
            int percentage = (received * 100) / total;
            logMessage(QString("ffmpeg download progress: %1% (%2 / %3 bytes)")
                       .arg(percentage)
                       .arg(received)
                       .arg(total));
        }
    });
    
    connect(reply, &QNetworkReply::finished, [this, reply]() {
        if (reply->error() == QNetworkReply::NoError) {
            // Save the downloaded zip file temporarily
            QString appDir = QCoreApplication::applicationDirPath();
            QString toolsDir = appDir + "/toolsmac";
            
            // Create toolsmac directory if it doesn't exist
            QDir dir;
            if (!dir.exists(toolsDir)) {
                if (!dir.mkpath(toolsDir)) {
                    logMessage("ERROR: Could not create toolsmac directory");
                    setButtonEnabled(true);
                    reply->deleteLater();
                    return;
                }
            }
            
            QString tempZipPath = toolsDir + "/ffmpeg_temp.zip";
            QString ffmpegPath = toolsDir + "/ffmpeg";
            
            // Save zip file
            QFile zipFile(tempZipPath);
            if (zipFile.open(QIODevice::WriteOnly)) {
                zipFile.write(reply->readAll());
                zipFile.close();
                
                // Extract ffmpeg binary using system unzip command
                QProcess *unzipProcess = new QProcess(this);
                connect(unzipProcess, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
                        [this, unzipProcess, tempZipPath, ffmpegPath](int exitCode, QProcess::ExitStatus exitStatus) {
                    
                    // Clean up zip file
                    QFile::remove(tempZipPath);
                    
                    if (exitStatus == QProcess::NormalExit && exitCode == 0) {
                        // Make ffmpeg executable
                        QFile ffmpegFile(ffmpegPath);
                        if (ffmpegFile.exists()) {
                            ffmpegFile.setPermissions(QFile::ReadOwner | QFile::WriteOwner | QFile::ExeOwner |
                                                     QFile::ReadGroup | QFile::ExeGroup |
                                                     QFile::ReadOther | QFile::ExeOther);
                            
                            logMessage("=== ffmpeg downloaded and extracted successfully ===");
                            logMessage(QString("Saved to: %1").arg(ffmpegPath));
                            
                            // Check installation after extraction
                            QTimer::singleShot(500, [this]() {
                                checkToolsInstallation();
                            });
                        } else {
                            logMessage("ERROR: ffmpeg binary not found after extraction");
                            setButtonEnabled(true);
                        }
                    } else {
                        logMessage("ERROR: Failed to extract ffmpeg zip file");
                        setButtonEnabled(true);
                    }
                    
                    unzipProcess->deleteLater();
                });
                
                // Extract only the ffmpeg binary from the zip
                unzipProcess->start("unzip", QStringList() << "-j" << tempZipPath << "ffmpeg" << "-d" << toolsDir);
                
                if (!unzipProcess->waitForStarted(5000)) {
                    logMessage("ERROR: Could not start unzip process");
                    QFile::remove(tempZipPath);
                    setButtonEnabled(true);
                    unzipProcess->deleteLater();
                }
            } else {
                logMessage("ERROR: Could not save ffmpeg zip file");
                logMessage("Check write permissions in application directory");
                setButtonEnabled(true);
            }
        } else {
            logMessage("ERROR: Failed to download ffmpeg");
            logMessage(QString("Error: %1").arg(reply->errorString()));
            logMessage("Please check your internet connection");
            setButtonEnabled(true);
        }
        
        reply->deleteLater();
    });
#endif
}

void ToolsManager::updateFfmpegMac()
{
#ifdef Q_OS_MAC
    // For updates, just download the latest version (same as install)
    downloadFfmpegMac();
#endif
}

// Windows Download Methods
void ToolsManager::downloadYtDlpWindows()
{
#ifdef Q_OS_WIN
    // GitHub URL for latest yt-dlp.exe
    QString url = "https://github.com/yt-dlp/yt-dlp/releases/latest/download/yt-dlp.exe";
    QNetworkRequest request(url);
    
    // Set user agent
    request.setRawHeader("User-Agent", "VimeoDownloader/1.0");
    
    logMessage(QString("Downloading yt-dlp from: %1").arg(url));
    
    // Start download
    QNetworkReply *reply = m_networkManager->get(request);
    
    connect(reply, &QNetworkReply::downloadProgress, [this](qint64 received, qint64 total) {
        if (total > 0) {
            int percentage = (received * 100) / total;
            logMessage(QString("yt-dlp download progress: %1% (%2 / %3 bytes)")
                       .arg(percentage)
                       .arg(received)
                       .arg(total));
        }
    });
    
    connect(reply, &QNetworkReply::finished, [this, reply]() {
        if (reply->error() == QNetworkReply::NoError) {
            // Save the downloaded file
            QString appDir = QCoreApplication::applicationDirPath();
            QString toolsDir = appDir + "/tools";
            
            // Create tools directory if it doesn't exist
            QDir dir;
            if (!dir.exists(toolsDir)) {
                if (!dir.mkpath(toolsDir)) {
                    logMessage("ERROR: Could not create tools directory");
                    setButtonEnabled(true);
                    reply->deleteLater();
                    return;
                }
            }
            
            QString ytDlpPath = toolsDir + "/yt-dlp.exe";
            
            QFile file(ytDlpPath);
            if (file.open(QIODevice::WriteOnly)) {
                file.write(reply->readAll());
                file.close();
                
                // Make executable (though Windows doesn't need this)
                file.setPermissions(QFile::ReadOwner | QFile::WriteOwner | QFile::ExeOwner |
                                   QFile::ReadGroup | QFile::ExeGroup |
                                   QFile::ReadOther | QFile::ExeOther);
                
                logMessage("=== yt-dlp.exe downloaded successfully ===");
                logMessage(QString("Saved to: %1").arg(ytDlpPath));
                
                // Check installation after download
                QTimer::singleShot(500, [this]() {
                    checkToolsInstallation();
                });
            } else {
                logMessage("ERROR: Could not save yt-dlp.exe");
                logMessage("Check write permissions in application directory");
                setButtonEnabled(true);
            }
        } else {
            logMessage("ERROR: Failed to download yt-dlp.exe");
            logMessage(QString("Error: %1").arg(reply->errorString()));
            logMessage("Please check your internet connection");
            setButtonEnabled(true);
        }
        
        reply->deleteLater();
    });
#endif
}

void ToolsManager::downloadFfmpegWindows()
{
#ifdef Q_OS_WIN
    // GitHub URL for latest ffmpeg.exe (using a reliable build)
    QString url = "https://github.com/BtbN/FFmpeg-Builds/releases/latest/download/ffmpeg-master-latest-win64-gpl.zip";
    QNetworkRequest request(url);
    
    // Set user agent
    request.setRawHeader("User-Agent", "VimeoDownloader/1.0");
    
    logMessage(QString("Downloading ffmpeg from: %1").arg(url));
    logMessage("Note: This will download a zip file that needs to be extracted manually");
    logMessage("For now, please download and extract ffmpeg.exe manually to the application directory");
    
    // TODO: Implement zip extraction
    // For now, just mark as not implemented
    logMessage("ERROR: Automatic ffmpeg installation not fully implemented on Windows yet");
    logMessage("Please download ffmpeg manually from: https://ffmpeg.org/download.html");
    logMessage("Extract ffmpeg.exe to the same directory as this application");
    
    setButtonEnabled(true);
#endif
}

// Helper Methods
void ToolsManager::logMessage(const QString &message)
{
    if (m_logOutput) {
        m_logOutput->append(message);
    }
}

void ToolsManager::setButtonEnabled(bool enabled)
{
    if (m_toolsButton) {
        m_toolsButton->setEnabled(enabled);
    }
}

void ToolsManager::setButtonText(const QString &text)
{
    if (m_toolsButton) {
        m_toolsButton->setText(text);
    }
}

void ToolsManager::setButtonStyle(const QString &styleClass)
{
    if (m_toolsButton) {
        m_toolsButton->setProperty("class", styleClass);
        m_toolsButton->style()->unpolish(m_toolsButton);
        m_toolsButton->style()->polish(m_toolsButton);
    }
}

QString ToolsManager::getYtDlpPath() const
{
#ifdef Q_OS_WIN
    // Windows: Use tools subdirectory
    QString appDir = QCoreApplication::applicationDirPath();
    return appDir + "/tools/yt-dlp.exe";
#elif defined(Q_OS_MAC)
    // macOS: Check toolsmac directory first, then fallback to system PATH
    QString appDir = QCoreApplication::applicationDirPath();
    QString localPath = appDir + "/toolsmac/yt-dlp";
    if (QFile::exists(localPath)) {
        return localPath;
    }
    // Fallback to system PATH
    return "yt-dlp";
#else
    // Linux: Use system PATH
    return "yt-dlp";
#endif
}

QString ToolsManager::getFfmpegPath() const
{
#ifdef Q_OS_WIN
    // Windows: Use tools subdirectory
    QString appDir = QCoreApplication::applicationDirPath();
    return appDir + "/tools/ffmpeg.exe";
#elif defined(Q_OS_MAC)
    // macOS: Check toolsmac directory first, then fallback to system PATH
    QString appDir = QCoreApplication::applicationDirPath();
    QString localPath = appDir + "/toolsmac/ffmpeg";
    if (QFile::exists(localPath)) {
        return localPath;
    }
    // Fallback to system PATH
    return "ffmpeg";
#else
    // Linux: Use system PATH
    return "ffmpeg";
#endif
}

QString ToolsManager::getBrewPath() const
{
#ifdef Q_OS_MAC
    // Check for Homebrew in common locations
    QStringList possiblePaths = {
        "/opt/homebrew/bin/brew",  // Apple Silicon Homebrew
        "/usr/local/bin/brew"      // Intel Homebrew
    };
    
    for (const QString &path : possiblePaths) {
        if (QFile::exists(path)) {
            return path;
        }
    }
    
    // Fallback to PATH
    return "brew";
#else
    return "brew";
#endif
}

