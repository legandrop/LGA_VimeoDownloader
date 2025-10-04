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

QT_BEGIN_NAMESPACE
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void onDownloadClicked();
    void onUrlChanged();
    void onInstallUpdateYtDlpClicked();
    void onSaveCredentialsClicked();
    void onBrowseFolderClicked();

private:
    void setupUI();
    void setupStyles();
    void setupConnections();
    void loadSettings();
    void saveSettings();
    void detectOperatingSystem();
    void checkYtDlpInstallation();
    void adjustWindowSize();
    void downloadYtDlpWindows();
    QString getConfigPath() const;
    
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
    QProgressBar *m_progressBar;
    
    QGroupBox *m_logGroup;
    QVBoxLayout *m_logLayout;
    QTextEdit *m_logOutput;
    
    QGroupBox *m_settingsGroup;
    QVBoxLayout *m_settingsLayout;
    QHBoxLayout *m_credentialsLayout;
    QHBoxLayout *m_folderLayout;
    QHBoxLayout *m_ytDlpLayout;
    
    // Credentials row
    QLineEdit *m_userInput;
    QLineEdit *m_passwordInput;
    QPushButton *m_saveCredentialsButton;
    
    // Folder row
    QLineEdit *m_downloadFolderInput;
    QPushButton *m_browseFolderButton;
    QPushButton *m_ytDlpButton;
    
    // Settings
    QSettings *m_settings;
    bool m_ytDlpInstalled;
};

#endif // MAINWINDOW_H
