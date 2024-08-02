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
	ui->statusBar->hide();
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
	tabToolbar->getTabWidget()->setTabEnabled(1, false); // блокируем вкладки
	tabToolbar->getTabWidget()->setTabEnabled(2, false);

	freqsSettings = new FreqsSettings;
	freqsSettings->setWindowIcon(this->windowIcon());
	QObject::connect(freqsSettings, SIGNAL(freqsSettingsWasChanged()), this, SLOT(onProjectChanges()));
	QObject::connect(ui->actionFreqs, &QAction::triggered, this, [this]()
	{
		freqsSettings->setProjectData(&projectData);
		freqsSettings->initializeField();
		// Показываем окно настроек частот
		freqsSettings->setWindowFlags(Qt::Window | Qt::WindowCloseButtonHint | Qt::WindowStaysOnTopHint);
		freqsSettings->show();
	});

	unitsSettings = new unitssettings;
	unitsSettings->setWindowIcon(this->windowIcon());
	QObject::connect(unitsSettings, SIGNAL(unitsSettingsWasChanged()), this, SLOT(onProjectChanges()));
	QObject::connect(ui->actionUnits, &QAction::triggered, this, [this]()
	{
		unitsSettings->setProjectData(&projectData);
		unitsSettings->initializeField();
		// Показываем окно настроек единиц
		unitsSettings->setWindowFlags(Qt::Window | Qt::WindowCloseButtonHint | Qt::WindowStaysOnTopHint);
		unitsSettings->show();
	});

	cdSettings = new CDSettings;
	cdSettings->setWindowIcon(this->windowIcon());
	QObject::connect(cdSettings, SIGNAL(cdSettingsWasChanged()), this, SLOT(onCDChanges()));
	QObject::connect(ui->actionCD, &QAction::triggered, this, [this]()
	{
		cdSettings->setProjectData(&projectData);
		cdSettings->setCDData(&cdData);
		cdSettings->initializeField();
		// Показываем окно настроек расчетной области
		cdSettings->setWindowFlags(Qt::Window | Qt::WindowCloseButtonHint | Qt::WindowStaysOnTopHint);
		cdSettings->show();
	});

	// соединение кнопок
	QObject::connect(ui->actionLoadStep, &QAction::triggered, this, [this]() // заменить позднее на загрузку модели
	{
		QMessageBox::information(this, "Kek", "Cheburek");
	});
	QObject::connect(ui->actionOpen, SIGNAL(triggered()), this, SLOT(onOpenProjectButtonClicked()));
	QObject::connect(ui->actionExit, &QAction::triggered, this, [this]() 
	{
		// Проверка на изменения
		if (projectData.hasUnsavedChanges || cdData.hasUnsavedChanges ) { // булев флаг в структуре ProjectData
			QMessageBox::StandardButton reply;
			reply = QMessageBox::question(this, tr("Сохранить изменения"),
				tr("В проект были внесены изменения. Хотите их сохранить?"),
				QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);
			if (reply == QMessageBox::Yes) {
				// Вызов метода для сохранения изменений
				saveProjectChanges();
			}
			else if (reply == QMessageBox::Cancel) {
				return; // Отменяем закрытие, если пользователь выбрал "Отмена"
			}
		}
		// Очистка полей структуры
		clearProjectData();
		// блокировка для стартового состояния
		ui->actionClose->setEnabled(false);
		ui->actionSave->setEnabled(false);
		ui->actionSaveAs->setEnabled(false);
		// проверка на изменения - хотите сохранить?
		close();
	});
	QObject::connect(ui->actionClose, &QAction::triggered, this, [this]()
	{
		// Проверка на изменения
		if (projectData.hasUnsavedChanges || cdData.hasUnsavedChanges) { // булев флаг в структуре ProjectData
			QMessageBox::StandardButton reply;
			reply = QMessageBox::question(this, tr("Сохранить изменения"),
				tr("В проект были внесены изменения. Хотите их сохранить?"),
				QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);
			if (reply == QMessageBox::Yes) {
				// Вызов метода для сохранения изменений
				saveProjectChanges();
			}
			else if (reply == QMessageBox::Cancel) {
				return; // Отменяем закрытие, если пользователь выбрал "Отмена"
			}
		}
		// Очистка полей структуры
		clearProjectData();
		// Установка стартового окна
		tabToolbar->SetCurrentTab(0);
		// Блокируем вкладки
		tabToolbar->getTabWidget()->setTabEnabled(1, false);
		tabToolbar->getTabWidget()->setTabEnabled(2, false);
		// блокировка для стартового состояния
		ui->actionClose->setEnabled(false);
		ui->actionSave->setEnabled(false);
		ui->actionSaveAs->setEnabled(false);
		// Устанавливаем заголовок окна
		setWindowTitle("DETAL CAD v.1");
	});
	QObject::connect(tabToolbar, &tt::TabToolbar::CurrentTabChanged2, this, &CMainWindow::displayMenuWidgets);



	// блокировка для стартового состояния
	ui->actionClose->setEnabled(false);
	ui->actionSave->setEnabled(false);
	ui->actionSaveAs->setEnabled(false);


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

