#include "LogSettings.h"

#include <map>

void LogSettings::init_default()
{
    const std::map<std::string,std::string>& default_map {
        {"log.compress_period", "0"},
        {"log.remove_period","0"},
        {"log.check_period", "5000"},
        {"log.file_name", "usagent.txt"},
        {"files_count", "3"},
        {"log.level", "trace"}
    };
    for(const auto& pair:default_map){
        set_value(pair.first,pair.second);
    }
}

LogSettings::LogSettings(const std::string &file_path)
    :Settings(file_path)
{
}
