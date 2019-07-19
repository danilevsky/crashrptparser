#ifndef MINIDUMP_H
#define MINIDUMP_H
#include <QString>

class MiniDump
{
public:
    MiniDump();

    QString CrashGUID;
    QString AppName;
    QString AppVersion;
    bool OSIs64Bit;
};

#endif // MINIDUMP_H
