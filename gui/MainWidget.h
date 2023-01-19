#ifndef MAINWIDGET_H
#define MAINWIDGET_H

#include <boost/shared_ptr.hpp>
#include <QWidget>

class Controller;
class LogController;

class MainWidget:public QWidget
{
private:
    std::string log_path_ {};
    std::shared_ptr<Controller> controller_ptr_ {nullptr};
    boost::shared_ptr<LogController> log_controller_ptr_ {nullptr};

    void callback(const std::string& msg);
public:
    explicit MainWidget(QWidget* parent=nullptr);
    virtual ~MainWidget();
};

#endif // MAINWIDGET_H
