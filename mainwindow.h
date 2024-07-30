/*
    TabToolbar - a small utility library for Qt, providing tabbed toolbars
	Copyright (C) 2018 Oleksii Sierov
	
    TabToolbar is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    TabToolbar is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with TabToolbar.  If not, see <http://www.gnu.org/licenses/>.
*/
#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include <TabToolbar/TabToolbar.h>
#include <TabToolbar/Page.h>
#include <TabToolbar/Group.h>
#include <TabToolbar/SubGroup.h>
#include <TabToolbar/StyleTools.h>
#include <TabToolbar/Builder.h>

// OCCT TEST STEP reading
#include <AIS_Shape.hxx>
#include <NCollection_DataMap.hxx>
#include <Prs3d_LineAspect.hxx>
#include <Prs3d_ShadingAspect.hxx>
#include <STEPCAFControl_Reader.hxx>
#include <TDataStd_Name.hxx>
#include <TDataStd_TreeNode.hxx>
#include <TDF_AttributeIterator.hxx>
#include <TDF_ChildIterator.hxx>
#include <TDF_Tool.hxx>
#include <TDocStd_Document.hxx>
#include <TopExp_Explorer.hxx>
#include <XCAFDoc_Location.hxx>
#include <XCAFDoc_Color.hxx>
#include <XCAFDoc_ColorTool.hxx>
#include <XCAFDoc_DocumentTool.hxx>
#include <XCAFDoc_LayerTool.hxx>
#include <XCAFDoc_MaterialTool.hxx>
#include <XCAFDoc_ShapeTool.hxx>
#include <AIS_ListOfInteractive.hxx>

#include <AIS_Shape.hxx>
#include <AIS_InteractiveContext.hxx>
#include <Prs3d_ShadingAspect.hxx>

#include <QStandardItemModel>


// VTK MOMENTS
#include <vtkSmartPointer.h>
#include <vtkPolyData.h>
#include <vtkPoints.h>
#include <vtkCellArray.h>
#include <vtkPolyDataMapper.h>
#include <vtkActor.h>
#include <vtkRenderer.h>
#include <TopoDS_Shape.hxx>
#include <TopExp_Explorer.hxx>
#include <TopoDS_Vertex.hxx>
#include <TopoDS_Edge.hxx>
#include <TopoDS_Face.hxx>
#include <BRep_Tool.hxx>
#include <BRepMesh_IncrementalMesh.hxx>
#include <BRepTools.hxx>
#include <BRepBuilderAPI.hxx>
#include <BRepAdaptor_Curve.hxx>
#include <gp_Pnt.hxx>
#include <TopoDS.hxx>


#include <vtkUnsignedCharArray.h>
#include <vtkPointData.h>
#include <Quantity_ColorRGBA.hxx>
#include <XCAFDoc_ColorTool.hxx>


#include "projectcreator.h"

namespace Ui {
class MainWindow;
}

class TTBuilder;
struct InternalData
{
	Handle(AIS_InteractiveObject) parent;
	TCollection_ExtendedString name;
	TopoDS_Shape shape;
	TopLoc_Location location;
	Standard_Boolean hasColors[XCAFDoc_ColorCurv + 1];
	Quantity_ColorRGBA colors[XCAFDoc_ColorCurv + 1];
};
class CMainWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit CMainWindow(QWidget* parent = 0);
    ~CMainWindow();
	void addShape(const TDF_Label &label, Handle(AIS_InteractiveObject) parent = nullptr);
	void addShape(const InternalData &data, Handle(AIS_Shape) &object);
	void load();
	void addToObjectsTree(const Handle(AIS_InteractiveObject) &obj,
		QStandardItem *parent = nullptr);
	TCollection_ExtendedString name(const Handle(AIS_InteractiveObject) &object) const;
	AIS_ListOfInteractive result() const;
	vtkSmartPointer<vtkPolyData> ConvertTopoDS_ShapeToPolyData(const TopoDS_Shape& shape);
	void AddShapeToVTKRenderer(vtkSmartPointer<vtkRenderer> renderer, const vtkSmartPointer<vtkPolyData>& polyData);
public slots:
	void displayMenuWidgets(int a);
	void onCreateProjectButtonClicked();
private:
	Handle(XCAFDoc_ShapeTool) shapeTool;
	Handle(XCAFDoc_ColorTool) colorTool;
	Handle(XCAFDoc_LayerTool) layerTool;
	Handle(XCAFDoc_MaterialTool) materialTool;
	typedef NCollection_DataMap<Handle(AIS_InteractiveObject), InternalData> InternalMap;
	InternalMap objects;
	QStandardItemModel *objectsModel;

    Ui::MainWindow* ui;
	tt::TabToolbar* tabToolbar;
    TTBuilder* ttb;
	vtkSmartPointer<vtkRenderer> mRenderer;
	
	QWidget *centralwidgetMenu;
	ProjectCreator* prCreator;
	void createFirstTab();
};

#endif // MAINWINDOW_H