void CMainWindow::saveProjectChanges()
{
	// удаляем *
	if(projectData.hasUnsavedChanges || cdData.hasUnsavedChanges){
		QString newTitle = this->windowTitle();
		newTitle.chop(1);
		setWindowTitle(newTitle);
	}
	if (cdData.hasUnsavedChanges) saveCDchanges();

	// Логика сохранения изменений в XML-файл
	QString filePath = currentProjectFilePath;  // путь к файлу проекта сохранен в переменной
	QFile file(filePath);
	if (!file.open(QIODevice::WriteOnly)) {
		QMessageBox::warning(this, tr("Ошибка"), tr("Не удалось сохранить изменения в проекте."));
		return;
	}

	QDomDocument document;
	QDomElement root = document.createElement("Project");
	document.appendChild(root);

	// Перезаписываем данные в XML
	QDomElement projectInfoElement = document.createElement("ProjectInfo");
	projectInfoElement.setAttribute("Name", projectData.name);
	projectInfoElement.setAttribute("Directory", projectData.directory);
	projectInfoElement.setAttribute("CreationDate", projectData.creationDate);
	projectInfoElement.setAttribute("LastModifiedDate", QDateTime::currentDateTime().toString(Qt::ISODate));  // Добавляем дату последнего изменения
	projectInfoElement.setAttribute("Author", projectData.author);
	root.appendChild(projectInfoElement);

	QDomElement unitsElement = document.createElement("Units");
	unitsElement.setAttribute("Geometry", projectData.geometryUnits);
	unitsElement.setAttribute("Frequency", projectData.frequencyUnits);
	unitsElement.setAttribute("Time", projectData.timeUnits);
	root.appendChild(unitsElement);

	QDomElement solverElement = document.createElement("Solver");
	solverElement.setAttribute("Type", projectData.solverType);
	root.appendChild(solverElement);

	QDomElement frequencyElement = document.createElement("FrequencyRange");
	frequencyElement.setAttribute("Min", projectData.freqMin);
	frequencyElement.setAttribute("Max", projectData.freqMax);
	root.appendChild(frequencyElement);

	if (projectData.useFreqStep) {
		QDomElement stepElement = document.createElement("FrequencyStep");
		stepElement.setAttribute("Value", projectData.freqStep);
		root.appendChild(stepElement);
	}
	else if (projectData.usePointsNumber) {
		QDomElement pointsElement = document.createElement("PointsNumber");
		pointsElement.setAttribute("Value", projectData.pointsNumber);
		root.appendChild(pointsElement);
	}

	QTextStream stream(&file);
	stream.setCodec("UTF-8");  // Устанавливаем кодировку UTF-8
	stream << document.toString();
	file.close();
}

