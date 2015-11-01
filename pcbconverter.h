#ifndef PCBCONVERTER_H
#define PCBCONVERTER_H

#include <QWidget>
#include "zoomablegerberview.h"
class DrillReader;
class GerberReader;
class PathContur;

class GerberReader;
class PathContur;

namespace Ui {
class PCBConverter;
}

class PCBConverter : public QWidget
{
    Q_OBJECT

public:
    explicit PCBConverter(QString firstLayer, QString secondLayer, QString drillPath, QWidget *parent = 0);
    ~PCBConverter();

protected:
    void closeEvent(QCloseEvent *event);

private slots:

    void on_pushButton_clicked();

    void on_exitButton_clicked();

    void on_pushButton_2_clicked();

    void on_generateButton_clicked();

    void on_previewButton_clicked();

    void on_pushButton_3_clicked();

    void on_outputFile_textChanged(const QString &arg1);

    void on_writeToFileButton_clicked();
    
    void on_startMilling_clicked();

    void dataForGenerateChanged();

    void on_methodComboBox_currentIndexChanged(int index);

    void on_cutEdges_stateChanged(int state);

    void on_pushButton_4_clicked();

    void gp_textChanged(const QString &arg1);

private:
    void setLayoutVisible(QLayout *layout, bool visible);
    void setFormRowVisible(int row, bool visible);
    void deleteGeneratedObjects();
    Ui::PCBConverter *ui;
    GerberReader *mGerberReader;
    PathContur *mPathContur1;
    PathContur *mPathContur2;
    DrillReader *mDrillReader;
    ZoomableGerberView mGerberView;
    QString exportedFileName;
};

#endif // PCBCONVERTER_H
