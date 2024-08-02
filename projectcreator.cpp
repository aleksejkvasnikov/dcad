#include "projectcreator.h"
#include "ui_projectcreator.h"

ProjectCreator::ProjectCreator(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::ProjectCreator)
{
    ui->setupUi(this);

	ui->freqStepLabel->hide();
	ui->freqStepLineEdit->hide();
	ui->freqStepUnitLabel->hide();

    // Установка валидаторов для ввода чисел
    QDoubleValidator *doubleValidator = new QDoubleValidator(this);
	QLocale locale(QLocale::English); // Устанавливаем локаль, в которой используется точка как десятичный разделитель
	locale.setNumberOptions(QLocale::RejectGroupSeparator); // Дополнительно, чтобы избежать проблем с разделителями групп
	doubleValidator->setLocale(locale);

    QIntValidator *intValidator = new QIntValidator(this);

    ui->freqMinEdit->setValidator(doubleValidator);
    ui->freqMaxEdit->setValidator(doubleValidator);
    ui->freqStepLineEdit->setValidator(doubleValidator);
    ui->pointsNumberEdit->setValidator(intValidator);

    ui->solverTextBrowser->setStyleSheet("QTextBrowser { border: none; background: transparent; }");
    ui->unitsTextBrowser->setStyleSheet("QTextBrowser { border: none; background: transparent; }");
    ui->freqsTextBrowser->setStyleSheet("QTextBrowser { border: none; background: transparent; }");

	setFixedSize(620,270);

	ui->pointsNumberEdit->setText("101");

}

ProjectCreator::~ProjectCreator()
{
    delete ui;
}

void ProjectCreator::clearAllInputData()
{
	ui->prNameEdit->clear();
	ui->dirPlaceEdit->clear();

	ui->dimsComboBox->setCurrentIndex(1);
	ui->freqsComboBox->setCurrentIndex(3);
	ui->timeComboBox->setCurrentIndex(1);

	ui->solverComboBox->setCurrentIndex(0);
	ui->freqMinEdit->clear();
	ui->freqMaxEdit->clear();

	ui->pointsNumberRadioButton->setChecked(true);
	ui->pointsNumberRadioButton->setChecked(false);

	ui->pointsNumberEdit->setText("101");
	ui->freqStepLineEdit->clear();
}


