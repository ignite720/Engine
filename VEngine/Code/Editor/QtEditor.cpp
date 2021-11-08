#include "QtEditor.h"
#include <QtWidgets/QApplication>
#include <QWidget>
#include <QFont>
#include <qgridlayout.h>
#include <qscrollarea.h>
#include <qpushbutton.h>
#include "EditorMainWindow.h"
#include "Input.h"
#include "RenderViewWidget.h"
#include "LogDock.h"
#include "PropertiesDock.h"
#include "AssetDock.h"
#include "WorldDock.h"
#include "SystemDock.h"
#include "Render/Material.h"
#include "Serialiser.h"
#include "Core.h"
#include "Render/MaterialSystem.h"

void QtEditor::Init(int argc, char* argv[])
{
    app = new QApplication(argc, argv); //Init order matters here. Qt wants a QApp before a QMainWindow/QWidget
    mainWindow = new EditorMainWindow();
    windowHwnd = (void*)mainWindow->renderView->winId();

    app->setWindowIcon(QIcon("Icons/engine_icon.png"));

    viewportWidth = mainWindow->renderView->size().width();
    viewportHeight = mainWindow->renderView->size().height();

    SetEditorFont();
    EnableDarkMode();
}

void QtEditor::Tick()
{
    mainWindow->Tick();

    app->processEvents();
    SetMousePos();

    //update property dock values if game is running
    if (Core::gameplayOn)
    {
        mainWindow->propertiesDock->ResetPropertyWidgetValues();
    }

    if (Input::GetKeyUp(Keys::F11))
    {
        mainWindow->HideAllDocks();
    }
}

void QtEditor::SetMousePos()
{
    QPoint mousePos = mainWindow->renderView->mapFromGlobal(QCursor::pos());
    viewportMouseX = mousePos.x();
    viewportMouseY = mousePos.y();
}

void QtEditor::Log(const std::wstring logMessage, ...)
{
    va_list args;
    va_start(args, logMessage);

    wchar_t msg[1024];
    _vsnwprintf(msg, 1024, logMessage.c_str(), args);
    va_end(args);

    mainWindow->logDock->Print(msg);
}

//TODO: C++'s std::format is good, but it's fucked up currently.
//Soon as it's green, think about replacing va_args here and in other spots.
void QtEditor::Log(const std::string logMessage, ...)
{
    va_list args;
    va_start(args, logMessage);

    char msg[1024];
    vsnprintf(msg, 1024, logMessage.c_str(), args);
    va_end(args);

    mainWindow->logDock->Print(msg);
}

void QtEditor::SetActorProps(Actor* actor)
{
    mainWindow->propertiesDock->DisplayActorProperties(actor);
}

void QtEditor::UpdateWorldList()
{
    mainWindow->worldDock->PopulateWorldActorList();
    mainWindow->systemDock->PopulateSystemLists();
}

void QtEditor::RefreshAssetList()
{
    mainWindow->assetDock->AssetFolderClicked();
}

void QtEditor::ClearProperties()
{
    mainWindow->propertiesDock->Clear();
}

void QtEditor::SetEditorFont()
{
    //Capcom have a cool talk on UI design and fonts for RE Engine
    //REF:https://www.youtube.com/watch?v=YhnIW2XY_wU
    QFont font;
    font.setStyleHint(QFont::SansSerif);
    font.setPixelSize(16);
    app->setFont(font);
}

void QtEditor::EnableDarkMode()
{
    //Stole all the scrollbar stuff from https://stackoverflow.com/questions/54595957/how-to-set-a-stylesheet-for-the-qscrollbar-in-a-qscrollarea
    app->setStyleSheet(
        "QMainWindow { background-color: rgb(255, 0, 0); border: none; } "

        "QDockWidget { background-color: rgb(37, 37, 37); border:0px; } "

        "QWidget { background-color: rgb(45, 45, 45); border-color: rgb(11, 11, 11); border-width: 1.2px; border-style: inset; } "

        "QPushButton { min-height:30px; min-width:50px; background-color: rgb(53, 53, 53); border-radius: 5px; } "
        "QPushButton:hover { background-color: rgb(80, 80, 80); } "
        "QPushButton:pressed { background-color: rgb(140, 140, 140); } "

        "QHeaderView { color: rgb(210, 210, 210); border: 0px; } "
        "QHeaderView:section { background-color: rgb(37, 37, 37); } "

        "QTabWidget { border: none; } "
        "QTabBar::tab { background: rgb(66, 66, 66); } "

        "QScrollBar::add-page:vertical, QScrollBar::sub-page:vertical { background: none;  border: 0px; } "
        "QScrollBar:vertical { background-color: #2A2929; width: 15px; margin: 15px 3px 15px 3px; border: 1px transparent #2A2929; border-radius: 4px } "
        "QScrollBar::handle:vertical { background-color: rgb(11, 11, 11); min-height: 5px; border-radius: 4px; } "
        "QScrollBar::sub-line:vertical { margin: 3px 0px 3px 0px; height: 10px; border-image: url(:/qss_icons/rc/up_arrow_disabled.png); width: 10px; subcontrol-position: top; subcontrol-origin: margin; } "
        "QScrollBar::add-line:vertical { margin: 3px 0px 3px 0px; border-image: url(:/qss_icons/rc/down_arrow_disabled.png); height: 10px;width: 10px; subcontrol-position: bottom; subcontrol-origin: margin; } "

        "QScrollBar::add-page:horizontal, QScrollBar::sub-page:horizontal { background: none;  border: 0px; } "
        "QScrollBar:horizontal { height: 15px; margin: 3px 15px 3px 15px; border: 1px transparent #2A2929; border-radius: 4px; } "
        "QScrollBar::handle:horizontal { background-color: rgb(11, 11, 11); min-width: 5px; border-radius: 4px; } "
        "QScrollBar::add-line:horizontal{ margin: 0px 3px 0px 3px; border-image: url(:/qss_icons/rc/right_arrow_disabled.png); width: 10px; height: 10px; subcontrol - position: right; subcontrol - origin: margin; } "
        "QScrollBar::sub-line:horizontal { margin: 0px 3px 0px 3px; border-image: url(:/qss_icons/rc/left_arrow_disabled.png); height: 10px; width: 10px; subcontrol-position: left; subcontrol-origin: margin; } "
    );

    //Set global font to  off white
    QPalette p = app->palette();
    QColor fontColour = QColor::fromRgb(215, 215, 215);
    p.setColor(QPalette::Text, fontColour);
    p.setColor(QPalette::WindowText, fontColour);
    p.setColor(QPalette::ButtonText, fontColour);
    p.setColor(QPalette::BrightText, fontColour);
    app->setPalette(p);
}

void QtEditor::SelectActorInWorldList()
{
    mainWindow->worldDock->SelectActorInList();
}
