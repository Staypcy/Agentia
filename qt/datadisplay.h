#ifndef DATADISPLAY_H
#define DATADISPLAY_H

#include <QWidget>
#include<QTextEdit>
#include<QLabel>

class DataDisplay : public QWidget
{
    Q_OBJECT
public:
    explicit DataDisplay(QWidget *parent = nullptr);
    void setLabel(const QString text);
public:
    QTextEdit* Display;
    QLabel* label;
signals:

};

#endif // DATADISPLAY_H
