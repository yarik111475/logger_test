#ifndef APPSETTINGS_H
#define APPSETTINGS_H

#include <map>
#include <string>
#include <functional>
#include <boost/noncopyable.hpp>
#include <boost/property_tree/ptree.hpp>

class AppSettings:private boost::noncopyable
{
private:
    boost::property_tree::ptree p_tree_;
    std::string file_path_ {};
    std::function<void(const std::string&)> log_func_ {nullptr};
    void set_default();

public:
    explicit AppSettings(const std::string& file_path);
    ~AppSettings();

    void read_settings(const std::string& file_path);
    void write_settings(const std::string& file_path);

    void set_value(const std::string& key, const std::string &value);
    std::string value(const std::string& key)const;

    inline void set_log_func(std::function<void(const std::string&)> log_func){
        log_func_=log_func;
    }
};

#endif // APPSETTINGS_H
