#include <QApplication>
#include "pcbconverter.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QString firstLayer;
    QString secondLayer;
    QString drillPath;
    if(argc>1)
        firstLayer = QString(argv[1]);
    if(argc>2)
        secondLayer = QString(argv[2]);
    if(argc>3)
        drillPath = QString(argv[3]);
    PCBConverter converter(firstLayer, secondLayer, drillPath);
    converter.show();
    return a.exec();
}
