#include "vimeodownloader/mainwindow.h"
#include "vimeodownloader/downloader.h"
#include "vimeodownloader/colorutils.h"

#include <QApplication>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QProgressBar>
#include <QTextEdit>
#include <QGroupBox>
#include <QMessageBox>
#include <QUrl>
#include <QDesktopServices>
#include <QFileDialog>
#include <QStandardPaths>
#include <QTimer>
#include <QScreen>
#include <QStyle>
#include <QStandardPaths>
#include <QDir>
#include <QProcess>
#include <QRegularExpression>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , m_centralWidget(nullptr)
    , m_mainLayout(nullptr)
    , m_inputGroup(nullptr)
    , m_inputLayout(nullptr)
    , m_urlLayout(nullptr)
    , m_urlLabel(nullptr)
    , m_urlInput(nullptr)
    , m_downloadButton(nullptr)
    , m_progressGroup(nullptr)
    , m_progressLayout(nullptr)
    , m_progressBar(nullptr)
    , m_statusLabel(nullptr)
    , m_logGroup(nullptr)
    , m_logLayout(nullptr)
    , m_logOutput(nullptr)
    , m_settingsGroup(nullptr)
    , m_settingsLayout(nullptr)
    , m_credentialsLayout(nullptr)
    , m_folderLayout(nullptr)
    , m_ytDlpLayout(nullptr)
    , m_userInput(nullptr)
    , m_passwordInput(nullptr)
    , m_saveCredentialsButton(nullptr)
    , m_downloadFolderInput(nullptr)
    , m_browseFolderButton(nullptr)
    , m_ytDlpButton(nullptr)
    , m_settings(nullptr)
    , m_ytDlpInstalled(false)
{
    // Inicializar configuración
    m_settings = new QSettings(getConfigPath(), QSettings::IniFormat, this);
    
    setupUI();
    setupStyles();
    setupConnections();
    loadSettings();
    checkYtDlpInstallation();
    
    // Configurar ventana
    setWindowTitle("Vimeo Downloader - LGA");
    setMinimumSize(550, 580);
    resize(650, 620);
    
    // Centrar ventana en pantalla
    move(QApplication::primaryScreen()->geometry().center() - frameGeometry().center());
}

MainWindow::~MainWindow()
{
    // Los widgets se limpian automáticamente por Qt
}

