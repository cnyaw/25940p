
#pragma once

#define GOOD_SUPPORT_GDIPLUS_IMG
#include "gx/img.h"

#include "game.h"
#include "app.h"

int const FPS = 60;

class CMainFrame : public CFrameWindowImpl<CMainFrame>, public CMessageFilter, public CIdleHandler, public COpenGL<CMainFrame>
{
public:
  DECLARE_FRAME_WND_CLASS(NULL, IDR_MAINFRAME)

  MyApp app;
  sw2::FpsHelper fps;

  virtual BOOL PreTranslateMessage(MSG* pMsg)
  {
    return CFrameWindowImpl<CMainFrame>::PreTranslateMessage(pMsg);
  }

  virtual BOOL OnIdle()
  {
    //
    // Keys.
    //

    int keys = 0;

    if (::GetKeyState(VK_UP) & 0x8000) {
      keys |= MyGame::KEY_UP;
    } else if (::GetKeyState(VK_DOWN) & 0x8000) {
      keys |= MyGame::KEY_DOWN;
    }

    if (::GetKeyState(VK_LEFT) & 0x8000) {
      keys |= MyGame::KEY_LEFT;
    } else if (::GetKeyState(VK_RIGHT) & 0x8000) {
      keys |= MyGame::KEY_RIGHT;
    }

    if (::GetKeyState(VK_RETURN) & 0x8000) {
      keys |= MyGame::KEY_SELECT;
    }

    if (::GetKeyState(VK_ESCAPE) & 0x8000) {
      keys |= MyGame::KEY_CANCEL;
    }

    if (::GetKeyState('Z') & 0x8000) {
      keys |= MyGame::KEY_FIRE;
    }

    //
    // Update.
    //

    if (GetFocus() == m_hWnd) {

      float const et = 1 / (float)FPS;
      app.update(et, keys);

      if (app.isQuit()) {
        PostMessage(WM_CLOSE);
        return TRUE;
      }

      RedrawWindow();
    }

    fps.wait();

    return FALSE;
  }

  BEGIN_MSG_MAP_EX(CMainFrame)
    MSG_WM_CREATE(OnCreate)
    CHAIN_MSG_MAP(CFrameWindowImpl<CMainFrame>)
    CHAIN_MSG_MAP(COpenGL<CMainFrame>)
  END_MSG_MAP()

  //
  // Message handler.
  //

  int OnCreate(LPCREATESTRUCT lpCreateStruct)
  {
    ::srand(::GetTickCount());
    fps.start(FPS);

    //
    // Register object for message filtering and idle updates.
    //

    CMessageLoop* pLoop = _Module.GetMessageLoop();
    ATLASSERT(pLoop != NULL);
    pLoop->AddMessageFilter(this);
    pLoop->AddIdleHandler(this);

    if (!app.init()) {
      MessageBox(
        _T("load script failed"),
        CString((LPCTSTR)IDR_MAINFRAME),
        MB_OK|MB_ICONERROR);
    }

    ResizeClient(app.viewWidth, app.viewHeight);

    SetMsgHandled(FALSE);
    return 0;
  }

  //
  // COpenGL<>.
  //

  void OnInit(void)
  {
    good::gx::GxImage img;
    if (img.load("res/font.bmp")) {
      img.convert32();
      img.flip();
    }
    app.initGL(img);
  }

  void OnRender(void)
  {
    app.render();
  }

  void OnResize(int width, int height)
  {
    app.resize(width, height);
  }
};
