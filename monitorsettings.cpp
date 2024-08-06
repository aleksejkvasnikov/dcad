#include "monitorsettings.h"
#include "ui_monitorsettings.h"

MonitorSettings::MonitorSettings(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MonitorSettings)
{
    ui->setupUi(this);

	setFixedSize(420, 280);

	QDoubleValidator *doubleValidator = new QDoubleValidator(this);
	QLocale locale(QLocale::English); // Устанавливаем локаль, в которой используется точка как десятичный разделитель
	locale.setNumberOptions(QLocale::RejectGroupSeparator); // Дополнительно, чтобы избежать проблем с разделителями групп
	doubleValidator->setLocale(locale);

	ui->freqEdit->setValidator(doubleValidator);
	ui->farFieldRadEdit->setValidator(doubleValidator);
	ui->xMaxEdit->setValidator(doubleValidator);
	ui->xMinEdit->setValidator(doubleValidator);
	ui->yMaxEdit->setValidator(doubleValidator);
	ui->yMinEdit->setValidator(doubleValidator);
	ui->zMaxEdit->setValidator(doubleValidator);
	ui->zMinEdit->setValidator(doubleValidator);

	ui->xMaxEdit->setCursorPosition(0);
	ui->xMaxEdit->setCursorPosition(0);
	ui->xMinEdit->setCursorPosition(0);
	ui->yMaxEdit->setCursorPosition(0);
	ui->yMinEdit->setCursorPosition(0);
	ui->zMaxEdit->setCursorPosition(0);
	ui->zMinEdit->setCursorPosition(0);
}

MonitorSettings::~MonitorSettings()
{
    delete ui;
}

void MonitorSettings::initializeField()
{
	// заполнение имени монитора (берем переменную monitors count из projectData, которая получается с помощью парсинга файла мониторов)
	// имя = farfield + номер + [частота расчетной области]
	// если новый - заполняем стандартно, если существует - то заполняем данными из проекта (из вектора монитора)
	if(newObject){
		ui->nameEdit->setEnabled(false);
		ui->nameEdit->setText("farfield" + QString::number(*currentMonIndex) + "[" +
			QString::number(cdData->freqValue) + "]");

		ui->freqEdit->setText(QString::number(cdData->freqValue));
		ui->freqLabel->setText(projectData->frequencyUnits);
		ui->farFieldRadio->setChecked(true);
		ui->farFieldRadEdit->setText(QString::number(1));

		// заполнение x Min x Max и тд
		// исправить
		ui->xMinEdit->setEnabled(false);
		ui->yMinEdit->setEnabled(false);
		ui->zMinEdit->setEnabled(false);
		ui->xMaxEdit->setEnabled(false);
		ui->yMaxEdit->setEnabled(false);
		ui->zMaxEdit->setEnabled(false);


		ui->xMinUnit->setText(projectData->geometryUnits);
		ui->yMinUnit->setText(projectData->geometryUnits);
		ui->zMinUnit->setText(projectData->geometryUnits);
		ui->xMaxUnit->setText(projectData->geometryUnits);
		ui->yMaxUnit->setText(projectData->geometryUnits);
		ui->zMaxUnit->setText(projectData->geometryUnits);

		double c = 299792458;
		double freqMultiplier = 1;

		if (projectData->frequencyUnits == "КГц") freqMultiplier = 10e3;
		else if (projectData->frequencyUnits == "МГц") freqMultiplier = 10e6;
		else if (projectData->frequencyUnits == "ГГц") freqMultiplier = 10e9;
		double freq = cdData->freqValue * freqMultiplier;

		if (!projectData->hasModel) {
			if (cdData->unitsType == 0) { // доля от lambda	
				ui->xMinEdit->setText(QString::number(0 - (c / freq) / cdData->xMin));
				ui->xMaxEdit->setText(QString::number(0 + (c / freq) / cdData->xMax));

				ui->yMinEdit->setText(QString::number(0 - (c / freq) / cdData->yMin));
				ui->yMaxEdit->setText(QString::number(0 + (c / freq) / cdData->yMax));

				ui->zMinEdit->setText(QString::number(0 - (c / freq) / cdData->zMin));
				ui->zMaxEdit->setText(QString::number(0 + (c / freq) / cdData->zMax));

			}
			else {
				ui->xMinEdit->setText(QString::number(0 - cdData->xMin));
				ui->yMinEdit->setText(QString::number(0 - cdData->yMin));
				ui->zMinEdit->setText(QString::number(0 - cdData->zMin));
				ui->xMaxEdit->setText(QString::number(0 + cdData->xMax));
				ui->yMaxEdit->setText(QString::number(0 + cdData->yMax));
				ui->zMaxEdit->setText(QString::number(0 + cdData->zMax));
			}
		}
		else {
			//границы модели!
		}
	}
	else {
		ui->nameEdit->setText(projectData->monitors[*currentMonIndex].name);

		ui->autoNameCheckBox->setChecked((bool)projectData->monitors[*currentMonIndex].nameBox);
		if (ui->autoNameCheckBox->isChecked()) 
			ui->nameEdit->setEnabled(false);

		ui->farFieldRadio->setChecked((bool)projectData->monitors[*currentMonIndex].type);
		ui->nearFieldRadio->setChecked(!(bool)projectData->monitors[*currentMonIndex].type);
		ui->freqEdit->setText(QString::number(projectData->monitors[*currentMonIndex].freqValue));

		if (ui->farFieldRadio->isChecked()) ui->farFieldRadEdit->setText(QString::number(projectData->monitors[*currentMonIndex].farFieldRadius));
		if (ui->nearFieldRadio->isChecked()) {
			ui->farFieldRadEdit->setEnabled(false);
		}

		ui->autoCheckBox->setChecked((bool)projectData->monitors[*currentMonIndex].placeBox);
		ui->xMinEdit->setText(QString::number(projectData->monitors[*currentMonIndex].xMin));
		ui->yMinEdit->setText(QString::number(projectData->monitors[*currentMonIndex].yMin));
		ui->zMinEdit->setText(QString::number(projectData->monitors[*currentMonIndex].zMin));
		ui->xMaxEdit->setText(QString::number(projectData->monitors[*currentMonIndex].xMax));
		ui->yMaxEdit->setText(QString::number(projectData->monitors[*currentMonIndex].yMax));
		ui->zMaxEdit->setText(QString::number(projectData->monitors[*currentMonIndex].zMax));
	}
	ui->xMaxEdit->setCursorPosition(0);
	ui->xMaxEdit->setCursorPosition(0);
	ui->xMinEdit->setCursorPosition(0);
	ui->yMaxEdit->setCursorPosition(0);
	ui->yMinEdit->setCursorPosition(0);
	ui->zMaxEdit->setCursorPosition(0);
	ui->zMinEdit->setCursorPosition(0);
}

