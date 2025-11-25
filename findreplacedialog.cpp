#include "findreplacedialog.h"

#include <QLabel>  // 添加 QLabel 头文件

FindReplaceDialog::FindReplaceDialog(QWidget *parent) : QDialog(parent)
{
    // 创建控件
    findLineEdit = new QLineEdit(this);
    replaceLineEdit = new QLineEdit(this);
    findButton = new QPushButton("查找下一个", this);
    replaceButton = new QPushButton("替换", this);
    replaceAllButton = new QPushButton("全部替换", this);

    // 布局管理
    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    QHBoxLayout *findLayout = new QHBoxLayout();
    findLayout->addWidget(new QLabel("查找内容："));
    findLayout->addWidget(findLineEdit);
    mainLayout->addLayout(findLayout);

    QHBoxLayout *replaceLayout = new QHBoxLayout();
    replaceLayout->addWidget(new QLabel("替换为："));
    replaceLayout->addWidget(replaceLineEdit);
    mainLayout->addLayout(replaceLayout);

    QHBoxLayout *buttonLayout = new QHBoxLayout();
    buttonLayout->addWidget(findButton);
    buttonLayout->addWidget(replaceButton);
    buttonLayout->addWidget(replaceAllButton);
    mainLayout->addLayout(buttonLayout);

    setLayout(mainLayout);

    // 连接信号与槽
    connect(findButton, &QPushButton::clicked, this, &FindReplaceDialog::onFindClicked);
    connect(replaceButton, &QPushButton::clicked, this, &FindReplaceDialog::onReplaceClicked);
    connect(replaceAllButton, &QPushButton::clicked, this, &FindReplaceDialog::onReplaceAllClicked);
}

FindReplaceDialog::~FindReplaceDialog() {}

void FindReplaceDialog::onFindClicked()
{
    emit findText(findLineEdit->text());
}

void FindReplaceDialog::onReplaceClicked()
{
    emit replaceText(findLineEdit->text(), replaceLineEdit->text());
}

void FindReplaceDialog::onReplaceAllClicked()
{
    emit replaceAllText(findLineEdit->text(), replaceLineEdit->text());
}
