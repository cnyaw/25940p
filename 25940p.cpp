
#include "stdafx.h"

#include "resource.h"

#include "swArchive.h"
#include "swStageStack.h"
#include "swGeometry.h"

#include "../stge/stge/stge.h"
#include "../stge/stge/par_yard.h"

//#define LOAD_SCRIPT_FROM_SCRIPT_CODE

#ifdef LOAD_SCRIPT_FROM_SCRIPT_CODE
#include "p25940.cpp"                   // Generate p25940.cpp from 25940.stge by stge::CodeGenerator.
#else
#include "p25940.c"                     // Generate p25940.c from p25940.zip(25940.stge) as uchar array P25940_MOD by bin2c tool.
#endif

#include "AtlOpengl.h"
#include "MainFrm.h"

CAppModule _Module;

int Run(LPTSTR /*lpstrCmdLine*/ = NULL, int nCmdShow = SW_SHOWDEFAULT)
{
    CGLMessageLoop theLoop;
    _Module.AddMessageLoop(&theLoop);

    CMainFrame wndMain;

    if(wndMain.CreateEx() == NULL)
    {
        ATLTRACE(_T("Main window creation failed!\n"));
        return 0;
    }

    wndMain.ShowWindow(nCmdShow);

    timeBeginPeriod(1) ;
    int nRet = theLoop.Run();
    timeEndPeriod(1) ;

    _Module.RemoveMessageLoop();
    return nRet;
}

int WINAPI _tWinMain(HINSTANCE hInstance, HINSTANCE /*hPrevInstance*/, LPTSTR lpstrCmdLine, int nCmdShow)
{
    HRESULT hRes = ::CoInitialize(NULL);
// If you are running on NT 4.0 or higher you can use the following call instead to
// make the EXE free threaded. This means that calls come in on a random RPC thread.
//    HRESULT hRes = ::CoInitializeEx(NULL, COINIT_MULTITHREADED);
    ATLASSERT(SUCCEEDED(hRes));

    // this resolves ATL window thunking problem when Microsoft Layer for Unicode (MSLU) is used
    ::DefWindowProc(NULL, 0, 0, 0L);

    AtlInitCommonControls(ICC_BAR_CLASSES);    // add flags to support other controls

    hRes = _Module.Init(NULL, hInstance);
    ATLASSERT(SUCCEEDED(hRes));

    int nRet = Run(lpstrCmdLine, nCmdShow);

    _Module.Term();
    ::CoUninitialize();

    return nRet;
}
