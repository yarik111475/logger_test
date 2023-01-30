#include "LogController.h"
#include "settings/LogSettings.h"

#include <ctime>
#include <boost/format.hpp>
#include <boost/process.hpp>
#include <boost/convert.hpp>
#include <boost/tokenizer.hpp>
#include <boost/bind/bind.hpp>
#include <boost/date_time.hpp>
#include <boost/filesystem.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/date_time/local_time_adjustor.hpp>
#include <boost/date_time/c_local_time_adjustor.hpp>

void LogController::tick(const boost::system::error_code &ec)
{
    //execute all needed operations
    execute_task();
    //restart timer
    timer_ptr_->expires_from_now(boost::posix_time::milliseconds(interval_));
    timer_ptr_->async_wait(std::bind(&LogController::tick,this,std::placeholders::_1));
    if(log_func_){
        const std::string& msg {"LogController::tick()"};
        //log_func_(msg);
    }
}

void LogController::execute_task()
{
    //check if 'path' for log files exists
    const boost::filesystem::path path {log_path_};
    if(!boost::filesystem::exists(path)){
        //log
        if(log_func_){
            const std::string& msg {(boost::format("LogController::execute_task(): path %1% not exists!")
                        % std::this_thread::get_id()).str()};
            log_func_(msg);
        }
        stop();
        return;
    }

    //try to rename found files
    try{
        rename_files(log_path_);
    }
    catch(const std::exception& ex){
        //log
        if(log_func_){
            const std::string& msg {(boost::format("LogController::rename_files(): exception '%1%'")
                        % ex.what()).str()};
            log_func_(msg);
        }
    }
    catch(...){
        //log
        if(log_func_){
            const std::string& msg {"LogController::rename_files(): unknown exception"};
            log_func_(msg);
        }
    }

    //try to copy with renaming found files
    try{
        //copy_files(log_path_);
    }
    catch(const std::exception& ex){
        //log
        if(log_func_){
            const std::string& msg {(boost::format("LogController::copy_files(): exception '%1%'")
                        % ex.what()).str()};
            log_func_(msg);
        }
    }
    catch(...){
        //log
        if(log_func_){
            const std::string& msg {"LogController::copy_files(): unknown exception"};
            log_func_(msg);
        }
    }

    //try to compress found files
    try{
        compress_files(log_path_);
    }
    catch(const std::exception& ex){
        //log
        if(log_func_){
            const std::string& msg {(boost::format("LogController::compress_files(): exception '%1%'")
                        % ex.what()).str()};
            log_func_(msg);
        }
    }
    catch(...){
        //log
        if(log_func_){
            const std::string& msg {"LogController::compress_files(): unknown exception"};
            log_func_(msg);
        }
    }

    //try to remove found zips
    try{
        remove_zips(log_path_);
    }
    catch(const std::exception& ex){
        //log
        if(log_func_){
            const std::string& msg {(boost::format("LogController::remove_zips(): exception '%1%'")
                        % ex.what()).str()};
            log_func_(msg);
        }
    }
    catch(...){
        //log
        if(log_func_){
            const std::string& msg {"LogController::remove_zips(): unknown exception"};
            log_func_(msg);
        }
    }

    //try to remove found files
    try{
        //remove_files(log_path_);
    }
    catch(const std::exception& ex){
        //log
        if(log_func_){
            const std::string& msg {(boost::format("LogController::remove_files(): exception '%1%'")
                        % ex.what()).str()};
            log_func_(msg);
        }
    }
    catch(...){
        //log
        if(log_func_){
            const std::string& msg {"LogController::remove_files(): unknown exception"};
            log_func_(msg);
        }
    }
}

