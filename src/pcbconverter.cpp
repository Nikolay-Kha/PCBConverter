#include "pcbconverter.h"
#include "ui_pcbconverter.h"
#include <QFileDialog>
#include <QMessageBox>
#include <QProcess>
#include "gerberreader.h"
#include "pcbpathcontur.h"
#include "printingpathcontur.h"
#include "gcodeexport.h"
#include "drillreader.h"
#include "settings.h"

PCBConverter::PCBConverter(QString firstLayer, QString secondLayer, QString drillPath, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::PCBConverter)
{

    mGerberReader = 0;
    mPathContur1 = 0;
    mPathContur2 = 0;
    mDrillReader = 0;
    ui->setupUi(this);
    Settings::load(this);
    ui->generateButton->setEnabled(QFile::exists(ui->firstLayer->text()));
    connect(ui->firstLayer, SIGNAL(textChanged(QString)), this, SLOT(gp_textChanged(QString)));
    connect(ui->boardOffset,SIGNAL(valueChanged(double)), this, SLOT(dataForGenerateChanged()));
    connect(ui->millingDiameter,SIGNAL(valueChanged(double)), this, SLOT(dataForGenerateChanged()));
    connect(ui->conturTimes,SIGNAL(valueChanged(int)), this, SLOT(dataForGenerateChanged()));
    connect(ui->nozzleDiameter,SIGNAL(valueChanged(double)), this, SLOT(dataForGenerateChanged()));
    connect(ui->secondLayer,SIGNAL(textChanged(QString)), this, SLOT(dataForGenerateChanged()));
    connect(ui->drillPath,SIGNAL(textChanged(QString)), this, SLOT(dataForGenerateChanged()));
    connect(ui->firstMirror,SIGNAL(currentIndexChanged(int)), this, SLOT(dataForGenerateChanged()));
    connect(ui->secondMirror,SIGNAL(currentIndexChanged(int)), this, SLOT(dataForGenerateChanged()));

    if(!firstLayer.isEmpty())
        ui->firstLayer->setText(QFileInfo(firstLayer).canonicalFilePath());
    if(!secondLayer.isEmpty())
        ui->secondLayer->setText(QFileInfo(secondLayer).canonicalFilePath());
    if(!drillPath.isEmpty())
        ui->drillPath->setText(QFileInfo(drillPath).canonicalFilePath());

    for(int i=0; i<ui->formLayout->count(); i++)
        if(ui->formLayout->itemAt(i)->layout())
            ui->formLayout->itemAt(i)->layout()->setContentsMargins(0,0,0,6);

    on_methodComboBox_currentIndexChanged(ui->methodComboBox->currentIndex());
    on_cutEdges_stateChanged(ui->cutEdges->checkState());
}

PCBConverter::~PCBConverter()
{
    deleteGeneratedObjects();
    delete ui;
}

void PCBConverter::deleteGeneratedObjects()
{
    mGerberView.clear();
    if(mGerberReader)
        delete mGerberReader;
    mGerberReader = 0;
    if(mPathContur1)
        delete mPathContur1;
    mPathContur1 = 0;
    if(mPathContur2)
        delete mPathContur2;
    mPathContur2 = 0;
    if(mDrillReader)
        delete mDrillReader;
    mDrillReader = 0;
}

void PCBConverter::on_pushButton_clicked()
{
    disconnect(ui->firstLayer, SIGNAL(textChanged(QString)), this, SLOT(gp_textChanged(QString)));
    QString fileName = QFileDialog::getOpenFileName(this, tr("Open File"),QFileInfo(ui->firstLayer->text()).canonicalPath(),"*.gtl *.gbr *.gbl (*.gtl *.gbr *.gbl);;*.* (*.*)");
    if(!fileName.isEmpty()) {
        ui->firstLayer->setText(fileName);
        gp_textChanged(fileName);
    }
    connect(ui->firstLayer, SIGNAL(textChanged(QString)), this, SLOT(gp_textChanged(QString)));
}

void PCBConverter::on_pushButton_2_clicked()
{
    QString path = ui->drillPath->text();
    if(path.isEmpty())
        ui->firstLayer->text();
    QString fileName = QFileDialog::getOpenFileName(this, tr("Open File"),QFileInfo(path).canonicalPath(),"*.drl (*.drl);;*.* (*.*)");
    if(!fileName.isEmpty()) {
        ui->drillPath->setText(fileName);
    }
}