void MainWindow::setupUI()
{
    // Widget central
    m_centralWidget = new QWidget(this);
    m_centralWidget->setObjectName("centralWidget");
    setCentralWidget(m_centralWidget);
    
    // Layout principal
    m_mainLayout = new QVBoxLayout(m_centralWidget);
    m_mainLayout->setSpacing(16);
    m_mainLayout->setContentsMargins(20, 20, 20, 20);
    
    // Video URL Group
    m_inputGroup = new QGroupBox("Video URL", this);
    m_inputLayout = new QVBoxLayout(m_inputGroup);
    m_inputLayout->setSpacing(8);
    
    m_urlLabel = new QLabel("Enter Vimeo video URL:", this);
    
    // Layout horizontal para URL y botón
    m_urlLayout = new QHBoxLayout();
    m_urlInput = new QLineEdit(this);
    m_urlInput->setPlaceholderText("https://vimeo.com/...");
    
    m_downloadButton = new QPushButton("Download", this);
    m_downloadButton->setObjectName("downloadButton");
    m_downloadButton->setProperty("class", "primary");
    m_downloadButton->setEnabled(false);
    m_downloadButton->setFixedWidth(100);
    
    m_urlLayout->addWidget(m_urlInput);
    m_urlLayout->addWidget(m_downloadButton);
    
    m_inputLayout->addWidget(m_urlLabel);
    m_inputLayout->addLayout(m_urlLayout);
    
    // Progress Group
    m_progressGroup = new QGroupBox("Progress", this);
    m_progressLayout = new QVBoxLayout(m_progressGroup);
    m_progressLayout->setSpacing(8);
    
    m_progressBar = new QProgressBar(this);
    m_progressBar->setVisible(false);
    
    m_statusLabel = new QLabel("Ready to download", this);
    m_statusLabel->setWordWrap(true);
    
    m_progressLayout->addWidget(m_progressBar);
    m_progressLayout->addWidget(m_statusLabel);
    
    // Download Log Group
    m_logGroup = new QGroupBox("Download Log", this);
    m_logLayout = new QVBoxLayout(m_logGroup);
    
    m_logOutput = new QTextEdit(this);
    m_logOutput->setReadOnly(true);
    m_logOutput->setMinimumHeight(200);
    m_logOutput->setMaximumHeight(250);
    m_logOutput->setFont(QFont("Courier", 10));
    
    m_logLayout->addWidget(m_logOutput);
    
    // Settings Group
    m_settingsGroup = new QGroupBox("Settings", this);
    m_settingsLayout = new QVBoxLayout(m_settingsGroup);
    m_settingsLayout->setSpacing(8);
    
    // First row: Username | Password | Save
    m_credentialsLayout = new QHBoxLayout();
    m_userInput = new QLineEdit(this);
    m_userInput->setPlaceholderText("Vimeo Username...");
    
    m_passwordInput = new QLineEdit(this);
    m_passwordInput->setEchoMode(QLineEdit::Password);
    m_passwordInput->setPlaceholderText("Vimeo Password...");
    
    m_saveCredentialsButton = new QPushButton("Save", this);
    m_saveCredentialsButton->setFixedWidth(100);
    
    m_credentialsLayout->addWidget(m_userInput);
    m_credentialsLayout->addWidget(m_passwordInput);
    m_credentialsLayout->addWidget(m_saveCredentialsButton);
    
    // Second row: Download Folder | Browse
    m_folderLayout = new QHBoxLayout();
    m_downloadFolderInput = new QLineEdit(this);
    m_downloadFolderInput->setPlaceholderText("Download Folder...");
    
    m_browseFolderButton = new QPushButton("Browse", this);
    m_browseFolderButton->setFixedWidth(100);
    
    m_folderLayout->addWidget(m_downloadFolderInput);
    m_folderLayout->addWidget(m_browseFolderButton);
    
    // Third row: yt-dlp button aligned right
    m_ytDlpLayout = new QHBoxLayout();
    m_ytDlpButton = new QPushButton("Checking yt-dlp...", this);
    m_ytDlpButton->setEnabled(false);
    m_ytDlpButton->setFixedWidth(120);
    
    m_ytDlpLayout->addStretch(); // Push button to the right
    m_ytDlpLayout->addWidget(m_ytDlpButton);
    
    m_settingsLayout->addLayout(m_credentialsLayout);
    m_settingsLayout->addLayout(m_folderLayout);
    m_settingsLayout->addLayout(m_ytDlpLayout);
    
    // Agregar todos los grupos al layout principal
    m_mainLayout->addWidget(m_inputGroup);
    m_mainLayout->addWidget(m_progressGroup);
    m_mainLayout->addWidget(m_settingsGroup);
    m_mainLayout->addWidget(m_logGroup);
}

void MainWindow::setupStyles()
{
    // Aplicar clase CSS al botón de descarga
    m_downloadButton->setProperty("class", "primary");
    
    // Forzar actualización de estilos para todos los botones
    style()->unpolish(m_downloadButton);
    style()->polish(m_downloadButton);
    
    style()->unpolish(m_saveCredentialsButton);
    style()->polish(m_saveCredentialsButton);
    
    style()->unpolish(m_browseFolderButton);
    style()->polish(m_browseFolderButton);
    
    style()->unpolish(m_ytDlpButton);
    style()->polish(m_ytDlpButton);
}

void MainWindow::setupConnections()
{
    // Connect UI signals
    connect(m_urlInput, &QLineEdit::textChanged, this, &MainWindow::onUrlChanged);
    connect(m_downloadButton, &QPushButton::clicked, this, &MainWindow::onDownloadClicked);
    connect(m_ytDlpButton, &QPushButton::clicked, this, &MainWindow::onInstallUpdateYtDlpClicked);
    connect(m_saveCredentialsButton, &QPushButton::clicked, this, &MainWindow::onSaveCredentialsClicked);
    connect(m_browseFolderButton, &QPushButton::clicked, this, &MainWindow::onBrowseFolderClicked);
}

