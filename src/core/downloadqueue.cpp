#include "vimeodownloader/downloadqueue.h"
#include "vimeodownloader/toolsmanager.h"

#include <QRegularExpression>
#include <QMutexLocker>
#include <QTimer>

DownloadQueue::DownloadQueue(QTextEdit *logOutput, QProgressBar *progressBar, QGroupBox *progressGroup, ToolsManager *toolsManager, QObject *parent)
    : QObject(parent)
    , m_logOutput(logOutput)
    , m_progressBar(progressBar)
    , m_progressGroup(progressGroup)
    , m_toolsManager(toolsManager)
    , m_currentProcess(nullptr)
    , m_isRunning(false)
    , m_isPaused(false)
    , m_completedCount(0)
    , m_totalCount(0)
    , m_hasCurrentDownload(false)
    , m_totalFragments(0)
    , m_currentFragment(0)
{
    updateProgressLabel();
}

DownloadQueue::~DownloadQueue()
{
    cleanupCurrentProcess();
}

void DownloadQueue::addDownload(const QString &url, const QString &username, const QString &password, const QString &downloadDir)
{
    QMutexLocker locker(&m_queueMutex);
    
    DownloadItem item(url, username, password, downloadDir);
    m_queue.enqueue(item);
    m_totalCount++;
    
    updateProgressLabel();
    
    logMessage(QString("=== Download Added to Queue ==="));
    logMessage(QString("URL: %1").arg(url));
    logMessage(QString("Queue position: %1 of %2").arg(m_queue.size()).arg(m_totalCount));
    logMessage("---");
    
    // Emit signal for total count update, but don't change current number
    emit downloadAddedToQueue(m_totalCount);
    
    // Auto-start queue if not running
    if (!m_isRunning && !m_isPaused) {
        QTimer::singleShot(100, this, &DownloadQueue::startQueue);
    }
}

void DownloadQueue::startQueue()
{
    if (m_isRunning) {
        return;
    }
    
    m_isRunning = true;
    m_isPaused = false;
    
    logMessage("=== Starting Download Queue ===");
    processNextDownload();
}

void DownloadQueue::pauseQueue()
{
    m_isPaused = true;
    
    if (m_currentProcess && m_currentProcess->state() == QProcess::Running) {
        logMessage("=== Pausing Download Queue ===");
        logMessage("Current download will finish, then queue will pause");
    } else {
        m_isRunning = false;
        logMessage("=== Download Queue Paused ===");
    }
}

void DownloadQueue::clearQueue()
{
    QMutexLocker locker(&m_queueMutex);
    
    // Cancel current download if running
    if (m_currentProcess && m_currentProcess->state() == QProcess::Running) {
        cancelCurrentDownload();
    }
    
    m_queue.clear();
    m_isRunning = false;
    m_isPaused = false;
    
    logMessage("=== Download Queue Cleared ===");
    updateProgressLabel();
    emit queueStatusChanged(m_completedCount, m_totalCount);
}

void DownloadQueue::resetQueue()
{
    QMutexLocker locker(&m_queueMutex);
    
    // Cancel current download if running
    if (m_currentProcess && m_currentProcess->state() == QProcess::Running) {
        cancelCurrentDownload();
    }
    
    // Clear everything and reset counters
    m_queue.clear();
    m_completedDownloads.clear();
    m_completedCount = 0;
    m_totalCount = 0;
    m_isRunning = false;
    m_isPaused = false;
    m_hasCurrentDownload = false;
    
    logMessage("=== Download Queue Reset - All counters cleared ===");
    updateProgressLabel();
    emit queueStatusChanged(0, 0);
}

void DownloadQueue::cancelCurrentDownload()
{
    if (m_currentProcess && m_currentProcess->state() == QProcess::Running) {
        logMessage("=== Cancelling Current Download ===");
        m_currentDownload.status = DownloadStatus::Cancelled;
        m_currentProcess->kill();
        m_currentProcess->waitForFinished(3000);
    }
}

void DownloadQueue::processNextDownload()
{
    QMutexLocker locker(&m_queueMutex);
    
    // Check if paused
    if (m_isPaused) {
        m_isRunning = false;
        logMessage("=== Queue Paused ===");
        return;
    }
    
    // Check if queue is empty
    if (m_queue.isEmpty()) {
        m_isRunning = false;
        m_hasCurrentDownload = false;
        
        if (m_completedCount > 0) {
            logMessage("=== All Downloads Completed ===");
            logMessage(QString("Total downloads processed: %1").arg(m_completedCount));
        }
        
        emit queueFinished();
        return;
    }
    
    // Get next download
    m_currentDownload = m_queue.dequeue();
    m_hasCurrentDownload = true;
    m_currentDownload.status = DownloadStatus::Downloading;
    m_currentDownload.startTime = QDateTime::currentDateTime();
    
    updateProgressLabel();
    emit downloadStarted(m_currentDownload);
    emit queueStatusChanged(m_completedCount + 1, m_totalCount);
    
    // Start download process
    startDownloadProcess(m_currentDownload);
}

