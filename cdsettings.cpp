#include "cdsettings.h"
#include "ui_cdsettings.h"

CDSettings::CDSettings(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::CDSettings)
{
    ui->setupUi(this);

    setFixedSize(417,250);

	QDoubleValidator *doubleValidator = new QDoubleValidator(this);
	QLocale locale(QLocale::English); // Устанавливаем локаль, в которой используется точка как десятичный разделитель
	locale.setNumberOptions(QLocale::RejectGroupSeparator); // Дополнительно, чтобы избежать проблем с разделителями групп
	doubleValidator->setLocale(locale);

	ui->freqsLineEdit->setValidator(doubleValidator);
	ui->xMaxEdit->setValidator(doubleValidator);
	ui->xMinEdit->setValidator(doubleValidator);
	ui->yMaxEdit->setValidator(doubleValidator);
	ui->yMinEdit->setValidator(doubleValidator);
	ui->zMaxEdit->setValidator(doubleValidator);
	ui->zMinEdit->setValidator(doubleValidator);

}

CDSettings::~CDSettings()
{
    delete ui;
}

void CDSettings::initializeField()
{
	QString midFreq = QString::number(projectData->freqMin.toDouble() + (projectData->freqMax.toDouble() - projectData->freqMin.toDouble())/2); 
	//cdData->freqValue - вдруг настройки частоты уже изменили
	ui->freqsLineEdit->setText(midFreq);

	QString equalDirectionValue = ui->xMinEdit->text();
	ui->xMinEdit->setText(QString::number(cdData->xMin));
	ui->yMinEdit->setText(QString::number(cdData->yMin));
	ui->zMinEdit->setText(QString::number(cdData->zMin));
	ui->xMaxEdit->setText(QString::number(cdData->xMax));
	ui->yMaxEdit->setText(QString::number(cdData->yMax));
	ui->zMaxEdit->setText(QString::number(cdData->zMax));

	ui->freqsCombo->setCurrentIndex(cdData->freqType);
	if (cdData->freqType == 0) {
		ui->freqsLineEdit->setEnabled(false);
	}
	else {
		ui->freqsLineEdit->setEnabled(true);
		ui->freqsLineEdit->setText(QString::number(cdData->freqValue));
	}
	ui->unitsTypeCombo->setCurrentIndex(cdData->unitsType);
	if (cdData->unitsType == 0) {
		ui->sizeUnitsLabel->setText("");
		ui->freqsCombo->setEnabled(true);
		if (ui->freqsCombo->currentIndex() == 1)
			ui->freqsLineEdit->setEnabled(true);
	}
	else if (cdData->unitsType == 1) {
		ui->sizeUnitsLabel->setText(projectData->geometryUnits);
		ui->freqsCombo->setEnabled(false);
		ui->freqsLineEdit->setEnabled(false);
	}
	ui->allDirectionsBox->setChecked((bool)cdData->allDirections);
	ui->yMinEdit->setEnabled((bool)!cdData->allDirections);
	ui->zMinEdit->setEnabled((bool)!cdData->allDirections);
	ui->xMaxEdit->setEnabled((bool)!cdData->allDirections);
	ui->yMaxEdit->setEnabled((bool)!cdData->allDirections);
	ui->zMaxEdit->setEnabled((bool)!cdData->allDirections);
}


