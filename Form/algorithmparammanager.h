#ifndef ALGORITHMPARAMMANAGER_H
#define ALGORITHMPARAMMANAGER_H

#include "Common/common.h"
#include <QObject>
#include <QMutex>

class AlgorithmParamManager : public QObject
{
    Q_OBJECT

public:
    static AlgorithmParamManager* instance();

    InThresholdParam getAlgParam() const;
    void setAlgParam(const InThresholdParam &param);

    void readConfig();
    void saveConfig();

signals:
    void algParamChanged(const InThresholdParam &param);

private:
    explicit AlgorithmParamManager(QObject *parent = nullptr);
    static AlgorithmParamManager* m_instance;
    static QMutex m_mutex;

    InThresholdParam m_algParam;
};

#endif // ALGORITHMPARAMMANAGER_H