void MonitorSettings::on_farFieldRadio_clicked()
{
	if (ui->farFieldRadio->isChecked()) {
		ui->farFieldRadEdit->setEnabled(true);
		QString name = ui->farFieldRadio->isChecked() ? "farfield" : "nearfield";
		ui->nameEdit->setText(name + QString::number(*currentMonIndex) + "[" +
			QString::number(cdData->freqValue) + "]");
	}
}

void MonitorSettings::on_nearFieldRadio_clicked()
{
	if (ui->nearFieldRadio->isChecked()) {
		ui->farFieldRadEdit->setEnabled(false);
		QString name = ui->farFieldRadio->isChecked() ? "farfield" : "nearfield";
		ui->nameEdit->setText(name + QString::number(*currentMonIndex) + "[" +
			QString::number(cdData->freqValue) + "]");
	}
}

void MonitorSettings::on_cancelButton_clicked()
{
	close();
}

void MonitorSettings::on_okButton_clicked()
{
	// TODO: 1. проверки и защиты
	//		 2. проверка корректности имени монитора

	// если объект новый - он добавляется в вектор мониторов
	// если объект существует - правятся поля через указатель
	if (newObject) {
		MonitorData newMonitor;
		newMonitor.name = ui->nameEdit->text();
		newMonitor.nameBox = ui->autoNameCheckBox->isChecked();
		newMonitor.type = ui->farFieldRadio->isChecked();
		newMonitor.freqValue = ui->freqEdit->text().toDouble();
		if (ui->farFieldRadio->isChecked()) newMonitor.farFieldRadius = ui->farFieldRadEdit->text().toDouble();
		newMonitor.placeBox = ui->autoCheckBox->isChecked();
		newMonitor.xMin = ui->xMinEdit->text().toDouble();
		newMonitor.yMin = ui->yMinEdit->text().toDouble();
		newMonitor.zMin = ui->zMinEdit->text().toDouble();
		newMonitor.xMax = ui->xMaxEdit->text().toDouble();
		newMonitor.yMax = ui->yMaxEdit->text().toDouble();
		newMonitor.zMax = ui->zMaxEdit->text().toDouble();
		newMonitor.id = *currentMonIndex;
		newMonitor.hasUnsavedChanges = true;
		projectData->monitors.push_back(newMonitor);
		*currentMonIndex = *currentMonIndex+1;
	}
	else {
		projectData->monitors[*currentMonIndex].name = ui->nameEdit->text();
		projectData->monitors[*currentMonIndex].nameBox = ui->autoNameCheckBox->isChecked();
		projectData->monitors[*currentMonIndex].type = ui->farFieldRadio->isChecked();
		projectData->monitors[*currentMonIndex].freqValue = ui->freqEdit->text().toDouble();
		if (ui->farFieldRadio->isChecked()) projectData->monitors[*currentMonIndex].farFieldRadius = ui->farFieldRadEdit->text().toDouble();
		projectData->monitors[*currentMonIndex].placeBox = ui->autoCheckBox->isChecked();
		projectData->monitors[*currentMonIndex].xMin = ui->xMinEdit->text().toDouble();
		projectData->monitors[*currentMonIndex].yMin = ui->yMinEdit->text().toDouble();
		projectData->monitors[*currentMonIndex].zMin = ui->zMinEdit->text().toDouble();
		projectData->monitors[*currentMonIndex].xMax = ui->xMaxEdit->text().toDouble();
		projectData->monitors[*currentMonIndex].yMax = ui->yMaxEdit->text().toDouble();
		projectData->monitors[*currentMonIndex].zMax = ui->zMaxEdit->text().toDouble();
		projectData->monitors[*currentMonIndex].id = *currentMonIndex;
		projectData->monitors[*currentMonIndex].hasUnsavedChanges = true;
	}
	emit monitorSettingsWasChanged();
	close();
}

