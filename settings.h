#ifndef SETTINGS_H
#define SETTINGS_H

class QWidget;

class Settings
{
public:
    static void save(QWidget *mainWidget);
    static void load(QWidget *mainWidget);
private:
    Settings();
};

#endif // SETTINGS_H