void PCBConverter::on_pushButton_4_clicked()
{
    QString path = ui->secondLayer->text();
    if(path.isEmpty())
        ui->firstLayer->text();
    QString fileName = QFileDialog::getOpenFileName(this, tr("Open File"),QFileInfo(path).canonicalPath(),"*.gtl *.gbr *.gbl (*.gtl *.gbr *.gbl);;*.* (*.*)");
    if(!fileName.isEmpty()) {
        ui->secondLayer->setText(fileName);
    }
}

void PCBConverter::gp_textChanged(const QString &arg1)
{
    dataForGenerateChanged();
    bool fexist = QFile::exists(arg1);
    ui->generateButton->setEnabled(fexist);
    if(fexist) {
        QString res;
        QFileInfo fi(arg1);
        QString fileDir = fi.canonicalPath();
        QString baseName = fi.baseName();
        if(baseName.right(5)=="-B_Cu" || baseName.right(5)=="-F_Cu")
            baseName = baseName.left(baseName.length()-5);
        res = fileDir+"/"+baseName+".drl";
        ui->outputFile->setText(fileDir+"/"+baseName+".ngc");
        QString secondLayer = fi.canonicalFilePath();
        secondLayer = secondLayer.replace("-B_Cu.gbl", "-F_Cu.gtl");
        if(secondLayer==fi.canonicalFilePath())
            secondLayer = secondLayer.replace("-F_Cu.gtl", "-B_Cu.gbl");
        if(secondLayer!=fi.canonicalFilePath()) {
            if(QFile::exists(secondLayer))
                ui->secondLayer->setText(secondLayer);
        }
        if(QFile::exists(res)) {
            ui->drillPath->setText(res);
        } else {
            QDir dir(fileDir);
            QStringList filters;
            filters << "*.drl";
            QStringList filesInDir = dir.entryList(filters);
            if(filesInDir.count()) {
                int findNumMax = -1;
                foreach(QString findName, filesInDir) {
                    int tmp = 0;
                    while(tmp<findName.length() && tmp<baseName.length()) {
                        if(findName[tmp]==baseName[tmp])
                            tmp++;
                        else
                            break;
                    }
                    if(tmp>findNumMax) {
                        res = fileDir+"/"+findName;
                    }
                }
                if(QFile::exists(res))
                    ui->drillPath->setText(res);
            }
        }
    }
}

void PCBConverter::on_exitButton_clicked()
{
    close();
}

void PCBConverter::closeEvent(QCloseEvent * /*event*/)
{
    mGerberView.close();
    Settings::save(this);
}

