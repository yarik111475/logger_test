#ifndef LOGCONTROLLER_H
#define LOGCONTROLLER_H

#include <functional>
#include <boost/asio.hpp>
#include <boost/thread.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/noncopyable.hpp>


class LogController:private boost::noncopyable
{
private:
    bool started_ {false};
    bool repeat_flag_ {false};
    bool start_stop_flag_ {false};
    std::mutex mtx_;
    std::condition_variable cv_;
    std::shared_ptr<std::thread> thread_ptr_ {nullptr};
    std::shared_ptr<boost::asio::deadline_timer> timer_ptr {nullptr};
    std::function<void(const std::string& msg)> callback_ {nullptr};

    void run();

    //temporary solution
public:
    void emplace_task();

public:
    explicit LogController();
    inline void set_callback(std::function<void(const std::string& msg)> callback){
        callback_=callback;
    }
    ~LogController();

    void start();
    void stop();
};

#endif // LOGCONTROLLER_H
