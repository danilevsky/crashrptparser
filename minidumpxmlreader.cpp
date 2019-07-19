#include "minidumpxmlreader.h"
#include <QtXml>
#include <QFile>
#include <iostream>
#include <QXmlReader>
#include "minidump.h"

MiniDumpXmlReader::MiniDumpXmlReader(MiniDump *miniDump):m_miniDump(miniDump)
{

}

bool MiniDumpXmlReader::read(QIODevice *device)
{
    m_reader.setDevice(device);
    while(m_reader.readNextStartElement()){


        std::wcout<<"parse name:"<<m_reader.name().toString().toStdWString()<<std::endl;

        if(m_reader.name() == "CrashRpt"){
            readCrashRpt();
        }else{
            //skip
        }
    }

    const bool noErr = (QXmlStreamReader::NoError == m_reader.error()
                        || QXmlStreamReader::PrematureEndOfDocumentError == m_reader.error() );
    if(!noErr){
        std::wcout<<"Error string:"<<m_reader.errorString().toStdWString();
    }

    return noErr;
}

void MiniDumpXmlReader::readCrashRpt()
{
    std::wcout<<"readCrashRpt"<<std::endl;

    while(m_reader.readNextStartElement()){
        std::wcout<<"m_reader name:"<<m_reader.name().toString().toStdWString()<<std::endl;

        if(m_reader.name() == "CrashGUID"){
            readGuid();
        }else if( m_reader.name() == "AppName" ){
            readAppName();
        } else if(m_reader.name() == "AppVersion"){
            readAppVersion();
        } else if(m_reader.name() == "OSIs64Bit"){
            readOSIs64Bit();
        }
        else{
            m_reader.skipCurrentElement();
        }
    }
}

void MiniDumpXmlReader::readGuid()
{
    m_miniDump->CrashGUID = m_reader.readElementText();
}

void MiniDumpXmlReader::readAppName()
{
    m_miniDump->AppName = m_reader.readElementText();
}

void MiniDumpXmlReader::readAppVersion()
{
    m_miniDump->AppVersion = m_reader.readElementText();
}

void MiniDumpXmlReader::readOSIs64Bit()
{
    QString tmp = m_reader.readElementText();
    bool val = false;
    if(!tmp.isEmpty()){
        val = (tmp == "1");
    }
    m_miniDump->OSIs64Bit = val;
}
