set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

find_package(Qt5Core       5.3 REQUIRED)
find_package(Qt5Gui        5.3 REQUIRED)
find_package(Qt5Widgets    5.3 REQUIRED)
find_package(Qt5OpenGL    5.3 REQUIRED)
find_package(Qt5Xml    5.3 REQUIRED)

find_package(OpenCASCADE REQUIRED PATHS ${OpenCASCADE_DIR} NO_DEFAULT_PATH)
include_directories(${OpenCASCADE_INCLUDE_DIR})

find_package(VTK COMPONENTS 
  vtkCommonColor
  vtkCommonCore
  vtkFiltersSources
  vtkCommonTransforms
  vtkCommonDataModel
  vtkInteractionStyle
  vtkInteractionWidgets
  vtkRenderingAnnotation
  vtkRenderingContextOpenGL2
  vtkRenderingCore
  vtkRenderingFreeType
  vtkRenderingGL2PSOpenGL2
  vtkRenderingOpenGL2
  vtkGUISupportQtOpenGL
  vtkGUISupportQt
  vtkRenderingQt
)

if (NOT VTK_FOUND)
  message(FATAL_ERROR "Cube: Unable to find the VTK build folder.")
endif()

set(SRCROOT ${PROJECT_SOURCE_DIR}/examples/Test)

set(SRC_LIST
    ${SRCROOT}/main.cpp
    ${SRCROOT}/mainwindow.cpp
    ${SRCROOT}/mainwindow.h
	${SRCROOT}/projectcreator.cpp
	${SRCROOT}/projectcreator.h
)

set(UIS
    ${SRCROOT}/mainwindow.ui
	${SRCROOT}/projectcreator.ui
)

set(RES
    ${SRCROOT}/res.qrc
)

add_executable(Test
    ${SRC_LIST}
    ${UIS}
    ${RES}
)

target_link_libraries(Test
    TabToolbar
    Qt5::Core
    Qt5::Gui
    Qt5::Widgets
	Qt5::OpenGL
	Qt5::Xml
	${VTK_LIBRARIES}
	${OpenCASCADE_LIBRARIES}
)

set_target_properties(Test PROPERTIES DEBUG_POSTFIX d)
set_target_properties(Test PROPERTIES FOLDER "Examples")

install(TARGETS Test
        RUNTIME DESTINATION ${INSTALL_MISC_DIR}/examples/Test COMPONENT examples
        BUNDLE DESTINATION ${INSTALL_MISC_DIR}/examples/Test COMPONENT examples)
        
install(FILES ${SRC_LIST} ${UIS} ${RES}
        DESTINATION ${INSTALL_MISC_DIR}/examples/Test
        COMPONENT examples)

install(DIRECTORY "${CMAKE_SOURCE_DIR}/examples/Test/icons"
        DESTINATION ${INSTALL_MISC_DIR}/examples/Test
        COMPONENT examples)
