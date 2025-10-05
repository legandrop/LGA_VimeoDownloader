#ifndef TOOLSMANAGER_H
#define TOOLSMANAGER_H

#include <QObject>
#include <QTextEdit>
#include <QPushButton>
#include <QProcess>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QTimer>

class ToolsManager : public QObject
{
    Q_OBJECT

public:
    explicit ToolsManager(QTextEdit *logOutput, QPushButton *toolsButton, QObject *parent = nullptr);
    ~ToolsManager();

    // Public interface
    void checkToolsInstallation();
    void installOrUpdateTools();
    bool areToolsInstalled() const;
    
    // Tool status getters
    bool isYtDlpInstalled() const { return m_ytDlpInstalled; }
    bool isFfmpegInstalled() const { return m_ffmpegInstalled; }
    
    // Tool path getters
    QString getYtDlpPath() const;
    QString getFfmpegPath() const;

signals:
    void toolsStatusChanged(bool allInstalled);
    void installationFinished(bool success);

private slots:
    void onInstallUpdateClicked();

private:
    // Detection methods
    void checkYtDlpInstallation();
    void checkFfmpegInstallation();
    void updateButtonState();
    
    // Installation methods - macOS
    void installYtDlpMac();
    void updateYtDlpMac();
    void installFfmpegMac();
    void updateFfmpegMac();
    
    // Installation methods - Windows
    void downloadYtDlpWindows();
    void downloadFfmpegWindows();
    
    // Helper methods
    void logMessage(const QString &message);
    void setButtonEnabled(bool enabled);
    void setButtonText(const QString &text);
    void setButtonStyle(const QString &styleClass);
    QString getBrewPath() const;
    
    // UI references
    QTextEdit *m_logOutput;
    QPushButton *m_toolsButton;
    
    // Tool status
    bool m_ytDlpInstalled;
    bool m_ffmpegInstalled;
    bool m_checkingTools;
    
    // Network manager for downloads
    QNetworkAccessManager *m_networkManager;
    
    // Process counters for async operations
    int m_pendingProcesses;
};

#endif // TOOLSMANAGER_H
