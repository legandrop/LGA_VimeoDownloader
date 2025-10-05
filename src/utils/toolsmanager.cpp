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
    // macOS: Check in common Homebrew locations first, then PATH
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
    // macOS: Check in common Homebrew locations first, then PATH
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
    // macOS: Use Homebrew
    if (allInstalled) {
        logMessage("=== Updating Tools ===");
        updateYtDlpMac();
        updateFfmpegMac();
    } else {
        logMessage("=== Installing Tools ===");
        if (!m_ytDlpInstalled) {
            installYtDlpMac();
        }
        if (!m_ffmpegInstalled) {
            installFfmpegMac();
        }
    }
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

// macOS Installation Methods
void ToolsManager::installYtDlpMac()
{
#ifdef Q_OS_MAC
    QProcess *process = new QProcess(this);
    
    // Connect process signals
    connect(process, &QProcess::readyReadStandardOutput, [this, process]() {
        QByteArray data = process->readAllStandardOutput();
        QString output = QString::fromUtf8(data).trimmed();
        if (!output.isEmpty()) {
            logMessage(output);
        }
    });
    
    connect(process, &QProcess::readyReadStandardError, [this, process]() {
        QByteArray data = process->readAllStandardError();
        QString output = QString::fromUtf8(data).trimmed();
        if (!output.isEmpty()) {
            logMessage("INFO: " + output);
        }
    });
    
    connect(process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            [this, process](int exitCode, QProcess::ExitStatus exitStatus) {
        
        if (exitStatus == QProcess::CrashExit || exitCode != 0) {
            logMessage("ERROR: Failed to install yt-dlp");
            logMessage("Try installing manually: brew install yt-dlp");
            logMessage("Make sure Homebrew is installed first");
        } else {
            logMessage("=== yt-dlp installed successfully ===");
        }
        
        // Check installation after process
        QTimer::singleShot(1000, [this]() {
            checkToolsInstallation();
        });
        
        process->deleteLater();
    });
    
    QString brewPath = getBrewPath();
    logMessage(QString("Executing: %1 install yt-dlp").arg(brewPath));
    process->start(brewPath, QStringList() << "install" << "yt-dlp");
    
    if (!process->waitForStarted(5000)) {
        logMessage("ERROR: Could not execute brew install");
        logMessage("Make sure Homebrew is installed and in PATH");
        logMessage("Install Homebrew: /bin/bash -c \"$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)\"");
        setButtonEnabled(true);
        process->deleteLater();
    }
#endif
}

void ToolsManager::updateYtDlpMac()
{
#ifdef Q_OS_MAC
    QProcess *process = new QProcess(this);
    
    // Connect process signals
    connect(process, &QProcess::readyReadStandardOutput, [this, process]() {
        QByteArray data = process->readAllStandardOutput();
        QString output = QString::fromUtf8(data).trimmed();
        if (!output.isEmpty()) {
            logMessage(output);
        }
    });
    
    connect(process, &QProcess::readyReadStandardError, [this, process]() {
        QByteArray data = process->readAllStandardError();
        QString output = QString::fromUtf8(data).trimmed();
        if (!output.isEmpty()) {
            logMessage("INFO: " + output);
        }
    });
    
    connect(process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            [this, process](int exitCode, QProcess::ExitStatus exitStatus) {
        
        if (exitStatus == QProcess::CrashExit || exitCode != 0) {
            logMessage("ERROR: Failed to update yt-dlp");
            logMessage("Try updating manually: brew upgrade yt-dlp");
        } else {
            logMessage("=== yt-dlp updated successfully ===");
        }
        
        // Check installation after process
        QTimer::singleShot(1000, [this]() {
            checkToolsInstallation();
        });
        
        process->deleteLater();
    });
    
    QString brewPath = getBrewPath();
    logMessage(QString("Executing: %1 upgrade yt-dlp").arg(brewPath));
    process->start(brewPath, QStringList() << "upgrade" << "yt-dlp");
    
    if (!process->waitForStarted(5000)) {
        logMessage("ERROR: Could not execute brew upgrade");
        logMessage("Make sure Homebrew is installed and in PATH");
        setButtonEnabled(true);
        process->deleteLater();
    }
#endif
}

