#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QProgressBar>
#include <QTextEdit>
#include <QGroupBox>
#include <QSettings>

class ToolsManager;
class DownloadQueue;

QT_BEGIN_NAMESPACE
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected:
    bool eventFilter(QObject *obj, QEvent *event) override;

private slots:
    void onDownloadClicked();
    void onUrlChanged();
    void onSaveCredentialsClicked();
    void onBrowseFolderClicked();
    void onToolsStatusChanged(bool allInstalled);
    void onDownloadStarted();
    void onDownloadCompleted();
    void onQueueStatusChanged(int current, int total);
    void onDownloadAddedToQueue(int totalCount);
    void onCancelClicked();
    void onLogToggleClicked();

private:
    void setupUI();
    void setupStyles();
    void setupConnections();
    void loadSettings();
    void saveSettings();
    void detectOperatingSystem();
    void adjustWindowSize();
    QString getConfigPath() const;
    bool isValidVideoUrl(const QString &url) const;
    
    // UI Components
    QWidget *m_centralWidget;
    QVBoxLayout *m_mainLayout;
    
    QGroupBox *m_inputGroup;
    QVBoxLayout *m_inputLayout;
    QHBoxLayout *m_urlLayout;
    QLineEdit *m_urlInput;
    QPushButton *m_downloadButton;
    
    QGroupBox *m_progressGroup;
    QVBoxLayout *m_progressLayout;
    QHBoxLayout *m_progressButtonLayout;
    QProgressBar *m_progressBar;
    QLabel *m_progressLabel;
    QPushButton *m_cancelButton;
    
    QGroupBox *m_logGroup;
    QVBoxLayout *m_logLayout;
    QTextEdit *m_logOutput;
    bool m_logExpanded;
    
    QGroupBox *m_settingsGroup;
    QVBoxLayout *m_settingsLayout;
    QHBoxLayout *m_credentialsLayout;
    QHBoxLayout *m_folderLayout;
    QHBoxLayout *m_toolsLayout;
    
    // Credentials row
    QLineEdit *m_userInput;
    QLineEdit *m_passwordInput;
    QPushButton *m_saveCredentialsButton;
    
    // Folder row
    QLineEdit *m_downloadFolderInput;
    QPushButton *m_browseFolderButton;
    QPushButton *m_toolsButton;
    
    // Settings
    QSettings *m_settings;
    
    // Tools manager
    ToolsManager *m_toolsManager;
    
    // Download queue
    DownloadQueue *m_downloadQueue;
};

#endif // MAINWINDOW_H
