#ifndef MAINWIDGET_H
#define MAINWIDGET_H

#include <boost/shared_ptr.hpp>
#include <QWidget>

class Settings;
class Controller;
class LogController;

class MainWidget:public QWidget
{
private:
    const std::string settings_filename_ {"log.conf"};
    std::string log_path_ {};
    std::string compressor_path_ {};
    std::shared_ptr<Settings> log_settings_ptr_ {nullptr};
    boost::shared_ptr<Controller> log_controller_ptr_ {nullptr};

    void callback(const std::string& msg);
public:
    explicit MainWidget(QWidget* parent=nullptr);
    virtual ~MainWidget();
};

#endif // MAINWIDGET_H
