#ifndef CONFIGREADER_H
#define CONFIGREADER_H

#include <QIODevice>

struct CdbConfig;
class ConfigReader
{
public:
    ConfigReader(CdbConfig* config, const bool is64bit);
    bool read(QIODevice* device);
private:
    CdbConfig* m_config;
    bool m_is64bit;
};

#endif // CONFIGREADER_H
