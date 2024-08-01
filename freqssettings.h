#ifndef FREQSSETTINGS_H
#define FREQSSETTINGS_H

#include <QMainWindow>
#include <QStringList>
#include <QDoubleValidator>
#include <QIntValidator>
#include <QMessageBox>
#include "ProjectData.h"
QT_BEGIN_NAMESPACE
namespace Ui { class FreqsSettings; }
QT_END_NAMESPACE

class FreqsSettings : public QMainWindow
{
    Q_OBJECT

public:
    FreqsSettings(QWidget *parent = nullptr);
    ~FreqsSettings();
	void setProjectData(ProjectData* pr) { projectData = pr; }
	void initializeField();
signals:
	void freqsSettingsWasChanged();
private slots:
    void on_cancelButton_clicked();

    void on_okButton_clicked();

private:
	ProjectData* projectData;
    Ui::FreqsSettings *ui;
};
#endif // FREQSSETTINGS_H