void CDSettings::on_allDirectionsBox_stateChanged(int arg1)
{
    // Проверяем, установлена ли галочка в QCheckBox
    bool isChecked = (arg1 == Qt::Checked);

    // Устанавливаем состояние для QLineEdit в зависимости от состояния QCheckBox
    ui->yMinEdit->setEnabled(!isChecked);
    ui->zMinEdit->setEnabled(!isChecked);
    ui->xMaxEdit->setEnabled(!isChecked);
    ui->yMaxEdit->setEnabled(!isChecked);
    ui->zMaxEdit->setEnabled(!isChecked);

	if (isChecked) {
		QString equalDirectionValue = ui->xMinEdit->text();
		ui->yMinEdit->setText(equalDirectionValue);
		ui->zMinEdit->setText(equalDirectionValue);
		ui->xMaxEdit->setText(equalDirectionValue);
		ui->yMaxEdit->setText(equalDirectionValue);
		ui->zMaxEdit->setText(equalDirectionValue);
	}
}
void CDSettings::on_okButton_clicked() {
	bool valid = true;
	QString errorMessage;

	QString freq = ui->freqsLineEdit->text();

	if (freq.toDouble() < projectData->freqMin.toDouble() || freq.toDouble() > projectData->freqMax.toDouble()) {
		valid = false;
		errorMessage = tr("Пожалуйста, введите корректное значение частоты.");
	}
	if (!valid)
		QMessageBox::warning(this, tr("Ошибка"), errorMessage);
	else {
		if (cdData->freqValue != freq.toDouble() || cdData->unitsType != ui->unitsTypeCombo->currentIndex() ||
			cdData->allDirections != ui->allDirectionsBox->isChecked() ||
			cdData->xMin != ui->xMinEdit->text().toDouble() || cdData->xMax != ui->xMaxEdit->text().toDouble() ||
			cdData->yMin != ui->yMinEdit->text().toDouble() || cdData->yMax != ui->yMaxEdit->text().toDouble() || 
			cdData->zMin != ui->zMinEdit->text().toDouble() || cdData->zMax != ui->zMaxEdit->text().toDouble())
		{
			cdData->hasUnsavedChanges = true;
			cdData->freqValue = freq.toDouble();
			cdData->unitsType = ui->unitsTypeCombo->currentIndex();
			cdData->freqType = ui->freqsCombo->currentIndex();
			cdData->allDirections = ui->allDirectionsBox->isChecked();
			cdData->xMin = ui->xMinEdit->text().toDouble();
			cdData->yMin = ui->yMinEdit->text().toDouble();
			cdData->zMin = ui->zMinEdit->text().toDouble();
			cdData->xMax = ui->xMaxEdit->text().toDouble();
			cdData->yMax = ui->yMaxEdit->text().toDouble();
			cdData->zMax = ui->zMaxEdit->text().toDouble();

			emit cdSettingsWasChanged();
		}
		close();
	}
}
void CDSettings::on_cancelButton_clicked() {
	close();
}
void CDSettings::on_freqsCombo_currentIndexChanged(int index)
{
	if (index == 0) {
		QString midFreq = QString::number(projectData->freqMin.toDouble() + (projectData->freqMax.toDouble() - projectData->freqMin.toDouble()) / 2.0);
		ui->freqsLineEdit->setText(midFreq);
		ui->freqsLineEdit->setEnabled(false);
	}
	else if (index == 1) {
		ui->freqsLineEdit->clear();
		ui->freqsLineEdit->setEnabled(true);
	}
}
void CDSettings::on_xMinEdit_textChanged(const QString &arg1) {
	if (ui->allDirectionsBox->isChecked()) {
		ui->yMinEdit->setText(arg1);
		ui->zMinEdit->setText(arg1);
		ui->xMaxEdit->setText(arg1);
		ui->yMaxEdit->setText(arg1);
		ui->zMaxEdit->setText(arg1);
	}
}
void CDSettings::on_unitsTypeCombo_currentIndexChanged(int index)
{
	if (index == 0) {
		//ui->size
		ui->sizeUnitsLabel->setText("");
		ui->freqsCombo->setEnabled(true);
		if(ui->freqsCombo->currentIndex()==1)
			ui->freqsLineEdit->setEnabled(true);
	}
	else if (index == 1) {
		ui->sizeUnitsLabel->setText(projectData->geometryUnits);
		ui->freqsCombo->setEnabled(false);
		ui->freqsLineEdit->setEnabled(false);
	}
}
