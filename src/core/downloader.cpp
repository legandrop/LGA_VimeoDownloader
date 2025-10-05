#include "vimeodownloader/downloader.h"

#include <QDebug>
#include <QStandardPaths>
#include <QDir>
#include <QCoreApplication>
#include <QRegularExpression>
#include <QFile>

Downloader::Downloader(QObject *parent)
    : QObject(parent)
    , m_process(nullptr)
    , m_isDownloading(false)
    , m_currentUrl("")
{
    setupYtDlp();
}

Downloader::~Downloader()
{
    if (m_process && m_process->state() != QProcess::NotRunning) {
        m_process->kill();
        m_process->waitForFinished(3000);
    }
    
    if (m_process) {
        delete m_process;
    }
}

void Downloader::setupYtDlp()
{
    // Inicializar el proceso
    m_process = new QProcess(this);
    
    // Conectar señales
    connect(m_process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this, &Downloader::onProcessFinished);
    connect(m_process, &QProcess::errorOccurred, this, &Downloader::onProcessError);
    connect(m_process, &QProcess::readyReadStandardOutput, this, &Downloader::onProcessOutput);
    connect(m_process, &QProcess::readyReadStandardError, this, &Downloader::onProcessOutput);
}

QString Downloader::getYtDlpPath()
{
    // Buscar yt-dlp en el PATH del sistema
    QString program = "yt-dlp";
    
#ifdef Q_OS_WIN
    program += ".exe";
#endif
    
    return program; // QProcess buscará en el PATH automáticamente
}

void Downloader::downloadVideo(const QString &url)
{
    if (m_isDownloading) {
        emit downloadError("Ya hay una descarga en progreso");
        return;
    }
    
    if (url.isEmpty()) {
        emit downloadError("URL vacía");
        return;
    }
    
    m_currentUrl = url;
    m_isDownloading = true;
    
    // Directorio de descarga (Escritorio del usuario)
    QString downloadDir = QStandardPaths::writableLocation(QStandardPaths::DesktopLocation);
    
    // Argumentos para yt-dlp
    QStringList arguments;
    arguments << "--output" << downloadDir + "/%(title)s.%(ext)s";
    arguments << "--format" << "bv*+ba/b"; // Best video + best audio, fallback to best single file
    arguments << "--merge-output-format" << "mp4"; // Force MP4 output when merging
    arguments << "--progress"; // Mostrar progreso
    
    // Add ffmpeg location for proper merging (if available)
    // Note: This downloader doesn't have access to ToolsManager, so we try common locations
#ifdef Q_OS_WIN
    QString appDir = QCoreApplication::applicationDirPath();
    QString ffmpegPath = appDir + "/tools/ffmpeg.exe";
    if (QFile::exists(ffmpegPath)) {
        arguments << "--ffmpeg-location" << ffmpegPath;
    }
#elif defined(Q_OS_MAC)
    QString appDir = QCoreApplication::applicationDirPath();
    QString ffmpegPath = appDir + "/toolsmac/ffmpeg";
    if (QFile::exists(ffmpegPath)) {
        arguments << "--ffmpeg-location" << ffmpegPath;
    }
#endif
    
    // Add cookies from browser for YouTube (helps avoid bot detection)
    if (url.contains("youtube.com") || url.contains("youtu.be")) {
        arguments << "--cookies-from-browser" << "chrome";
    }
    
    arguments << url;
    
    emit logMessage(QString("Ejecutando: yt-dlp %1").arg(arguments.join(" ")));
    emit downloadStarted();
    
    // Iniciar el proceso
    QString ytDlpPath = getYtDlpPath();
    m_process->start(ytDlpPath, arguments);
    
    if (!m_process->waitForStarted(5000)) {
        m_isDownloading = false;
        emit downloadError("No se pudo iniciar yt-dlp. Asegúrate de que esté instalado y en el PATH.");
        return;
    }
    
    emit logMessage("Descarga iniciada...");
}

void Downloader::cancelDownload()
{
    if (m_process && m_process->state() != QProcess::NotRunning) {
        m_process->kill();
        emit logMessage("Descarga cancelada por el usuario");
    }
}

bool Downloader::isDownloading() const
{
    return m_isDownloading;
}

void Downloader::onProcessFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    m_isDownloading = false;
    
    if (exitStatus == QProcess::CrashExit) {
        emit downloadError("El proceso yt-dlp se cerró inesperadamente");
        emit downloadFinished(false, "Proceso terminado inesperadamente");
    } else if (exitCode == 0) {
        emit logMessage("Descarga completada exitosamente");
        emit downloadFinished(true, "Descarga completada");
    } else {
        QString errorMsg = QString("yt-dlp terminó con código de error: %1").arg(exitCode);
        emit downloadError(errorMsg);
        emit downloadFinished(false, errorMsg);
    }
}

void Downloader::onProcessError(QProcess::ProcessError error)
{
    m_isDownloading = false;
    
    QString errorMsg;
    switch (error) {
    case QProcess::FailedToStart:
        errorMsg = "No se pudo iniciar yt-dlp. Verifica que esté instalado.";
        break;
    case QProcess::Crashed:
        errorMsg = "yt-dlp se cerró inesperadamente.";
        break;
    case QProcess::Timedout:
        errorMsg = "Tiempo de espera agotado.";
        break;
    case QProcess::WriteError:
        errorMsg = "Error de escritura en el proceso.";
        break;
    case QProcess::ReadError:
        errorMsg = "Error de lectura del proceso.";
        break;
    default:
        errorMsg = "Error desconocido en el proceso.";
        break;
    }
    
    emit downloadError(errorMsg);
    emit downloadFinished(false, errorMsg);
}

void Downloader::onProcessOutput()
{
    if (!m_process) return;
    
    // Leer salida estándar
    QByteArray data = m_process->readAllStandardOutput();
    if (!data.isEmpty()) {
        QString output = QString::fromUtf8(data).trimmed();
        emit logMessage(output);
        
        // Intentar extraer progreso si está disponible
        if (output.contains("%")) {
            // Buscar patrones de progreso típicos de yt-dlp
            // Ejemplo: "[download]  45.2% of 123.45MiB at 1.23MiB/s ETA 00:30"
            QRegularExpression progressRegex("\\[download\\]\\s+(\\d+(?:\\.\\d+)?)%");
            QRegularExpressionMatch match = progressRegex.match(output);
            if (match.hasMatch()) {
                bool ok;
                double progress = match.captured(1).toDouble(&ok);
                if (ok) {
                    emit downloadProgress(static_cast<int>(progress));
                }
            }
        }
    }
    
    // Leer salida de error
    QByteArray errorData = m_process->readAllStandardError();
    if (!errorData.isEmpty()) {
        QString errorOutput = QString::fromUtf8(errorData).trimmed();
        emit logMessage(QString("ERROR: %1").arg(errorOutput));
    }
}
