#ifndef DATADISPLAY_H
#define DATADISPLAY_H

#include <QWidget>
#include<QTextEdit>
#include<QLabel>
#include<QContextMenuEvent>
#include<QMouseEvent>

class DataDisplay : public QWidget
{
    Q_OBJECT
public:
    explicit DataDisplay(QWidget *parent = nullptr);
    void contextMenuEvent(QContextMenuEvent *event)override;
    void mouseDoubleClickEvent(QMouseEvent *event)override;
public slots:
    void setOutData();
public:
    QTextEdit* Display;
signals:

};

#endif // DATADISPLAY_H
