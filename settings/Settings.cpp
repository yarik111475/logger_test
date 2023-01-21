#include "Settings.h"

#include <boost/filesystem.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/property_tree/ini_parser.hpp>

Settings::Settings(const std::string &file_path):file_path_{file_path}
{
}

Settings::~Settings()
{
    write_settings();
}

void Settings::read_settings()
{
    if(!boost::filesystem::exists(file_path_)){
        init_default();
        boost::property_tree::ini_parser::write_ini(file_path_,p_tree_);
    }
    boost::property_tree::read_ini(file_path_, p_tree_);
}

void Settings::write_settings()
{
    boost::property_tree::write_ini(file_path_,p_tree_);
}

void Settings::set_value(const std::string &key, const std::string &value)
{
    p_tree_.put(boost::trim_copy(key),boost::trim_copy(value));
}

std::string Settings::value(const std::string &key) const
{
    const std::string& value {boost::trim_copy(p_tree_.get<std::string>(key,""))};
    return value;
}
