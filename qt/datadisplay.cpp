#include "datadisplay.h"
#include<QVBoxLayout>

DataDisplay::DataDisplay(QWidget *parent)
    : QWidget{parent}
{
    Display=new QTextEdit(this);

}

void DataDisplay::setLabel(const QString text)
{
    QVBoxLayout layout(Display);
    label->setText(text);
    layout.addWidget(label);
}


