#ifndef DUMPXMLREADER_H
#define DUMPXMLREADER_H
#include <QString>
#include <QIODevice>
#include <QXmlStreamReader>
#include <QHash>

class MiniDump;
class MiniDumpXmlReader
{
public:
    MiniDumpXmlReader(MiniDump* miniDump);
    bool read(QIODevice *device );
private:
    MiniDump* m_miniDump;
    QXmlStreamReader m_reader;

    void readCrashRpt();
    void readGuid();
    void readAppName();
    void readAppVersion();
    void readOSIs64Bit();
};

#endif // DUMPXMLREADER_H
