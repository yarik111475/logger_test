#ifndef LOGSETTINGS_H
#define LOGSETTINGS_H

#include "Settings.h"

class LogSettings : public Settings
{
protected:
    virtual void init_default() override;

public:
    explicit LogSettings(const std::string& file_path);
    virtual ~LogSettings()=default;
};

#endif // LOGSETTINGS_H
