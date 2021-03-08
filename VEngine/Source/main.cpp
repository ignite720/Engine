#include "..\EditorMainWindow.h"
#include "RenderSystem.h"
#include "CoreSystem.h"
#include "UISystem.h"
#include "Obj.h"
#include "Camera.h"
#include "Audio.h"
#include "AudioSystem.h"
#include "Input.h"
#include "Actor.h"
#include "ShaderFactory.h"
#include "DebugMenu.h"
#include "Raycast.h"
#include "World.h"
#include "FileSystem.h"
#include "Debug.h"
#include "FBXImporter.h"
#include "WorldEditor.h"
#include "TimerSystem.h"
#include "MathHelpers.h"
#include "Console.h"
#include "Profiler.h"
#include "TestActor.h"
#include "ConsoleDock.h"
#include "RenderViewWidget.h"
#include "WorldDock.h"

//For throwing the program into fullscreen for profilers, getting rid of Qt
#define NO_EDITOR

int main(int argc, char *argv[])
{
    HR(CoInitialize(NULL)); //For the WIC functions

    //Qt setup
    //TODO: this is junk. Think about making an LevelEditor class or some bullshit so that the constructors
    //can call this and you don't need the weird global pointers and these two going out of scope.
    //You want to pass in argc to the constructor too.
    QApplication qApplication(argc, nullptr);
    EditorMainWindow editorMainWindow;
    gEditorMainWindow = &editorMainWindow;
    gQApplication = &qApplication;

#ifndef NO_EDITOR
    //QApplication qApplication(argc, argv);
#endif //PROFILE_NOEDITOR
    gProfiler.Init();

    //FBX setup
    FBXImporter::Init();

    SetActiveCamera(&editorCamera);
    editorCamera.bEditorCamera = true;

    //Systems setup
    gCoreSystem.SetTimerFrequency();
#ifdef PROFILE_NOEDITOR
    gRenderSystem->Init(gCoreSystem.mainWindow);
#else
    gRenderSystem.Init((HWND)gEditorMainWindow->renderViewWidget->winId());
#endif
    gAudioSystem.Init();
    gUISystem.Init();
    gWorldEditor.Init();

    //Qt late init
    gEditorMainWindow->worldDock->PopulateWorldList();


    ActorSystem ac;
    ac.modelName = "cube.fbx";
    ac.shaderName = L"shaders.hlsl";
    ac.textureName = L"texture.png";
    ac.CreateActors<Actor>(&gRenderSystem, 1);


    GetWorld()->AddActorSystem(ac);

    gRenderSystem.Flush();
    gRenderSystem.WaitForPreviousFrame();

    //MAIN LOOP
    while (gCoreSystem.bMainLoop)
    {
        const float deltaTime = gCoreSystem.deltaTime;

        gCoreSystem.StartTimer();
        gCoreSystem.HandleMessages();

        gQApplication->processEvents();
        gEditorMainWindow->Tick();

        gFileSystem.Tick();
        gUISystem.Tick(gEditorMainWindow);

        gTimerSystem.Tick(deltaTime);

        GetActiveCamera()->Tick(deltaTime);

        gRenderSystem.Tick();
        gRenderSystem.RenderSetup(deltaTime);

        gWorldEditor.Tick(nullptr, gEditorMainWindow);

        gRenderSystem.Render(deltaTime);
        gRenderSystem.RenderEnd(deltaTime);

        //UI RENDERING
        if (gUISystem.bAllUIActive)
        {
            //gUISystem.d2dRenderTarget->BeginDraw();
            gConsole.Tick();
            gConsole.DrawViewItems();
            debugMenu.Tick(GetWorld(), deltaTime);
            //gUISystem.RenderAllUIViews();
            //gUISystem.d2dRenderTarget->EndDraw();
        }

        gRenderSystem.Flush();

        //PRESENT
        gRenderSystem.Present();

        gRenderSystem.WaitForPreviousFrame();

        gInputSystem.InputReset();
        gProfiler.Clean();

        gCoreSystem.EndTimer();
    }

    gUISystem.Cleanup();
    qApp->quit();

    return 0;
}
