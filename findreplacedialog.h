#ifndef FINDREPLACEDIALOG_H
#define FINDREPLACEDIALOG_H

#include <QDialog>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>

class FindReplaceDialog : public QDialog
{
    Q_OBJECT

public:
    explicit FindReplaceDialog(QWidget *parent = nullptr);
    ~FindReplaceDialog();

signals:
    void findText(const QString &text);
    void replaceText(const QString &findText, const QString &replaceText);
    void replaceAllText(const QString &findText, const QString &replaceText);

private slots:
    void onFindClicked();
    void onReplaceClicked();
    void onReplaceAllClicked();

private:
    QLineEdit *findLineEdit;
    QLineEdit *replaceLineEdit;
    QPushButton *findButton;
    QPushButton *replaceButton;
    QPushButton *replaceAllButton;
};

#endif // FINDREPLACEDIALOG_H