void MainWindow::onDownloadClicked()
{
    QString url = m_urlInput->text().trimmed();
    QString user = m_settings->value("vimeo/username", "").toString();
    QString password = m_settings->value("vimeo/password", "").toString();
    QString downloadDir = m_settings->value("download/folder", "").toString();
    
    if (url.isEmpty()) {
        QMessageBox::warning(this, "Error", "Please enter a valid URL.");
        return;
    }
    
    if (user.isEmpty() || password.isEmpty()) {
        QMessageBox::warning(this, "Error", "Please save Vimeo credentials first.");
        return;
    }
    
    if (downloadDir.isEmpty()) {
        QMessageBox::warning(this, "Error", "Please set a download folder first.");
        return;
    }
    
    // Validate Vimeo URL
    if (!url.contains("vimeo.com", Qt::CaseInsensitive)) {
        QMessageBox::warning(this, "Error", "Please enter a valid Vimeo URL.");
        return;
    }
    
    if (!m_ytDlpInstalled) {
        QMessageBox::warning(this, "Error", "yt-dlp is not installed. Please install it first.");
        return;
    }
    
    // Show progress
    m_progressBar->setVisible(true);
    m_progressBar->setRange(0, 100); // Set range for percentage
    m_progressBar->setValue(0); // Start at 0%
    m_statusLabel->setText("Downloading video...");
    m_downloadButton->setEnabled(false);
    
    // Initial log
    m_logOutput->append(QString("=== Starting Download ==="));
    m_logOutput->append(QString("URL: %1").arg(url));
    m_logOutput->append(QString("User: %1").arg(user));
    m_logOutput->append(QString("Download Folder: %1").arg(downloadDir));
    m_logOutput->append("---");
    
    // Execute yt-dlp with credentials
    QProcess *process = new QProcess(this);
    
    // yt-dlp arguments
    QStringList arguments;
    arguments << "-u" << user;
    arguments << "-p" << password;
    arguments << "--output" << downloadDir + "/%(title)s.%(ext)s";
    arguments << "--format" << "best"; // Use best available format
    arguments << url;
    
    // Conectar señales del proceso
    connect(process, &QProcess::readyReadStandardOutput, [this, process]() {
        QByteArray data = process->readAllStandardOutput();
        QString output = QString::fromUtf8(data).trimmed();
        if (!output.isEmpty()) {
            m_logOutput->append(output);
            
            // Parse progress from yt-dlp output
            // Look for patterns like: [download]  21.6% of  654.62MiB at    4.83MiB/s ETA 01:46
            QRegularExpression progressRegex("\\[download\\]\\s+(\\d+(?:\\.\\d+)?)%");
            QRegularExpressionMatch match = progressRegex.match(output);
            if (match.hasMatch()) {
                bool ok;
                double progress = match.captured(1).toDouble(&ok);
                if (ok) {
                    m_progressBar->setValue(static_cast<int>(progress));
                    m_statusLabel->setText(QString("Downloading... %1%").arg(QString::number(progress, 'f', 1)));
                }
            }
            
            // Check for completion
            if (output.contains("100% of") && output.contains("in ")) {
                m_progressBar->setValue(100);
                m_statusLabel->setText("Download completed successfully");
            }
        }
    });
    
    connect(process, &QProcess::readyReadStandardError, [this, process]() {
        QByteArray data = process->readAllStandardError();
        QString output = QString::fromUtf8(data).trimmed();
        if (!output.isEmpty()) {
            m_logOutput->append("ERROR: " + output);
        }
    });
    
    connect(process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            [this, process](int exitCode, QProcess::ExitStatus exitStatus) {
        m_progressBar->setVisible(false);
        m_downloadButton->setEnabled(true);
        
        if (exitStatus == QProcess::CrashExit) {
            m_statusLabel->setText("Error: Process crashed unexpectedly");
            m_logOutput->append("ERROR: yt-dlp process crashed unexpectedly");
        } else if (exitCode == 0) {
            m_statusLabel->setText("Download completed successfully");
            m_logOutput->append("=== Download completed successfully ===");
        } else {
            m_statusLabel->setText(QString("Download error (code: %1)").arg(exitCode));
            m_logOutput->append(QString("ERROR: yt-dlp finished with error code: %1").arg(exitCode));
        }
        
        process->deleteLater();
    });
    
    // Start the process
    m_logOutput->append(QString("Executing: yt-dlp %1").arg(arguments.join(" ").replace(password, "***")));
    process->start("yt-dlp", arguments);
    
    if (!process->waitForStarted(5000)) {
        m_progressBar->setVisible(false);
        m_downloadButton->setEnabled(true);
        m_statusLabel->setText("Error: Could not start yt-dlp");
        m_logOutput->append("ERROR: Could not start yt-dlp. Verify it's installed.");
        process->deleteLater();
    }
}

