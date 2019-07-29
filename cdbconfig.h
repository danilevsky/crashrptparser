#ifndef CDBCONFIG_H
#define CDBCONFIG_H

#include <QString>

struct CdbConfig
{
    QString MiniDumpPath;
    QString SymbolsPath;
    QString SrcPath;
    QString LogPath;
    QString ImagePath;
    QString MinVersion;
};

#endif // CDBCONFIG_H
