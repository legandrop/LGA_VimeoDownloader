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


// Constante para mantener consistencia de ancho del grupo settings
constexpr int SETTINGS_GROUP_WIDTH = 520;


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
    , m_settingsExpanded(false)
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
    , m_maxWindowWidth(550) // Ancho mínimo para evitar problemas cuando settings inicia colapsado
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
    connect(m_toolsManager, &ToolsManager::toolsStatusChanged, this, &MainWindow::onToolsStatusChangedForInitialState);
    m_toolsManager->checkToolsInstallation();

    // Set initial settings state based on credentials (tools status will be handled by signal)
    setInitialSettingsState();
    
    // Initialize download queue
    m_downloadQueue = new DownloadQueue(m_logOutput, m_progressBar, m_progressGroup, this);
    connect(m_downloadQueue, &DownloadQueue::downloadStarted, this, &MainWindow::onDownloadStarted);
    connect(m_downloadQueue, &DownloadQueue::downloadCompleted, this, &MainWindow::onDownloadCompleted);
    connect(m_downloadQueue, &DownloadQueue::queueStatusChanged, this, &MainWindow::onQueueStatusChanged);
    connect(m_downloadQueue, &DownloadQueue::downloadAddedToQueue, this, &MainWindow::onDownloadAddedToQueue);
    
    // Configurar ventana
    setWindowTitle("LGA_VimeoDownloader v0.81");

    // Ajustar tamaño inicial y establecer ancho máximo
    adjustWindowSize();
    // Después del ajuste inicial, aseguramos que el ancho máximo esté establecido
    // y que todos los widgets estén completamente inicializados
    QTimer::singleShot(500, this, [this]() {
        adjustWindowSize();
    });
    
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
    m_mainLayout->setContentsMargins(16, 20, 12, 20);

    // Establecer restricción fija para evitar redimensionamiento automático
    // pero permitir ajustes manuales cuando cambie la visibilidad de widgets internos (como el log)
    m_mainLayout->setSizeConstraint(QLayout::SetFixedSize);
    
    // Video URL Group
    m_inputGroup = new QGroupBox("Video URL", this);
    // Política de tamaño que permite ajuste mínimo pero mantiene estabilidad
    m_inputGroup->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
    m_inputLayout = new QVBoxLayout(m_inputGroup);
    m_inputLayout->setSpacing(8);
    
    // Layout horizontal para URL y botón
    m_urlLayout = new QHBoxLayout();
    m_urlInput = new QLineEdit(this);
    m_urlInput->setPlaceholderText("https://vimeo.com/... or https://youtube.com/...");
    m_urlInput->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    m_downloadButton = new QPushButton("Download", this);
    m_downloadButton->setEnabled(true); // Temporalmente habilitado para testing
    m_downloadButton->setFixedWidth(110);

    m_urlLayout->addWidget(m_urlInput);
    m_urlLayout->addWidget(m_downloadButton);
    
    m_inputLayout->addLayout(m_urlLayout);
    
    // Progress Group
    m_progressGroup = new QGroupBox("Progress (0/0)", this);
    // Política de tamaño que permite ajuste mínimo pero mantiene estabilidad
    m_progressGroup->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
    m_progressLayout = new QVBoxLayout(m_progressGroup);
    m_progressLayout->setSpacing(8);
    
    // Progress bar and cancel button in same line (like URL layout)
    m_progressButtonLayout = new QHBoxLayout();
    m_progressBar = new QProgressBar(this);
    m_progressBar->setRange(0, 100);
    m_progressBar->setValue(0);
    m_progressBar->setTextVisible(false); // Hide percentage text when inactive
    m_progressBar->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

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
    
    // Settings Group - clickable like log group (starts expanded)
    m_settingsGroup = new QGroupBox("Settings ⌄", this);
    m_settingsGroup->setObjectName("settingsGroupBox");
    m_settingsGroup->setProperty("collapsed", false); // Not collapsed initially
    m_settingsGroup->setCursor(Qt::PointingHandCursor);
    // Política de tamaño que permite ajuste mínimo pero mantiene estabilidad
    m_settingsGroup->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
    m_settingsLayout = new QVBoxLayout(m_settingsGroup);
    m_settingsLayout->setSpacing(8);
    // Agregar padding interno consistente con otras secciones cuando esté expandido
    m_settingsLayout->setContentsMargins(10, 10, 10, 4);
    
    // First row: Username | Password | Save
    m_credentialsLayout = new QHBoxLayout();
    m_userInput = new QLineEdit(this);
    m_userInput->setPlaceholderText("Vimeo Username...");
    m_userInput->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    m_passwordInput = new QLineEdit(this);
    m_passwordInput->setEchoMode(QLineEdit::Password);
    m_passwordInput->setPlaceholderText("Vimeo Password...");
    m_passwordInput->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    m_saveCredentialsButton = new QPushButton("Save", this);
    m_saveCredentialsButton->setFixedWidth(110);

    m_credentialsLayout->addWidget(m_userInput);
    m_credentialsLayout->addWidget(m_passwordInput);
    m_credentialsLayout->addWidget(m_saveCredentialsButton);

    // Agregar padding interno consistente con otros grupos
    m_credentialsLayout->setContentsMargins(10, 4, 10, 4);
    
    // Second row: Download Folder | Browse
    m_folderLayout = new QHBoxLayout();
    m_downloadFolderInput = new QLineEdit(this);
    m_downloadFolderInput->setPlaceholderText("Download Folder...");
    m_downloadFolderInput->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    m_browseFolderButton = new QPushButton("Browse", this);
    m_browseFolderButton->setFixedWidth(110);

    m_folderLayout->addWidget(m_downloadFolderInput);
    m_folderLayout->addWidget(m_browseFolderButton);

    // Agregar padding interno consistente con otros grupos
    m_folderLayout->setContentsMargins(10, 4, 10, 4);
    
    // Third row: tools button aligned right
    m_toolsLayout = new QHBoxLayout();
    m_toolsButton = new QPushButton("Checking Tools...", this);
    m_toolsButton->setObjectName("toolsButton");
    m_toolsButton->setEnabled(false);
    m_toolsButton->setFixedWidth(110);

    // Usar un widget spacer fijo en lugar de addStretch() para evitar recálculos
    QWidget *spacer = new QWidget(this);
    spacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    spacer->setFixedHeight(0);

    m_toolsLayout->addWidget(spacer);
    m_toolsLayout->addWidget(m_toolsButton);

    // Agregar padding interno consistente con otros grupos
    m_toolsLayout->setContentsMargins(10, 4, 10, 10);
    
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
    connect(m_urlInput, &QLineEdit::returnPressed, this, &MainWindow::onDownloadClicked);
    connect(m_downloadButton, &QPushButton::clicked, this, &MainWindow::onDownloadClicked);
    connect(m_saveCredentialsButton, &QPushButton::clicked, this, &MainWindow::onSaveCredentialsClicked);
    connect(m_browseFolderButton, &QPushButton::clicked, this, &MainWindow::onBrowseFolderClicked);
    connect(m_cancelButton, &QPushButton::clicked, this, &MainWindow::onCancelClicked);

    // Install event filter for log group box to capture clicks
    m_logGroup->installEventFilter(this);

    // Install event filter for settings group box to capture clicks
    m_settingsGroup->installEventFilter(this);
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
    
    // Temporalmente siempre habilitado para testing - comentar esta línea después de probar
    m_downloadButton->setEnabled(true);
    // m_downloadButton->setEnabled(isValidUrl && hasCredentials && hasDownloadDir && m_toolsManager->areToolsInstalled());
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