void MonitorSettings::on_autoNameCheckBox_stateChanged(int arg1)
{
	QString name = ui->farFieldRadio->isChecked() ? "farfield" : "nearfield";

	// если активно, то поле для заполнения имен не редактируется
	if (ui->autoNameCheckBox->isChecked()) {
		ui->nameEdit->setEnabled(false);
		ui->nameEdit->setText(name + QString::number(*currentMonIndex) + "[" +
			QString::number(cdData->freqValue) + "]");

	} else
		ui->nameEdit->setEnabled(true);
}

void MonitorSettings::on_autoCheckBox_stateChanged(int arg1)
{
	double c = 299792458;
	double freqMultiplier = 1;

	if (projectData->frequencyUnits == "КГц") freqMultiplier = 10e3;
	else if (projectData->frequencyUnits == "МГц") freqMultiplier = 10e6;
	else if (projectData->frequencyUnits == "ГГц") freqMultiplier = 10e9;
	double freq = cdData->freqValue * freqMultiplier;

	if (ui->autoCheckBox->isChecked()) {
		ui->xMinEdit->setEnabled(false);
		ui->yMinEdit->setEnabled(false);
		ui->zMinEdit->setEnabled(false);
		ui->xMaxEdit->setEnabled(false);
		ui->yMaxEdit->setEnabled(false);
		ui->zMaxEdit->setEnabled(false);

		if (!projectData->hasModel) { // нет модели
			if (cdData->unitsType == 0) { // доля от lambda	
				ui->xMinEdit->setText(QString::number(0 - (c / freq) / cdData->xMin));
				ui->xMaxEdit->setText(QString::number(0 + (c / freq) / cdData->xMax));
				ui->yMinEdit->setText(QString::number(0 - (c / freq) / cdData->yMin));
				ui->yMaxEdit->setText(QString::number(0 + (c / freq) / cdData->yMax));
				ui->zMinEdit->setText(QString::number(0 - (c / freq) / cdData->zMin));
				ui->zMaxEdit->setText(QString::number(0 + (c / freq) / cdData->zMax));
			}
			else {
				ui->xMinEdit->setText(QString::number(0 - cdData->xMin));
				ui->yMinEdit->setText(QString::number(0 - cdData->yMin));
				ui->zMinEdit->setText(QString::number(0 - cdData->zMin));
				ui->xMaxEdit->setText(QString::number(0 + cdData->xMax));
				ui->yMaxEdit->setText(QString::number(0 + cdData->yMax));
				ui->zMaxEdit->setText(QString::number(0 + cdData->zMax));
			}
		}
		else {
			if (ui->nearFieldRadio->isChecked()) {
				if (cdData->unitsType == 0) { // доля от lambda	
					ui->xMinEdit->setText(QString::number(0 - (c / freq) / cdData->xMin));
					ui->xMaxEdit->setText(QString::number(0 + (c / freq) / cdData->xMax));
					ui->yMinEdit->setText(QString::number(0 - (c / freq) / cdData->yMin));
					ui->yMaxEdit->setText(QString::number(0 + (c / freq) / cdData->yMax));
					ui->zMinEdit->setText(QString::number(0 - (c / freq) / cdData->zMin));
					ui->zMaxEdit->setText(QString::number(0 + (c / freq) / cdData->zMax));
				}
				else {
					ui->xMinEdit->setText(QString::number(0 - cdData->xMin));
					ui->yMinEdit->setText(QString::number(0 - cdData->yMin));
					ui->zMinEdit->setText(QString::number(0 - cdData->zMin));
					ui->xMaxEdit->setText(QString::number(0 + cdData->xMax));
					ui->yMaxEdit->setText(QString::number(0 + cdData->yMax));
					ui->zMaxEdit->setText(QString::number(0 + cdData->zMax));
				}
			} else {
				//границы модели!
			}
		}
	}
	else {
		ui->xMinEdit->setEnabled(true);
		ui->yMinEdit->setEnabled(true);
		ui->zMinEdit->setEnabled(true);
		ui->xMaxEdit->setEnabled(true);
		ui->yMaxEdit->setEnabled(true);
		ui->zMaxEdit->setEnabled(true);
	}
	ui->xMaxEdit->setCursorPosition(0);
	ui->xMaxEdit->setCursorPosition(0);
	ui->xMinEdit->setCursorPosition(0);
	ui->yMaxEdit->setCursorPosition(0);
	ui->yMinEdit->setCursorPosition(0);
	ui->zMaxEdit->setCursorPosition(0);
	ui->zMinEdit->setCursorPosition(0);
	// в режиме авто: для монитора дальнего поля берутся координаты границ объекта,
	// а для источника - границы расчетной области
	// цифры автоматически подставляются в поля
}