void DownloadQueue::startDownloadProcess(const DownloadItem &item)
{
    // Clean up any existing process
    cleanupCurrentProcess();
    
    // Create new process
    m_currentProcess = new QProcess(this);
    
    // Connect signals
    connect(m_currentProcess, &QProcess::readyReadStandardOutput, this, &DownloadQueue::onDownloadOutput);
    connect(m_currentProcess, &QProcess::readyReadStandardError, this, &DownloadQueue::onDownloadError);
    connect(m_currentProcess, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this, &DownloadQueue::onDownloadFinished);
    
    // Prepare yt-dlp arguments
    QStringList arguments;
    arguments << "-u" << item.username;
    arguments << "-p" << item.password;
    arguments << "--output" << item.downloadDir + "/%(title)s.%(ext)s";
    arguments << "--format" << "bv*+ba/b"; // Best video + best audio, fallback to best single file
    arguments << item.url;
    
    // Activate progress bar and show percentage text
    m_progressBar->setTextVisible(true);
    m_progressBar->setValue(0);
    
    // Reset fragment tracking for new download
    m_totalFragments = 0;
    m_currentFragment = 0;
    
    // Log start
    logMessage(QString("=== Starting Download %1 of %2 ===").arg(m_completedCount + 1).arg(m_totalCount));
    logMessage(QString("URL: %1").arg(item.url));
    logMessage(QString("User: %1").arg(item.username));
    logMessage(QString("Download Folder: %1").arg(item.downloadDir));
    logMessage("---");
    
    // Start process
    QString ytDlpPath = m_toolsManager->getYtDlpPath();
    logMessage(QString("Executing: %1 %2").arg(ytDlpPath).arg(arguments.join(" ").replace(item.password, "***")));
    m_currentProcess->start(ytDlpPath, arguments);
    
    if (!m_currentProcess->waitForStarted(5000)) {
        logMessage("ERROR: Could not start yt-dlp. Verify it's installed.");
        m_currentDownload.status = DownloadStatus::Failed;
        m_currentDownload.errorMessage = "Could not start yt-dlp";
        onDownloadFinished(-1, QProcess::CrashExit);
    }
}

void DownloadQueue::onDownloadOutput()
{
    if (!m_currentProcess) return;
    
    QByteArray data = m_currentProcess->readAllStandardOutput();
    QString output = QString::fromUtf8(data).trimmed();
    
    if (!output.isEmpty()) {
        logMessage(output);
        
        // Check for total fragments info (YouTube HLS downloads)
        QRegularExpression fragmentsRegex("\\[hlsnative\\] Total fragments: (\\d+)");
        QRegularExpressionMatch fragmentsMatch = fragmentsRegex.match(output);
        if (fragmentsMatch.hasMatch()) {
            m_totalFragments = fragmentsMatch.captured(1).toInt();
            logMessage(QString("Detected HLS download with %1 fragments").arg(m_totalFragments));
        }
        
        // Parse progress from yt-dlp output
        QRegularExpression progressRegex("\\[download\\]\\s+(\\d+(?:\\.\\d+)?)%.*\\(frag (\\d+)/(\\d+)\\)");
        QRegularExpressionMatch match = progressRegex.match(output);
        
        if (match.hasMatch()) {
            // Fragment-based progress (YouTube HLS)
            bool ok;
            double fragmentProgress = match.captured(1).toDouble(&ok);
            int currentFrag = match.captured(2).toInt();
            int totalFrag = match.captured(3).toInt();
            
            if (ok && totalFrag > 0) {
                // Update fragment info if we have it
                if (m_totalFragments == 0) {
                    m_totalFragments = totalFrag;
                }
                m_currentFragment = currentFrag;
                
                // Calculate overall progress: (completed fragments + current fragment progress) / total fragments
                double overallProgress = ((double)(currentFrag - 1) + (fragmentProgress / 100.0)) / (double)totalFrag * 100.0;
                int progressInt = static_cast<int>(overallProgress);
                
                // Ensure progress doesn't exceed 100% and is monotonic
                progressInt = qMin(progressInt, 100);
                if (progressInt >= m_currentDownload.progress) {
                    m_currentDownload.progress = progressInt;
                    m_progressBar->setValue(progressInt);
                    emit downloadProgress(progressInt);
                }
            }
        } else {
            // Regular progress (Vimeo or non-fragmented downloads)
            QRegularExpression simpleProgressRegex("\\[download\\]\\s+(\\d+(?:\\.\\d+)?)%");
            QRegularExpressionMatch simpleMatch = simpleProgressRegex.match(output);
            if (simpleMatch.hasMatch()) {
                bool ok;
                double progress = simpleMatch.captured(1).toDouble(&ok);
                if (ok) {
                    int progressInt = static_cast<int>(progress);
                    m_currentDownload.progress = progressInt;
                    m_progressBar->setValue(progressInt);
                    emit downloadProgress(progressInt);
                }
            }
        }
        
        // Check for completion
        if (output.contains("100% of") && output.contains("in ")) {
            m_progressBar->setValue(100);
            m_currentDownload.progress = 100;
        }
        
        // Extract title if available
        if (m_currentDownload.title.isEmpty()) {
            QRegularExpression titleRegex("\\[download\\] Destination: (.+)");
            QRegularExpressionMatch titleMatch = titleRegex.match(output);
            if (titleMatch.hasMatch()) {
                QString fullPath = titleMatch.captured(1);
                QStringList pathParts = fullPath.split("/");
                if (!pathParts.isEmpty()) {
                    m_currentDownload.title = pathParts.last();
                }
            }
        }
    }
}

