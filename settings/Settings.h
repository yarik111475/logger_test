#ifndef SETTINGS_H
#define SETTINGS_H

#include <map>
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
    void set_default();

public:
    explicit Settings(const std::string& file_path);
    ~Settings();

    void read_settings(const std::string& file_path);
    void write_settings(const std::string& file_path);

    void set_value(const std::string& key, const std::string &value);
    std::string value(const std::string& key)const;

    inline void set_log_func(std::function<void(const std::string&)> log_func){
        log_func_=log_func;
    }
};

#endif // SETTINGS_H
