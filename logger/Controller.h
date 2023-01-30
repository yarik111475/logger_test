#ifndef CONTROLLER_H
#define CONTROLLER_H

#include <memory>
#include <boost/asio.hpp>
#include <boost/noncopyable.hpp>

class Settings;

class Controller:private boost::noncopyable
{
private:
    //compressor variables
#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
    std::string compressor_name_ {"7za.exe"};
#endif

#if defined(__linux) || defined(__linux__) || defined(__gnu_linux__)
    std::string compressor_name_ {"7zzs"};
#endif
    //common variables
    int interval_ {};
    bool started_ {false};
    std::string log_path_ {};
    std::string log_file_name_ {};
    std::string compressor_path_ {};
    std::shared_ptr<Settings> log_settings_ptr_ {nullptr};
    std::function<void(const std::string& msg)> log_func_ {nullptr};

    //boost variables
    boost::asio::io_service io_service_;
    std::shared_ptr<std::thread> asio_thread_ptr_ {nullptr};
    std::shared_ptr<boost::asio::deadline_timer> timer_ptr_ {nullptr};

    //timer operations
    void tick(const boost::system::error_code& ec);
    void execute_task();

    //file operations
    void rename_files(const std::string& path);
    void compress_files(const std::string& path);
    void remove_file(const std::string& file_name,const std::string& file_path);

    //inner logger initialization
    void init_logger();

public:
    enum log_level{
        trace,
        debug,
        info,
        warn,
        err,
        critical,
        off
    };

    explicit Controller(std::shared_ptr<Settings> log_settings_ptr, const std::string& log_path, const std::string& compressor_path);
    inline void set_log_func(std::function<void(const std::string& msg)> log_func){
        log_func_=log_func;
    }
    ~Controller();

    void start();
    void stop();
    void write_log(log_level level, const std::string& msg);
};

#endif // CONTROLLER_H
