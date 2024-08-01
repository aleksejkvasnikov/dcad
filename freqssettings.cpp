#include "freqssettings.h"
#include "ui_freqssettings.h"

FreqsSettings::FreqsSettings(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::FreqsSettings)
{
    ui->setupUi(this);
	// Установка валидаторов для ввода чисел
	QDoubleValidator *doubleValidator = new QDoubleValidator(this);
	QLocale locale(QLocale::English); // Устанавливаем локаль, в которой используется точка как десятичный разделитель
	locale.setNumberOptions(QLocale::RejectGroupSeparator); // Дополнительно, чтобы избежать проблем с разделителями групп
	doubleValidator->setLocale(locale);

	QIntValidator *intValidator = new QIntValidator(this);

	ui->fMinEdit->setValidator(doubleValidator);
	ui->fMaxEdit->setValidator(doubleValidator);

	ui->valueEdit->setValidator(intValidator);

    setFixedSize(300,170);
}

FreqsSettings::~FreqsSettings()
{
    delete ui;
}


void FreqsSettings::on_cancelButton_clicked()
{
	close();
}

void FreqsSettings::on_okButton_clicked()
{
	bool valid = true;
	QString errorMessage;

	QString newFmin = ui->fMinEdit->text();
	QString newFmax = ui->fMaxEdit->text();
	bool newHasPoints = ui->pNumberRadio->isChecked();
	bool newHasStep = ui->fStepRadio->isChecked();

	if ( newFmin.toDouble() < 0 || newFmax.toDouble() < 0 || newFmin.toDouble() >= newFmax.toDouble()) {
		valid = false;
		errorMessage = tr("Пожалуйста, введите корректные значения диапазона частот.");
	}
	if(!valid)
		QMessageBox::warning(this, tr("Ошибка"), errorMessage);
	else
	{
		if (projectData->freqMin != newFmin || projectData->freqMax != newFmax ||
			projectData->useFreqStep != newHasStep || projectData->usePointsNumber != newHasPoints)
		{
			projectData->hasUnsavedChanges = true;
			projectData->freqMin = newFmin;
			projectData->freqMax = newFmax;
			projectData->useFreqStep = newHasStep;
			projectData->usePointsNumber = newHasPoints;
			if (newHasStep) {
				projectData->freqStep = ui->valueEdit->text();
			}
			else {
				projectData->pointsNumber = ui->valueEdit->text();
			}
			emit freqsSettingsWasChanged();		
		}
		close();
	}
}

void FreqsSettings::initializeField()
{
	// Устанавливаем значени¤ полей freqsSettings из структуры projectData
	ui->fMaxEdit->setText(projectData->freqMax);
	ui->fMinEdit->setText(projectData->freqMin);
	ui->fMaxUnitsLabel->setText(projectData->frequencyUnits);
	ui->fMinUnitsLabel->setText(projectData->frequencyUnits);

	if (projectData->useFreqStep) {
		ui->fStepRadio->setChecked(true);
		ui->valueEdit->setText(projectData->freqStep);
	}
	else if (projectData->usePointsNumber) {
		ui->pNumberRadio->setChecked(true);
		ui->valueEdit->setText(projectData->pointsNumber);
	}
	else {
		ui->fStepRadio->setChecked(false);
		ui->pNumberRadio->setChecked(false);
		ui->valueEdit->clear();
	}
}