void LogController::rename_files(const std::string &path)
{
    boost::filesystem::directory_iterator begin(path);
    boost::filesystem::directory_iterator end;
    std::vector<std::pair<std::string,std::string>> file_list {};

    //parse directory
    while(begin!=end){
        const auto& status {begin->status()};
        if(status.type()==boost::filesystem::regular_file){
            const boost::filesystem::path path_ {begin->path()};
            const std::string file_path=path_.string();
            const std::string file_name=path_.filename().string();
            file_list.push_back({file_name,file_path});
        }
        ++begin;
    }

    //rename all found files
    for(size_t i=0;i<file_list.size();++i){
        const std::string file_name {file_list.at(i).first};
        const std::string file_path {file_list.at(i).second};

        const boost::filesystem::path path_(file_path);
        const std::string& file_extension {path_.extension().string()};

        //check extension
        if(file_extension!=".txt"){
            continue;
        }

        //check file_name parts
        std::vector<std::string> split_results;
        boost::split(split_results, file_name, boost::is_any_of("."));
        if(split_results.empty() || split_results.size()!=3){
            continue;
        }

        //log
        if(log_func_){
            const std::string& msg {(boost::format("LogController::rename_files(): file: '%1%', path: '%2%'")
                                                   % file_name
                                                   % file_path).str()};
            log_func_(msg);
        }

        //get 'last_write_time' timestamp for file
        const std::time_t& dt {boost::filesystem::last_write_time(file_path)};
        boost::posix_time::ptime ptime_=boost::posix_time::from_time_t(dt);
        ptime_=boost::date_time::c_local_adjustor<boost::posix_time::ptime>::utc_to_local(ptime_);
        const boost::gregorian::date date_ {ptime_.date()};

        //create new file name for save file
        const std::string& new_file_name {boost::posix_time::to_iso_string(ptime_)};

        /*
        //create new file name for save file
        const std::string& new_file_name_str {(boost::format("%4d-%02d-%02dT%02d_%02d_%02d")
                    % date_.year()
                    % date_.month().as_number()
                    % date_.day()
                    % ptime_.time_of_day().hours()
                    % ptime_.time_of_day().minutes()
                    % ptime_.time_of_day().seconds()).str()};
        */

        //create path for save file
        boost::filesystem::path new_path_(file_path);
        new_path_=new_path_.remove_filename();
        const std::string& new_path_str {new_path_.string()};

        //rename file with 'last_write_time' timestamp
        boost::system::error_code ec;
        boost::filesystem::rename(file_path, new_path_str + "/" + new_file_name + ".txt",ec);
        if(ec!=boost::system::errc::success){
            //TODO log error here
        }
    }
}

void LogController::copy_files(const std::string &path)
{
    boost::filesystem::directory_iterator begin(path);
    boost::filesystem::directory_iterator end;
    std::vector<std::pair<std::string,std::string>> file_list {};

    //parse directory
    while(begin!=end){
        const auto& status {begin->status()};
        if(status.type()==boost::filesystem::regular_file){
            const boost::filesystem::path path_ {begin->path()};
            const std::string file_path=path_.string();
            const std::string file_name=path_.filename().string();
            file_list.push_back({file_name,file_path});
        }
        ++begin;
    }

    //copy with renaming all found files
    for(size_t i=0;i<file_list.size();++i){
        const std::string file_name {file_list.at(i).first};
        const std::string file_path {file_list.at(i).second};

        const boost::filesystem::path path_(file_path);
        const std::string& file_extension {path_.extension().string()};

        //check extension
        if(file_extension!=".txt"){
            continue;
        }

        //check file_name parts
        std::vector<std::string> split_results;
        boost::split(split_results, file_name, boost::is_any_of("."));
        if(split_results.empty() || split_results.size()!=3){
            continue;
        }

        //log
        if(log_func_){
            const std::string& msg {(boost::format("LogController::copy_files(): file: '%1%', path: '%2%'")
                                                   % file_name
                                                   % file_path).str()};
            log_func_(msg);
        }

        //get 'last_write_time' timestamp for file
        const std::time_t& dt {boost::filesystem::last_write_time(file_path)};
        boost::posix_time::ptime ptime_=boost::posix_time::from_time_t(dt);
        ptime_=boost::date_time::c_local_adjustor<boost::posix_time::ptime>::utc_to_local(ptime_);
        const boost::gregorian::date date_ {ptime_.date()};

        //create new file name for copy file
        const std::string& new_file_name {boost::posix_time::to_iso_string(ptime_)};

        /*
        //create new file name for save file
        const std::string& new_file_name_str {(boost::format("%4d-%02d-%02dT%02d_%02d_%02d")
                    % date_.year()
                    % date_.month().as_number()
                    % date_.day()
                    % ptime_.time_of_day().hours()
                    % ptime_.time_of_day().minutes()
                    % ptime_.time_of_day().seconds()).str()};
        */

        //create path for copy file
        boost::filesystem::path new_path_(file_path);
        new_path_=new_path_.remove_filename();
        const std::string& new_path_str {new_path_.string()};

        //copy file with 'last_write_time' timestamp
        boost::system::error_code ec;
        boost::filesystem::copy_file(file_path, new_path_str + "/" + new_file_name + ".txt",ec);
        if(ec!=boost::system::errc::success){
            //TODO log error here
        }
    }
}

