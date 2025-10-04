#include "vimeodownloader/mainwindow.h"
#include "vimeodownloader/downloader.h"
#include "vimeodownloader/colorutils.h"
#include "vimeodownloader/toolsmanager.h"
#include "vimeodownloader/downloadqueue.h"

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
#include <QCoreApplication>
#include <QFile>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QTimer>
#include <QMouseEvent>
#include <QEvent>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , m_centralWidget(nullptr)
    , m_mainLayout(nullptr)
    , m_inputGroup(nullptr)
    , m_inputLayout(nullptr)
    , m_urlLayout(nullptr)
    , m_urlInput(nullptr)
    , m_downloadButton(nullptr)
    , m_progressGroup(nullptr)
    , m_progressLayout(nullptr)
    , m_progressButtonLayout(nullptr)
    , m_progressBar(nullptr)
    , m_progressLabel(nullptr)
    , m_cancelButton(nullptr)
    , m_logGroup(nullptr)
    , m_logLayout(nullptr)
    , m_logOutput(nullptr)
    , m_logExpanded(false)
    , m_settingsGroup(nullptr)
    , m_settingsLayout(nullptr)
    , m_credentialsLayout(nullptr)
    , m_folderLayout(nullptr)
    , m_toolsLayout(nullptr)
    , m_userInput(nullptr)
    , m_passwordInput(nullptr)
    , m_saveCredentialsButton(nullptr)
    , m_downloadFolderInput(nullptr)
    , m_browseFolderButton(nullptr)
    , m_toolsButton(nullptr)
    , m_settings(nullptr)
    , m_toolsManager(nullptr)
    , m_downloadQueue(nullptr)
{
    // Inicializar configuración
    m_settings = new QSettings(getConfigPath(), QSettings::IniFormat, this);
    
    setupUI();
    setupStyles();
    setupConnections();
    loadSettings();
    detectOperatingSystem();
    
    // Initialize tools manager
    m_toolsManager = new ToolsManager(m_logOutput, m_toolsButton, this);
    connect(m_toolsManager, &ToolsManager::toolsStatusChanged, this, &MainWindow::onToolsStatusChanged);
    m_toolsManager->checkToolsInstallation();
    
    // Initialize download queue
    m_downloadQueue = new DownloadQueue(m_logOutput, m_progressBar, m_progressGroup, this);
    connect(m_downloadQueue, &DownloadQueue::downloadStarted, this, &MainWindow::onDownloadStarted);
    connect(m_downloadQueue, &DownloadQueue::downloadCompleted, this, &MainWindow::onDownloadCompleted);
    connect(m_downloadQueue, &DownloadQueue::queueStatusChanged, this, &MainWindow::onQueueStatusChanged);
    connect(m_downloadQueue, &DownloadQueue::downloadAddedToQueue, this, &MainWindow::onDownloadAddedToQueue);
    
    // Configurar ventana
    setWindowTitle("Vimeo Downloader - LGA");
    adjustWindowSize();
    
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
    
    // Layout horizontal para URL y botón
    m_urlLayout = new QHBoxLayout();
    m_urlInput = new QLineEdit(this);
    m_urlInput->setPlaceholderText("https://vimeo.com/... or https://youtube.com/...");
    
    m_downloadButton = new QPushButton("Download", this);
    m_downloadButton->setObjectName("downloadButton");
    m_downloadButton->setProperty("class", "primary");
    m_downloadButton->setEnabled(false);
    m_downloadButton->setFixedWidth(110);
    
    m_urlLayout->addWidget(m_urlInput);
    m_urlLayout->addWidget(m_downloadButton);
    
    m_inputLayout->addLayout(m_urlLayout);
    
    // Progress Group
    m_progressGroup = new QGroupBox("Progress (0/0)", this);
    m_progressLayout = new QVBoxLayout(m_progressGroup);
    m_progressLayout->setSpacing(8);
    
    // Progress bar and cancel button in same line (like URL layout)
    m_progressButtonLayout = new QHBoxLayout();
    m_progressBar = new QProgressBar(this);
    m_progressBar->setRange(0, 100);
    m_progressBar->setValue(0);
    m_progressBar->setTextVisible(false); // Hide percentage text when inactive
    
    m_cancelButton = new QPushButton("Cancel", this);
    m_cancelButton->setObjectName("cancelButton");
    m_cancelButton->setFixedWidth(110); // Same width as download button
    // No danger class - same color as other buttons
    
    m_progressButtonLayout->addWidget(m_progressBar);
    m_progressButtonLayout->addWidget(m_cancelButton);
    
    // Store reference to the group box title for updates
    m_progressLabel = nullptr; // We'll use the group box title instead
    
    m_progressLayout->addLayout(m_progressButtonLayout);
    
    // Log Group - restored to original with clickable title (starts collapsed)
    m_logGroup = new QGroupBox("Log >", this);
    m_logGroup->setObjectName("logGroupBox");
    m_logGroup->setProperty("collapsed", true); // Set collapsed property for CSS
    m_logGroup->setCursor(Qt::PointingHandCursor);
    m_logGroup->setFixedHeight(35); // Altura aumentada en 10px más
    m_logLayout = new QVBoxLayout(m_logGroup);
    m_logLayout->setContentsMargins(0, 0, 0, 0); // Sin márgenes cuando colapsado
    m_logLayout->setSpacing(0); // No spacing between widgets
    
    m_logOutput = new QTextEdit(this);
    m_logOutput->setReadOnly(true);
    m_logOutput->setMinimumHeight(200);
    m_logOutput->setMaximumHeight(250);
    m_logOutput->setFont(QFont("Courier", 10));
    m_logOutput->hide(); // Start hidden
    
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
    m_saveCredentialsButton->setFixedWidth(110);
    
    m_credentialsLayout->addWidget(m_userInput);
    m_credentialsLayout->addWidget(m_passwordInput);
    m_credentialsLayout->addWidget(m_saveCredentialsButton);
    
    // Second row: Download Folder | Browse
    m_folderLayout = new QHBoxLayout();
    m_downloadFolderInput = new QLineEdit(this);
    m_downloadFolderInput->setPlaceholderText("Download Folder...");
    
    m_browseFolderButton = new QPushButton("Browse", this);
    m_browseFolderButton->setFixedWidth(110);
    
    m_folderLayout->addWidget(m_downloadFolderInput);
    m_folderLayout->addWidget(m_browseFolderButton);
    
    // Third row: tools button aligned right
    m_toolsLayout = new QHBoxLayout();
    m_toolsButton = new QPushButton("Checking Tools...", this);
    m_toolsButton->setObjectName("toolsButton");
    m_toolsButton->setEnabled(false);
    m_toolsButton->setFixedWidth(110);
    
    m_toolsLayout->addStretch(); // Push button to the right
    m_toolsLayout->addWidget(m_toolsButton);
    
    m_settingsLayout->addLayout(m_credentialsLayout);
    m_settingsLayout->addLayout(m_folderLayout);
    m_settingsLayout->addLayout(m_toolsLayout);
    
    // Agregar todos los grupos al layout principal
    m_mainLayout->addWidget(m_inputGroup);
    m_mainLayout->addWidget(m_progressGroup);
    m_mainLayout->addWidget(m_settingsGroup);
    m_mainLayout->addWidget(m_logGroup);
    
    // Agregar un spacer al final para empujar todo hacia arriba cuando el log está colapsado
    m_mainLayout->addStretch();
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
    
    style()->unpolish(m_toolsButton);
    style()->polish(m_toolsButton);
    
    style()->unpolish(m_cancelButton);
    style()->polish(m_cancelButton);
}

