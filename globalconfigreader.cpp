#include "globalconfigreader.h"

#include <QTextStream>
#include <QJsonDocument>
#include <QJsonObject>
#include <iostream>
#include "globalconfig.h"

namespace json {
    const QString cdbPath() {return "cdb_path";}
    const QString zipExtractorPath() {return "zip_path";}
}

GlobalConfigReader::GlobalConfigReader(GlobalConfig *config):m_config(config)
{

}

bool GlobalConfigReader::read(QIODevice *device)
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

    QJsonObject cfgObj = doc.object();

    if(!cfgObj.isEmpty()){

        if(cfgObj.contains(json::cdbPath())){
            m_config->CdbPath = cfgObj.value(json::cdbPath()).toString();
        }

        if(cfgObj.contains(json::zipExtractorPath())){
            m_config->ZipExtractorPath = cfgObj.value(json::zipExtractorPath()).toString();
        }

        ok = true;
    }else{
        std::wcerr<<"Config object not found"<<std::endl;
    }

    return ok;

}