void PCBConverter::on_generateButton_clicked()
{
    ui->statusLabel->setText("Generating...");
    update();
    deleteGeneratedObjects();
    QString gerberFile1 = ui->firstLayer->text();
    QString gerberFile2 = ui->secondLayer->text();
#ifdef Q_OS_WIN32
    gerberFile1 = gerberFile1.replace("/", "\\");
    gerberFile2 = gerberFile2.replace("/", "\\");
#endif
    Mirror mirror1 = M_NO;
    if(ui->firstMirror->currentIndex() == 1) mirror1 = M_VERTICAL;
    if(ui->firstMirror->currentIndex() == 2) mirror1 = M_HORIZONTAL;
    if(ui->firstMirror->currentIndex() == 3) mirror1 = M_BOTH;
    Mirror mirror2 = M_NO;
    if(ui->secondMirror->currentIndex() == 1) mirror2 = M_VERTICAL;
    if(ui->secondMirror->currentIndex() == 2) mirror2 = M_HORIZONTAL;
    if(ui->secondMirror->currentIndex() == 3) mirror2 = M_BOTH;
    Mirror dmirror = (Mirror)((int)mirror1 ^ (int)mirror2);
    mGerberReader = new GerberReader(gerberFile1.toLocal8Bit().data(), gerberFile2.toLocal8Bit().data(), ui->boardOffset->value(), mirror1, mirror2);
    if(mGerberReader->firstLayerPath()) {
        if(mGerberReader->getWarnings())
            QMessageBox::warning(this, "Gerber warning", tr(mGerberReader->getWarnings()));
        if(ui->methodComboBox->currentIndex()==0)
            mPathContur1 = new PCBPathContur(mGerberReader->firstLayerPath(), mGerberReader->getMax().x,  mGerberReader->getMax().y, ui->millingDiameter->value(), ui->conturTimes->value());
        else
            mPathContur1 = new PrintingPathContur(mGerberReader->firstLayerPath(), mGerberReader->getMax().x,  mGerberReader->getMax().y, ui->nozzleDiameter->value());
        if(ui->methodComboBox->currentIndex()==0)
            mPathContur2 = new PCBPathContur(mGerberReader->secondLayerPath(), mGerberReader->getMax().x,  mGerberReader->getMax().y, ui->millingDiameter->value(), ui->conturTimes->value());
        else
            mPathContur2 = new PrintingPathContur(mGerberReader->secondLayerPath(), mGerberReader->getMax().x,  mGerberReader->getMax().y, ui->nozzleDiameter->value());
        if(mPathContur1->first()) {
            if(mPathContur1->getWarnings())
                QMessageBox::warning(this, "Contur generator warning", tr(mPathContur1->getWarnings()));
            if(mPathContur2->getWarnings())
                QMessageBox::warning(this, "Contur generator warning", tr(mPathContur1->getWarnings()));

            if(!ui->drillPath->text().isEmpty()) {
                if(QFile(ui->drillPath->text()).exists()) {
                    mDrillReader = new DrillReader(ui->drillPath->text().toLocal8Bit().data(), mGerberReader->getMin(), mGerberReader->getMax(), mirror1);
                }
            }

            ui->statusLabel->setText("");
            mGerberView.set(mGerberReader, mPathContur1, mPathContur2, mDrillReader, dmirror);
            if(!ui->drillPath->text().isEmpty()) {
                if(mDrillReader==0)
                    ui->statusLabel->setText("Error read drill file. ");
                else if(mDrillReader->firstDrill()==0) {
                    ui->statusLabel->setText("Empty on invalid drill file. ");
                }
            }
            if(!gerberFile2.isEmpty() && mGerberReader->secondLayerPath()==0)
                ui->statusLabel->setText(ui->statusLabel->text()+"Error read second layer.");

            ui->previewButton->setEnabled(true);
            on_previewButton_clicked();
            on_outputFile_textChanged(ui->outputFile->text());
            return;
        } else
            ui->statusLabel->setText("Error while generate contur");
    } else
        ui->statusLabel->setText("Error while reading gerber file");
    ui->previewButton->setEnabled(false);
    on_outputFile_textChanged(ui->outputFile->text());
}

void PCBConverter::on_previewButton_clicked()
{
    mGerberView.show();
    mGerberView.activateWindow();
    mGerberView.raise();
}

void PCBConverter::on_pushButton_3_clicked()
{
    QString fileName = QFileDialog::getSaveFileName(this, tr("Save File"),ui->outputFile->text(),"*.ngc (*.ngc);;*.* (*.*)");
    if(!fileName.isEmpty()) {
        ui->outputFile->setText(fileName);
    }
}

void PCBConverter::on_outputFile_textChanged(const QString &arg1)
{
    if(mPathContur1)
        ui->writeToFileButton->setEnabled(!arg1.isEmpty() && mPathContur1->first()!=0);
    else
        ui->writeToFileButton->setEnabled(false);
    ui->startMilling->setEnabled(!exportedFileName.isEmpty() && exportedFileName==arg1);
}