void ToolsManager::installFfmpegMac()
{
#ifdef Q_OS_MAC
    QProcess *process = new QProcess(this);
    
    // Connect process signals
    connect(process, &QProcess::readyReadStandardOutput, [this, process]() {
        QByteArray data = process->readAllStandardOutput();
        QString output = QString::fromUtf8(data).trimmed();
        if (!output.isEmpty()) {
            logMessage(output);
        }
    });
    
    connect(process, &QProcess::readyReadStandardError, [this, process]() {
        QByteArray data = process->readAllStandardError();
        QString output = QString::fromUtf8(data).trimmed();
        if (!output.isEmpty()) {
            logMessage("INFO: " + output);
        }
    });
    
    connect(process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            [this, process](int exitCode, QProcess::ExitStatus exitStatus) {
        
        if (exitStatus == QProcess::CrashExit || exitCode != 0) {
            logMessage("ERROR: Failed to install ffmpeg");
            logMessage("Try installing manually: brew install ffmpeg");
        } else {
            logMessage("=== ffmpeg installed successfully ===");
        }
        
        // Check installation after process
        QTimer::singleShot(1000, [this]() {
            checkToolsInstallation();
        });
        
        process->deleteLater();
    });
    
    QString brewPath = getBrewPath();
    logMessage(QString("Executing: %1 install ffmpeg").arg(brewPath));
    process->start(brewPath, QStringList() << "install" << "ffmpeg");
    
    if (!process->waitForStarted(5000)) {
        logMessage("ERROR: Could not execute brew install ffmpeg");
        logMessage("Make sure Homebrew is installed and in PATH");
        setButtonEnabled(true);
        process->deleteLater();
    }
#endif
}

void ToolsManager::updateFfmpegMac()
{
#ifdef Q_OS_MAC
    QProcess *process = new QProcess(this);
    
    // Connect process signals
    connect(process, &QProcess::readyReadStandardOutput, [this, process]() {
        QByteArray data = process->readAllStandardOutput();
        QString output = QString::fromUtf8(data).trimmed();
        if (!output.isEmpty()) {
            logMessage(output);
        }
    });
    
    connect(process, &QProcess::readyReadStandardError, [this, process]() {
        QByteArray data = process->readAllStandardError();
        QString output = QString::fromUtf8(data).trimmed();
        if (!output.isEmpty()) {
            logMessage("INFO: " + output);
        }
    });
    
    connect(process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            [this, process](int exitCode, QProcess::ExitStatus exitStatus) {
        
        if (exitStatus == QProcess::CrashExit || exitCode != 0) {
            logMessage("ERROR: Failed to update ffmpeg");
            logMessage("Try updating manually: brew upgrade ffmpeg");
        } else {
            logMessage("=== ffmpeg updated successfully ===");
        }
        
        // Check installation after process
        QTimer::singleShot(1000, [this]() {
            checkToolsInstallation();
        });
        
        process->deleteLater();
    });
    
    QString brewPath = getBrewPath();
    logMessage(QString("Executing: %1 upgrade ffmpeg").arg(brewPath));
    process->start(brewPath, QStringList() << "upgrade" << "ffmpeg");
    
    if (!process->waitForStarted(5000)) {
        logMessage("ERROR: Could not execute brew upgrade ffmpeg");
        logMessage("Make sure Homebrew is installed and in PATH");
        setButtonEnabled(true);
        process->deleteLater();
    }
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
#else
    // macOS/Linux: Use system PATH
    return "yt-dlp";
#endif
}

QString ToolsManager::getFfmpegPath() const
{
#ifdef Q_OS_WIN
    // Windows: Use tools subdirectory
    QString appDir = QCoreApplication::applicationDirPath();
    return appDir + "/tools/ffmpeg.exe";
#else
    // macOS/Linux: Use system PATH
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
