#ifndef VIDEOPASSWORDDIALOG_H
#define VIDEOPASSWORDDIALOG_H

#include <QDialog>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>

class VideoPasswordDialog : public QDialog
{
    Q_OBJECT

public:
    explicit VideoPasswordDialog(const QString &videoUrl, QWidget *parent = nullptr);
    ~VideoPasswordDialog();

    QString getVideoPassword() const;

private slots:
    void onOkClicked();
    void onCancelClicked();

private:
    QLabel *m_messageLabel;
    QLineEdit *m_passwordInput;
    QPushButton *m_okButton;
    QPushButton *m_cancelButton;

    QString m_videoPassword;
};

#endif // VIDEOPASSWORDDIALOG_H
