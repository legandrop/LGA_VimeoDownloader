#ifndef DOWNLOADQUEUE_H
#define DOWNLOADQUEUE_H

#include <QObject>
#include <QQueue>
#include <QProcess>
#include <QTextEdit>
#include <QProgressBar>
#include <QGroupBox>
#include <QTimer>
#include <QMutex>

#include "downloaditem.h"

class DownloadQueue : public QObject
{
    Q_OBJECT

public:
    explicit DownloadQueue(QTextEdit *logOutput, QProgressBar *progressBar, QGroupBox *progressGroup, QObject *parent = nullptr);
    ~DownloadQueue();

    // Queue management
    void addDownload(const QString &url, const QString &username, const QString &password, const QString &downloadDir);
    void startQueue();
    void pauseQueue();
    void clearQueue();
    void resetQueue(); // Complete reset including counters
    void cancelCurrentDownload();
    
    // Status getters
    bool isRunning() const { return m_isRunning; }
    bool isPaused() const { return m_isPaused; }
    int getCurrentIndex() const { return m_completedCount; }
    int getTotalCount() const { return m_totalCount; }
    int getQueueSize() const { return m_queue.size(); }
    
    // Current download info
    DownloadItem getCurrentDownload() const;
    QList<DownloadItem> getCompletedDownloads() const { return m_completedDownloads; }

signals:
    void downloadStarted(const DownloadItem &item);
    void downloadProgress(int percentage);
    void downloadCompleted(const DownloadItem &item);
    void downloadFailed(const DownloadItem &item, const QString &error);
    void queueFinished();
    void queueStatusChanged(int current, int total);
    void downloadAddedToQueue(int totalCount);

private slots:
    void processNextDownload();
    void onDownloadFinished(int exitCode, QProcess::ExitStatus exitStatus);
    void onDownloadOutput();
    void onDownloadError();

private:
    void updateProgressLabel();
    void logMessage(const QString &message);
    void startDownloadProcess(const DownloadItem &item);
    void cleanupCurrentProcess();
    
    // UI references
    QTextEdit *m_logOutput;
    QProgressBar *m_progressBar;
    QGroupBox *m_progressGroup;
    
    // Queue management
    QQueue<DownloadItem> m_queue;
    QList<DownloadItem> m_completedDownloads;
    DownloadItem m_currentDownload;
    
    // Process management
    QProcess *m_currentProcess;
    QMutex m_queueMutex;
    
    // Status tracking
    bool m_isRunning;
    bool m_isPaused;
    int m_completedCount;
    int m_totalCount;
    bool m_hasCurrentDownload;
};

#endif // DOWNLOADQUEUE_H
