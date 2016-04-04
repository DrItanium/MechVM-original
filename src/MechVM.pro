HEADERS = \
abstraction.h \
Archive.h \
BGBase.h \
BGString.h \
BGVector.h \
BipedMech.h \
Color.h \
Config.h \
dialogs.h \
Database_MW2.h \
ExceptionBase.h \
FileCache.h \
FramerateCounter.h \
GLButton.h \
GLComboBox.h \
GLContextMenu.h \
GLLabel.h \
GLLineEdit.h \
GLScrollbar.h \
GLSlider.h \
GLSplitter.h \
GLTableView.h \
GLWindowContainer.h \
GLWindow.h \
Heightfield.h \
intersections.h \
LZdecode.h \
Matrix.h \
MechVM.h \
MechVM.h \
MechLab.h \
MechShell.h \
MechSim.h \
MechWarriorInstallers.h \
MechWarriorIIPRJ.h \
MechWarrior3ZBD.h \
Mesh.h \
MeshPolygon.h \
Mesh2.h \
MWBase.h \
MW2MechImporter.h \
Point3D.h \
QuadMech.h \
random.h \
RenderableObject.h \
Texture.h \
TextureCompiler.h \
Toolbar.h \
VehicleAI.h \
Vehicles.h \
XMLTree.h

SOURCES = \
Archive.cpp \
BGBase.cpp \
BGString.cpp \
BipedMech.cpp \
Color.cpp \
Config.cpp \
Database_MW2.cpp \
dialogs.cpp \
FileCache.cpp \
FramerateCounter.cpp \
GLButton.cpp \
GLComboBox.cpp \
GLContextMenu.cpp \
GLLabel.cpp \
GLLineEdit.cpp \
GLScrollbar.cpp \
GLSlider.cpp \
GLSplitter.cpp \
GLTableView.cpp \
GLWindowContainer.cpp \
GLWindow.cpp \
Heightfield.cpp \
intersections.cpp \
LZdecode.cpp \
Matrix.cpp \
MechVM.cpp \
MechLab.cpp \
MechShell.cpp \
MechSim.cpp \
MechWarriorInstallers.cpp \
MechWarriorIIPRJ.cpp \
MechWarrior3ZBD.cpp \
Mesh.cpp \
MeshPolygon.cpp \
Mesh2.cpp \
MWBase.cpp \
MW2MechImporter.cpp \
Point3D.cpp \
QuadMech.cpp \
RenderableObject.cpp \
Texture.cpp \
TextureCompiler.cpp \
Toolbar.cpp \
VehicleAI.cpp \
Vehicles.cpp \
XMLTree.cpp

TARGET = ../mechvm

INCLUDEPATH = /opt/gnome/include/gtk-1.2 /opt/gnome/include/glib-2.0 /opt/gnome/include/glib-1.2 /opt/gnome/lib/glib-2.0/include /usr/include/SDL

LIBS	+= `pkg-config --cflags --libs glu sdl freeglut`

QMAKE_CXXFLAGS += -std=c++11
QMAKE_CFLAGS += -std=c99
