#include "datadisplay.h"
#include<QVBoxLayout>
#include<QDialog>
#include<QDialogButtonBox>
#include<QMenu>

DataDisplay::DataDisplay(QWidget *parent)
    : QWidget{parent}
{
    Display=new QTextEdit(this);
    Display->setReadOnly(true);

    QVBoxLayout*lay=new QVBoxLayout(this);
    lay->addWidget(Display);
    lay->setContentsMargins(0,0,0,0);
    setLayout(lay);
    Display->setContextMenuPolicy(Qt::NoContextMenu);
    Display->setAttribute(Qt::WA_TransparentForMouseEvents);
}

void DataDisplay::contextMenuEvent(QContextMenuEvent *event)
{
    QMenu *menu=new QMenu(this);
    QAction *action=menu->addAction("打开查看详情");
    connect(action,&QAction::triggered,this,&DataDisplay::setOutData);
    menu->exec(event->globalPos());
}

void DataDisplay::mouseDoubleClickEvent(QMouseEvent *event)
{
    if(event->button()==Qt::LeftButton){
        setOutData();
    }
    QWidget::mouseDoubleClickEvent(event);
}

void DataDisplay::setOutData()
{
    qDebug()<<"正在弹出窗口";
    QDialog*dlg=new QDialog(this);
    dlg->setWindowTitle("详情数据:");
    dlg->resize(300,400);

    QVBoxLayout*lay=new QVBoxLayout(dlg);
    QTextEdit*text=new QTextEdit(dlg);
    text->setReadOnly(true);
    text->setPlainText(Display->toPlainText());
    lay->addWidget(text);

    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Close, dlg);
    connect(buttonBox, &QDialogButtonBox::rejected, dlg, &QDialog::reject);
    lay->addWidget(buttonBox);

    dlg->setAttribute(Qt::WA_DeleteOnClose);
    dlg->show();
}
