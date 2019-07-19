#include "configreader.h"
#include <QTextStream>
#include <QJsonDocument>
#include <QJsonObject>
#include <iostream>
#include "cdbconfig.h"

namespace json {
    const QString x86(){ return "x86"; }
    const QString x64(){ return "x64"; }
    const QString SymbolsPath() { return "symbols_path";}
    const QString SrcPath() { return "src_path"; }
    const QString ImagePath() { return "image_path"; }
}

ConfigReader::ConfigReader(CdbConfig *config, const bool is64bit):m_config(config),m_is64bit(is64bit)
{
}

bool ConfigReader::read(QIODevice *device)
{
    QTextStream stream(device);
    stream.setCodec("UTF-8");
    QByteArray text = stream.readAll().toUtf8();

    QJsonParseError err;
    QJsonDocument doc = QJsonDocument::fromJson(text,&err);

    bool ok = false;
    if (err.error != QJsonParseError::NoError || !doc.isObject())
    {
        std::wcerr<<"Config file corrupted"<<std::endl;
        return ok;
    }

    QJsonObject cfgObj;
    QJsonObject obj = doc.object();
    if(m_is64bit && obj.contains(json::x64())){
        cfgObj = obj.value(json::x64()).toObject();
    }else{
        if(obj.contains(json::x86())){
            cfgObj = obj.value(json::x86()).toObject();
        }
    }

    if(!cfgObj.isEmpty()){
        if(cfgObj.contains(json::SrcPath())){
            m_config->SrcPath = cfgObj.value(json::SrcPath()).toString();
        }

        if(cfgObj.contains(json::SymbolsPath())){
            m_config->SymbolsPath = cfgObj.value(json::SymbolsPath()).toString();
        }

        if(cfgObj.contains(json::ImagePath())){
            m_config->ImagePath = cfgObj.value(json::ImagePath()).toString();
        }

        ok = true;
    }else{
        std::wcerr<<"Config object not found"<<std::endl;
    }

    return ok;
}