bool MainWindow::shouldShowSettingsExpanded()
{
    // Settings debe abrir expandido si:
    // 1. Usuario o contraseña están vacíos (no guardados)
    QString user = m_settings->value("vimeo/username", "").toString();
    QString password = m_settings->value("vimeo/password", "").toString();
    bool credentialsEmpty = user.isEmpty() || password.isEmpty();

    // 2. O si la carpeta de destino está vacía
    QString downloadDir = m_settings->value("download/folder", "").toString();
    bool downloadDirEmpty = downloadDir.isEmpty();

    // 3. O si las herramientas no están instaladas
    bool toolsNotInstalled = m_toolsManager && !m_toolsManager->areToolsInstalled();

    return credentialsEmpty || downloadDirEmpty || toolsNotInstalled;
}

void MainWindow::setInitialSettingsState()
{
    // Determinar estado inicial basado en credenciales, carpeta de destino y herramientas
    QString user = m_settings->value("vimeo/username", "").toString();
    QString password = m_settings->value("vimeo/password", "").toString();
    QString downloadDir = m_settings->value("download/folder", "").toString();

    bool credentialsEmpty = user.isEmpty() || password.isEmpty();
    bool downloadDirEmpty = downloadDir.isEmpty();

    // Settings inicia expandido si no hay credenciales o no hay carpeta de destino
    m_settingsExpanded = credentialsEmpty || downloadDirEmpty;

    // Configurar estado visual inicial
    if (m_settingsExpanded) {
        m_settingsGroup->setTitle("Settings ⌄");
        m_settingsGroup->setProperty("collapsed", false);
        m_settingsGroup->setFixedHeight(QWIDGETSIZE_MAX);
        m_settingsGroup->setMinimumHeight(0);
        m_settingsGroup->setMaximumHeight(QWIDGETSIZE_MAX);
        // Mantener ancho consistente cuando está expandido
        m_settingsGroup->setMinimumWidth(SETTINGS_GROUP_WIDTH);
        m_settingsGroup->setMaximumWidth(SETTINGS_GROUP_WIDTH);
        m_settingsLayout->setContentsMargins(3, 2, 3, 3);
        m_settingsLayout->setSpacing(8);
        // Show all settings widgets
        m_userInput->show();
        m_passwordInput->show();
        m_saveCredentialsButton->show();
        m_downloadFolderInput->show();
        m_browseFolderButton->show();
        m_toolsButton->show();
    } else {
        m_settingsGroup->setTitle("Settings >");
        m_settingsGroup->setProperty("collapsed", true);
        m_settingsGroup->setFixedHeight(35);
        // Establecer ancho mínimo fijo para evitar que otros grupos se contraigan
        // Basado en el tamaño típico cuando está expandido con todos los controles
        m_settingsGroup->setMinimumWidth(SETTINGS_GROUP_WIDTH); // Ancho conservador para settings expandido
        m_settingsGroup->setMaximumWidth(SETTINGS_GROUP_WIDTH);
        m_settingsLayout->setContentsMargins(0, 0, 0, 0);
        m_settingsLayout->setSpacing(0);
        // Hide all settings widgets initially
        m_userInput->hide();
        m_passwordInput->hide();
        m_saveCredentialsButton->hide();
        m_downloadFolderInput->hide();
        m_browseFolderButton->hide();
        m_toolsButton->hide();
    }

    // Force style refresh to apply new property
    m_settingsGroup->style()->unpolish(m_settingsGroup);
    m_settingsGroup->style()->polish(m_settingsGroup);
}