void ProjectCreator::on_dirPlaceButton_clicked()
{
    QString dir = QFileDialog::getExistingDirectory(this, tr("Выбрать папку"), "",
                                                    QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
    if (!dir.isEmpty()) {
        ui->dirPlaceEdit->setText(dir);
    }
}

void ProjectCreator::on_exitButton1_clicked()
{
	ui->stackedWidget->setCurrentIndex(0);
	clearAllInputData();
    close();
}

void ProjectCreator::on_cancelButton1_clicked()
{
	ui->stackedWidget->setCurrentIndex(0);
	clearAllInputData();
    close();
}

void ProjectCreator::on_nextButton1_clicked()
{
    QString projectName = ui->prNameEdit->text();
    QString dirPath = ui->dirPlaceEdit->text();

    if (projectName.isEmpty() || dirPath.isEmpty()) {
        QMessageBox::warning(this, tr("Ошибка"), tr("Пожалуйста, заполните все обязательные поля"));
    } else if (!isProjectNameValid(projectName)) {
        QMessageBox::warning(this, tr("Ошибка"), tr("Имя проекта содержит недопустимые символы"));
    } else if (!QDir(dirPath).exists()) {
        QMessageBox::warning(this, tr("Ошибка"), tr("Указанный путь не существует"));
    } else {
        ui->stackedWidget->setCurrentIndex(1);
    }
}

bool ProjectCreator::isProjectNameValid(const QString &name) {
    // Проверка на недопустимые символы (например, недопустимые символы для имени файла)
    QRegularExpression regex("[\\/:*?\"<>|.,!]");
    return !name.isEmpty() && !regex.match(name).hasMatch();
}

void ProjectCreator::on_backButton2_clicked()
{
    ui->stackedWidget->setCurrentIndex(0);
}

void ProjectCreator::on_cancelButton2_clicked()
{
	ui->stackedWidget->setCurrentIndex(0);
	clearAllInputData();
    close();
}

void ProjectCreator::on_nextButton2_clicked()
{
	ui->freqMaxUnitLabel->setText(ui->freqsComboBox->currentText());
	ui->freqMinUnitLabel->setText(ui->freqsComboBox->currentText());
	ui->freqStepUnitLabel->setText(ui->freqsComboBox->currentText());
    ui->stackedWidget->setCurrentIndex(2);
}

void ProjectCreator::on_freqsComboBox_currentTextChanged(const QString &arg1)
{
    ui->freqMaxUnitLabel->setText(arg1);
    ui->freqMinUnitLabel->setText(arg1);
    ui->freqStepUnitLabel->setText(arg1);
}

void ProjectCreator::on_backButton3_clicked()
{
    ui->stackedWidget->setCurrentIndex(1);
}

void ProjectCreator::on_cancelButton3_clicked()
{
	ui->stackedWidget->setCurrentIndex(0);
	clearAllInputData();
    close();
}

void ProjectCreator::on_freqStepRadioButton_clicked()
{
    ui->freqStepLabel->show();
    ui->freqStepLineEdit->show();
    ui->freqStepUnitLabel->show();

    ui->pointsNumberEdit->hide();
    ui->pointsNumberLabel->hide();
}

void ProjectCreator::on_pointsNumberRadioButton_clicked()
{
    ui->freqStepLabel->hide();
    ui->freqStepLineEdit->hide();
    ui->freqStepUnitLabel->hide();

    ui->pointsNumberEdit->show();
    ui->pointsNumberLabel->show();
}

void ProjectCreator::on_nextButton3_clicked()
{
    bool valid = true;
    QString errorMessage;

    // Проверка freqMinEdit и freqMaxEdit
    if (!isValidDouble(ui->freqMinEdit) || !isValidDouble(ui->freqMaxEdit)
            || ui->freqMaxEdit->text().toDouble()<0 || ui->freqMinEdit->text().toDouble()<0) {
        valid = false;
        errorMessage = tr("Пожалуйста, введите корректные значения диапазона частот.");
    } else {
        double freqMin = ui->freqMinEdit->text().toDouble();
        double freqMax = ui->freqMaxEdit->text().toDouble();
        if (freqMin >= freqMax) {
            valid = false;
            errorMessage = tr("Минимальная частота должна быть меньше максимальной.");
        }
    }

    // Проверка freqStepLineEdit, если активен freqStepRadioButton
    if (valid && ui->freqStepRadioButton->isChecked()) {
        if (!isValidDouble(ui->freqStepLineEdit) || ui->freqStepLineEdit->text().toDouble()<= 0) {
            valid = false;
            errorMessage = tr("Пожалуйста, введите корректное значение шага частоты.");
        }
    }

    // Проверка pointsNumberEdit, если активен pointsNumberRadioButton
    if (valid && ui->pointsNumberRadioButton->isChecked()) {
        if (!isValidInt(ui->pointsNumberEdit) || ui->pointsNumberEdit->text().toInt()<= 0) {
            valid = false;
            errorMessage = tr("Пожалуйста, введите корректное значение числа точек.");
        }
    }

    // Если все проверки пройдены, перейти на следующую вкладку
    if (valid) {
        QString commonStyle = "style='font-size:16px; color: #000000;'";

		QString solverTypePic;
		if (ui->solverComboBox->currentIndex() == 0) solverTypePic = "<li><img src=':/icons/icons/mom.png' width='24' height='24' style='margin-left: auto; margin-right: auto;'></li>";
		else solverTypePic = "<li><img src=':/icons/icons/fdtd.png' width='24' height='24' style='margin-left: auto; margin-right: auto;'></li>";
		QString solverText = "<b " + commonStyle + ">Решатель:</b><hr>"
			"<ul style='margin: 0; padding:0; list-style-type: none;'>"
			"<li><b style='font-size:14px; color: #000;'>" + ui->solverComboBox->currentText() + "</b></li>" +
			solverTypePic + "</ul>";
        ui->solverTextBrowser->setHtml(solverText);

        QString unitsText = "<b " + commonStyle + ">Единицы измерения:</b><hr>"
                            "<ul style='margin: 0; padding:0; list-style-type: circle;'>"
                            "<li><b>Размеры:</b> " + ui->dimsComboBox->currentText() + "</li>"
                            "<li><b>Частота:</b> " + ui->freqsComboBox->currentText() + "</li>"
                            "<li><b>Время:</b> " + ui->timeComboBox->currentText() + "</li>"
                            "</ul>";
        ui->unitsTextBrowser->setHtml(unitsText);

        QString freqsText = "<b " + commonStyle + ">Диапазон частот:</b><hr>"
                            "<ul style='margin: 0; padding:0; list-style-type: circle;'>"
                            "<li><b>f min:</b> " + ui->freqMinEdit->text() + " " + ui->freqsComboBox->currentText() +  "</li>"
                            "<li><b>f max:</b> " + ui->freqMaxEdit->text() + " " + ui->freqsComboBox->currentText() +  "</li>";
        if (ui->freqStepRadioButton->isChecked()) {
            freqsText += "<li><b>f step:</b> " + ui->freqStepLineEdit->text() + " " + ui->freqsComboBox->currentText() +  "</li>";
        } else if (ui->pointsNumberRadioButton->isChecked()) {
            freqsText += "<li><b>Число точек:</b> " + ui->pointsNumberEdit->text() + "</li>";
        }
        freqsText += "</ul>";
        ui->freqsTextBrowser->setHtml(freqsText);


        ui->stackedWidget->setCurrentIndex(3);
    } else {
        QMessageBox::warning(this, tr("Ошибка"), errorMessage);
    }
}

void ProjectCreator::saveCDSettings()
{
	// Получаем путь к папке, где расположен .exe файл программы
	QString iniFilePath = QCoreApplication::applicationDirPath() + "/settings.ini";
	qDebug() << iniFilePath;

	// Создаем объект QSettings для чтения из INI-файла
	QSettings settings(iniFilePath, QSettings::IniFormat);
	// Чтение значений из INI-файла
	double xMin = settings.value("cdSettings/XMin", 0).toDouble();
	double yMin = settings.value("cdSettings/YMin", 0).toDouble();
	double zMin = settings.value("cdSettings/ZMin", 0).toDouble();
	double xMax = settings.value("cdSettings/XMax", 0).toDouble();
	double yMax = settings.value("cdSettings/YMax", 0).toDouble();
	double zMax = settings.value("cdSettings/ZMax", 0).toDouble();
	short unsigned int unitsType = settings.value("cdSettings/UnitsType", 0).toUInt();
	short unsigned int freqType = settings.value("cdSettings/FreqType", 0).toUInt();
	short unsigned int allDirections = settings.value("cdSettings/AllDirections", 0).toUInt();

	QString projectPath = ui->dirPlaceEdit->text();
	QString projectName = ui->prNameEdit->text();
	QString fullPath = QDir(projectPath).filePath(projectName);

	// Создаем объект QDomDocument для создания XML документа
	QDomDocument document;

	// Создаем корневой элемент
	QDomElement root = document.createElement("CDSettings");
	document.appendChild(root);

	// Создаем элемент для координат
	QDomElement coordinatesElement = document.createElement("Coordinates");
	root.appendChild(coordinatesElement);

	// Добавляем координаты
	coordinatesElement.appendChild(document.createElement("XMin")).appendChild(document.createTextNode(QString::number(xMin)));
	coordinatesElement.appendChild(document.createElement("XMax")).appendChild(document.createTextNode(QString::number(xMax)));
	coordinatesElement.appendChild(document.createElement("YMin")).appendChild(document.createTextNode(QString::number(yMin)));
	coordinatesElement.appendChild(document.createElement("YMax")).appendChild(document.createTextNode(QString::number(yMax)));
	coordinatesElement.appendChild(document.createElement("ZMin")).appendChild(document.createTextNode(QString::number(zMin)));
	coordinatesElement.appendChild(document.createElement("ZMax")).appendChild(document.createTextNode(QString::number(zMax)));

	// Создаем элемент для частоты
	QDomElement frequencyElement = document.createElement("Frequency");
	root.appendChild(frequencyElement);

	// Добавляем значение частоты
	frequencyElement.appendChild(document.createElement("Value")).
		appendChild(document.createTextNode(QString::number(ui->freqMinEdit->text().toDouble() + (ui->freqMaxEdit->text().toDouble() - ui->freqMinEdit->text().toDouble())/2.0)));

	// Создаем элемент для единиц измерения
	QDomElement additionalElement = document.createElement("Additional");
	root.appendChild(additionalElement);

	// Добавляем тип единиц измерения
	additionalElement.appendChild(document.createElement("UnitsType")).appendChild(document.createTextNode(QString::number(unitsType)));
	additionalElement.appendChild(document.createElement("FreqType")).appendChild(document.createTextNode(QString::number(freqType)));
	additionalElement.appendChild(document.createElement("AllDirections")).appendChild(document.createTextNode(QString::number(allDirections)));

	// Сохраняем документ в файл
	QString cdfilePath = QDir(fullPath).filePath("cdSettings.set");
	QFile file(cdfilePath);
	if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
		qWarning("Не удалось открыть файл для записи");
		return;
	}

	QTextStream stream(&file);
	stream.setCodec("UTF-8");
	stream << document.toString();
	file.close();
}

