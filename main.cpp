#include <QCoreApplication>
#include <QCommandLineOption>
#include <QCommandLineParser>
#include <QFileInfo>
#include <QProcess>
#include <iostream>
#include <QTextStream>
#include <QDir>
#include <string>

#include "processrunner.h"
#include "minidumpxmlreader.h"
#include "minidump.h"
#include "cdbconfig.h"
#include "configreader.h"
#include "globalconfig.h"
#include "globalconfigreader.h"

const QString DefaultZipExctractor = "C://Program Files//7-Zip//7z.exe";
const QString DeafultCdbPath = QString("\"C:\\Program Files (x86)\\Windows Kits\\10\\Debuggers\\x64\\cdb.exe\"");

bool handleCDB(CdbConfig *cdbConfig);

bool unzip(const QString& archive, const QString& folder);

bool readGlobalConfig(GlobalConfig* consig);
bool readConfig(const QString& appName, const bool is64Bit, CdbConfig* config);

GlobalConfig g_config;

void writeUuids(const QString& filename, const QStringList& uuidList);
void readUuids(const QString& filename, QStringList& outUuidList);

QString normalizePath(const QString& path){
    return QDir::toNativeSeparators(QFileInfo(path).absoluteFilePath());
}


int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    a.setApplicationName("CrashRpt Parser");
    a.setApplicationVersion("1.0.1");

    g_config.CdbPath = DeafultCdbPath;
    g_config.ZipExtractorPath = DefaultZipExctractor;

    readGlobalConfig(&g_config);

    QCommandLineParser parser;
    parser.setApplicationDescription("Test helper");
    parser.addHelpOption();
    parser.addVersionOption();

    const QCommandLineOption dumpFolderOption("dumps", "Dumps folder","dumps",qApp->applicationDirPath());
    parser.addOption(dumpFolderOption);

    const QCommandLineOption reportolderOption("reports", "Report folder","reports",qApp->applicationDirPath());
    parser.addOption(reportolderOption);

    parser.process(a.arguments());

    QString dumpsFolder = parser.value(dumpFolderOption);
    QString reportsFodler= parser.value(reportolderOption);
    const QString ParsedFilePath = QString("%1/parsed.txt").arg(dumpsFolder);
    const QString CorrupedFilePath = QString("%1/corrupted.txt").arg(dumpsFolder);

    //enum zip archives in folder
    std::wcout<<"Scan crashrpt folder..."<<std::endl;
    QDir dumpsDir(dumpsFolder);
    QFileInfoList crashRptArchives = dumpsDir.entryInfoList(QStringList()<<"*.zip", QDir::Files);
    std::wcout<<"Total crashrpt found:"<<crashRptArchives.count()<<std::endl;

    if(crashRptArchives.isEmpty()){
        return 0;
    }

    QStringList parsedCrashes;

    readUuids(ParsedFilePath, parsedCrashes);

    QStringList corruptedCrashes;


    int counter = 0;
    foreach (QFileInfo crashrptInfo, crashRptArchives) {

        if(parsedCrashes.contains(crashrptInfo.completeBaseName())){
            continue;
        }

        std::wcout<<crashrptInfo.completeBaseName().toStdWString()<<std::endl;
        //unzip archive to tmp folder
        QString folder = QString("%1/%2")
                .arg( crashrptInfo.dir().absolutePath() )
                .arg( crashrptInfo.completeBaseName() );
        folder = normalizePath(folder);
        QString archive = normalizePath(crashrptInfo.absoluteFilePath());

        bool ok = unzip( archive, folder );

        //parse xml and extract bit OS build
        //read crashrpt xml
        MiniDump miniDump;
        if(ok){
            QString xmlPath = QString("%1/crashrpt.xml").arg(folder);
            QFile file(xmlPath);

            ok = file.open(QFile::ReadOnly | QFile::Text);
            if(!ok){
                std::wcerr<<"Cannot open xml file:"<<xmlPath.toStdWString()<<std::endl;
            }

            MiniDumpXmlReader xmlReader(&miniDump);
            ok = xmlReader.read(&file);
            if(!ok){
                std::wcerr<<"Cannot parse xml file:"<<xmlPath.toStdWString()<<std::endl;
            }
        }

        //read config for app
        CdbConfig cdbConfig;
        if(ok){
            ok = readConfig(miniDump.AppName, miniDump.OSIs64Bit, &cdbConfig);
            if(!ok){
                std::wcerr<<"Connot read config for app:"<<miniDump.AppName.toStdWString()<<std::endl;
            }
        }

        //analize dump
        QString miniDumpPath = QString("%1/crashdump.dmp")
                .arg(folder);

        cdbConfig.MiniDumpPath = normalizePath(miniDumpPath);

        QString logPathFolder = reportsFodler;
        if(logPathFolder.isEmpty()){
            logPathFolder = folder;
        }

        logPathFolder.append(QString("/%1").arg(miniDump.AppName));
        if (!QFile::exists(logPathFolder)) {
            QDir dir;
            dir.mkpath(logPathFolder);
        }

        QString logPath = QString("%1/%2_report.txt")
                .arg(logPathFolder)
                .arg(crashrptInfo.completeBaseName());

        cdbConfig.LogPath = normalizePath(logPath);

        if(ok){
            ok = handleCDB(&cdbConfig);
        }

        if(ok){
            counter++;
            parsedCrashes << crashrptInfo.completeBaseName();
        }else{
            corruptedCrashes<< crashrptInfo.completeBaseName();
        }
    }

    std::wcout<<"[OK] parsed files:"<<counter<<std::endl;
    std::wcout<<"[FAILED] parsed files:"<<corruptedCrashes.count()<<std::endl;

    writeUuids( ParsedFilePath , parsedCrashes);
    writeUuids( CorrupedFilePath, corruptedCrashes);
    return 0;
}

