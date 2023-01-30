#include "Controller.h"
#include "settings/Settings.h"

#include <iostream>
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
        //log_func_(msg);
    }
}

void Controller::execute_task()
{

    const boost::filesystem::path path {log_path_};
    if(!boost::filesystem::exists(path)){
        if(log_func_){
            const std::string& msg {(boost::format("LogController::execute_task(): path %1% not exists!")
                        % std::this_thread::get_id()).str()};
            log_func_(msg);
        }
        stop();
        return;
    }

    boost::filesystem::directory_iterator begin(path);
    boost::filesystem::directory_iterator end;
    std::vector<std::pair<std::string,std::string>> file_list {};

    rename_files(log_path_);
    compress_files(log_path_);
}

void Controller::rename_files(const std::string &path)
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
        const std::string& file_name {file_list.at(i).first};
        const std::string& file_path {file_list.at(i).second};

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
            const std::string& msg {(boost::format("Controller::rename_file(): file: '%1%', path: '%2%'")
                                                   % file_name
                                                   % file_path).str()};
            log_func_(msg);
        }

        //get 'creation_time' timestamp for file
        const std::time_t& dt {boost::filesystem::creation_time(file_path)};
        boost::posix_time::ptime pt=boost::posix_time::from_time_t(dt);
        pt=boost::date_time::c_local_adjustor<boost::posix_time::ptime>::utc_to_local(pt);

        //create new file name for save file
        const std::string& new_file_name_str {boost::posix_time::to_iso_extended_string(pt)};

        //create path for save file
        boost::filesystem::path new_path_(file_path);
        new_path_=new_path_.remove_filename();
        const std::string& new_path_str {new_path_.string()};

        //rename file with 'creation_time' timestamp
        boost::system::error_code ec;
        boost::filesystem::rename(file_path, new_path_str + "/" + new_file_name_str + ".txt",ec);
        if(ec!=boost::system::errc::success){
            //TODO log error here
        }
    }
}

void Controller::compress_files(const std::string &path)
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

        if(boost::algorithm::starts_with(file_name_, "usagent")){
            continue;
        }

        //check extension
        if(file_extension_!=".txt"){
            continue;
        }

        std::vector<std::string> split_results;
        boost::split(split_results, file_name,boost::is_any_of("."));
        if(split_results.empty() || split_results.size()!=2){
            continue;
        }
        //log
        if(log_func_){
            const std::string& msg {(boost::format("Controller::compress_file(): file: '%1%', path: '%2%'")
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
        int success=boost::process::system(program, args);
    }
}

void Controller::remove_file(const std::string &file_name, const std::string &file_path)
{
    std::vector<std::string> split_results;
    boost::split(split_results, file_name,boost::is_any_of("."));
    if(split_results.empty() || split_results.size()!=2 || split_results.at(1)!="zip"){
        return;
    }
    //log
    if(log_func_){
        const std::string& msg {(boost::format("Controller::remove_file(): operate with file: '%1%'")
                                               % file_name).str()};
        log_func_(msg);
    }

    //get file 'timestamp'
    const boost::posix_time::ptime& file_dt {boost::posix_time::from_iso_extended_string(split_results.at(0))};
    //get current time
    const boost::posix_time::ptime& current_dt {boost::posix_time::second_clock::local_time()};
    //get delta
    const auto& delta_dt {current_dt-file_dt};
    const long& delta_seconds {delta_dt.ticks()};
}

void Controller::init_logger()
{
    //crete log filename
    std::string filename {"usagent.txt"};
    const bool& filename_ok {boost::conversion::try_lexical_convert<std::string>(log_settings_ptr_->value("log.file_name"),filename)};

    //create max logfile size
    int filesize {1024 * 1024};
    const bool& filesize_ok {boost::conversion::try_lexical_convert<int>(log_settings_ptr_->value("log.file_size"),filesize)};

    //create max additional log files count
    int filescount {5};
    const bool& filescount_ok {boost::conversion::try_lexical_convert<int>(log_settings_ptr_->value("log.files_count"),filescount)};

    //create log level
    int level {0};
    const bool level_ok {boost::conversion::try_lexical_convert<int>(log_settings_ptr_->value("log.level"),level)};
}

Controller::Controller(std::shared_ptr<Settings> log_settings_ptr, const std::string &log_path, const std::string &compressor_path)
    :log_settings_ptr_{log_settings_ptr},log_path_{log_path},compressor_path_{compressor_path}
{
    //create check period
    const bool& interval_ok {boost::conversion::try_lexical_convert<int>(log_settings_ptr_->value("log.check_period"),interval_)};
    if(!interval_ok){
        interval_=5000;
    }

    //init inner logger
    init_logger();
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
        const std::string& msg {"Controller::start()"};
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
        const std::string& msg {"Controller::stop()"};
        log_func_(msg);
    }
}

void Controller::write_log(log_level level, const std::string &msg)
{

}
