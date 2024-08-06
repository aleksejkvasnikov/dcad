#ifndef MONITORSETTINGS_H
#define MONITORSETTINGS_H

#include <QMainWindow>
#include "ProjectData.h"
#include <QDoubleValidator>
#include <QMessageBox>
QT_BEGIN_NAMESPACE
namespace Ui { class MonitorSettings; }
QT_END_NAMESPACE

class MonitorSettings : public QMainWindow
{
    Q_OBJECT

public:
    MonitorSettings(QWidget *parent = nullptr);
    ~MonitorSettings();
	void setProjectData(ProjectData* pr) { projectData = pr; }
	void setCDData(CDData* cd) { cdData = cd; }
	void initializeField();
	void setNewObject(bool b) { newObject = b; }
	void setCurrentMonIndex(int* i) { currentMonIndex = i; }
signals:
	void monitorSettingsWasChanged();

private slots:
    void on_cancelButton_clicked();

    void on_okButton_clicked();

    void on_autoNameCheckBox_stateChanged(int arg1);

    void on_autoCheckBox_stateChanged(int arg1);

	void on_farFieldRadio_clicked();

	void on_nearFieldRadio_clicked();

private:
	bool newObject = true;
	int* currentMonIndex;
	ProjectData* projectData;
	CDData* cdData;
    Ui::MonitorSettings *ui;
};
#endif // MONITORSETTINGS_H
