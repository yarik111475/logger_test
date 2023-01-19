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


class LogController:private boost::noncopyable
{
private:
    //common variables
    bool started_ {false};
    bool start_stop_flag_ {false};
    std::mutex mtx_;
    std::condition_variable cv_;
    std::queue<char> task_queue_;
    std::shared_ptr<std::thread> thread_ptr_ {nullptr};
    std::function<void(const std::string& msg)> log_func_ {nullptr};
    std::string log_path_ {};

    //boost variables
    int interval_ {};
    boost::asio::io_service io_service_;
    std::shared_ptr<std::thread> asio_thread_ptr_ {nullptr};
    std::shared_ptr<boost::asio::deadline_timer> timer_ptr_ {nullptr};

    void run();
    void tick(const boost::system::error_code& ec);

    //temporary solution
//public:
    void emplace_task();
    void execute_task();
    void operate_file(const std::string& file_name,const std::string& file_path);

public:
    explicit LogController(int interval, const std::string& log_path);
    inline void set_log_func(std::function<void(const std::string& msg)> log_func){
        log_func_=log_func;
    }
    ~LogController();

    void start();
    void stop();
};

#endif // LOGCONTROLLER_H