void MainWindow::setupConnections()
{
    // Connect UI signals
    connect(m_urlInput, &QLineEdit::textChanged, this, &MainWindow::onUrlChanged);
    connect(m_downloadButton, &QPushButton::clicked, this, &MainWindow::onDownloadClicked);
    connect(m_saveCredentialsButton, &QPushButton::clicked, this, &MainWindow::onSaveCredentialsClicked);
    connect(m_browseFolderButton, &QPushButton::clicked, this, &MainWindow::onBrowseFolderClicked);
    connect(m_cancelButton, &QPushButton::clicked, this, &MainWindow::onCancelClicked);
    
    // Install event filter for log group box to capture clicks
    m_logGroup->installEventFilter(this);
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
    
    // Validate video URL (Vimeo or YouTube)
    if (!isValidVideoUrl(url)) {
        QMessageBox::warning(this, "Error", "Please enter a valid Vimeo or YouTube URL.");
        return;
    }
    
    if (!m_toolsManager->areToolsInstalled()) {
        QMessageBox::warning(this, "Error", "Required tools are not installed. Please install them first.");
        return;
    }
    
    // Add to download queue
    m_downloadQueue->addDownload(url, user, password, downloadDir);
    
    // Clear URL input for next download
    m_urlInput->clear();
}

