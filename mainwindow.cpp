#include <QMenu>
#include <QCheckBox>
#include <QComboBox>
#include <QSpinBox>
#include <QLineEdit>
#include <QTextEdit>
#include <QLabel>
#include <QMessageBox>
#include <QPushButton>
#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QDebug>
// VTK
#include "vtkLookupTable.h"
#include <vtkGenericOpenGLRenderWindow.h>
#include <vtkNew.h>
#include <vtkOrientationMarkerWidget.h>
#include <vtkRenderer.h>
#include "vtkAxesActor.h"
#include "vtkAutoInit.h"
// QT
#include <qlistwidget.h>
#include <qtextbrowser.h>

VTK_MODULE_INIT(vtkRenderingOpenGL2)
VTK_MODULE_INIT(vtkInteractionStyle)
VTK_MODULE_INIT(vtkRenderingFreeType)
CMainWindow::CMainWindow(QWidget* parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->statusBar->addPermanentWidget(new QLabel(" ")); // footer status
	
	ui->treeView->setMaximumWidth(200); // дерево
	ui->textEdit->setMaximumHeight(200); // консоль

	// VTK Инициализация
	vtkNew<vtkGenericOpenGLRenderWindow> renderWindow; // задание VTK окна
	ui->openGLWidget->SetRenderWindow(renderWindow);

	mRenderer = vtkSmartPointer<vtkRenderer>::New();
	ui->openGLWidget->GetRenderWindow()->AddRenderer(mRenderer);
	mRenderer->GradientBackgroundOn();
	mRenderer->SetBackground(.9, .9, .9);
	mRenderer->SetBackground2(.8, .8, .8);
	mRenderer->SetUseDepthPeeling(1);

	vtkAxesActor *axes = vtkAxesActor::New(); // 3д объект для оси
	vtkOrientationMarkerWidget *widget = vtkOrientationMarkerWidget::New();
	widget->SetDefaultRenderer(mRenderer);
	widget->SetOrientationMarker(axes);
	widget->SetInteractor(ui->openGLWidget->GetRenderWindow()->GetInteractor());
	widget->EnabledOn();

	// OCCT step parsing
	Handle(TDocStd_Document) anXdeDoc = new TDocStd_Document("BinXCAF");
	STEPCAFControl_Reader reader;
	reader.SetColorMode(Standard_True);
	reader.SetNameMode(Standard_True);
	reader.SetMatMode(Standard_True);
	if (reader.ReadFile("D:/Proekti/DETAL/HEH.stp") != IFSelect_RetDone || !reader.Transfer(anXdeDoc)) {
		std::cout << "Error";
	}
	const auto mainLabel = anXdeDoc->Main();
	shapeTool = XCAFDoc_DocumentTool::ShapeTool(mainLabel);
	colorTool = XCAFDoc_DocumentTool::ColorTool(mainLabel);
	layerTool = XCAFDoc_DocumentTool::LayerTool(mainLabel);

	TDF_LabelSequence shapes;
	shapeTool->GetFreeShapes(shapes);
	for (const auto &label : shapes) {
		addShape(label);
	}
	objectsModel = new QStandardItemModel(ui->treeView);
	load(); // загрузка объекта в дерево
	ui->treeView->setModel(objectsModel);


	// Инициализация тулбара
    tt::Builder ttb(this);
	tabToolbar = ttb.CreateTabToolbar(":/tt/tabtoolbar.json");
	
	addToolBar(Qt::TopToolBarArea, tabToolbar);
	

	QObject::connect(ui->actionLoadStep, &QAction::triggered, this, [this]() // заменить позднее на загрузку модели
	{
		QMessageBox::information(this, "Kek", "Cheburek");
	});

	tt::Page* filePage = (tt::Page*)ttb["File"];
	QObject::connect(tabToolbar, &tt::TabToolbar::CurrentTabChanged2, this, &CMainWindow::displayMenuWidgets);


	tabToolbar->SetStyle("Kool");
	createFirstTab(); // создание стартового окна
	tabToolbar->SetCurrentTab(0); // установка стартового окна
	qApp->setStyleSheet(tabToolbar->styleSheet());
	foreach(QWidget *widget, qApp->allWidgets()) {
		widget->update();
	}
}

CMainWindow::~CMainWindow()
{
    delete ui;
}

