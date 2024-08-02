#include "unitssettings.h"
#include "ui_unitssettings.h"

unitssettings::unitssettings(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::unitssettings)
{
    ui->setupUi(this);
    setFixedSize(180,145);
}

unitssettings::~unitssettings()
{
    delete ui;
}

void unitssettings::initializeField()
{
	ui->dimsCombo->setCurrentText(projectData->geometryUnits);
	ui->freqsCombo->setCurrentText(projectData->frequencyUnits);
	ui->timeCombo->setCurrentText(projectData->timeUnits);
}


void unitssettings::on_okButton_clicked()
{
	QString dims = ui->dimsCombo->currentText();
	QString freqs = ui->freqsCombo->currentText();
	QString time = ui->timeCombo->currentText();

	if (projectData->geometryUnits != dims || projectData->frequencyUnits != freqs || projectData->timeUnits != time)
	{
		projectData->hasUnsavedChanges = true;
		projectData->geometryUnits = dims;
		projectData->frequencyUnits = freqs;
		projectData->timeUnits = time;
		emit unitsSettingsWasChanged();
	}
	close();
}

void unitssettings::on_cancelButton_clicked()
{
	close();
}
