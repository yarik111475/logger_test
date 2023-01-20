#include "Settings.h"

#include <boost/tokenizer.hpp>
#include <boost/filesystem.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ini_parser.hpp>
#include <boost/property_tree/json_parser.hpp>

void Settings::set_default()
{
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
}

Settings::Settings(const std::string &file_path):file_path_{file_path}
{
    read_settings(file_path_);
}

Settings::~Settings()
{
    write_settings(file_path_);
}

void Settings::read_settings(const std::string &file_path)
{
    if(!boost::filesystem::exists(file_path)){
        set_default();
        boost::property_tree::ini_parser::write_ini(file_path,p_tree_);
    }
    boost::property_tree::read_ini(file_path, p_tree_);
}

void Settings::write_settings(const std::string &file_path)
{
    boost::property_tree::write_ini(file_path,p_tree_);
}

void Settings::set_value(const std::string &key, const std::string &value)
{
    p_tree_.put(boost::trim_copy(key),boost::trim_copy(value));
}

std::string Settings::value(const std::string &key) const
{
    std:: string value=p_tree_.get<std::string>(key,"");
    return std::string {};
}
