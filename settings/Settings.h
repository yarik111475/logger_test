#ifndef SETTINGS_H
#define SETTINGS_H

#include <string>
#include <functional>
#include <boost/noncopyable.hpp>
#include <boost/property_tree/ptree.hpp>

class Settings:private boost::noncopyable
{
protected:
    boost::property_tree::ptree p_tree_;
    std::string file_path_ {};
    std::function<void(const std::string&)> log_func_ {nullptr};
    virtual void init_default()=0;

public:
    explicit Settings(const std::string& file_path);
    ~Settings();

    void read_settings();
    void write_settings();

    void set_value(const std::string& key, const std::string &value);
    std::string value(const std::string& key)const;

    inline void set_log_func(std::function<void(const std::string&)> log_func){
        log_func_=log_func;
    }
};

#endif // SETTINGS_H
