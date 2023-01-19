#include "Controller.h"

#include <QTimer>

void Controller::slot_timeout()
{
    std::lock_guard<std::mutex> lock(mtx_);
    task_queue_.emplace('0');
    cv_.notify_one();
}

void Controller::slot_execute_task()
{
    if(log_func_){
        log_func_("task executed");
    }
}

Controller::Controller(int interval, const QString &log_path, QObject *parent)
    :QThread {parent}, interval_{interval},log_path_{log_path}
{

}

Controller::~Controller()
{
    stop();
}

void Controller::start()
{
    if(started_){
        stop();
    }
    run();
}

void Controller::stop()
{

}

void Controller::run()
{
    timer_ptr_.reset(new QTimer);
    timer_ptr_->moveToThread(this);
    timer_ptr_->setInterval(interval_);
    QObject::connect(timer_ptr_.get(),&QTimer::timeout,this,&Controller::slot_timeout);
    //timer_ptr_->start();

    start_stop_flag_=true;
    while(start_stop_flag_){
        std::unique_lock<std::mutex> lock(mtx_);
        cv_.wait(lock,[this](){
            return !task_queue_.empty();
        });

        task_queue_.pop();
        lock.unlock();

        slot_execute_task();
    }
}