void MainWindow::onUrlChanged()
{
    QString url = m_urlInput->text().trimmed();
    QString user = m_settings->value("vimeo/username", "").toString();
    QString password = m_settings->value("vimeo/password", "").toString();
    QString downloadDir = m_settings->value("download/folder", "").toString();
    
    bool isValidUrl = !url.isEmpty() && isValidVideoUrl(url);
    bool hasCredentials = !user.isEmpty() && !password.isEmpty();
    bool hasDownloadDir = !downloadDir.isEmpty();
    
    m_downloadButton->setEnabled(isValidUrl && hasCredentials && hasDownloadDir && m_toolsManager->areToolsInstalled());
}

void MainWindow::onToolsStatusChanged(bool allInstalled)
{
    // Update download button state when tools status changes
    onUrlChanged();
}

void MainWindow::onDownloadStarted()
{
    // Disable download button while downloading
    m_downloadButton->setEnabled(false);
}

void MainWindow::onDownloadCompleted()
{
    // Re-enable download button after download completes
    onUrlChanged(); // This will check all conditions and enable if appropriate
}

void MainWindow::onQueueStatusChanged(int current, int total)
{
    // Update progress group title
    m_progressGroup->setTitle(QString("Progress (%1/%2)").arg(current).arg(total));
}

void MainWindow::onDownloadAddedToQueue(int totalCount)
{
    // When a download is added, only update the total count, keep current number unchanged
    QString currentTitle = m_progressGroup->title();
    QRegularExpression regex("Progress \\((\\d+)/(\\d+)\\)");
    QRegularExpressionMatch match = regex.match(currentTitle);
    
    int currentNumber = 0;
    if (match.hasMatch()) {
        currentNumber = match.captured(1).toInt();
    }
    
    // Update only the total count, keep current number
    m_progressGroup->setTitle(QString("Progress (%1/%2)").arg(currentNumber).arg(totalCount));
}