void LogController::compress_files(const std::string &path)
{
    boost::filesystem::directory_iterator begin(path);
    boost::filesystem::directory_iterator end;
    std::vector<std::pair<std::string,std::string>> file_list {};

    //parse directory
    while(begin!=end){
        const auto& status {begin->status()};
        if(status.type()==boost::filesystem::regular_file){
            const boost::filesystem::path path_ {begin->path()};
            const std::string file_path=path_.string();
            const std::string file_name=path_.filename().string();
            file_list.push_back({file_name,file_path});
        }
        ++begin;
    }

    //compress all found files
    for(size_t i=0;i<file_list.size();++i){
        const std::string file_name {file_list.at(i).first};
        const std::string file_path {file_list.at(i).second};

        const boost::filesystem::path path_(file_path);
        const std::string file_name_=path_.stem().string();;
        const std::string& file_extension_ {path_.extension().string()};

        //check file_name prefix
        if(boost::algorithm::starts_with(file_name_, "usagent")){
            continue;
        }

        //check extension
        if(file_extension_!=".txt"){
            continue;
        }

        //check file_name parts
        std::vector<std::string> split_results;
        boost::split(split_results, file_name,boost::is_any_of("."));
        if(split_results.empty() || split_results.size()!=2){
            continue;
        }
        //log
        if(log_func_){
            const std::string& msg {(boost::format("LogController::compress_files(): file: '%1%', path: '%2%'")
                                                   % file_name
                                                   % file_path).str()};
            log_func_(msg);
        }

        //create path for save file
        boost::filesystem::path current_path(file_path);
        current_path=current_path.remove_filename();
        //create program
        const boost::filesystem::path& program {compressor_path_ + "/" + compressor_name_};
        //create args
        std::vector<std::string> args {
            "a", "-y", "-sdel", "-mx9", current_path.string() + "/" + split_results.at(0) + ".zip", file_path
        };
        //execute compress
        const int& success {boost::process::system(program, args)};
    }
}