void CMainWindow::saveCDchanges()
{
	// Получаем директорию, где находится файл проекта
	QDir projectDir = QFileInfo(currentProjectFilePath).absoluteDir();

	// Формируем путь до файла cdSettings.set
	QString settingsFilePath = projectDir.filePath("cdSettings.set");

	// Создаем объект QDomDocument для создания XML документа
	QDomDocument document;

	// Создаем корневой элемент CDSettings
	QDomElement root = document.createElement("CDSettings");
	document.appendChild(root);

	// Создаем элемент для координат
	QDomElement coordinatesElement = document.createElement("Coordinates");
	root.appendChild(coordinatesElement);

	// Добавляем координаты
	coordinatesElement.appendChild(document.createElement("XMin")).appendChild(document.createTextNode(QString::number(cdData.xMin)));
	coordinatesElement.appendChild(document.createElement("XMax")).appendChild(document.createTextNode(QString::number(cdData.xMax)));
	coordinatesElement.appendChild(document.createElement("YMin")).appendChild(document.createTextNode(QString::number(cdData.yMin)));
	coordinatesElement.appendChild(document.createElement("YMax")).appendChild(document.createTextNode(QString::number(cdData.yMax)));
	coordinatesElement.appendChild(document.createElement("ZMin")).appendChild(document.createTextNode(QString::number(cdData.zMin)));
	coordinatesElement.appendChild(document.createElement("ZMax")).appendChild(document.createTextNode(QString::number(cdData.zMax)));

	// Создаем элемент для частоты
	QDomElement frequencyElement = document.createElement("Frequency");
	root.appendChild(frequencyElement);

	// Добавляем значение частоты
	frequencyElement.appendChild(document.createElement("Value")).appendChild(document.createTextNode(QString::number(cdData.freqValue)));

	// Создаем элемент для дополнительных настроек
	QDomElement additionalElement = document.createElement("Additional");
	root.appendChild(additionalElement);

	// Добавляем дополнительные настройки
	additionalElement.appendChild(document.createElement("UnitsType")).appendChild(document.createTextNode(QString::number(cdData.unitsType)));
	additionalElement.appendChild(document.createElement("FreqType")).appendChild(document.createTextNode(QString::number(cdData.freqType)));
	additionalElement.appendChild(document.createElement("AllDirections")).appendChild(document.createTextNode(QString::number(cdData.allDirections)));

	// Сохранение XML в файл
	QFile file(settingsFilePath);
	if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
		QMessageBox::warning(this, tr("Ошибка"), tr("Не удалось сохранить изменения расчетной области."));
		return;
	}

	QTextStream stream(&file);
	stream.setCodec("UTF-8");
	stream << document.toString();
	file.close();
}

inline void CMainWindow::updateLabelPosition() {
	int labelWidth = backPicLabel->width();
	int labelHeight = backPicLabel->height();
	int windowWidth = this->width();
	int windowHeight = this->height();

	// Устанавливаем метку в нижний правый угол с отступом в 10 пикселей
	backPicLabel->move(windowWidth - labelWidth - 10, windowHeight - labelHeight - 10);
	backPicLabel->lower();
}

void CMainWindow::clearProjectData()
{
	// Очистка данных в структуре ProjectData
	projectData.name.clear();
	projectData.directory.clear();
	projectData.creationDate.clear();
	projectData.author.clear();
	projectData.geometryUnits.clear();
	projectData.frequencyUnits.clear();
	projectData.timeUnits.clear();
	projectData.solverType.clear();
	projectData.freqMin.clear();
	projectData.freqMax.clear();
	projectData.freqStep.clear();
	projectData.pointsNumber.clear();
	projectData.useFreqStep = false;
	projectData.usePointsNumber = false;
	projectData.hasUnsavedChanges = false; // Сбрасываем флаг изменений

	cdData.xMin = 0;
	cdData.xMax = 0;
	cdData.yMin = 0;
	cdData.yMax = 0;
	cdData.zMin = 0;
	cdData.zMax = 0;
	cdData.allDirections = 0;
	cdData.freqValue = 0;
	cdData.hasUnsavedChanges = false;
	cdData.freqType = 0;
	cdData.unitsType = 0;
}

