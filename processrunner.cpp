#include "processrunner.h"

#include <Windows.h>
#include <QFileInfo>
#include <QDir>
#include <iostream>
#include <string>

void writeMessage(const QString& msg){
    std::wcout <<msg.toStdWString()<<std::endl;
}
ProcessRunner::ProcessRunner()
{

}

int ProcessRunner::run(const QString &applicationPath
                       , const QString &arguments
                       , const int timeoutMs)
{

    HANDLE hJob = ::CreateJobObject(NULL, NULL);
    if(hJob == NULL)
    {
//        HRESULT hr = __HRESULT_FROM_WIN32(::GetLastError());

        writeMessage(QString("Create Job object [FAILED]. Error:'%1'").arg(::GetLastError()));

        return -1;
    }

    JOBOBJECT_EXTENDED_LIMIT_INFORMATION limitInformation = {0};
    limitInformation.BasicLimitInformation.LimitFlags = JOB_OBJECT_LIMIT_KILL_ON_JOB_CLOSE;

    if(0 == SetInformationJobObject(hJob, JobObjectExtendedLimitInformation, &limitInformation, sizeof(limitInformation)))
    {
        writeMessage(QString("Set information job object [FAILED]. Error:'%1'").arg(::GetLastError()));
        return -1;
    }

    QString commandLine = QString("%1 %2").arg(applicationPath).arg(arguments);

    writeMessage(QString("Run application command: '%1'").arg(commandLine));


    //run proc
    STARTUPINFO si = {sizeof(si)};
    PROCESS_INFORMATION pi;
    ::CreateProcessW(NULL
                  , (LPWSTR)commandLine.toStdWString().c_str()
                  , NULL
                  , NULL
                  , FALSE
                  , CREATE_SUSPENDED
                  , NULL
                  , NULL
                  , &si
                  , &pi);

    AssignProcessToJobObject(hJob, pi.hProcess);
    ResumeThread(pi.hThread);
    CloseHandle(pi.hThread);

    HANDLE handles[2];
    handles[0] = pi.hProcess;
    handles[1] = hJob;

    int errorCode = 0;
    const DWORD timeout = timeoutMs > 0?timeoutMs:INFINITE;
    DWORD dw = WaitForMultipleObjects(2, handles, FALSE, timeout);
    switch (dw - WAIT_OBJECT_0) {
    case 0:
        //proc complete

        break;
    case 1:
        //timeout
        errorCode = 1;
        break;
    }

    ::CloseHandle(pi.hProcess);
    ::CloseHandle(hJob);

    writeMessage("Proc complete");
    return errorCode;
}
