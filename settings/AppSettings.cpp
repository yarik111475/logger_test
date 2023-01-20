#include "AppSettings.h"

#include <boost/tokenizer.hpp>
#include <boost/filesystem.hpp>
#include <boost/algorithm/string.hpp>

void AppSettings::read_settings(const std::string &path)
{
    const boost::filesystem::path& file_path(path);
    if(!boost::filesystem::exists(file_path)){
        return;
    }
    boost::filesystem::ifstream in(file_path);
    std::string line;
    while(std::getline(in,line)){
        //check line is empty
        if(line.empty()){
            continue;
        }
        //check line starts with '#' (comment case)
        if(line.substr(0,1)=="#"){
            continue;
        }
        //split readed line
        std::vector<std::string> line_parts;
        boost::split(line_parts,line,boost::is_any_of("="));
        //check splitted results and emplace them into settings_map_
        if(line_parts.size()==2){
            const std::string& key {line_parts.at(0)};
            const std::string& value {line_parts.at(1)};
            settings_map_.emplace(key,value);
        }
    }
}

void AppSettings::write_settings(const std::string &path)
{

}

void AppSettings::set_value(const std::string &key, const std::string &value)
{
    settings_map_.emplace(key,value);
}

std::string AppSettings::value(const std::string &key) const
{
    const auto& found {settings_map_.find(key)};
    if(found!=settings_map_.end()){
        return found->second;
    }
    return std::string {};
}
