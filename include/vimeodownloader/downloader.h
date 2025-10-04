#ifndef DOWNLOADER_H
#define DOWNLOADER_H

#include <QObject>
#include <QProcess>
#include <QString>
#include <QTimer>

class Downloader : public QObject
{
    Q_OBJECT

public:
    explicit Downloader(QObject *parent = nullptr);
    ~Downloader();

    void downloadVideo(const QString &url);
    void cancelDownload();
    bool isDownloading() const;

signals:
    void downloadStarted();
    void downloadProgress(int percentage);
    void downloadFinished(bool success, const QString &message);
    void downloadError(const QString &error);
    void logMessage(const QString &message);

private slots:
    void onProcessFinished(int exitCode, QProcess::ExitStatus exitStatus);
    void onProcessError(QProcess::ProcessError error);
    void onProcessOutput();

private:
    void setupYtDlp();
    QString getYtDlpPath();
    
    QProcess *m_process;
    bool m_isDownloading;
    QString m_currentUrl;
};

#endif // DOWNLOADER_H
