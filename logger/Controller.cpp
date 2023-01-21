#include "Controller.h"
#include "settings/Settings.h"

#include <boost/format.hpp>
#include <boost/process.hpp>
#include <boost/convert.hpp>
#include <boost/tokenizer.hpp>
#include <boost/bind/bind.hpp>
#include <boost/date_time.hpp>
#include <boost/filesystem.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/filesystem/operations.hpp>

void Controller::tick(const boost::system::error_code &ec)
{
    //execute all needed operations
    execute_task();
    //restart timer
    timer_ptr_->expires_from_now(boost::posix_time::milliseconds(interval_));
    timer_ptr_->async_wait(std::bind(&Controller::tick,this,std::placeholders::_1));
    if(log_func_){
        const std::string& msg {(boost::format("Controller::tick(): thread: %1%")
                    % std::this_thread::get_id()).str()};
        log_func_(msg);
    }
}

void Controller::execute_task()
{
    const boost::filesystem::path path {log_path_};
    if(!boost::filesystem::exists(path)){
        if(log_func_){
            const std::string& msg {(boost::format("Controller::execute_task(): path %1% not exists!")
                        % std::this_thread::get_id()).str()};
            log_func_(msg);
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
        //rename_file(file_name,file_path);
        //try to compress file
        //compress_file(file_name,file_path);
        //try to remove file
        //remove_file(file_name,file_path);

        ++map_begin;
    }

    if(log_func_){
        const std::string& msg {(boost::format("Controller::execute_task(): thread: %1%")
                    % std::this_thread::get_id()).str()};
        log_func_(msg);
    }
}

Controller::Controller(std::shared_ptr<Settings> log_settings_ptr, const std::string &log_path, const std::string &compressor_path)
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

Controller::~Controller()
{
    stop();
}

void Controller::start()
{
    if(started_){
        stop();
    }

    timer_ptr_.reset(new boost::asio::deadline_timer(io_service_,boost::posix_time::milliseconds(interval_)));
    boost::system::error_code ec;
    timer_ptr_->async_wait(std::bind(&Controller::tick,this,std::placeholders::_1));

    //start asio io_service thread
    asio_thread_ptr_.reset(new std::thread([this](){
        io_service_.run();
    }));

    started_=true;

    if(log_func_){
        const std::string& msg {(boost::format("Controller::start(): thread: %1%")
                    % std::this_thread::get_id()).str()};
        log_func_(msg);
    }
}

void Controller::stop()
{
    if(!started_){
        return;
    }
    io_service_.stop();
    if(asio_thread_ptr_ && asio_thread_ptr_->joinable()){
        asio_thread_ptr_->detach();
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }

    started_=false;
    if(log_func_){
        const std::string& msg {(boost::format("Controller::stop(): thread: %1%")
                    % std::this_thread::get_id()).str()};
        log_func_(msg);
    }
}