void CMainWindow::addShape(const TDF_Label & label, Handle(AIS_InteractiveObject) parent)
{
	TDF_Label origLabel;
	if (XCAFDoc_ShapeTool::IsReference(label)) {
		XCAFDoc_ShapeTool::GetReferredShape(label, origLabel);
	}
	else {
		origLabel = label;
	}

	InternalData data;
	Handle(TDataStd_Name) name;
	if (label.FindAttribute(TDataStd_Name::GetID(), name)) {
		data.name = name->Get();
	}
	XCAFDoc_ShapeTool::GetShape(label, data.shape);
	Handle(XCAFDoc_Location) location;
	if (label.FindAttribute(XCAFDoc_Location::GetID(), location)) {
		data.location = location->Get();
	}

	if (materialTool) {
		Handle(TCollection_HAsciiString) name, description, densName, densValType;
		Standard_Real density;
		if (materialTool->GetMaterial(origLabel, name, description, density, densName, densValType)) {
			qDebug() << "Found material for" << QString::fromStdU16String(data.name.ToExtString())
				<< name->ToCString()
				<< description->ToCString()
				<< density
				<< densName->ToCString()
				<< densValType->ToCString();
		}
	}

	if (colorTool) {
		for (int i = XCAFDoc_ColorGen; i <= XCAFDoc_ColorCurv; ++i) {
			Quantity_ColorRGBA clr;
			data.hasColors[i] = colorTool->GetColor(origLabel, static_cast<XCAFDoc_ColorType>(i), clr);
			if (data.hasColors[i]) {
				data.colors[i] = clr;
			}
		}
	}

	if (layerTool) {
		TDF_LabelSequence labels;
		if (layerTool->GetLayers(origLabel, labels)) {
			for (const auto &lbl : labels) {
				TCollection_ExtendedString name;
				layerTool->GetLayer(lbl, name);
				qDebug() << "Layer" << QString::fromStdU16String(name.ToExtString());
			}
		}
	}

	Handle(AIS_Shape) obj = new AIS_Shape(TopoDS_Shape());
	auto drawer = obj->Attributes();
	auto shAspect = drawer->ShadingAspect();
	auto fbAspect = drawer->FaceBoundaryAspect();
	if (data.hasColors[XCAFDoc_ColorGen]) {
		shAspect->SetColor(data.colors[XCAFDoc_ColorGen].GetRGB());
		fbAspect->SetColor(data.colors[XCAFDoc_ColorGen].GetRGB());
	}
	if (data.hasColors[XCAFDoc_ColorSurf]) {
		shAspect->SetColor(data.colors[XCAFDoc_ColorSurf].GetRGB());
	}
	if (data.hasColors[XCAFDoc_ColorCurv]) {
		fbAspect->SetColor(data.colors[XCAFDoc_ColorCurv].GetRGB());
	}

	if (parent) {
		data.parent = parent;
		parent->AddChild(obj);
	}

	if (XCAFDoc_ShapeTool::IsAssembly(label)) {
		for (TDF_ChildIterator it(label); it.More(); it.Next()) {
			addShape(it.Value(), obj);
		}
	}
	else {
		addShape(data, obj);
	}
	objects.Bind(obj, data);
}

void CMainWindow::addShape(const InternalData & data, Handle(AIS_Shape)& object)
{
	if (data.shape.ShapeType() == TopAbs_COMPOUND) {
		for (TopExp_Explorer anExp(data.shape, TopAbs_SOLID); anExp.More(); anExp.Next()) {
			InternalData chData = data;
			chData.name.Clear();
			chData.shape = anExp.Current();
			chData.parent = object;
			Handle(AIS_Shape) chObj = new AIS_Shape(TopoDS_Shape());
			auto drawer = chObj->Attributes();
			auto shAspect = drawer->ShadingAspect();
			auto fbAspect = drawer->FaceBoundaryAspect();
			if (chData.hasColors[XCAFDoc_ColorGen]) {
				shAspect->SetColor(data.colors[XCAFDoc_ColorGen].GetRGB());
				fbAspect->SetColor(data.colors[XCAFDoc_ColorGen].GetRGB());
			}
			if (chData.hasColors[XCAFDoc_ColorSurf]) {
				shAspect->SetColor(data.colors[XCAFDoc_ColorSurf].GetRGB());
			}
			if (chData.hasColors[XCAFDoc_ColorCurv]) {
				fbAspect->SetColor(data.colors[XCAFDoc_ColorCurv].GetRGB());
			}

			object->AddChild(chObj);
			addShape(chData, chObj);
			objects.Bind(chObj, chData);

			// Добавление TopoDS_Shape в VTK Renderer
			vtkSmartPointer<vtkPolyData> polyData = ConvertTopoDS_ShapeToPolyData(chData.shape);
			AddShapeToVTKRenderer(mRenderer, polyData);
		}
	}
	else {
		object->SetShape(data.shape);

		// Добавление TopoDS_Shape в VTK Renderer
		vtkSmartPointer<vtkPolyData> polyData = ConvertTopoDS_ShapeToPolyData(data.shape);
		AddShapeToVTKRenderer(mRenderer, polyData);
	}
}