void LogController::remove_zips(const std::string &path)
{
    boost::filesystem::directory_iterator begin(path);
    boost::filesystem::directory_iterator end;
    std::vector<std::pair<std::string,std::string>> file_list {};

    //parse directory
    while(begin!=end){
        const auto& status {begin->status()};
        if(status.type()==boost::filesystem::regular_file){
            const boost::filesystem::path path_ {begin->path()};
            file_list.push_back({path_.string(),path_.filename().string()});
        }
        ++begin;
    }

    //remove all found files
    for(size_t i=0;i<file_list.size();++i){
        const std::string file_name {file_list.at(i).first};
        const std::string file_path {file_list.at(i).second};

        const boost::filesystem::path path_(file_path);
        const std::string base_file_name_=path_.stem().string();;
        const std::string& file_extension_ {path_.extension().string()};

        //check extension
        if(".zip"!=file_extension_){
            continue;
        }

        //create date and time from file_name
        const boost::posix_time::ptime& file_ptime_ {boost::posix_time::from_iso_string(base_file_name_)};
        const boost::gregorian::date& file_date_ {file_ptime_.date()};

        //create local now date
        const boost::gregorian::date& now_date_ {boost::posix_time::second_clock::local_time().date()};

        //get days delta
        boost::gregorian::date_duration days_delta {now_date_-file_date_};

        //get days offset from log settings
        long days_offset {9};
        const bool& date_offset_ok {boost::conversion::try_lexical_convert<long>(log_settings_ptr_->value("log.remove_period"), days_offset)};

        //compare days delta and days offset
        if(days_delta.days() > days_offset){
            boost::system::error_code ec;
            const bool& success {boost::filesystem::remove(file_path,ec)};
            if(!success){
                //TODO log error here
            }
        }
    }
}

void LogController::remove_files(const std::string &path)
{
    boost::filesystem::directory_iterator begin(path);
    boost::filesystem::directory_iterator end;
    std::vector<std::pair<std::string,std::string>> file_list {};

    //parse directory
    while(begin!=end){
        const auto& status {begin->status()};
        if(status.type()==boost::filesystem::regular_file){
            const boost::filesystem::path path_ {begin->path()};
            file_list.push_back({path_.string(),path_.filename().string()});
        }
        ++begin;
    }

    //remove all found files
    for(size_t i=0;i<file_list.size();++i){
        const std::string file_name {file_list.at(i).first};
        const std::string file_path {file_list.at(i).second};

        const boost::filesystem::path path_(file_path);
        const std::string base_file_name_=path_.stem().string();;
        const std::string& file_extension_ {path_.extension().string()};

        //check extension
        if(".txt"!=file_extension_){
            continue;
        }

        //check file_name parts
        std::vector<std::string> split_results;
        boost::split(split_results, file_name, boost::is_any_of("."));
        if(split_results.empty() || split_results.size()!=2){
            continue;
        }

        //create date and time from file_name
        const boost::posix_time::ptime& file_ptime_ {boost::posix_time::from_iso_string(base_file_name_)};
        const boost::gregorian::date& file_date_ {file_ptime_.date()};

        //create local now date
        const boost::gregorian::date& now_date_ {boost::posix_time::second_clock::local_time().date()};

        //get days delta
        boost::gregorian::date_duration days_delta {now_date_-file_date_};

        //get days offset from log settings
        long days_offset {9};
        const bool& date_offset_ok {boost::conversion::try_lexical_convert<long>(log_settings_ptr_->value("log.remove_period"), days_offset)};

        //compare days delta and days offset
        if(days_delta.days() > days_offset){
            boost::system::error_code ec;
            const bool& success {boost::filesystem::remove(file_path,ec)};
            if(!success){
                //TODO log error here
            }
        }
    }
}

LogController::LogController(std::shared_ptr<Settings> log_settings_ptr, const std::string &log_path, const std::string &compressor_path)
    :log_settings_ptr_{log_settings_ptr},log_path_{log_path},compressor_path_{compressor_path}
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

    timer_ptr_.reset(new boost::asio::deadline_timer(io_service_,boost::posix_time::milliseconds(interval_)));
    boost::system::error_code ec;
    timer_ptr_->async_wait(std::bind(&LogController::tick,this,std::placeholders::_1));

    //start asio io_service thread
    asio_thread_ptr_.reset(new std::thread([this](){
        io_service_.run();
    }));

    started_=true;

    if(log_func_){
        const std::string& msg {"LogController::start()"};
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

    started_=false;
    if(log_func_){
        const std::string& msg {"LogController::stop()"};
        log_func_(msg);
    }
}