void MainWindow::onCancelClicked()
{
    if (m_downloadQueue) {
        // Reset entire queue and all counters
        m_downloadQueue->resetQueue();
        
        // Re-enable download button
        onUrlChanged();
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

bool MainWindow::isValidVideoUrl(const QString &url) const
{
    if (url.isEmpty()) {
        return false;
    }
    
    // Check for Vimeo URLs
    if (url.contains("vimeo.com", Qt::CaseInsensitive)) {
        return true;
    }
    
    // Check for YouTube URLs
    if (url.contains("youtube.com", Qt::CaseInsensitive) || 
        url.contains("youtu.be", Qt::CaseInsensitive)) {
        return true;
    }
    
    return false;
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

void MainWindow::detectOperatingSystem()
{
#ifdef Q_OS_MAC
    m_logOutput->append("=== System Information ===");
    m_logOutput->append("Operating System: macOS");
    m_logOutput->append("Tools installation method: brew");
    m_logOutput->append("Supported platforms: Vimeo, YouTube");
    m_logOutput->append("===========================");
#elif defined(Q_OS_WIN)
    m_logOutput->append("=== System Information ===");
    m_logOutput->append("Operating System: Windows");
    m_logOutput->append("Tools installation method: Download from GitHub");
    m_logOutput->append("Tools location: Application directory");
    m_logOutput->append("Supported platforms: Vimeo, YouTube");
    m_logOutput->append("===========================");
#else
    m_logOutput->append("=== System Information ===");
    m_logOutput->append("Operating System: Linux/Other");
    m_logOutput->append("Tools installation: Manual installation required");
    m_logOutput->append("Supported platforms: Vimeo, YouTube");
    m_logOutput->append("===========================");
#endif
}


bool MainWindow::eventFilter(QObject *obj, QEvent *event)
{
    if (obj == m_logGroup && event->type() == QEvent::MouseButtonPress) {
        QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);
        if (mouseEvent->button() == Qt::LeftButton) {
            onLogToggleClicked();
            return true;
        }
    }
    return QMainWindow::eventFilter(obj, event);
}

void MainWindow::onLogToggleClicked()
{
    m_logExpanded = !m_logExpanded;
    
    if (m_logExpanded) {
        m_logGroup->setTitle("Log ⌄");
        m_logGroup->setProperty("collapsed", false); // Not collapsed
        m_logGroup->setFixedHeight(QWIDGETSIZE_MAX); // Permitir que se expanda
        m_logGroup->setMinimumHeight(0); // Sin altura mínima
        m_logGroup->setMaximumHeight(QWIDGETSIZE_MAX); // Sin límite máximo
        m_logLayout->setContentsMargins(3, 2, 3, 3); // Márgenes consistentes con CSS
        m_logLayout->setSpacing(2); // Espaciado pequeño entre widgets
        m_logOutput->show();
    } else {
        m_logGroup->setTitle("Log >");
        m_logGroup->setProperty("collapsed", true); // Collapsed
        m_logGroup->setFixedHeight(35); // Altura aumentada en 10px más
        m_logLayout->setContentsMargins(0, 0, 0, 0); // Sin márgenes cuando colapsado
        m_logLayout->setSpacing(0); // No spacing between widgets
        m_logOutput->hide();
    }
    
    // Force style refresh to apply new property
    m_logGroup->style()->unpolish(m_logGroup);
    m_logGroup->style()->polish(m_logGroup);
    
    // Adjust window size after toggling
    QTimer::singleShot(50, this, &MainWindow::adjustWindowSize);
}

void MainWindow::adjustWindowSize()
{
    // Forzar el cálculo del tamaño de todos los widgets
    m_centralWidget->adjustSize();
    
    // Obtener el tamaño sugerido por el layout
    QSize sizeHint = m_centralWidget->sizeHint();
    
    // Reducir significativamente el margen extra - el problema estaba aquí
    int extraWidth = 20;  // Margen extra horizontal reducido
    int extraHeight = 0; // Margen extra vertical reducido (solo para barra de título)
    
    // Calcular tamaño final usando principalmente el sizeHint del layout
    int finalWidth = qMax(550, sizeHint.width() + extraWidth);
    int finalHeight;
    
    if (m_logExpanded) {
        // Cuando el log está expandido, usar el tamaño sugerido con margen mínimo
        finalHeight = sizeHint.height() + extraHeight;
        finalHeight = qMax(500, finalHeight); // Mínimo razonable para expandido
    } else {
        // Cuando el log está colapsado, usar el tamaño sugerido con margen mínimo
        finalHeight = sizeHint.height() + extraHeight;
        finalHeight = qMax(350, finalHeight); // Mínimo más bajo para colapsado
    }
    
    // Establecer tamaño mínimo y actual
    setMinimumSize(finalWidth, finalHeight);
    resize(finalWidth, finalHeight);
}
