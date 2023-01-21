#ifndef LOGCONTROLLER_H
#define LOGCONTROLLER_H

#include <functional>

#include <queue>
#include <mutex>
#include <thread>
#include <memory>
#include <string>
#include <boost/asio.hpp>
#include <boost/noncopyable.hpp>
#include <boost/system/error_code.hpp>

class Settings;

class LogController:private boost::noncopyable
{
private:
    //constants
    const int token_size_ {3};

    //compressor variables
#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
    std::string compressor_name_ {"7za.exe"};
#endif

#if defined(__linux) || defined(__linux__) || defined(__gnu_linux__)
    std::string compressor_name_ {"7zzs"};
#endif
    //log settings from file
    std::shared_ptr<Settings> log_settings_ptr_ {nullptr};

    //common variables
    bool started_ {false};
    bool start_stop_flag_ {false};
    std::mutex mtx_;
    std::condition_variable cv_;
    std::queue<char> task_queue_;
    std::shared_ptr<std::thread> queue_thread_ptr_ {nullptr};
    std::function<void(const std::string& msg)> log_func_ {nullptr};

    int interval_ {};
    std::string log_path_ {};
    std::string log_file_name_ {};
    std::string compressor_path_ {};

    //boost variables
    boost::asio::io_service io_service_;
    std::shared_ptr<std::thread> asio_thread_ptr_ {nullptr};
    std::shared_ptr<boost::asio::deadline_timer> timer_ptr_ {nullptr};

    void run();
    void tick(const boost::system::error_code& ec);

    //temporary solution
//public:
    //task operations
    void emplace_task();
    void execute_task();

    //file operations
    void rename_file(const std::string& file_name,const std::string& file_path);
    void compress_file(const std::string& file_name,const std::string& file_path);
    void remove_file(const std::string& file_name,const std::string& file_path);

    void operate_file(const std::string& file_name,const std::string& file_path);

public:
    explicit LogController(std::shared_ptr<Settings> log_settings_ptr, const std::string& log_path, const std::string& compressor_path);
    inline void set_log_func(std::function<void(const std::string& msg)> log_func){
        log_func_=log_func;
    }
    ~LogController();

    void start();
    void stop();
};

#endif // LOGCONTROLLER_H
