#ifndef PROCESSRUNNER_H
#define PROCESSRUNNER_H

#include <QString>

class ProcessRunner
{
public:
    ProcessRunner();

    int run(const QString& applicationPath
            , const QString& arguments
            , const int timeoutMs = 0);
};

#endif // PROCESSRUNNER_H