void MainWindow::onToolsStatusChangedForInitialState(bool allInstalled)
{
    // Si las herramientas no están instaladas, asegurar que settings esté expandido
    if (!allInstalled && !m_settingsExpanded) {
        m_settingsExpanded = true;

        m_settingsGroup->setTitle("Settings ⌄");
        m_settingsGroup->setProperty("collapsed", false);
        m_settingsGroup->setFixedHeight(QWIDGETSIZE_MAX);
        m_settingsGroup->setMinimumHeight(0);
        m_settingsGroup->setMaximumHeight(QWIDGETSIZE_MAX);
        // Mantener ancho consistente cuando está expandido
        m_settingsGroup->setMinimumWidth(SETTINGS_GROUP_WIDTH);
        m_settingsGroup->setMaximumWidth(SETTINGS_GROUP_WIDTH);
        m_settingsLayout->setContentsMargins(3, 2, 3, 3);
        m_settingsLayout->setSpacing(8);

        // Show all settings widgets
        m_userInput->show();
        m_passwordInput->show();
        m_saveCredentialsButton->show();
        m_downloadFolderInput->show();
        m_browseFolderButton->show();
        m_toolsButton->show();

        // Force style refresh to apply new property
        m_settingsGroup->style()->unpolish(m_settingsGroup);
        m_settingsGroup->style()->polish(m_settingsGroup);

        // Adjust window size after expanding settings
        QTimer::singleShot(100, this, &MainWindow::adjustWindowSize);
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

    if (obj == m_settingsGroup && event->type() == QEvent::MouseButtonPress) {
        QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);
        if (mouseEvent->button() == Qt::LeftButton) {
            onSettingsToggleClicked();
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

    // Adjust window size after toggling with a small delay to ensure proper layout calculation
    QTimer::singleShot(100, this, &MainWindow::adjustWindowSize);
}

void MainWindow::onSettingsToggleClicked()
{
    m_settingsExpanded = !m_settingsExpanded;

    if (m_settingsExpanded) {
        m_settingsGroup->setTitle("Settings ⌄");
        m_settingsGroup->setProperty("collapsed", false); // Not collapsed
        m_settingsGroup->setFixedHeight(QWIDGETSIZE_MAX); // Permitir que se expanda
        m_settingsGroup->setMinimumHeight(0); // Sin altura mínima
        m_settingsGroup->setMaximumHeight(QWIDGETSIZE_MAX); // Sin límite máximo
        // Mantener ancho consistente cuando está expandido también
        m_settingsGroup->setMinimumWidth(SETTINGS_GROUP_WIDTH);
        m_settingsGroup->setMaximumWidth(SETTINGS_GROUP_WIDTH);
        m_settingsLayout->setContentsMargins(3, 2, 3, 3); // Márgenes consistentes con CSS
        m_settingsLayout->setSpacing(8); // Espaciado normal
        // Show all settings widgets
        m_userInput->show();
        m_passwordInput->show();
        m_saveCredentialsButton->show();
        m_downloadFolderInput->show();
        m_browseFolderButton->show();
        m_toolsButton->show();
    } else {
        m_settingsGroup->setTitle("Settings >");
        m_settingsGroup->setProperty("collapsed", true); // Collapsed
        m_settingsGroup->setFixedHeight(35); // Altura compacta
        // Usar ancho fijo consistente para mantener el layout estable
        m_settingsGroup->setMinimumWidth(SETTINGS_GROUP_WIDTH);
        m_settingsGroup->setMaximumWidth(SETTINGS_GROUP_WIDTH);
        m_settingsLayout->setContentsMargins(0, 0, 0, 0); // Sin márgenes cuando colapsado
        m_settingsLayout->setSpacing(0); // No spacing between widgets
        // Hide all settings widgets
        m_userInput->hide();
        m_passwordInput->hide();
        m_saveCredentialsButton->hide();
        m_downloadFolderInput->hide();
        m_browseFolderButton->hide();
        m_toolsButton->hide();
    }

    // Force style refresh to apply new property
    m_settingsGroup->style()->unpolish(m_settingsGroup);
    m_settingsGroup->style()->polish(m_settingsGroup);

    // Adjust window size after toggling with a small delay to ensure proper layout calculation
    QTimer::singleShot(100, this, &MainWindow::adjustWindowSize);
}

// Height constants for window sizing (configurable values) - defined here for easy adjustment

void MainWindow::adjustWindowSize()
{
    // Height constants for window sizing (configurable values) - defined here for easy adjustment
    static const int MIN_HEIGHT_BOTH_EXPANDED = 600;      // Ambas secciones expandidas
    static const int MIN_HEIGHT_BOTH_COLLAPSED = 300;    // Ambas secciones contraídas
    static const int MIN_HEIGHT_LOG_EXPANDED = 500;      // Solo Log expandido, Settings contraído
    static const int MIN_HEIGHT_SETTINGS_EXPANDED = 350; // Solo Settings expandido, Log contraído
    static const int MAX_HEIGHT_LOG_EXPANDED = 650;      // Máximo cuando Log está expandido
    static const int MAX_HEIGHT_SETTINGS_EXPANDED = 620;  // Máximo cuando Settings está expandido

    // Forzar el cálculo del tamaño de todos los widgets
    m_centralWidget->adjustSize();

    // Obtener el tamaño sugerido por el layout
    QSize sizeHint = m_centralWidget->sizeHint();

    // El layout tiene restricción fija, así que establecemos el tamaño manualmente
    int extraWidth = 5;
    int currentWidth = sizeHint.width() + extraWidth;

    // Siempre mantener el ancho máximo registrado, independientemente del estado de expansión
    if (currentWidth > m_maxWindowWidth) {
        m_maxWindowWidth = currentWidth;
    }

    // Siempre usar el ancho máximo para evitar que las secciones se achiquen
    int finalWidth = m_maxWindowWidth > 0 ? m_maxWindowWidth : currentWidth;

    // Calcular altura según el estado del log y settings
    int finalHeight;

    // Usar el sizeHint cuando ambas secciones están en el mismo estado (ambas expandidas o ambas contraídas)
    bool anyExpanded = m_logExpanded || m_settingsExpanded;
    bool bothSameState = (m_logExpanded && m_settingsExpanded) || (!m_logExpanded && !m_settingsExpanded);

    if (bothSameState) {
        // Cuando ambas están en el mismo estado, usar el sizeHint que funciona bien
        finalHeight = sizeHint.height();
        finalHeight = anyExpanded ? qMax(MIN_HEIGHT_BOTH_EXPANDED, finalHeight) : qMax(MIN_HEIGHT_BOTH_COLLAPSED, finalHeight);
    } else {
        // Una sección expandida y otra contraída - usar constantes específicas
        if (m_logExpanded && !m_settingsExpanded) {
            // Solo Log expandido, Settings contraído
            finalHeight = sizeHint.height();
            if (finalHeight > MAX_HEIGHT_LOG_EXPANDED) {
                finalHeight = MAX_HEIGHT_LOG_EXPANDED;
            }
            finalHeight = qMax(MIN_HEIGHT_LOG_EXPANDED, finalHeight);
        } else if (!m_logExpanded && m_settingsExpanded) {
            // Solo Settings expandido, Log contraído
            finalHeight = sizeHint.height();
            if (finalHeight > MAX_HEIGHT_SETTINGS_EXPANDED) {
                finalHeight = MAX_HEIGHT_SETTINGS_EXPANDED;
            }
            finalHeight = qMax(MIN_HEIGHT_SETTINGS_EXPANDED, finalHeight);
        } else {
            // Fallback - ambas contraídas (aunque no debería llegar aquí)
            finalHeight = sizeHint.height();
            finalHeight = qMax(MIN_HEIGHT_BOTH_COLLAPSED, finalHeight);
        }
    }

    // Ajustar el tamaño de la ventana al tamaño óptimo
    setFixedSize(finalWidth, finalHeight);

    // También establecer el tamaño mínimo para permitir algo de flexibilidad
    setMinimumSize(400, finalHeight);
    setMaximumSize(QWIDGETSIZE_MAX, QWIDGETSIZE_MAX);
}