void MainWindow::onUrlChanged()
{
    QString url = m_urlInput->text().trimmed();
    QString user = m_settings->value("vimeo/username", "").toString();
    QString password = m_settings->value("vimeo/password", "").toString();
    QString downloadDir = m_settings->value("download/folder", "").toString();
    
    bool isValidUrl = !url.isEmpty() && url.contains("vimeo.com", Qt::CaseInsensitive);
    bool hasCredentials = !user.isEmpty() && !password.isEmpty();
    bool hasDownloadDir = !downloadDir.isEmpty();
    
    m_downloadButton->setEnabled(isValidUrl && hasCredentials && hasDownloadDir && m_ytDlpInstalled);
    
    if (!url.isEmpty() && !isValidUrl) {
        m_statusLabel->setText("Invalid URL - must be from Vimeo");
    } else if (isValidUrl && !hasCredentials) {
        m_statusLabel->setText("Save Vimeo credentials first");
    } else if (isValidUrl && hasCredentials && !hasDownloadDir) {
        m_statusLabel->setText("Set download folder first");
    } else if (isValidUrl && hasCredentials && hasDownloadDir && !m_ytDlpInstalled) {
        m_statusLabel->setText("yt-dlp is not installed");
    } else if (isValidUrl && hasCredentials && hasDownloadDir && m_ytDlpInstalled) {
        m_statusLabel->setText("Ready to download");
    } else {
        m_statusLabel->setText("Enter a Vimeo URL");
    }
}

void MainWindow::onInstallUpdateYtDlpClicked()
{
    m_ytDlpButton->setEnabled(false);
    
    if (m_ytDlpInstalled) {
        // If yt-dlp is installed, use yt-dlp -U to update
        m_logOutput->append("=== Updating yt-dlp ===");
        
        QProcess *process = new QProcess(this);
        
        // Connect process signals
        connect(process, &QProcess::readyReadStandardOutput, [this, process]() {
            QByteArray data = process->readAllStandardOutput();
            QString output = QString::fromUtf8(data).trimmed();
            if (!output.isEmpty()) {
                m_logOutput->append(output);
            }
        });
        
        connect(process, &QProcess::readyReadStandardError, [this, process]() {
            QByteArray data = process->readAllStandardError();
            QString output = QString::fromUtf8(data).trimmed();
            if (!output.isEmpty()) {
                m_logOutput->append("INFO: " + output);
            }
        });
        
        connect(process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
                [this, process](int exitCode, QProcess::ExitStatus exitStatus) {
            
            if (exitStatus == QProcess::CrashExit || exitCode != 0) {
                m_logOutput->append("ERROR: Failed to update yt-dlp");
                m_logOutput->append("Try updating manually: yt-dlp -U");
            } else {
                m_logOutput->append("=== yt-dlp updated successfully ===");
            }
            
            // Check installation after process
            QTimer::singleShot(1000, [this]() {
                checkYtDlpInstallation();
            });
            
            process->deleteLater();
        });
        
        m_logOutput->append("Executing: yt-dlp -U");
        process->start("yt-dlp", QStringList() << "-U");
        
        if (!process->waitForStarted(5000)) {
            m_logOutput->append("ERROR: Could not execute yt-dlp -U");
            m_ytDlpButton->setEnabled(true);
            process->deleteLater();
        }
    } else {
        // If yt-dlp is not installed, try to install with pip
        m_logOutput->append("=== Installing yt-dlp ===");
        
        QProcess *process = new QProcess(this);
        
        // Connect process signals
        connect(process, &QProcess::readyReadStandardOutput, [this, process]() {
            QByteArray data = process->readAllStandardOutput();
            QString output = QString::fromUtf8(data).trimmed();
            if (!output.isEmpty()) {
                m_logOutput->append(output);
            }
        });
        
        connect(process, &QProcess::readyReadStandardError, [this, process]() {
            QByteArray data = process->readAllStandardError();
            QString output = QString::fromUtf8(data).trimmed();
            if (!output.isEmpty()) {
                m_logOutput->append("INFO: " + output);
            }
        });
        
        connect(process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
                [this, process](int exitCode, QProcess::ExitStatus exitStatus) {
            
            if (exitStatus == QProcess::CrashExit || exitCode != 0) {
                m_logOutput->append("ERROR: Failed to install yt-dlp");
                m_logOutput->append("Try installing manually: pip install yt-dlp");
                m_logOutput->append("Or use: brew install yt-dlp (on macOS)");
            } else {
                m_logOutput->append("=== yt-dlp installed successfully ===");
            }
            
            // Check installation after process
            QTimer::singleShot(1000, [this]() {
                checkYtDlpInstallation();
            });
            
            process->deleteLater();
        });
        
        // Try to install with pip
#ifdef Q_OS_MAC
        m_logOutput->append("Executing: pip3 install yt-dlp");
        process->start("pip3", QStringList() << "install" << "yt-dlp");
#else
        m_logOutput->append("Executing: pip install yt-dlp");
        process->start("pip", QStringList() << "install" << "yt-dlp");
#endif
        
        if (!process->waitForStarted(5000)) {
            m_logOutput->append("ERROR: Could not execute pip. Install yt-dlp manually:");
            m_logOutput->append("pip install yt-dlp");
            m_logOutput->append("Or use: brew install yt-dlp (on macOS)");
            m_ytDlpButton->setEnabled(true);
            process->deleteLater();
        }
    }
}

