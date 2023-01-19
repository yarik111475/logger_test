#include "LogController.h"

void LogController::run()
{
    while(start_stop_flag_){
        std::unique_lock<std::mutex> lock(mtx_);
        cv_.wait(lock, [this](){
            return repeat_flag_;
        });

        repeat_flag_=false;
        lock.unlock();

        if(!start_stop_flag_){
            return;
        }

        const std::string msg {"task executed"};
        if(callback_){
            callback_(msg);
        }
    }
}

void LogController::emplace_task()
{
    std::lock_guard<std::mutex> lock(mtx_);
    repeat_flag_=true;
    cv_.notify_one();
}

LogController::LogController()
{

}

LogController::~LogController()
{
    stop();
}

void LogController::start()
{
    if(started_){
        stop();
    }
    std::lock_guard<std::mutex> lock(mtx_);
    start_stop_flag_=true;
    thread_ptr_.reset(new std::thread(&LogController::run,this));

    const std::string msg {"started"};
    if(callback_){
        callback_(msg);
    }
    started_=true;
}

void LogController::stop()
{
    if(!started_){
        return;
    }
    std::lock_guard<std::mutex> lock(mtx_);
    start_stop_flag_=false;
    repeat_flag_=true;
    cv_.notify_one();

    const std::string msg {"stopped"};
    if(callback_){
        callback_(msg);
    }

    if(thread_ptr_ && thread_ptr_->joinable()){
        thread_ptr_->detach();
    }
    started_=false;
}
