#ifndef PROJECTCREATOR_H
#define PROJECTCREATOR_H

#include <QMainWindow>
#include <QFileDialog>
#include <QMessageBox>
#include <QRegularExpression>
#include <QDoubleValidator>
#include <QIntValidator>
#include <QLineEdit>
QT_BEGIN_NAMESPACE
namespace Ui { class ProjectCreator; }
QT_END_NAMESPACE

struct ProjectSettings{
    QString projectDir;
    QString projectName;
};


class ProjectCreator : public QMainWindow
{
    Q_OBJECT

public:
    ProjectCreator(QWidget *parent = nullptr);
    ~ProjectCreator();

private slots:
    void on_dirPlaceButton_clicked();

    void on_exitButton1_clicked();

    void on_cancelButton1_clicked();

    void on_nextButton1_clicked();

    void on_backButton2_clicked();

    void on_cancelButton2_clicked();

    void on_nextButton2_clicked();

    void on_freqsComboBox_currentTextChanged(const QString &arg1);

    void on_backButton3_clicked();

    void on_cancelButton3_clicked();

    void on_freqStepRadioButton_clicked();

    void on_pointsNumberRadioButton_clicked();

    void on_nextButton3_clicked();

    void on_backButton4_clicked();

    void on_cancelButton4_clicked();

private:
    bool isValidDouble(QLineEdit *lineEdit);
    bool isValidInt(QLineEdit *lineEdit);
    bool isProjectNameValid(const QString &name);

    Ui::ProjectCreator *ui;
    ProjectSettings* prSet;
};


#endif // PROJECTCREATOR_H
