#ifndef CONTROLLER_H
#define CONTROLLER_H

#include <queue>
#include <memory>
#include <thread>
#include <mutex>
#include <functional>
#include <condition_variable>
#include <QThread>

class QTimer;

class Controller : public QThread
{
    Q_OBJECT
private:
    int interval_ {};
    QString log_path_ {};
    bool start_stop_flag_ {false};
    bool started_ {false};
    std::mutex mtx_;
    std::queue<char> task_queue_;
    std::condition_variable cv_;
    std::shared_ptr<QTimer> timer_ptr_ {nullptr};
    std::function<void(const std::string& msg)> log_func_ {nullptr};

private slots:
    void slot_timeout();
    void slot_execute_task();

public:
    explicit Controller(int interval, const QString& log_path, QObject* parent=nullptr);
    virtual ~Controller();
    void start();
    void stop();

    inline void set_log_func(std::function<void(const std::string& msg)> log_func){
        log_func_=log_func;
    }

protected:
    virtual void run()override;
};

#endif // CONTROLLER_H
