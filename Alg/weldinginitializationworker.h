#ifndef WELDINGINITIALIZATIONWORKER_H
#define WELDINGINITIALIZATIONWORKER_H

#include <QObject>

namespace Alg {

class WeldingInitializationWorker : public QObject
{
    Q_OBJECT
public:
    explicit WeldingInitializationWorker(QObject *parent = nullptr);

public slots:
    void doInitialize();
signals:
    void initializationFinished(bool success);
};

} // namespace Alg

#endif // WELDINGINITIALIZATIONWORKER_H