void CMainWindow::load()
{
	objectsModel->setHorizontalHeaderItem(0, new QStandardItem("Name"));

	const auto objects = result();
	for (const auto &obj : objects) {
		//myOccView->getContext()->Display(obj, Standard_True);
		if (objectsModel) {
			addToObjectsTree(obj);
		}
	}

}

void CMainWindow::addToObjectsTree(const Handle(AIS_InteractiveObject)& obj, QStandardItem * parent)
{
	auto name = QString::fromStdU16String(this->name(obj).ToExtString());
	if (name.isEmpty()) {
		name = "noname";
	}
	auto item = new QStandardItem(name);
	item->setData(QVariant::fromValue(static_cast<void *>(obj.get())));
	if (parent) {
		parent->appendRow(item);
	}
	else {
		objectsModel->appendRow(item);
	}
	auto chList = obj->Children();
	for (const auto &ch : chList) {
		auto child = Handle(AIS_InteractiveObject)::DownCast(ch);
		if (child) {
			addToObjectsTree(child, item);
		}
	}
}

TCollection_ExtendedString CMainWindow::name(const Handle(AIS_InteractiveObject)& object) const
{
	InternalData data;
	if (objects.Find(object, data)) {
		return data.name;
	}
	return TCollection_ExtendedString();
}

AIS_ListOfInteractive CMainWindow::result() const
{
	AIS_ListOfInteractive ret;
	for (InternalMap::Iterator it(objects); it.More(); it.Next()) {
		if (it.Value().parent.IsNull()) {
			ret.Append(it.Key());
		}
	}
	return ret;
}

inline vtkSmartPointer<vtkPolyData> CMainWindow::ConvertTopoDS_ShapeToPolyData(const TopoDS_Shape & shape) {
	vtkSmartPointer<vtkPolyData> polyData = vtkSmartPointer<vtkPolyData>::New();
	vtkSmartPointer<vtkPoints> points = vtkSmartPointer<vtkPoints>::New();
	vtkSmartPointer<vtkCellArray> polys = vtkSmartPointer<vtkCellArray>::New();
	vtkSmartPointer<vtkUnsignedCharArray> colors = vtkSmartPointer<vtkUnsignedCharArray>::New();
	colors->SetNumberOfComponents(3);
	colors->SetName("Colors");

	// Create mesh for shape
	BRepMesh_IncrementalMesh(shape, 0.1);

	// Extract vertices and faces
	TopExp_Explorer expFace(shape, TopAbs_FACE);
	for (; expFace.More(); expFace.Next()) {
		TopoDS_Face face = TopoDS::Face(expFace.Current());

		Quantity_ColorRGBA color;
		bool hasColor = colorTool->GetColor(shape, XCAFDoc_ColorSurf, color);

		TopExp_Explorer expVertex(face, TopAbs_VERTEX);
		std::vector<vtkIdType> faceVertexIds;
		for (; expVertex.More(); expVertex.Next()) {
			TopoDS_Vertex vertex = TopoDS::Vertex(expVertex.Current());
			gp_Pnt pnt = BRep_Tool::Pnt(vertex);
			vtkIdType pointId = points->InsertNextPoint(pnt.X(), pnt.Y(), pnt.Z());
			faceVertexIds.push_back(pointId);

			if (hasColor) {
				unsigned char rgb[3];
				rgb[0] = static_cast<unsigned char>(color.GetRGB().Red() * 255);
				rgb[1] = static_cast<unsigned char>(color.GetRGB().Green() * 255);
				rgb[2] = static_cast<unsigned char>(color.GetRGB().Blue() * 255);
				colors->InsertNextTypedTuple(rgb);
			}
		}

		// Add cells to polys
		if (faceVertexIds.size() >= 3) {
			polys->InsertNextCell(faceVertexIds.size(), faceVertexIds.data());
		}
	}

	polyData->SetPoints(points);
	polyData->SetPolys(polys);
	if (colors->GetNumberOfTuples() > 0) {
		polyData->GetPointData()->SetScalars(colors);
	}

	return polyData;
}

