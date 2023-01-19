#include <QDebug>
#include <QPushButton>
#include <QHBoxLayout>

#include "MainWidget.h"
#include "logger/LogController.h"

void MainWidget::callback(const std::string &msg)
{
    qDebug()<<QString::fromStdString(msg);
}

MainWidget::MainWidget(QWidget *parent):QWidget{parent}
{


    QPushButton* start_btn_ptr=new QPushButton("Start");
    QObject::connect(start_btn_ptr, &QPushButton::clicked,[this](){
        log_controller_ptr_.reset(new LogController);
        log_controller_ptr_->set_callback(std::bind(&MainWidget::callback,this,std::placeholders::_1));
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
            log_controller_ptr_->emplace_task();
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
