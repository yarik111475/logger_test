#include "LogController.h"
#include <boost/bind.hpp>

void LogController::run()
{
    while(start_stop_flag_){
        std::unique_lock<std::mutex> lock(mtx_);
        cv_.wait(lock, [this](){
            return !task_queue_.empty();
        });

        //task_queue_.front();
        task_queue_.pop();
        lock.unlock();

        if(!start_stop_flag_){
            return;
        }

        execute_task();
    }
}

void LogController::tick(const boost::system::error_code &ec)
{
    emplace_task();
    timer_ptr_->expires_from_now(boost::posix_time::milliseconds(interval_));
    timer_ptr_->async_wait(boost::bind(&LogController::tick,this,boost::asio::placeholders::error()));
}

void LogController::emplace_task()
{
    std::lock_guard<std::mutex> lock(mtx_);
    task_queue_.emplace('0');
    cv_.notify_one();
}

void LogController::execute_task()
{
    const std::string msg {"task executed"};
    if(log_func_){
        log_func_(msg);
    }
}

LogController::LogController(int interval, const std::string &log_path)
    :interval_{interval},log_path_{log_path}
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

    //start task queue thread
    thread_ptr_.reset(new std::thread([this](){
        run();
    }));

    timer_ptr_.reset(new boost::asio::deadline_timer(io_service_,boost::posix_time::milliseconds(interval_)));
    timer_ptr_->async_wait(boost::bind(&LogController::tick,this,boost::asio::placeholders::error()));

    //start asio io_service thread
    asio_thread_ptr_.reset(new std::thread([this](){
        io_service_.run();
    }));

    started_=true;
    const std::string msg {"started"};
    if(log_func_){
        log_func_(msg);
    }
}

void LogController::stop()
{
    if(!started_){
        return;
    }
    io_service_.stop();
    if(asio_thread_ptr_ && asio_thread_ptr_->joinable()){
        asio_thread_ptr_->detach();
    }

    std::lock_guard<std::mutex> lock(mtx_);
    start_stop_flag_=false;
    task_queue_.emplace('0');
    cv_.notify_one();

    if(thread_ptr_ && thread_ptr_->joinable()){
        thread_ptr_->detach();
    }

    started_=false;
    const std::string msg {"stopped"};
    if(log_func_){
        log_func_(msg);
    }
}
