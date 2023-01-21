#include "LogController.h"
#include "settings/Settings.h"

#include <boost/format.hpp>
#include <boost/process.hpp>
#include <boost/convert.hpp>
#include <boost/tokenizer.hpp>
#include <boost/bind/bind.hpp>
#include <boost/filesystem.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/date_time.hpp>

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

        //try to rename file
        rename_file(file_name,file_path);
        //try to compress file
        compress_file(file_name,file_path);
        //try to remove file
        remove_file(file_name,file_path);

        ++map_begin;
    }

    const std::string& msg {"LogController::execute_task(): task executed"};
    if(log_func_){
        log_func_(msg);
    }
}

void LogController::rename_file(const std::string &file_name, const std::string &file_path)
{
    if(log_func_){
        const std::string& msg {(boost::format("LogController::rename_file(): try to rename file: '%1%', path: '%2%'")
                    % file_name
                    % file_path).str()};
        log_func_(msg);
    }


    boost::char_separator<char> sep(".");
    boost::tokenizer<boost::char_separator<char>> tokens(file_name,sep);
    //check tokens count in file_name
    const size_t tokens_count=std::distance(tokens.begin(),tokens.end());
    if(tokens_count==token_size_){

    }
}

void LogController::compress_file(const std::string &file_name, const std::string &file_path)
{
    if(log_func_){
        const std::string& msg {(boost::format("LogController::compress_file(): try to compress file: '%1%', path: '%2%'")
                    % file_name
                    % file_path).str()};
        log_func_(msg);
    }

    boost::char_separator<char> sep(".");
    boost::tokenizer<boost::char_separator<char>> tokens(file_name,sep);
    //check tokens count in file_name
    const size_t tokens_count=std::distance(tokens.begin(),tokens.end());
    if(tokens_count==token_size_){

    }
}

void LogController::remove_file(const std::string &file_name, const std::string &file_path)
{
    if(log_func_){
        const std::string& msg {(boost::format("LogController::remove_file(): try to remove file: '%1%', path: '%2%'")
                    % file_name
                    % file_path).str()};
        log_func_(msg);
    }

    boost::char_separator<char> sep(".");
    boost::tokenizer<boost::char_separator<char>> tokens(file_name,sep);
    //check tokens count in file_name
    const size_t tokens_count=std::distance(tokens.begin(),tokens.end());
    if(tokens_count==token_size_){

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

    //full compressor path with file_name
    const std::string& compressor_full_path {compressor_path_ +"/" + compressor_name_};

    //check if compressor exists
    if(!boost::filesystem::exists(compressor_full_path)){
        if(log_func_){
            const std::string& msg {(boost::format("LogController::operate_file(): compressor not exists', path: '%1%'")
                        % compressor_full_path).str()};
            log_func_(msg);
        }
        return;
    }

    //create result compressed files path
    boost::filesystem::path out_path(file_path);
    out_path=out_path.remove_filename();

    const std::string& compressor_args {" a -y " + out_path.string() + "/" + file_name + ".zip "  + file_path};
    const int& result {boost::process::system(compressor_full_path +compressor_args)};
}

LogController::LogController(std::shared_ptr<Settings> log_settings_ptr, const std::string &log_path, const std::string &compressor_path)
    :log_settings_ptr_{log_settings_ptr},log_path_{log_path},compressor_path_{compressor_path}
{
    //create check period
    const bool& interval_ok {boost::conversion::try_lexical_convert<int>(log_settings_ptr_->value("log.check_period"),interval_)};
    if(!interval_ok){
        interval_=1000;
    }
    //crete log filename
    const bool filename_ok {boost::conversion::try_lexical_convert<std::string>(log_settings_ptr->value("log.file_name"),log_file_name_)};
    if(!filename_ok){
        log_file_name_="log.txt";
    }
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
    queue_thread_ptr_.reset(new std::thread([this](){
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
    const std::string& msg {"LogController::start(): started"};
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

    if(queue_thread_ptr_ && queue_thread_ptr_->joinable()){
        queue_thread_ptr_->detach();
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }

    started_=false;
    const std::string& msg {"LogController::stop(): stopped"};
    if(log_func_){
        log_func_(msg);
    }
}
