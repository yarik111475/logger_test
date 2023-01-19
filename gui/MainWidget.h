#ifndef MAINWIDGET_H
#define MAINWIDGET_H
#include <QWidget>


class MainWidget:public QWidget
{
public:
    explicit MainWidget(QWidget* parent=nullptr);
    virtual ~MainWidget()=default;
};

#endif // MAINWIDGET_H
