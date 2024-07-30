#include "projectcreator.h"
#include "ui_projectcreator.h"

ProjectCreator::ProjectCreator(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::ProjectCreator)
{
    ui->setupUi(this);
    prSet = new ProjectSettings;

    ui->pointsNumberEdit->hide();
    ui->pointsNumberLabel->hide();


    // Установка валидаторов для ввода чисел
    QDoubleValidator *doubleValidator = new QDoubleValidator(this);
    QIntValidator *intValidator = new QIntValidator(this);

    ui->freqMinEdit->setValidator(doubleValidator);
    ui->freqMaxEdit->setValidator(doubleValidator);
    ui->freqStepLineEdit->setValidator(doubleValidator);
    ui->pointsNumberEdit->setValidator(intValidator);

    ui->solverTextBrowser->setStyleSheet("QTextBrowser { border: none; background: transparent; }");
    ui->unitsTextBrowser->setStyleSheet("QTextBrowser { border: none; background: transparent; }");
    ui->freqsTextBrowser->setStyleSheet("QTextBrowser { border: none; background: transparent; }");

    resize(620,100);

}

ProjectCreator::~ProjectCreator()
{
    delete ui;
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
    close();
}

void ProjectCreator::on_cancelButton1_clicked()
{
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
    close();
}

void ProjectCreator::on_nextButton2_clicked()
{
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

        QString solverText = "<b " + commonStyle + ">Решатель:</b><hr>"
                             "<ul style='margin: 0; padding:0; list-style-type: disc;'>"
                             "<li><b style='font-size:14px; color: #000;'>" + ui->solverComboBox->currentText() + "</b></li>"
                             "</ul>";
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
                            "<li><b>fMin:</b> " + ui->freqMinEdit->text() + " " + ui->freqsComboBox->currentText() +  "</li>"
                            "<li><b>fMax:</b> " + ui->freqMaxEdit->text() + " " + ui->freqsComboBox->currentText() +  "</li>";
        if (ui->freqStepRadioButton->isChecked()) {
            freqsText += "<li><b>fStep:</b> " + ui->freqStepLineEdit->text() + " " + ui->freqsComboBox->currentText() +  "</li>";
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
    close();
}
