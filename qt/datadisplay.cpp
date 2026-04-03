#include "datadisplay.h"
#include<QVBoxLayout>

DataDisplay::DataDisplay(QWidget *parent)
    : QWidget{parent}
{
    Display=new QTextEdit(this);

}