void CMainWindow::openProject(QString filePath)
{
	QFile file(filePath);
	if (!file.open(QIODevice::ReadOnly)) {
		QMessageBox::warning(this, tr("Ошибка"), tr("Не удалось открыть файл проекта."));
		return;
	}

	QDomDocument document;
	if (!document.setContent(&file)) {
		QMessageBox::warning(this, tr("Ошибка"), tr("Не удалось разобрать XML из файла проекта."));
		file.close();
		return;
	}
	file.close();

	QDomElement root = document.documentElement();
	if (root.tagName() != "Project") {
		QMessageBox::warning(this, tr("Ошибка"), tr("Неверный формат файла проекта."));
		return;
	}
	QDomElement projectInfoElement = root.firstChildElement("ProjectInfo");
	if (!projectInfoElement.isNull()) {
		projectData.name = projectInfoElement.attribute("Name");
		projectData.directory = projectInfoElement.attribute("Directory");
		projectData.creationDate = projectInfoElement.attribute("CreationDate");
		projectData.lastModifiedDate = projectInfoElement.attribute("LastModifiedDate");
		projectData.author = projectInfoElement.attribute("Author");
	}

	QDomElement unitsElement = root.firstChildElement("Units");
	if (!unitsElement.isNull()) {
		projectData.geometryUnits = unitsElement.attribute("Geometry");
		projectData.frequencyUnits = unitsElement.attribute("Frequency");
		projectData.timeUnits = unitsElement.attribute("Time");
	}

	QDomElement solverElement = root.firstChildElement("Solver");
	if (!solverElement.isNull()) {
		projectData.solverType = solverElement.attribute("Type");
	}

	QDomElement frequencyElement = root.firstChildElement("FrequencyRange");
	if (!frequencyElement.isNull()) {
		projectData.freqMin = frequencyElement.attribute("Min");
		projectData.freqMax = frequencyElement.attribute("Max");
	}

	QDomElement stepElement = root.firstChildElement("FrequencyStep");
	if (!stepElement.isNull()) {
		projectData.freqStep = stepElement.attribute("Value");
		projectData.useFreqStep = true;
		projectData.usePointsNumber = false;
	}

	QDomElement pointsElement = root.firstChildElement("PointsNumber");
	if (!pointsElement.isNull()) {
		projectData.pointsNumber = pointsElement.attribute("Value");
		projectData.usePointsNumber = true;
		projectData.useFreqStep = false;
	}

	currentProjectFilePath = filePath;

	setWindowTitle("DETAL CAD v.1: " + projectData.name);

	tabToolbar->getTabWidget()->setTabEnabled(1, true); // разблокируем вкладки
	tabToolbar->getTabWidget()->setTabEnabled(2, true);

	// загружаем файл настроек расчетной области
	openCDsettingFile(filePath);

	// разблокируем
	ui->actionClose->setEnabled(true);
	ui->actionSave->setEnabled(true);
	ui->actionSaveAs->setEnabled(true);

	tabToolbar->SetCurrentTab(1);
}

