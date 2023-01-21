#include <QDebug>
#include <QtGlobal>
#include <QPushButton>
#include <QHBoxLayout>

#include <boost/format.hpp>
#include <boost/date_time.hpp>
#include <boost/filesystem.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/date_time/local_time_adjustor.hpp>
#include <boost/date_time/c_local_time_adjustor.hpp>

#include "MainWidget.h"
#include "settings/LogSettings.h"
#include "logger/LogController.h"

void MainWidget::callback(const std::string &msg)
{
    qDebug()<<QString::fromStdString(msg);
}

MainWidget::MainWidget(QWidget *parent):QWidget{parent}
{

#ifdef Q_OS_LINUX
    log_path_="/home/yaroslav/Qt/log_files";
    compressor_path_="/home/yaroslav/Qt/3rdparty/7zip";
#endif
#ifdef Q_OS_WINDOWS
    log_path_="C:\\log_test";
#endif


    log_settings_ptr_.reset(new LogSettings(log_path_ + "/" + settings_filename_));
    log_settings_ptr_->read_settings();
    log_settings_ptr_->set_value("log.level", "debug");

    const time_t time=boost::filesystem::creation_time(log_path_ + "/" + settings_filename_);
    boost::posix_time::ptime pt=boost::posix_time::from_time_t(time);
    pt=boost::date_time::c_local_adjustor<boost::posix_time::ptime>::utc_to_local(pt);
    std::string s=boost::posix_time::to_iso_extended_string(pt);

    QPushButton* start_btn_ptr=new QPushButton("Start");
    QObject::connect(start_btn_ptr, &QPushButton::clicked,[this](){
        log_controller_ptr_.reset(new LogController(log_settings_ptr_,log_path_,compressor_path_));
        log_controller_ptr_->set_log_func(std::bind(&MainWidget::callback,this,std::placeholders::_1));
        log_controller_ptr_->start();


    });

    QPushButton* stop_btn_ptr=new QPushButton("Stop");
    QObject::connect(stop_btn_ptr,&QPushButton::clicked,[this](){
        if(log_controller_ptr_){
            log_controller_ptr_->stop();
        }
    });

    QPushButton* emplace_btn_ptr=new QPushButton("Emplace");
    QObject::connect(emplace_btn_ptr,&QPushButton::clicked,[this](){
        if(log_controller_ptr_){
            //log_controller_ptr_->emplace_task();
        }
    });

    QHBoxLayout* hboxlayout_ptr=new QHBoxLayout;
    hboxlayout_ptr->addWidget(start_btn_ptr);
    hboxlayout_ptr->addWidget(emplace_btn_ptr);
    hboxlayout_ptr->addWidget(stop_btn_ptr);
    setLayout(hboxlayout_ptr);
}

MainWidget::~MainWidget()
{
    if(log_controller_ptr_){
        log_controller_ptr_->stop();
    }
}