bool ProjectCreator::isValidDouble(QLineEdit *lineEdit) {
    bool ok;
    lineEdit->text().toDouble(&ok);
    return ok;
}

bool ProjectCreator::isValidInt(QLineEdit *lineEdit) {
    bool ok;
    lineEdit->text().toInt(&ok);
    return ok;
}

void ProjectCreator::on_backButton4_clicked()
{
    ui->stackedWidget->setCurrentIndex(2);
}

void ProjectCreator::on_cancelButton4_clicked()
{
	ui->stackedWidget->setCurrentIndex(0);
	clearAllInputData();
    close();
}

void ProjectCreator::on_createButton4_clicked()
{
	// Получаем значения из интерфейса
	QString projectPath = ui->dirPlaceEdit->text();
	QString projectName = ui->prNameEdit->text();
	QString fullPath = QDir(projectPath).filePath(projectName);

	// Создаем папку проекта
	QDir dir;
	if (!dir.mkpath(fullPath)) {
		QMessageBox::warning(this, tr("Ошибка"), tr("Не удалось создать папку проекта."));
		return;
	}

	// Получаем текущую дату и время
	QString creationDate = QDateTime::currentDateTime().toString(Qt::ISODate);

	// Получаем имя текущего пользователя Windows
	QString authorName = qgetenv("USERNAME");
	if (authorName.isEmpty())
		authorName = qgetenv("USER");  // Для Unix-подобных систем

	// Создаем XML документ
	QDomDocument document;
	QDomElement root = document.createElement("Project");
	document.appendChild(root);

	// Добавляем данные в XML
	QDomElement projectInfoElement = document.createElement("ProjectInfo");
	projectInfoElement.setAttribute("Name", projectName);
	projectInfoElement.setAttribute("Directory", fullPath);
	projectInfoElement.setAttribute("CreationDate", creationDate);
	projectInfoElement.setAttribute("LastModifiedDate", creationDate);  // Добавляем дату последнего изменения
	projectInfoElement.setAttribute("Author", authorName); // может быть проблема с кириллицей
	root.appendChild(projectInfoElement);

	QDomElement unitsElement = document.createElement("Units");
	unitsElement.setAttribute("Geometry", ui->dimsComboBox->currentText());
	unitsElement.setAttribute("Frequency", ui->freqsComboBox->currentText());
	unitsElement.setAttribute("Time", ui->timeComboBox->currentText());
	root.appendChild(unitsElement);

	QDomElement solverElement = document.createElement("Solver");
	solverElement.setAttribute("Type", ui->solverComboBox->currentText());
	root.appendChild(solverElement);

	QDomElement frequencyElement = document.createElement("FrequencyRange");
	frequencyElement.setAttribute("Min", ui->freqMinEdit->text());
	frequencyElement.setAttribute("Max", ui->freqMaxEdit->text());
	root.appendChild(frequencyElement);

	if (ui->freqStepRadioButton->isChecked()) {
		QDomElement stepElement = document.createElement("FrequencyStep");
		stepElement.setAttribute("Value", ui->freqStepLineEdit->text());
		root.appendChild(stepElement);
	}
	else if (ui->pointsNumberRadioButton->isChecked()) {
		QDomElement pointsElement = document.createElement("PointsNumber");
		pointsElement.setAttribute("Value", ui->pointsNumberEdit->text());
		root.appendChild(pointsElement);
	}

	// Сохранение XML в файл
	QString filePath = QDir(fullPath).filePath(projectName+".proj");
	QFile file(filePath);
	if (!file.open(QIODevice::WriteOnly)) {
		QMessageBox::warning(this, tr("Ошибка"), tr("Не удалось создать файл проекта."));
		return;
	}

	QTextStream stream(&file);
	stream.setCodec("UTF-8");  // Устанавливаем кодировку UTF-8
	stream << document.toString();
	file.close();
	// создание файла настроек расчетной области
	saveCDSettings();
	// Выполнение оставшихся операций
	ui->stackedWidget->setCurrentIndex(0);
	clearAllInputData();
	emit projectIsReady(filePath);
	close();
}