bool handleCDB(CdbConfig* cdbConfig){

    std::wcout<<"Handle cdb..."<<std::endl;

    QStringList arguments;
    if( !cdbConfig->MiniDumpPath.isEmpty() ){
        arguments.push_back("-z");
        arguments.push_back( QString("\"%1\"").arg(cdbConfig->MiniDumpPath) );
    }else{
        std::wcerr<<"Mini dump not set"<<std::endl;
        return false;
    }

    if(!cdbConfig->SymbolsPath.isEmpty()){
        arguments.push_back("-y");
        arguments.push_back( QString("\"%1\"").arg(cdbConfig->SymbolsPath) );
    }

    if(!cdbConfig->SrcPath.isEmpty()){
        arguments.push_back("-srcpath");
        arguments.push_back( QString("\"%1\"").arg(cdbConfig->SrcPath) );
    }

    if(!cdbConfig->ImagePath.isEmpty()){
        arguments.push_back("-i");
        arguments.push_back( QString("\"%1\"").arg(cdbConfig->ImagePath) );
    }

    //script commands
    arguments.push_back("-c");
    arguments.push_back("\".lastevent;.ecxr;k;.logclose;q\"");

    arguments.push_back("-lines");

    if(!cdbConfig->LogPath.isEmpty()){
        arguments.push_back("-logau");
        arguments.push_back( QString("\"%1\"").arg(cdbConfig->LogPath) );
    }

    ProcessRunner runner;
    int ret = runner.run(g_config.CdbPath,arguments.join(" "));
    const bool success = (ret == 0);
    if(ret == 1){
        std::wcerr<<"Err CDB timeout"<<std::endl;
    }

    std::wcout<<"Handle cdb [COMPLETE]"<<std::endl;

    return success;
}

bool unzip(const QString& archive
           , const QString& folder){

    std::wcout<<L"Unzip ..."<<std::endl;
    ProcessRunner runner;

    QString arguments = QString("e \"%1\" -o\"%2\" -y")
            .arg( archive )
            .arg( folder );

    int ret = runner.run(g_config.ZipExtractorPath,arguments);
    if(ret != 0){
        std::wcerr<<"Unzip [FAILED]";
    }else{
        std::wcerr<<"Unzip [OK]";
    }

    return (ret == 0);
}

bool readConfig(const QString& appName, const bool is64Bit, CdbConfig* config){

    //read config for app from
    QString configPath = QString("%1/%2.config")
            .arg(qApp->applicationDirPath())
            .arg(appName);
    QFile file(configPath);
    if(!file.exists()){
        std::wcerr<<"Config for:"<<appName.toStdWString()<<" not found"<<std::endl;
        return false;
    }

    bool result = false;
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)){
        ConfigReader configReader(config, is64Bit);
        result = configReader.read(&file);
    }
    else{
        std::wcerr<<"Can not open config file:"<<configPath.toStdWString()<<std::endl;
    }
    file.close();

    return result;
}

bool readGlobalConfig(GlobalConfig* config)
{
    QString configPath = QString("%1/global.config")
            .arg(qApp->applicationDirPath());

    QFile file(configPath);
    if(!file.exists()){
        std::wcerr<<"Global config not found"<<std::endl;
        return false;
    }

    bool result = false;
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)){
        GlobalConfigReader configReader(config);
        result = configReader.read(&file);
    }
    else{
        std::wcerr<<"Can not open config file:"<<configPath.toStdWString()<<std::endl;
    }
    file.close();

    return result;
}

void writeUuids(const QString& filename, const QStringList& uuidList)
{
    if(uuidList.isEmpty()){
        return;
    }

    QFile file(filename);
    if (file.open(QIODevice::ReadWrite | QIODevice::Text | QIODevice::Append)){
        QTextStream stream(&file);
        stream.setCodec("UTF-8");
        foreach (const QString& uuid, uuidList) {
            stream<<uuid<<"\r\n";
        }
    }
    else{
        std::wcerr<<"Can not open file:"<<filename.toStdWString()<<std::endl;
    }
    file.close();
}

void readUuids(const QString& filename, QStringList& outUuidList)
{
    if(filename.isEmpty()){
        return;
    }

    QFile file(filename);
    if(!file.exists()){
        std::wcout<<"File not found:"<<filename.toStdWString()<<std::endl;
        return;
    }

    if (file.open(QIODevice::ReadOnly | QIODevice::Text)){
        QTextStream stream(&file);
        stream.setCodec("UTF-8");
        QString line;
        do {
            line = stream.readLine();
            outUuidList.push_back(line);
        } while (!line.isNull());
    }
    else{
        std::wcerr<<"Can not open file:"<<filename.toStdWString()<<std::endl;
    }
    file.close();
}
