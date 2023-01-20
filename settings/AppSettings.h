#ifndef APPSETTINGS_H
#define APPSETTINGS_H

#include <map>
#include <string>
#include <functional>
#include <boost/noncopyable.hpp>

class AppSettings:private boost::noncopyable
{
private:
    std::map<std::string,std::string> settings_map_;
    std::function<void(const std::string&)> log_func_ {nullptr};
public:
    explicit AppSettings()=default;
    ~AppSettings()=default;

    void read_settings(const std::string& path);
    void write_settings(const std::string& path);

    void set_value(const std::string& key, const std::string &value);
    std::string value(const std::string& key)const;

    inline void set_log_func(std::function<void(const std::string&)> log_func){
        log_func_=log_func;
    }
};

#endif // APPSETTINGS_H
