#ifndef UNITSSETTINGS_H
#define UNITSSETTINGS_H

#include <QMainWindow>
#include "ProjectData.h"
QT_BEGIN_NAMESPACE
namespace Ui { class unitssettings; }
QT_END_NAMESPACE

class unitssettings : public QMainWindow
{
    Q_OBJECT

public:
    unitssettings(QWidget *parent = nullptr);
    ~unitssettings();
	void setProjectData(ProjectData* pr) { projectData = pr; }
	void initializeField();
signals:
	void unitsSettingsWasChanged();
private slots:
    void on_okButton_clicked();

    void on_cancelButton_clicked();

private:
	ProjectData* projectData;
    Ui::unitssettings *ui;
};
#endif // UNITSSETTINGS_H
