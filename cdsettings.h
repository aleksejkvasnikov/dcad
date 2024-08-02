#ifndef CDSETTINGS_H
#define CDSETTINGS_H

#include <QMainWindow>
#include "ProjectData.h"
#include <QDoubleValidator>
#include <QMessageBox>
QT_BEGIN_NAMESPACE
namespace Ui { class CDSettings; }
QT_END_NAMESPACE

class CDSettings : public QMainWindow
{
    Q_OBJECT

public:
    CDSettings(QWidget *parent = nullptr);
    ~CDSettings();
	void setProjectData(ProjectData* pr) { projectData = pr; }
	void setCDData(CDData* cd) { cdData = cd; }
	void initializeField();
signals:
	void cdSettingsWasChanged();
private slots:
	void on_xMinEdit_textChanged(const QString &arg1);

    void on_allDirectionsBox_stateChanged(int arg1);

    void on_freqsCombo_currentIndexChanged(int index);

    void on_unitsTypeCombo_currentIndexChanged(int index);

	void on_okButton_clicked();

	void on_cancelButton_clicked();

private:
	ProjectData* projectData;
	CDData* cdData;
    Ui::CDSettings *ui;
};
#endif // CDSETTINGS_H