inline void CMainWindow::AddShapeToVTKRenderer(vtkSmartPointer<vtkRenderer> renderer, const vtkSmartPointer<vtkPolyData>& polyData) {
	vtkSmartPointer<vtkPolyDataMapper> mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
	mapper->SetInputData(polyData);

	vtkSmartPointer<vtkActor> actor = vtkSmartPointer<vtkActor>::New();
	actor->SetMapper(mapper);

	renderer->AddActor(actor);
}

// функция создания стартового окна - необходимо переделать
void CMainWindow::createFirstTab()
{
	prCreator = new ProjectCreator;
	centralwidgetMenu = new QWidget(this);

	QVBoxLayout *verticalLayout_5;
	QHBoxLayout *mainHorLayout;
	QVBoxLayout *leftVertLayout;
	QLabel *newProjectLabel;
	QPushButton *createProjectButton;
	QFrame *line_2;
	QHBoxLayout *leftHorLayout;
	QVBoxLayout *leftVertLayoutIn1;
	QLabel *latestFilesLabel;
	QListWidget *latestListWidget;
	QVBoxLayout *leftVertLayoutIn2;
	QLabel *examplesLabel;
	QListWidget *examplesListWidget;
	QFrame *line;
	QVBoxLayout *rightVertLayout;
	QLabel *guideLabel;
	QPushButton *guideButton;
	QPushButton *aboutButton;
	QLabel *infoLabel;
	QTextBrowser *infoTextBrowser;


	verticalLayout_5 = new QVBoxLayout(centralwidgetMenu);
	verticalLayout_5->setObjectName(QString::fromUtf8("verticalLayout_5"));
	mainHorLayout = new QHBoxLayout();
	mainHorLayout->setObjectName(QString::fromUtf8("mainHorLayout"));
	leftVertLayout = new QVBoxLayout();
	leftVertLayout->setObjectName(QString::fromUtf8("leftVertLayout"));
	newProjectLabel = new QLabel(centralwidgetMenu);
	newProjectLabel->setObjectName(QString::fromUtf8("newProjectLabel"));
	newProjectLabel->setText("Новый проект:");
	newProjectLabel->setStyleSheet("font-family: Arial; font-size: 16px; font-weight: bold; color: #333333;");

	leftVertLayout->addWidget(newProjectLabel);

	createProjectButton = new QPushButton(centralwidgetMenu);
	createProjectButton->setObjectName(QString::fromUtf8("createProjectButton"));
	createProjectButton->setFixedWidth(1063 * 0.6);
	createProjectButton->setFixedHeight(523 * 0.6);
	createProjectButton->setStyleSheet(
		"QPushButton {"
		"    border: none;"                      // Убираем границы
		"    background: none;"                      // Убираем 
		"    background-color: transparent;"     // Прозрачная заливка
		"    border-image: url(:/icons/icons/default.png);" // Картинка по умолчанию
		"    background-repeat: none;"      // Не повторяем изображение
		"}"
		"QPushButton:hover {"
		"    border: none;"                      // Убираем границы
		"    background: none;"                      // Убираем 
		"    border-image: url(:/icons/icons/hover.png);" // Картинка при нажатии
		"    background-repeat: none;"      // Не повторяем изображение
		"}"
		"QPushButton:pressed {"
		"    border: none;"                      // Убираем границы
		"    background: none;"                      // Убираем 
		"    border-image: url(:/icons/icons/pressed.png);" // Картинка при нажатии
		"    background-repeat: none;"      // Не повторяем изображение
		"}"
	);
	connect(createProjectButton, &QPushButton::clicked, this, &CMainWindow::onCreateProjectButtonClicked);

	leftVertLayout->addWidget(createProjectButton);

	line_2 = new QFrame(centralwidgetMenu);
	line_2->setObjectName(QString::fromUtf8("line_2"));
	line_2->setFrameShape(QFrame::HLine);
	line_2->setFrameShadow(QFrame::Sunken);

	leftVertLayout->addWidget(line_2);

	leftHorLayout = new QHBoxLayout();
	leftHorLayout->setObjectName(QString::fromUtf8("leftHorLayout"));
	leftVertLayoutIn1 = new QVBoxLayout();
	leftVertLayoutIn1->setObjectName(QString::fromUtf8("leftVertLayoutIn1"));
	latestFilesLabel = new QLabel(centralwidgetMenu);
	latestFilesLabel->setObjectName(QString::fromUtf8("latestFilesLabel"));
	latestFilesLabel->setText("Последние файлы:");
	latestFilesLabel->setStyleSheet("font-family: Arial; font-size: 16px; font-weight: bold; color: #333333;");

	leftVertLayoutIn1->addWidget(latestFilesLabel);

	latestListWidget = new QListWidget(centralwidgetMenu);
	latestListWidget->setObjectName(QString::fromUtf8("latestListWidget"));
	latestListWidget->setStyleSheet(
		"QListWidget {"
		"    border: 1px solid #dcdcdc;"               // Граница виджета
		"    background-color: rgb(240, 240, 240);" // Легкая прозрачность фона
		"}"
		"QListWidget::item {"
		"    background-color: rgba(240, 240, 240, 1);" // Полупрозрачная заливка элементов (для видимости)
		"    border: none;"                           // Убираем границы элементов
		"    padding: 10px;"                          // Добавляем отступы для элементов
		"    margin: 2px 0;"                          // Добавляем отступы между элементами
		"    font-family: Arial, sans-serif;"         // Шрифт
		"    font-size: 14px;"                        // Размер шрифта
		"    color: #333;"                            // Цвет текста
		"}"
		"QListWidget::item:selected {"
		"    background-color: #0078d7;"              // Цвет фона для выделенного элемента
		"    color: #ffffff;"                         // Цвет текста для выделенного элемента
		"}"
	);

	// Добавление нескольких элементов в список с путями и расширениями
	latestListWidget->addItem("Project 1 - C:/Projects/Project1/project1.proj");
	latestListWidget->addItem("Project 2 - C:/Projects/Project2/project2.proj");
	latestListWidget->addItem("Project 3 - C:/Projects/Project3/project3.proj");
	latestListWidget->addItem("Project 4 - C:/Projects/Project4/project4.proj");
	latestListWidget->addItem("Project 5 - C:/Projects/Project5/project5.proj");

	leftVertLayoutIn1->addWidget(latestListWidget);


	leftHorLayout->addLayout(leftVertLayoutIn1);

	leftVertLayoutIn2 = new QVBoxLayout();
	leftVertLayoutIn2->setObjectName(QString::fromUtf8("leftVertLayoutIn2"));
	examplesLabel = new QLabel(centralwidgetMenu);
	examplesLabel->setObjectName(QString::fromUtf8("examplesLabel"));
	examplesLabel->setText("Тестовые примеры:");
	examplesLabel->setStyleSheet("font-family: Arial; font-size: 16px; font-weight: bold; color: #333333;");

	leftVertLayoutIn2->addWidget(examplesLabel);

	examplesListWidget = new QListWidget(centralwidgetMenu);
	examplesListWidget->setObjectName(QString::fromUtf8("examplesListWidget"));
	examplesListWidget->setStyleSheet(
		"QListWidget {"
		"    border: 1px solid #dcdcdc;"               // Граница виджета
		"    background-color: rgb(240, 240, 240);" // Легкая прозрачность фона
		"}"
		"QListWidget::item {"
		"    background-color: rgba(240, 240, 240, 1);" // Полупрозрачная заливка элементов (для видимости)
		"    border: none;"                           // Убираем границы элементов
		"    padding: 10px;"                          // Добавляем отступы для элементов
		"    margin: 2px 0;"                          // Добавляем отступы между элементами
		"    font-family: Arial, sans-serif;"         // Шрифт
		"    font-size: 14px;"                        // Размер шрифта
		"    color: #333;"                            // Цвет текста
		"}"
		"QListWidget::item:selected {"
		"    background-color: #0078d7;"              // Цвет фона для выделенного элемента
		"    color: #ffffff;"                         // Цвет текста для выделенного элемента
		"}"
	);

	// Добавление нескольких элементов в список с путями и расширениями
	examplesListWidget->addItem("Example 1.proj");
	examplesListWidget->addItem("Example 2.proj");
	examplesListWidget->addItem("Example 3.proj");
	examplesListWidget->addItem("Example 4.proj");
	examplesListWidget->addItem("Example 5.proj");

	leftVertLayoutIn2->addWidget(examplesListWidget);


	leftHorLayout->addLayout(leftVertLayoutIn2);


	leftVertLayout->addLayout(leftHorLayout);


	mainHorLayout->addLayout(leftVertLayout);

	line = new QFrame(centralwidgetMenu);
	line->setObjectName(QString::fromUtf8("line"));
	line->setFrameShape(QFrame::VLine);
	line->setFrameShadow(QFrame::Sunken);

	mainHorLayout->addWidget(line);

	rightVertLayout = new QVBoxLayout();
	rightVertLayout->setObjectName(QString::fromUtf8("rightVertLayout"));
	guideLabel = new QLabel(centralwidgetMenu);
	guideLabel->setObjectName(QString::fromUtf8("guideLabel"));
	guideLabel->setText("Справка:");
	guideLabel->setStyleSheet("font-family: Arial; font-size: 16px; font-weight: bold; color: #333333;");

	rightVertLayout->addWidget(guideLabel);

	guideButton = new QPushButton(centralwidgetMenu);
	guideButton->setObjectName(QString::fromUtf8("guideButton"));
	guideButton->setIcon(QIcon(":/icons/icons/Help-browser.png"));
	guideButton->setIconSize(QSize(36, 36));
	guideButton->setStyleSheet(
		"QPushButton {"
		"    border: 1px solid #dcdcdc;"                      // Убираем границы
		"    background-color: transparent;"     // Прозрачный фон
		"    text-align: left;"                  // Выровнять текст по левому краю
		"}"
	);
	guideButton->setText("Руководство пользователя");

	rightVertLayout->addWidget(guideButton);

	aboutButton = new QPushButton(centralwidgetMenu);
	aboutButton->setObjectName(QString::fromUtf8("aboutButton"));
	aboutButton->setIcon(QIcon(":/icons/icons/Help-browser.png"));
	aboutButton->setIconSize(QSize(36, 36));
	aboutButton->setStyleSheet(
		"QPushButton {"
		"    border: 1px solid #dcdcdc;"                      // Убираем границы
		"    background-color: transparent;"     // Прозрачный фон
		"    text-align: left;"                  // Выровнять текст по левому краю
		"}"
	);
	aboutButton->setText("О программе");

	rightVertLayout->addWidget(aboutButton);

	infoLabel = new QLabel(centralwidgetMenu);
	infoLabel->setObjectName(QString::fromUtf8("infoLabel"));
	infoLabel->setText("Информация:");
	infoLabel->setStyleSheet("font-family: Arial; font-size: 16px; font-weight: bold; color: #333333;");

	rightVertLayout->addWidget(infoLabel);

	infoTextBrowser = new QTextBrowser(centralwidgetMenu);
	infoTextBrowser->setObjectName(QString::fromUtf8("infoTextBrowser"));
	infoTextBrowser->setStyleSheet(QString::fromUtf8("background-color: rgb(240, 240, 240); border: none;"));
	infoTextBrowser->setPlainText("Проект: Project 1\n"
		"Расположение: C:/Projects/Project1/project1.proj\n"
		"Дата создания: 12 января 2024 г.\n"
		"Последнее изменение: 15 марта 2024 г.\n"
		"Автор: Иван Иванов\n"
		"Описание:\n"
		"Этот проект включает в себя разработку и симуляцию фазированных антенных решеток. Основные цели проекта включают оптимизацию структуры решеток для повышения эффективности и уменьшения помех");

	rightVertLayout->addWidget(infoTextBrowser);


	mainHorLayout->addLayout(rightVertLayout);


	verticalLayout_5->addLayout(mainHorLayout);
	centralwidgetMenu->hide();
}
void CMainWindow::onCreateProjectButtonClicked() {
	prCreator->show();
}
void CMainWindow::displayMenuWidgets(int a) {
	tabToolbar->SetCurrentTab(a);

	if (a == 0) {
		// Удаление текущего центрального виджета, если он существует
		if (this->centralWidget() != nullptr) {
			this->centralWidget()->hide();
			this->centralWidget()->setParent(nullptr);
		}

		// Установка нового центрального виджета
		this->setCentralWidget(centralwidgetMenu);
		centralwidgetMenu->show();
	}
	else {
		// Удаление текущего центрального виджета, если он существует
		if (this->centralWidget() != nullptr) {
			this->centralWidget()->hide();
			this->centralWidget()->setParent(nullptr);
		}

		// Установка другого центрального виджета
		this->setCentralWidget(ui->centralWidget);
		ui->centralWidget->show();
	}
}