void MainWindow::onSaveCredentialsClicked()
{
    QString user = m_userInput->text().trimmed();
    QString password = m_passwordInput->text().trimmed();
    
    if (user.isEmpty() || password.isEmpty()) {
        QMessageBox::warning(this, "Warning", "Please enter both username and password.");
        return;
    }
    
    m_settings->setValue("vimeo/username", user);
    m_settings->setValue("vimeo/password", password);
    m_settings->sync();
    
    m_logOutput->append("Vimeo credentials saved successfully.");
    onUrlChanged(); // Update button state
}


void MainWindow::onBrowseFolderClicked()
{
    QString currentFolder = m_settings->value("download/folder", QStandardPaths::writableLocation(QStandardPaths::DesktopLocation)).toString();
    
    QString folder = QFileDialog::getExistingDirectory(this, "Select Download Folder", currentFolder);
    
    if (!folder.isEmpty()) {
        m_downloadFolderInput->setText(folder);
        
        // Auto-save the selected folder
        m_settings->setValue("download/folder", folder);
        m_settings->sync();
        
        m_logOutput->append(QString("Download folder saved: %1").arg(folder));
        onUrlChanged(); // Update button state
    }
}

void MainWindow::loadSettings()
{
    QString user = m_settings->value("vimeo/username", "").toString();
    QString password = m_settings->value("vimeo/password", "").toString();
    QString downloadFolder = m_settings->value("download/folder", "").toString();
    
    // Only set text if values exist, otherwise keep placeholders
    if (!user.isEmpty()) {
        m_userInput->setText(user);
    }
    if (!password.isEmpty()) {
        m_passwordInput->setText(password);
    }
    if (!downloadFolder.isEmpty()) {
        m_downloadFolderInput->setText(downloadFolder);
    }
}

void MainWindow::checkYtDlpInstallation()
{
    m_logOutput->append("Checking yt-dlp installation...");
    
    QProcess *process = new QProcess(this);
    
    connect(process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            [this, process](int exitCode, QProcess::ExitStatus exitStatus) {
        
        if (exitStatus == QProcess::NormalExit && exitCode == 0) {
            m_ytDlpInstalled = true;
            m_ytDlpButton->setText("Update yt-dlp");
            m_ytDlpButton->setEnabled(true);
            m_logOutput->append("✓ yt-dlp is installed and available");
        } else {
            m_ytDlpInstalled = false;
            m_ytDlpButton->setText("Install yt-dlp");
            m_ytDlpButton->setEnabled(true);
            m_logOutput->append("✗ yt-dlp is not installed");
        }
        
        onUrlChanged(); // Update download button state
        process->deleteLater();
    });
    
    // Check if yt-dlp is available
    process->start("yt-dlp", QStringList() << "--version");
    
    if (!process->waitForStarted(3000)) {
        m_ytDlpInstalled = false;
        m_ytDlpButton->setText("Install yt-dlp");
        m_ytDlpButton->setEnabled(true);
        m_logOutput->append("✗ yt-dlp is not installed");
        onUrlChanged();
        process->deleteLater();
    }
}

QString MainWindow::getConfigPath() const
{
    // Crear la carpeta de configuración siguiendo el patrón de PipeSync
    QString appDataPath;
    
#ifdef Q_OS_WIN
    // Windows: %APPDATA%\LGA\VimeoDownloader\config.ini
    appDataPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    appDataPath = appDataPath.replace("/VimeoDownloader", "").replace("\\VimeoDownloader", "");
    appDataPath += "/VimeoDownloader";
#elif defined(Q_OS_MAC)
    // macOS: ~/Library/Application Support/LGA/VimeoDownloader/config.ini
    appDataPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    appDataPath = appDataPath.replace("/VimeoDownloader", "");
    appDataPath += "/VimeoDownloader";
#else
    // Linux: ~/.config/LGA/VimeoDownloader/config.ini
    appDataPath = QStandardPaths::writableLocation(QStandardPaths::ConfigLocation);
    appDataPath += "/LGA/VimeoDownloader";
#endif
    
    QDir dir(appDataPath);
    if (!dir.exists()) {
        dir.mkpath(".");
    }
    
    return appDataPath + "/config.ini";
}
