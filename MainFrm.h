
#pragma once

#include "game25940p.h"

int const FPS = 60;

class CMainFrame : public CFrameWindowImpl<CMainFrame>, public CMessageFilter, public CIdleHandler, public COpenGL<CMainFrame>
{
public:
  DECLARE_FRAME_WND_CLASS(NULL, IDR_MAINFRAME)

  float d;

  int viewWidth, viewHeight;
  float hWidth, hHeight;

  MyGame g;

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
      g.update(et, keys);

      if (g.mQuit) {
        PostMessage(WM_CLOSE);
        return TRUE;
      }

      RedrawWindow();
    }

    ::Sleep(timeLeft());

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

    //
    // Register object for message filtering and idle updates.
    //

    CMessageLoop* pLoop = _Module.GetMessageLoop();
    ATLASSERT(pLoop != NULL);
    pLoop->AddMessageFilter(this);
    pLoop->AddIdleHandler(this);

    viewWidth = viewHeight = 400;
    ResizeClient(viewWidth, viewHeight);

    if (!g.init(viewWidth, viewHeight)) {
      MessageBox(
        _T("load script failed"),
        CString((LPCTSTR)IDR_MAINFRAME),
        MB_OK|MB_ICONERROR);
    }

    SetMsgHandled(FALSE);
    return 0;
  }

  //
  // COpenGL<>.
  //

  void OnInit(void)
  {
    glShadeModel(GL_SMOOTH);            // Enable Smooth Shading.
    glClearColor(0.0f, 0.0f, 0.0f, 0.5f); // Black Background.
    glClearDepth(1.0f);                 // Depth Buffer Setup.
    glEnable(GL_DEPTH_TEST);            // Enables Depth Testing.
    glDepthFunc(GL_LEQUAL);             // The Type Of Depth Testing To Do.
    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST); // Really Nice Perspective Calculations.

    //
    // Load font texture.
    //

    {
      CBitmap bmFont;
      bmFont.LoadBitmap(IDB_BITMAP1);

      CClientDC dc(m_hWnd);

      BITMAPINFO bmi = {0};
      bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
      bmi.bmiHeader.biWidth = 256;
      bmi.bmiHeader.biHeight = 256;
      bmi.bmiHeader.biPlanes = 1;
      bmi.bmiHeader.biBitCount = 24;
      bmi.bmiHeader.biCompression = BI_RGB;

      unsigned char* pPixel;

      CBitmap bmTmp;
      bmTmp = ::CreateDIBSection(dc, &bmi, DIB_RGB_COLORS, (void**)&pPixel, NULL, 0);

      CDC memdc1, memdc2;

      memdc1.CreateCompatibleDC(dc);
      memdc1.SelectBitmap(bmFont);

      memdc2.CreateCompatibleDC(dc);
      memdc2.SelectBitmap(bmTmp);

      memdc2.BitBlt(0, 0, 256, 256, memdc1, 0, 0, SRCCOPY);

      GLuint texture = 0;
      glGenTextures(1, &texture);
      glBindTexture(GL_TEXTURE_2D, texture);
      glTexImage2D(GL_TEXTURE_2D, 0, 3, 256, 256, 0, GL_RGB, GL_UNSIGNED_BYTE, pPixel);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    }
  }

  void OnRender(void)
  {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);  // Clear Screen And Depth Buffer

    float bdOffsetX = .0f, bdOffsetY = .0f;

    if (g.mPlaying && .0f < g.player.mShakeTime) {
      bdOffsetX = sw2::rangeRand(-3.0f, 3.0f);
      bdOffsetY = sw2::rangeRand(-3.0f, 3.0f);
    }

    //
    // Draw bounding.
    //

    glLoadIdentity();
    glTranslatef(bdOffsetX, bdOffsetY, -d);
    glScalef(viewWidth/2.0f, viewHeight/2.0f, 1);
    glColor3f(0.5f, 0.5f, 0.5f);
    drawRect();

    //
    // Draw stars.
    //

    float offsetx = g.starx / 200.0f;
    float offsety = g.stary / 200.0f;

    for (int i = g.om2.objects.first(); -1 != i; i = g.om2.objects.next(i)) {

      stge::Object const& o = g.om2.objects[i];

      float xx = offsetx * (70.0f - 22.0f * o.user[0]);
      float yy = offsety * (70.0f - 22.0f * o.user[0]);
      float x = o.x - xx, y = o.y + yy;

      if (200.0f < x) {
        x -= 400.0f;
      } else if (-200.0f > x) {
        x += 400.0f;
      }

      if (200.0f < y) {
        y -= 400.0f;
      } else if (-200.0f > y) {
        y += 400.0f;
      }

      glLoadIdentity();
      glTranslatef(x, y, -d);
      glColor3f(o.user[1], o.user[2], o.user[3]);
      drawPoint();
    }

    //
    // Draw objects.
    //

    sw2::RECT_t<int> rc(rcBound);
    rc.inflate(-25, -25);

    for (int i = g.om.objects.first(); -1 != i; i = g.om.objects.next(i)) {

      MyObject const& o = g.om.objects[i];

      if (!rc.ptInRect(sw2::POINT_t<int>((int)o.x, (int)o.y))) {
        continue;
      }

      glLoadIdentity();
      glTranslatef(o.x, -o.y, -d);

      switch ((int)o.user[0])
      {
      case MyObject::OBJ_BULLET:        // Enemy bullet.
        switch ((int)o.user[1])
        {
        case 1:                         // Lazer.
          glRotatef(-(o.direction + 90), 0, 0, 1);
          glScalef(3, 8, 3);
          glColor3f(1, 1, 1);
          fillRect();
          break;
        default:                        // Triangle, aim.
          glRotatef(-(o.direction + 90), 0, 0, 1);
          glRotatef(o.mRot, 0, 1, 0);
          glScalef(3.5f, 3.5f, 3.5f);
          glColor3f(1, 0, 0);
          drawTriangle();
          break;
        }
        break;
      case MyObject::OBJ_OBJECT:        // Object.
        glRotatef(o.mRot/5.0f, 1, 1, 1);
        glScalef(6, 6, 6);
        glColor3f(.5f, .5f, .5f);
        drawCube();
        break;
      }
    }

    //
    // Draw effect.
    //

    for (int i = g.om4.objects.first(); -1 != i; i = g.om4.objects.next(i)) {

      MyObject const& o = g.om4.objects[i];

      if (!rc.ptInRect(sw2::POINT_t<int>((int)o.x, (int)o.y))) {
        continue;
      }

      glLoadIdentity();
      glTranslatef(o.x, -o.y, -d);

      switch ((int)o.user[0])
      {
      case MyObject::OBJ_PARTICLE:      // Effect particle.
        glColor3f(o.user[1], o.user[2], o.user[3]);
        drawPoint();
        break;
      case MyObject::OBJ_FRAGMENT:      // Destroy particle.
        glRotatef(-(o.direction + 90), 0, 0, 1);
        glRotatef(o.mRot, 0, 1, 0);
        glScalef(3, 3, 3);
        glColor3f(.5f, .5f, .5f);
        drawRect();
        break;
      }
    }

    //
    // Draw my bullet.
    //

    for (int i = g.om5.objects.first(); -1 != i; i = g.om5.objects.next(i)) {

      stge::Object const& o = g.om5.objects[i];

      if (!rc.ptInRect(sw2::POINT_t<int>((int)o.x, (int)o.y))) {
        continue;
      }

      glLoadIdentity();
      glTranslatef(o.x, -o.y, -d);
      glScalef(2, 2, 2);
      glColor3f(0.5f, 0.5f, 1.0f);
      drawRect();
    }

    //
    // Draw HP.
    //

    if (g.mPlaying && 0 < g.player.mHP && !g.player.mFinish) {

      for (int i = 0; i < g.player.mHP; ++i) {
        glLoadIdentity();
        glTranslatef(-180 + 16.0f * i + bdOffsetX, -(190.0f + bdOffsetY), -d);
        glColor3f(1, 0, 0);
        glScalef(7, 2, 4);
        fillRect();
      }

      if (5 > g.player.mHP) {
        float len = (12.0f * g.player.mHealTime / 10.0f) / 2.0f;
        glLoadIdentity();
        glTranslatef(-180 + 16.0f * g.player.mHP - (7 - len) + bdOffsetX, -(190.0f + bdOffsetY), -d);
        glColor3f(1, 0, 0);
        glScalef(len, 2, 4);
        fillRect();
      }

      for (int i = 0; i < 5; ++i) {
        glLoadIdentity();
        glTranslatef(-180 + 16.0f * i + bdOffsetX, -(190.0f + bdOffsetY), -d);
        glColor3f(1, 1, 1);
        glScalef(7, 2, 4);
        drawRect();
      }
    }

    //
    // Draw player.
    //

    if (g.mPlaying && 0 < g.player.mHP) {
      glLoadIdentity();
      glTranslatef(g.player.x, -g.player.y, -d);
      glRotatef(g.mPlayerRot, 0, 1, 0);
      glScalef(6, 6, 6);
      glColor3f(1, 1, 1);
      drawTriangle();
    }

    //
    // Draw menu/char.
    //

    for (int i = g.om3.objects.first(); -1 != i; i = g.om3.objects.next(i)) {

      stge::Object const& o = g.om3.objects[i];

      if (!rc.ptInRect(sw2::POINT_t<int>((int)o.x, (int)o.y))) {
        continue;
      }

      glLoadIdentity();
      glTranslatef(o.x, -o.y, -d);
      glScalef(o.user[2], 2 * o.user[2], o.user[2]);
      glColor3f(1, 1, 1);
      drawChar((int)o.user[1]);
    }

    //
    // Draw menu cursor.
    //

    if (-1 != g.mMenuItem) {
      glLoadIdentity();
      glTranslatef(g.cursor.x, -g.cursor.y, -d);
      glRotatef(-90, 0, 0, 1);
      glRotatef(g.mCursorRot, 0, 1, 0);
      glScalef(6, 6, 6);
      glColor3f(1, 1, 0);
      drawTriangle();
    }
  }

  void OnResize(int width, int height)
  {
    if (height == 0) {
      height = 1;
    }

    glViewport(0, 0, width, height);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    //
    // Calculate The Aspect Ratio Of The Window.
    //

    gluPerspective(45.0f, (GLfloat)width / (GLfloat)height, 1.0f, 1000.0f);
    hWidth = width / 2.0f;
    hHeight = height / 2.0f;
    float deg2pi = 3.1415926f / 180.0f;
    d = hHeight / tanf((45.0f/2.0f) * deg2pi);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
  }

  //
  // Helper.
  //

  DWORD timeLeft()
  {
    static DWORD nextTime = ::GetTickCount() + (int)(1000/(float)FPS);
    DWORD now = ::GetTickCount();

    DWORD left = 0;
    if(nextTime > now) {
      left = nextTime - now;
    }

    nextTime += (int)(1000/(float)FPS);

    return left;
  }

  //
  // Draw.
  //

  void drawChar(int c)
  {
    int idx = c - ' ';

    int cw = 12, ch = 24;
    int cch = 15;

    int x = cw * (idx % cch);
    int y = ch * (idx / cch);

    float x0 = x / 256.0f, y0 = (256 - y) / 256.0f;
    float x1 = (x + cw) / 256.0f, y1 = (256 - y - ch) / 256.0f;

    glEnable(GL_TEXTURE_2D);
    glDisable(GL_DEPTH_TEST);
    glBegin(GL_QUADS);
      glTexCoord2f(x0, y1);
      glVertex3f(-1, -1, 0);
      glTexCoord2f(x1, y1);
      glVertex3f(1, -1, 0);
      glTexCoord2f(x1, y0);
      glVertex3f(1, 1, 0);
      glTexCoord2f(x0, y0);
      glVertex3f(-1, 1, 0);
    glEnd();
    glDisable(GL_TEXTURE_2D);
    glEnable(GL_DEPTH_TEST);
  }

  void drawCube()
  {
    glBegin(GL_LINE_LOOP);
      glVertex3f( 1.0f, 1.0f,-1.0f);
      glVertex3f(-1.0f, 1.0f,-1.0f);
      glVertex3f(-1.0f, 1.0f, 1.0f);
      glVertex3f( 1.0f, 1.0f, 1.0f);
    glEnd();
    glBegin(GL_LINE_LOOP);
      glVertex3f( 1.0f,-1.0f, 1.0f);
      glVertex3f(-1.0f,-1.0f, 1.0f);
      glVertex3f(-1.0f,-1.0f,-1.0f);
      glVertex3f( 1.0f,-1.0f,-1.0f);
    glEnd();
    glBegin(GL_LINE_LOOP);
      glVertex3f( 1.0f, 1.0f, 1.0f);
      glVertex3f(-1.0f, 1.0f, 1.0f);
      glVertex3f(-1.0f,-1.0f, 1.0f);
      glVertex3f( 1.0f,-1.0f, 1.0f);
    glEnd();
    glBegin(GL_LINE_LOOP);
      glVertex3f( 1.0f,-1.0f,-1.0f);
      glVertex3f(-1.0f,-1.0f,-1.0f);
      glVertex3f(-1.0f, 1.0f,-1.0f);
      glVertex3f( 1.0f, 1.0f,-1.0f);
    glEnd();
    glBegin(GL_LINE_LOOP);
      glVertex3f(-1.0f, 1.0f, 1.0f);
      glVertex3f(-1.0f, 1.0f,-1.0f);
      glVertex3f(-1.0f,-1.0f,-1.0f);
      glVertex3f(-1.0f,-1.0f, 1.0f);
    glEnd();
    glBegin(GL_LINE_LOOP);
      glVertex3f( 1.0f, 1.0f,-1.0f);
      glVertex3f( 1.0f, 1.0f, 1.0f);
      glVertex3f( 1.0f,-1.0f, 1.0f);
      glVertex3f( 1.0f,-1.0f,-1.0f);
    glEnd();
  }

  void drawPoint()
  {
    glBegin(GL_POINTS);
      glVertex2f(0, 0);
    glEnd();
  }
  void drawRect()
  {
    glBegin(GL_LINE_LOOP);
      glVertex2f(-1, 1);
      glVertex2f(1, 1);
      glVertex2f(1, -1);
      glVertex2f(-1, -1);
    glEnd();
  }

  void drawTriangle()
  {
    glBegin(GL_LINE_LOOP);
      glVertex2d(0, 1);
      glVertex2d(-1, -1);
      glVertex2d(1, -1);
    glEnd();
  }

  void fillRect()
  {
    glBegin(GL_QUADS);
      glVertex2f(-1, 1);
      glVertex2f(1, 1);
      glVertex2f(1, -1);
      glVertex2f(-1, -1);
    glEnd();
  }
};
