#ifndef GLOBALCONFIGREADER_H
#define GLOBALCONFIGREADER_H
#include <QIODevice>

struct GlobalConfig;
class GlobalConfigReader
{
public:
    GlobalConfigReader(GlobalConfig* config);
    bool read(QIODevice* device);
private:
    GlobalConfig* m_config;
};

#endif // GLOBALCONFIGREADER_H
