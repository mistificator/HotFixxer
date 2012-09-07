#ifndef QHOTFIXXER_H
#define QHOTFIXXER_H

#include <QObject>

class QHotFixxer : public QObject
{
    Q_OBJECT
public:
    explicit QHotFixxer(QObject * _parent = 0);
    ~QHotFixxer();
    void fix();
signals:
    void finished();
public slots:

private:
    struct Data;
    Data * d;
};

#endif // QHOTFIXXER_H