void CMainWindow::openCDsettingFile(QString projectFilePath)
{
	// Получаем директорию, где находится файл проекта
	QDir projectDir = QFileInfo(projectFilePath).absoluteDir();

	// Формируем путь до файла cdSettings.set
	QString settingsFilePath = projectDir.filePath("cdSettings.set");

	// Открываем файл cdSettings.set
	QFile file(settingsFilePath);
	if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
		QMessageBox::warning(this, tr("Ошибка"), tr("Не удалось открыть файл настроек расчетной области."));
		return;
	}

	// Загружаем XML данные
	QDomDocument document;
	if (!document.setContent(&file)) {
		QMessageBox::warning(this, tr("Ошибка"), tr("Не удалось разобрать XML из файла настроек расчетной области."));
		file.close();
		return;
	}
	file.close();

	// Парсим XML и заполняем структуру CDData
	QDomElement root = document.documentElement();
	if (root.tagName() != "CDSettings") {
		QMessageBox::warning(this, tr("Ошибка"), tr("Неверный формат XML файла настроек расчетной области."));
		return;
	}

	QDomElement coordinatesElement = root.firstChildElement("Coordinates");
	if (!coordinatesElement.isNull()) {
		cdData.xMin = coordinatesElement.firstChildElement("XMin").text().toDouble();
		cdData.xMax = coordinatesElement.firstChildElement("XMax").text().toDouble();
		cdData.yMin = coordinatesElement.firstChildElement("YMin").text().toDouble();
		cdData.yMax = coordinatesElement.firstChildElement("YMax").text().toDouble();
		cdData.zMin = coordinatesElement.firstChildElement("ZMin").text().toDouble();
		cdData.zMax = coordinatesElement.firstChildElement("ZMax").text().toDouble();
	}

	QDomElement frequencyElement = root.firstChildElement("Frequency");
	if (!frequencyElement.isNull()) {
		cdData.freqValue = frequencyElement.firstChildElement("Value").text().toDouble();
	}

	QDomElement additionalElement = root.firstChildElement("Additional");
	if (!additionalElement.isNull()) {
		cdData.unitsType = additionalElement.firstChildElement("UnitsType").text().toUShort();
		cdData.freqType = additionalElement.firstChildElement("FreqType").text().toUShort();
		cdData.allDirections = additionalElement.firstChildElement("AllDirections").text().toUShort();
	}
}

