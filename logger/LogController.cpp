#include "LogController.h"

#include <boost/format.hpp>
#include <boost/tokenizer.hpp>
#include <boost/bind/bind.hpp>
#include <boost/filesystem.hpp>
#include <boost/filesystem/operations.hpp>

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
    timer_ptr_->async_wait(std::bind(&LogController::tick,this,std::placeholders::_1));
}

void LogController::emplace_task()
{
    std::lock_guard<std::mutex> lock(mtx_);
    task_queue_.emplace('0');
    cv_.notify_one();
}

void LogController::execute_task()
{
    const boost::filesystem::path path {log_path_};
    if(!boost::filesystem::exists(path)){
        if(log_func_){
            log_func_((boost::format("LogController::execute_task():path '%1%' not exists!")
                                    % path).str());
        }
        stop();
        return;
    }
    boost::filesystem::directory_iterator begin(path);
    boost::filesystem::directory_iterator end;
    std::map<std::string, std::string> file_map;

    //parse directory
    while(begin!=end){
        const auto& status {begin->status()};
        if(status.type()==boost::filesystem::regular_file){
            const std::string& file_path {begin->path().string()};
            const std::string file_name {begin->path().filename().string()};
            file_map.emplace(file_name,file_path);
        }
        ++begin;
    }

    //iterate all found files
    auto map_begin {file_map.begin()};
    while(map_begin!=file_map.end()){
        const std::string& file_name {map_begin->first};
        const std::string& file_path {map_begin->second};

        boost::char_separator<char> sep(".");
        boost::tokenizer<boost::char_separator<char>> tokens(file_name,sep);
        //check tokens count in file_name
        const size_t tokens_count=std::distance(tokens.begin(),tokens.end());
        if(tokens_count==2){
            operate_file(file_name,file_path);
        }
        ++map_begin;
    }

    const std::string msg {"LogController::execute_task(): task executed"};
    if(log_func_){
        log_func_(msg);
    }
}

void LogController::operate_file(const std::string &file_name, const std::string &file_path)
{
    if(log_func_){
        const std::string& msg {(boost::format("LogController::operate_file(): operate with file: '%1%', path: '%2%'")
                    % file_name
                    % file_path).str()};
        log_func_(msg);
    }
}

LogController::LogController(int interval, const std::string &log_path)
    :log_path_{log_path},interval_{interval}
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
    boost::system::error_code ec;
    timer_ptr_->async_wait(std::bind(&LogController::tick,this,std::placeholders::_1));

    //start asio io_service thread
    asio_thread_ptr_.reset(new std::thread([this](){
        io_service_.run();
    }));

    started_=true;
    const std::string msg {"LogController::start(): started"};
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
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }

    std::lock_guard<std::mutex> lock(mtx_);
    start_stop_flag_=false;
    task_queue_.emplace('0');
    cv_.notify_one();

    if(thread_ptr_ && thread_ptr_->joinable()){
        thread_ptr_->detach();
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }

    started_=false;
    const std::string msg {"LogController::stop(): stopped"};
    if(log_func_){
        log_func_(msg);
    }
}
