#include "AppSettings.h"

#include <boost/tokenizer.hpp>
#include <boost/filesystem.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/property_tree/ini_parser.hpp>
#include <boost/property_tree/json_parser.hpp>

void AppSettings::read_settings(const std::string &path)
{
    const boost::filesystem::path& file_path(path);
    if(!boost::filesystem::exists(file_path)){
        const std::map<std::string,std::string>& default_map {
            {"log.log_compress_time", "0"},
            {"log.log_remove_time","0"},
            {"log.log_check_time", "5000"},
            {"log.max_log_size", "1024"},
            {"log.level", "trace"}
        };
        for(const auto& pair:default_map){
            set_value(pair.first,pair.second);
        }
        boost::property_tree::ini_parser::write_ini(path,p_tree_);
    }
    boost::property_tree::read_ini(path, p_tree_);
}

void AppSettings::write_settings(const std::string &path)
{
    boost::property_tree::write_ini(path,p_tree_);
}

void AppSettings::set_value(const std::string &key, const std::string &value)
{
    p_tree_.put(boost::trim_copy(key),boost::trim_copy(value));
}

std::string AppSettings::value(const std::string &key) const
{
    std:: string value=p_tree_.get<std::string>(key,"");
    return std::string {};
}