// функция создания стартового окна - необходимо переделать
void CMainWindow::createFirstTab()
{
	prCreator = new ProjectCreator;
	connect(prCreator, SIGNAL(projectIsReady(QString)), this, SLOT(openProject(QString)));

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
	createProjectButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

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
	latestListWidget->addItem("Project 6 - C:/Projects/Project5/project5.proj");
	latestListWidget->addItem("Project 7 - C:/Projects/Project5/project5.proj");
	latestListWidget->addItem("Project 8 - C:/Projects/Project5/project5.proj");
	latestListWidget->addItem("Project 9 - C:/Projects/Project5/project5.proj");
	latestListWidget->addItem("Project 10 - C:/Projects/Project5/project5.proj");
	latestListWidget->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
	latestListWidget->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
	latestListWidget->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
	latestListWidget->setMaximumHeight(350);
	latestListWidget->setMinimumHeight(140);

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
	examplesListWidget->addItem("Example 6.proj");
	examplesListWidget->addItem("Example 7.proj");
	examplesListWidget->addItem("Example 8.proj");
	examplesListWidget->addItem("Example 9.proj");
	examplesListWidget->addItem("Example 10.proj");
	examplesListWidget->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
	examplesListWidget->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
	examplesListWidget->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
	examplesListWidget->setMaximumHeight(350);
	examplesListWidget->setMinimumHeight(140);

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
		"    border: 1px solid #dcdcdc;"          // Устанавливаем границы
		"    background-color: transparent;"      // Прозрачный фон
		"    text-align: left;"                   // Выровнять текст по левому краю
		"}"
		"QPushButton:hover {"
		"    background-color: #dcdcdc;"          // Серый фон при наведении
		"}"
	);
	guideButton->setText("Руководство пользователя");
	//guideButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);
	guideButton->setMaximumWidth(300);
	guideButton->setMinimumWidth(300);
	rightVertLayout->addWidget(guideButton);

	aboutButton = new QPushButton(centralwidgetMenu);
	aboutButton->setObjectName(QString::fromUtf8("aboutButton"));
	aboutButton->setIcon(QIcon(":/icons/icons/Help-browser.png"));
	aboutButton->setIconSize(QSize(36, 36));
	aboutButton->setStyleSheet(
		"QPushButton {"
		"    border: 1px solid #dcdcdc;"          // Устанавливаем границы
		"    background-color: transparent;"      // Прозрачный фон
		"    text-align: left;"                   // Выровнять текст по левому краю
		"}"
		"QPushButton:hover {"
		"    background-color: #dcdcdc;"          // Серый фон при наведении
		"}"
	);
	aboutButton->setText("О программе");
	aboutButton->setMaximumWidth(300);
	aboutButton->setMinimumWidth(300);
	//aboutButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);

	rightVertLayout->addWidget(aboutButton);

	infoLabel = new QLabel(centralwidgetMenu);
	infoLabel->setObjectName(QString::fromUtf8("infoLabel"));
	infoLabel->setText("Информация:");
	infoLabel->setStyleSheet("font-family: Arial; font-size: 16px; font-weight: bold; color: #333333;");

	rightVertLayout->addWidget(infoLabel);

	infoTextBrowser = new QTextBrowser(centralwidgetMenu);
	infoTextBrowser->setObjectName(QString::fromUtf8("infoTextBrowser"));
	infoTextBrowser->setStyleSheet(QString::fromUtf8("background: transparent; border: none;"));
	infoTextBrowser->setPlainText("Проект: Project 1\n"
		"Расположение: C:/Projects/Project1/project1.proj\n"
		"Дата создания: 12 января 2024 г.\n"
		"Последнее изменение: 15 марта 2024 г.\n"
		"Автор: Иван Иванов\n"
		"Описание:\n"
		"Этот проект включает в себя разработку и симуляцию фазированных антенных решеток. Основные цели проекта включают оптимизацию структуры решеток для повышения эффективности и уменьшения помех");
	infoTextBrowser->setMaximumWidth(300);
	infoTextBrowser->setMinimumWidth(300);
	//infoTextBrowser->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);

	rightVertLayout->addWidget(infoTextBrowser);

	mainHorLayout->addLayout(rightVertLayout);
	QLabel* emptyHorLabel = new QLabel("");
	emptyHorLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
	mainHorLayout->addWidget(emptyHorLabel);

	verticalLayout_5->addLayout(mainHorLayout);

	QLabel* emptyVertLabel = new QLabel("");
	emptyVertLabel->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::MinimumExpanding);
	verticalLayout_5->addWidget(emptyVertLabel);

	backPicLabel = new QLabel(" ", this);
	backPicLabel->setMinimumWidth(300);
	backPicLabel->setMinimumHeight(163);
	backPicLabel->setStyleSheet("background-image: url(:/icons/icons/back.png);"
		"background-position: bottom right;"
		"background-repeat: no-repeat;");
	updateLabelPosition();


	centralwidgetMenu->hide();
}
void CMainWindow::onCreateProjectButtonClicked() {
	prCreator->setWindowIcon(this->windowIcon());
	prCreator->setWindowFlags(prCreator->windowFlags() | Qt::WindowStaysOnTopHint);
	prCreator->show();
}

void CMainWindow::onOpenProjectButtonClicked()
{
	QString filePath = QFileDialog::getOpenFileName(this, tr("Открыть проект"), "", tr("Project Files (*.proj)"));
	if (filePath.isEmpty())
		return;
	openProject(filePath);
}

void CMainWindow::onProjectChanges()
{
	//projectData.hasUnsavedChanges = true; изменение флага есть в классах настроек
	QString currentTitle = this->windowTitle();
	if (!currentTitle.endsWith("*")) {
		setWindowTitle(currentTitle + "*");
	}
}
void CMainWindow::onCDChanges() {
	// вызов метода обновления расчетной области (3д куб)
	//cdData.hasUnsavedChanges = true; изменение флага есть в классе настроек
	onProjectChanges();
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