void DownloadQueue::onDownloadError()
{
    if (!m_currentProcess) return;
    
    QByteArray data = m_currentProcess->readAllStandardError();
    QString output = QString::fromUtf8(data).trimmed();
    
    if (!output.isEmpty()) {
        logMessage("ERROR: " + output);
        m_currentDownload.errorMessage += output + "\n";
    }
}

void DownloadQueue::onDownloadFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    if (!m_hasCurrentDownload) return;
    
    // Deactivate progress bar and hide percentage text
    m_progressBar->setTextVisible(false);
    m_progressBar->setValue(0);
    m_currentDownload.finishTime = QDateTime::currentDateTime();
    
    if (exitStatus == QProcess::CrashExit) {
        m_currentDownload.status = DownloadStatus::Failed;
        if (m_currentDownload.errorMessage.isEmpty()) {
            m_currentDownload.errorMessage = "Process crashed unexpectedly";
        }
        logMessage("ERROR: yt-dlp process crashed unexpectedly");
        emit downloadFailed(m_currentDownload, m_currentDownload.errorMessage);
    } else if (exitCode == 0) {
        m_currentDownload.status = DownloadStatus::Completed;
        m_currentDownload.progress = 100;
        logMessage("=== Download completed successfully ===");
        emit downloadCompleted(m_currentDownload);
    } else {
        m_currentDownload.status = DownloadStatus::Failed;
        if (m_currentDownload.errorMessage.isEmpty()) {
            m_currentDownload.errorMessage = QString("Process finished with error code: %1").arg(exitCode);
        }
        logMessage(QString("ERROR: yt-dlp finished with error code: %1").arg(exitCode));
        emit downloadFailed(m_currentDownload, m_currentDownload.errorMessage);
    }
    
    // Add to completed downloads
    m_completedDownloads.append(m_currentDownload);
    m_completedCount++;
    m_hasCurrentDownload = false;
    
    updateProgressLabel();
    emit queueStatusChanged(m_completedCount, m_totalCount);
    
    // Clean up process
    cleanupCurrentProcess();
    
    // Process next download after a short delay
    QTimer::singleShot(1000, this, &DownloadQueue::processNextDownload);
}

DownloadItem DownloadQueue::getCurrentDownload() const
{
    return m_hasCurrentDownload ? m_currentDownload : DownloadItem();
}

void DownloadQueue::updateProgressLabel()
{
    if (m_progressGroup) {
        int currentNumber = m_hasCurrentDownload ? m_completedCount + 1 : m_completedCount;
        QString text = QString("Progress (%1/%2)").arg(currentNumber).arg(m_totalCount);
        m_progressGroup->setTitle(text);
    }
}

void DownloadQueue::logMessage(const QString &message)
{
    if (m_logOutput) {
        m_logOutput->append(message);
    }
}

void DownloadQueue::cleanupCurrentProcess()
{
    if (m_currentProcess) {
        if (m_currentProcess->state() == QProcess::Running) {
            m_currentProcess->kill();
            m_currentProcess->waitForFinished(3000);
        }
        m_currentProcess->deleteLater();
        m_currentProcess = nullptr;
    }
}