void PCBConverter::on_writeToFileButton_clicked()
{
    QString outputFile = ui->outputFile->text();
    if(mPathContur1==0 || outputFile.isEmpty())
        return;
    ui->statusLabel->setText("Exporting...");
    update();
#ifdef Q_OS_WIN32
    outputFile = outputFile.replace("/", "\\");
#endif
    GCodeExport::Mode mode = GCodeExport::Milling;
    if(ui->methodComboBox->currentIndex()==1) mode = GCodeExport::Printing;
    if(ui->methodComboBox->currentIndex()==2) mode = GCodeExport::Extrudering;
    if(ui->methodComboBox->currentIndex()==3) mode = GCodeExport::Lasering;
    GCodeExport exporter(outputFile.toLocal8Bit().data(), mPathContur1, mPathContur2, \
                                 mode, ui->millingDepth->value(), ui->edgeMillingDepth->value(), \
                                 ui->zSafe->value(), ui->millingSpeed->value(), ui->edgeMillingSpeed->value(), \
                                 ui->spindleRpm->value(), ui->millingDiameter->value(), \
                                 mDrillReader, ui->drillingSpeed->value(), ui->drillingLiftSpeed->value(), \
                                 ui->drillingSpindleSpeed->value(), ui->drillDepth->value(), ui->drillZSafe->value(), \
                                 ui->driilThSame->checkState()==Qt::Checked, ui->useProbe->checkState()==Qt::Checked, \
                                 ui->filamentDiameter->value(), ui->nozzleDiameter->value(), \
                                 ui->extruderTemperature->value(), ui->printingSpeed->value(), ui->cutEdges->checkState()==Qt::Checked, \
                                 ui->energy->value(), ui->laserXoffset->value(), ui->laserYoffset->value());
    if(exporter.time()>=0.0L){
        exportedFileName =  ui->outputFile->text();
        ui->startMilling->setEnabled(true);
        char timeBuffer[255];
        exporter.time(timeBuffer);
        QString filament = "";
        if(ui->methodComboBox->currentIndex()==2) {
            long mm = exporter.filament()+0.5L;
            filament = "\nFilament used: "+QString::number(mm)+" mm.";
        }
        ui->statusLabel->setText("Export done.\nApproximate time "+QString(timeBuffer)+filament);
    } else {
        ui->statusLabel->setText("Error while export");
    }
}

void PCBConverter::on_startMilling_clicked()
{
    QString gcodecnc = "gcodecnc";
    QString outputFile = exportedFileName;
#ifdef Q_OS_WIN32
    outputFile = outputFile.replace("/", "\\");
    gcodecnc +=".exe";
#endif
    QStringList arg;
    arg << outputFile;
    if(QProcess::startDetached(gcodecnc, arg))
        ui->statusLabel->setText("");
    else
        ui->statusLabel->setText(gcodecnc+" not found");
}

void PCBConverter::dataForGenerateChanged()
{
    ui->previewButton->setEnabled(false);
    ui->writeToFileButton->setEnabled(false);
    ui->startMilling->setEnabled(false);
}

void PCBConverter::setLayoutVisible(QLayout *layout, bool visible)
{
    if(layout) {
        QWidget *w = ui->formLayout->labelForField(layout);
        if(w) {
            w->setVisible(visible);
        }
        for(int i=0; i<layout->count(); i++)
            if(layout->itemAt(i)->widget())
                layout->itemAt(i)->widget()->setVisible(visible);
            else if (layout->itemAt(i)->spacerItem()) {
                if(visible)
                    layout->itemAt(i)->spacerItem()->changeSize(1,1, QSizePolicy::Expanding, QSizePolicy::Fixed);
                else
                    layout->itemAt(i)->spacerItem()->changeSize(0,0, QSizePolicy::Fixed, QSizePolicy::Fixed);
            }
        layout->setContentsMargins(0,0,0,visible?6:0);
    }
}

void PCBConverter::on_methodComboBox_currentIndexChanged(int index)
{
    bool isPCBVisible = index == 0;
    setLayoutVisible(ui->horizontalLayout_6, isPCBVisible);
    setLayoutVisible(ui->horizontalLayout_7, isPCBVisible);
    setLayoutVisible(ui->horizontalLayout_8, isPCBVisible);

    setLayoutVisible(ui->horizontalLayout_25, !isPCBVisible && index==2);
    setLayoutVisible(ui->horizontalLayout_26, !isPCBVisible && index==2);
    setLayoutVisible(ui->horizontalLayout_27, !isPCBVisible);
    setLayoutVisible(ui->horizontalLayout_28, !isPCBVisible);
    setLayoutVisible(ui->horizontalLayout_30, index==3);
    setLayoutVisible(ui->horizontalLayout_31, !isPCBVisible);
    setLayoutVisible(ui->horizontalLayout_32, !isPCBVisible);
}

void PCBConverter::on_cutEdges_stateChanged(int state)
{
    mGerberView.setEdgesVisible(state==Qt::Checked);
}

