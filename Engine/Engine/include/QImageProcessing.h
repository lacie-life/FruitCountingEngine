#ifndef QIMAGEPROCESSING_H
#define QIMAGEPROCESSING_H

#include <QObject>

class QImageProcessing : public QObject
{
    Q_OBJECT

public:
    explicit QImageProcessing(QObject *parent = nullptr);
    ~QImageProcessing();

signals:

};

#endif // QIMAGEPROCESSING_H