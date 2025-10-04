#ifndef DOWNLOADITEM_H
#define DOWNLOADITEM_H

#include <QString>
#include <QDateTime>

enum class DownloadStatus {
    Pending,
    Downloading,
    Completed,
    Failed,
    Cancelled
};

struct DownloadItem {
    QString url;
    QString username;
    QString password;
    QString downloadDir;
    QString title;
    DownloadStatus status;
    QDateTime addedTime;
    QDateTime startTime;
    QDateTime finishTime;
    int progress;
    QString errorMessage;
    
    DownloadItem() 
        : status(DownloadStatus::Pending)
        , addedTime(QDateTime::currentDateTime())
        , progress(0) 
    {}
    
    DownloadItem(const QString &url, const QString &user, const QString &pass, const QString &dir)
        : url(url)
        , username(user)
        , password(pass)
        , downloadDir(dir)
        , status(DownloadStatus::Pending)
        , addedTime(QDateTime::currentDateTime())
        , progress(0)
    {}
    
    bool isFinished() const {
        return status == DownloadStatus::Completed || 
               status == DownloadStatus::Failed || 
               status == DownloadStatus::Cancelled;
    }
    
    QString getStatusString() const {
        switch (status) {
            case DownloadStatus::Pending: return "Pending";
            case DownloadStatus::Downloading: return "Downloading";
            case DownloadStatus::Completed: return "Completed";
            case DownloadStatus::Failed: return "Failed";
            case DownloadStatus::Cancelled: return "Cancelled";
            default: return "Unknown";
        }
    }
};

#endif // DOWNLOADITEM_H
