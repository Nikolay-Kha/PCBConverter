#include "settings.h"
#include <QWidget>
#include <QSettings>
#include <QXmlStreamReader>
#include <QApplication>
#include <QLineEdit>
#include <QComboBox>
#include <QSpinBox>
#include <QCheckBox>

bool readXmlFile( QIODevice& device, QSettings::SettingsMap& map )
{
    QXmlStreamReader xmlReader( &device );

    QString currentElementName;
    while( !xmlReader.atEnd() )
    {
        xmlReader.readNext();
        while( xmlReader.isStartElement() )
        {
            if( xmlReader.name() == "SettingsMap" )
            {
                                xmlReader.readNext();
                continue;
            }

            if( !currentElementName.isEmpty() )
            {
                currentElementName += "/";
            }
            currentElementName += xmlReader.name().toString();
            xmlReader.readNext();
        }

        if( xmlReader.isEndElement() )
        {
            continue;
        }

        if( xmlReader.isCharacters() && !xmlReader.isWhitespace() )
        {
            QString key = currentElementName;
            QString value = xmlReader.text().toString();

            map[ key ] = value;

            currentElementName.clear();
        }
    }

     if( xmlReader.hasError() )
     {
        return false;
     }

    return true;
}

bool writeXmlFile( QIODevice& device, const QSettings::SettingsMap& map )
{
    QXmlStreamWriter xmlWriter( &device );
    xmlWriter.setAutoFormatting( true );

    xmlWriter.writeStartDocument();
        xmlWriter.writeStartElement( "SettingsMap" );

    QSettings::SettingsMap::const_iterator mi = map.begin();
    for( ; mi != map.end(); ++mi )
    {
        QStringList groups = mi.key().split("/");
        foreach( QString groupName, groups )
        {
            xmlWriter.writeStartElement( groupName );
        }

        xmlWriter.writeCharacters( mi.value().toString() );

        foreach( QString groupName, groups )
        {
            xmlWriter.writeEndElement();
        }
    }

        xmlWriter.writeEndElement();
    xmlWriter.writeEndDocument();

    return true;
}

QSettings *getSettings()
{
    const QSettings::Format xmlFormat = QSettings::registerFormat("xml", readXmlFile, writeXmlFile);
    QString path = QApplication::applicationFilePath();
    if(path.right(4)==".exe")
        path = path.left(path.length()-4);
    path += ".xml";
    return new QSettings(path, xmlFormat);
}

Settings::Settings()
{

}

void saveWidget(QWidget *widget, QSettings *settings)
{
    QObjectList list = widget->children();
    foreach(QObject *obj, list) {
        QWidget *w = qobject_cast<QWidget *>(obj);
        if(w)
            saveWidget(w, settings);
    }


    QLineEdit *edit = qobject_cast<QLineEdit *>(widget);
    if(edit) {
        if(qobject_cast<QAbstractSpinBox*>(edit->parent()))
            return;
        if(!edit->text().isEmpty())
            settings->setValue(widget->objectName(), edit->text());
        else
            settings->remove(widget->objectName());
    } else {
        QComboBox *combo = qobject_cast<QComboBox *>(widget);
        if(combo) {
            settings->setValue(widget->objectName(), combo->currentIndex());
        } else {
            QSpinBox *spin = qobject_cast<QSpinBox *>(widget);
            if(spin) {
                settings->setValue(widget->objectName(), spin->value());
            } else {
                QDoubleSpinBox *dspin = qobject_cast<QDoubleSpinBox *>(widget);
                if(dspin) {
                    settings->setValue(widget->objectName(), dspin->value());
                } else {
                    QCheckBox *box = qobject_cast<QCheckBox*>(widget);
                    if(box) {
                        settings->setValue(widget->objectName(), box->checkState());
                    }
                }
            }
        }
    }

}

void Settings::save(QWidget *mainWidget)
{
    QSettings *settings = getSettings();

    saveWidget(mainWidget, settings);

    settings->deleteLater();
}

QWidget *findWidget(QWidget *wdg, const QString &name)
{
    if(wdg->objectName()==name)
        return wdg;
    QObjectList list = wdg->children();
    foreach(QObject *obj, list) {
        QWidget *w = qobject_cast<QWidget *>(obj);
        if(w) {
            w = findWidget(w, name);
            if(w)
                return w;
        }

    }
    return NULL;
}

void restoreValue(QWidget *widget, QVariant value)
{
    bool ok;
    qreal f = value.toReal(&ok);
    if(ok) {
        QComboBox *combo = qobject_cast<QComboBox *>(widget);
        if(combo) {
            combo->setCurrentIndex(qRound(f));
            return;
        } else {
            QSpinBox *spin = qobject_cast<QSpinBox *>(widget);
            if(spin) {
                spin->setValue(qRound(f));
                return;
            } else {
                QDoubleSpinBox *dspin = qobject_cast<QDoubleSpinBox *>(widget);
                if(dspin) {
                    dspin->setValue(f);
                    return;
                } else {
                    QCheckBox *box = qobject_cast<QCheckBox*>(widget);
                    if(box) {
                        box->setCheckState((Qt::CheckState)qRound(f));
                        return;
                    }
                }
            }
        }
    }
    QString str = value.toString();
    if(!str.isEmpty())
    {
        QLineEdit *edit = qobject_cast<QLineEdit *>(widget);
        if(edit) {
            edit->setText(str);
            return;
        }
    }
}


void Settings::load(QWidget *mainWidget)
{
    QSettings *settings = getSettings();
    QStringList list = settings->allKeys();
    foreach(QString key, list){
        QWidget *wdg = findWidget(mainWidget, key);
        if(wdg)
            restoreValue(wdg, settings->value(key,QString("")));
    }

    settings->deleteLater();
}

