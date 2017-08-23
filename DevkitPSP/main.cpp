/*
 * PSP Software Development Kit - http://www.pspdev.org
 * -----------------------------------------------------------------------
 * Licensed under the BSD license, see LICENSE in PSPSDK root for details.
 *
 * Copyright (c) 2005 Jesper Svennevid
 */

#include <pspkernel.h>
#include <pspdisplay.h>
#include <pspdebug.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>

#include <sstream>
#include <iterator>

#include <pspgu.h>
#include <pspgum.h>
#include <pspctrl.h>

#define ARM9 // FAKE
#include "swArchive.h"
#include "swGeometry.h"
#include "swStageStack.h"

#include "stge/stge.h"

#ifdef min
#undef min
#endif
#ifdef max
#undef max
#endif

#include "stge/par_yard.h"

#include "script.h"

void stgeLoadScript(stge::ScriptManager& scm)
{
    std::stringstream ss(std::string(script, script + size_script));
    stge::Parser::parse(ss, scm);
}

#define LOAD_SCRIPT_FROM_SCRIPT_CODE
#include "game25940p.h"

#include "font.h"

int const FPS = 60;
#define BUF_WIDTH (512)
#define SCR_WIDTH (480)
#define SCR_HEIGHT (272)

#define GAME_WIDTH 400
#define GAME_HEIGHT 400

MyGame g;

PSP_MODULE_INFO("25940p", 0, 1, 1);
PSP_MAIN_THREAD_ATTR(THREAD_ATTR_USER);

static unsigned int __attribute__((aligned(16))) list[262144];

struct Vertex
{
    float x,y,z;
};

struct Sprite
{
    float u, v;
    unsigned int color;
    float x, y, z;
};

void drawPoint()
{
    static struct Vertex __attribute__((aligned(16))) v_point[1] =
    {
        { 0, 0, 0}
    };
    sceGumDrawArray(GU_POINTS,GU_VERTEX_32BITF|GU_TRANSFORM_3D,1,0,v_point);
}

void drawTriangle()
{
    static struct Vertex __attribute__((aligned(16))) v_triangle[6] =
    {
        { 0, 1, 0},
        {-1,-1, 0},
        {-1,-1, 0},
        { 1,-1, 0},
        { 1,-1, 0},
        { 0, 1, 0}
    };
    sceGumDrawArray(GU_LINES,GU_VERTEX_32BITF|GU_TRANSFORM_3D,6,0,v_triangle);
}

void fillTriangle()
{
    static struct Vertex __attribute__((aligned(16))) v_triangle[3] =
    {
        { 0, 1, 0},
        { 1,-1, 0},
        {-1,-1, 0}
    };
    sceGumDrawArray(GU_TRIANGLES,GU_VERTEX_32BITF|GU_TRANSFORM_3D,3,0,v_triangle);
}

void drawRect()
{
    static struct Vertex __attribute__((aligned(16))) v_rect[8] =
    {
        {-1, 1, 0},
        { 1, 1, 0},
        { 1, 1, 0},
        { 1,-1, 0},
        { 1,-1, 0},
        {-1,-1, 0},
        {-1,-1, 0},
        {-1, 1, 0}
    };
    sceGumDrawArray(GU_LINES,GU_VERTEX_32BITF|GU_TRANSFORM_3D,8,0,v_rect);
}

void fillRect()
{
    static struct Vertex __attribute__((aligned(16))) v_rect[6] =
    {
        {-1,-1, 0},
        {-1, 1, 0},
        { 1, 1, 0},
        {-1,-1, 0},
        { 1, 1, 0},
        { 1,-1, 0}
    };
    sceGumDrawArray(GU_TRIANGLES,GU_VERTEX_32BITF|GU_TRANSFORM_3D,6,0,v_rect);
}

void drawCube()
{
    static struct Vertex __attribute__((aligned(16))) v_cube[12*3] =
    {
        {-1,-1, 1}, // 0
        {-1, 1, 1}, // 4
        { 1, 1, 1}, // 5

        {-1,-1, 1}, // 0
        { 1, 1, 1}, // 5
        { 1,-1, 1}, // 1

        {-1,-1,-1}, // 3
        { 1,-1,-1}, // 2
        { 1, 1,-1}, // 6

        {-1,-1,-1}, // 3
        { 1, 1,-1}, // 6
        {-1, 1,-1}, // 7

        { 1,-1,-1}, // 0
        { 1,-1, 1}, // 3
        { 1, 1, 1}, // 7

        { 1,-1,-1}, // 0
        { 1, 1, 1}, // 7
        { 1, 1,-1}, // 4

        {-1,-1,-1}, // 0
        {-1, 1,-1}, // 3
        {-1, 1, 1}, // 7

        {-1,-1,-1}, // 0
        {-1, 1, 1}, // 7
        {-1,-1, 1}, // 4

        {-1, 1,-1}, // 0
        { 1, 1,-1}, // 1
        { 1, 1, 1}, // 2

        {-1, 1,-1}, // 0
        { 1, 1, 1}, // 2
        {-1, 1, 1}, // 3

        {-1,-1,-1}, // 4
        {-1,-1, 1}, // 7
        { 1,-1, 1}, // 6

        {-1,-1,-1}, // 4
        { 1,-1, 1}, // 6
        { 1,-1,-1}, // 5
    };
    sceGumDrawArray(GU_TRIANGLES,GU_VERTEX_32BITF|GU_TRANSFORM_3D,12*3,0,v_cube);
}

void drawChar(int c)
{
    int idx = c - ' ';

    int cw = 12, ch = 24;
    int cch = 15;

    int x = cw * (idx % cch);
    int y = ch * (idx / cch);

    float x0 = x / 256.0f, y0 = y / 256.0f;
    float x1 = (x + cw) / 256.0f, y1 = (y + ch) / 256.0f;

    Sprite* v = (Sprite*)sceGuGetMemory(sizeof(Sprite) * 2);

    v[0].u = x0;
    v[0].v = y0;
    v[0].color = 0xffffffff;
    v[0].x = -1;
    v[0].y = -1;
    v[0].z = 0;
    v[1].u = x1;
    v[1].v = y1;
    v[1].color = 0xffffffff;
    v[1].x = 1;
    v[1].y = 1;
    v[1].z = 0;

    sceGumDrawArray(GU_SPRITES, GU_TEXTURE_32BITF | GU_COLOR_8888 | GU_VERTEX_32BITF | GU_TRANSFORM_3D, 2, 0, v);
}

int exitCallback(int arg1, int arg2, void *common)
{
    sceKernelExitGame();
    return 0;
}

int callbackThread(SceSize args, void *argp)
{
    int cbid;

    cbid = sceKernelCreateCallback("Exit Callback", exitCallback, NULL);
    sceKernelRegisterExitCallback(cbid);

    sceKernelSleepThreadCB();

    return 0;
}

int setupCallbacks(void)
{
    int thid = 0;

    thid = sceKernelCreateThread("update_thread", callbackThread, 0x11, 0xFA0, 0, 0);
    if(thid >= 0)
    {
        sceKernelStartThread(thid, 0, 0);
    }

    return thid;
}

static unsigned int staticOffset = 0;

static unsigned int getMemorySize(unsigned int width, unsigned int height, unsigned int psm)
{
    switch (psm)
    {
        case GU_PSM_T4:
            return (width * height) >> 1;

        case GU_PSM_T8:
            return width * height;

        case GU_PSM_5650:
        case GU_PSM_5551:
        case GU_PSM_4444:
        case GU_PSM_T16:
            return 2 * width * height;

        case GU_PSM_8888:
        case GU_PSM_T32:
            return 4 * width * height;

        default:
            return 0;
    }
}

void* getStaticVramBuffer(unsigned int width, unsigned int height, unsigned int psm)
{
    unsigned int memSize = getMemorySize(width,height,psm);
    void* result = (void*)staticOffset;
    staticOffset += memSize;

    return result;
}

int main(int argc, char* argv[])
{
    g.init(GAME_WIDTH, GAME_HEIGHT);
    setupCallbacks();

    // setup GU

    void* fbp0 = getStaticVramBuffer(BUF_WIDTH,SCR_HEIGHT,GU_PSM_8888);
    void* fbp1 = getStaticVramBuffer(BUF_WIDTH,SCR_HEIGHT,GU_PSM_8888);
    void* zbp = getStaticVramBuffer(BUF_WIDTH,SCR_HEIGHT,GU_PSM_4444);

    sceGuInit();

    sceGuStart(GU_DIRECT,list);
    sceGuDrawBuffer(GU_PSM_8888,fbp0,BUF_WIDTH);
    sceGuDispBuffer(SCR_WIDTH,SCR_HEIGHT,fbp1,BUF_WIDTH);
    sceGuDepthBuffer(zbp,BUF_WIDTH);
    sceGuOffset(2048 - (SCR_WIDTH/2),2048 - (SCR_HEIGHT/2));
    sceGuViewport(2048,2048,SCR_WIDTH,SCR_HEIGHT);
    sceGuDepthRange(65535,0);
    sceGuScissor(0,0,SCR_WIDTH,SCR_HEIGHT);
    sceGuEnable(GU_SCISSOR_TEST);
    sceGuDepthFunc(GU_GEQUAL);
    sceGuEnable(GU_DEPTH_TEST);
    sceGuFrontFace(GU_CW);
    sceGuShadeModel(GU_SMOOTH);
    sceGuEnable(GU_CULL_FACE);
    sceGuEnable(GU_CLIP_PLANES);

    sceGuTexMode(GU_PSM_8888, 0, 0, 0);
    sceGuTexImage(0, 256, 256, 256, font);
    sceGuTexFunc(GU_TFX_MODULATE, GU_TCC_RGB);
    sceGuTexEnvColor(0x0);
    sceGuTexOffset(0.0f, 0.0f);
    sceGuTexWrap(GU_CLAMP,GU_CLAMP);
    sceGuTexFilter(GU_NEAREST, GU_NEAREST);
    sceGuTexScale(1,1);
    sceGuAmbientColor(0xffffffff);

    sceGuFinish();
    sceGuSync(0,0);

    sceDisplayWaitVblankStart();
    sceGuDisplay(GU_TRUE);

    // run sample

    float d;
    float hWidth, hHeight;

    sceCtrlSetSamplingCycle(0);
    sceCtrlSetSamplingMode(PSP_CTRL_MODE_DIGITAL);

    while(!g.mQuit)
    {
        int keys = 0;

        SceCtrlData pad = {0};
        sceCtrlReadBufferPositive(&pad, 1);

        if (pad.Buttons & PSP_CTRL_CIRCLE)
            keys |= MyGame::KEY_FIRE|MyGame::KEY_SELECT;
        if (pad.Buttons & PSP_CTRL_SQUARE)
            keys |= MyGame::KEY_CANCEL;

        if (pad.Buttons & PSP_CTRL_UP)
            keys |= MyGame::KEY_UP;
        else if (pad.Buttons & PSP_CTRL_DOWN)
            keys |= MyGame::KEY_DOWN;
        if (pad.Buttons & PSP_CTRL_LEFT)
            keys |= MyGame::KEY_LEFT;
        else if (pad.Buttons & PSP_CTRL_RIGHT)
            keys |= MyGame::KEY_RIGHT;

        //
        float const et = 1 / (float)FPS;
        g.update(et, keys);

        //
        hWidth = SCR_WIDTH / 2.0f;
        hHeight = SCR_HEIGHT / 2.0f;
        float deg2pi = GU_PI / 180.0f;
        d = hHeight / tanf((45.0f/2.0f) * deg2pi);

        d *= GAME_HEIGHT/(float)SCR_HEIGHT; // scale to fit screen.

        sw2::RECT_t<int> rc(rcBound);
        rc.inflate(-25,-25);

        // BEGIN DRAW
        sceGuStart(GU_DIRECT,list);

        sceGuDisable(GU_TEXTURE_2D);
        sceGuEnable(GU_DEPTH_TEST);

        sceGuClearColor(0xff000000);
        sceGuClearDepth(0);
        sceGuClear(GU_COLOR_BUFFER_BIT|GU_DEPTH_BUFFER_BIT);

        sceGumMatrixMode(GU_PROJECTION);
        sceGumLoadIdentity();
        sceGumPerspective(45.0f,16.0f/9.0f,0.5f,1000.0f);

        sceGumMatrixMode(GU_VIEW);
        sceGumLoadIdentity();

        sceGumMatrixMode(GU_MODEL);
        sceGumLoadIdentity();

        float bdOffsetX = .0f, bdOffsetY = .0f;

        if (g.mPlaying && .0f < g.player.mShakeTime)
        {
            bdOffsetX = sw2::rangeRand(-3.0f,3.0f);
            bdOffsetY = sw2::rangeRand(-3.0f,3.0f);
        }

        // draw stars
        float offsetx = g.starx / 200.0f;
        float offsety = g.stary / 200.0f;

        sceGuDisable(GU_CULL_FACE);
        for (int i = g.om2.objects.first(); -1 != i; i = g.om2.objects.next(i))
        {
            stge::Object const& o = g.om2.objects[i];

            float xx = offsetx * (70.0f - 22.0f * o.user[0]);
            float yy = offsety * (70.0f - 22.0f * o.user[0]);
            float x = o.x - xx, y = o.y + yy;
            if (200.0f < x)
                x -= 400.0f;
            else if (-200.0f > x)
                x += 400.0f;
            if (200.0f < y)
                y -= 400.0f;
            else if (-200.0f > y)
                y += 400.0f;

            sceGumLoadIdentity();
            ScePspFVector3 trans = {x,y,-d};
            sceGumTranslate(&trans);
            sceGuColor(0xff000000 + ((int)(o.user[1] * 255) & 0xff) + (((int)(o.user[2] * 255) & 0xff) << 8) + (((int)(o.user[3] * 255) & 0xff) << 16));
            drawPoint();
        }
        sceGuEnable(GU_CULL_FACE);

        // draw objects
        for (int i = g.om.objects.first(); -1 != i; i = g.om.objects.next(i))
        {
            MyObject const& o = g.om.objects[i];

            if (!rc.ptInRect(sw2::POINT_t<int>((int)o.x,(int)o.y)))
                continue;

            sceGumLoadIdentity();
            ScePspFVector3 trans = {o.x,-o.y,-d};
            sceGumTranslate(&trans);

            switch ((int)o.user[0])
            {
            case MyObject::OBJ_BULLET: // enemy bullet
                switch ((int)o.user[1])
                {
                case 1: // lazer
                    {
                        ScePspFVector3 s = {3,8,3};
                        sceGumRotateZ(-(o.direction + 90) * (GU_PI/180.0f));
                        sceGumScale(&s);
                        sceGuColor(0xffffffff);
                        //fillRect();
                        fillTriangle();
                    }
                    break;
                default: // triangle,aim
                    {
                        ScePspFVector3 s = {3.5f,3.5f,3.5f};
                        sceGumRotateZ(-(o.direction + 90) * (GU_PI/180.0f));
                        sceGumRotateY(o.mRot * (GU_PI/180.0f));
                        sceGumScale(&s);
                        sceGuColor(0xff0000ff);
                        sceGuDisable(GU_CULL_FACE);
                        drawTriangle();
                        sceGuEnable(GU_CULL_FACE);
                    }
                    break;
                }
                break;
            case MyObject::OBJ_OBJECT: // object
                {
                    float val = o.mRot/5.0f * (GU_PI/180.0f);
                    ScePspFVector3 rot = { val, val, val};
                    ScePspFVector3 s = {6,6,6};
                    sceGumRotateXYZ(&rot);
                    sceGumScale(&s);
                    sceGuColor(0xff808080);
                    drawCube();
                }
                break;
            }
        }

        // draw effect
        for (int i = g.om4.objects.first(); -1 != i; i = g.om4.objects.next(i))
        {
            MyObject const& o = g.om4.objects[i];

            if (!rc.ptInRect(sw2::POINT_t<int>((int)o.x,(int)o.y)))
                continue;

            sceGumLoadIdentity();
            ScePspFVector3 trans = {o.x,-o.y,-d};
            sceGumTranslate(&trans);

            switch ((int)o.user[0])
            {
            case MyObject::OBJ_PARTICLE: // effect particle
                sceGuColor(0xff000000 + ((int)(o.user[1] * 255) & 0xff) + (((int)(o.user[2] * 255) & 0xff) << 8) + (((int)(o.user[3] * 255) & 0xff) << 16));
                sceGuDisable(GU_CULL_FACE);
                drawPoint();
                sceGuEnable(GU_CULL_FACE);
                break;
            case MyObject::OBJ_FRAGMENT: // destroy particle
                {
                    sceGumRotateY(o.mRot * (GU_PI/180.0f));
                    sceGumRotateZ(-(o.direction + 90) * (GU_PI/180.0f));
                    ScePspFVector3 s = {3,3,3};
                    sceGumScale(&s);
                    sceGuColor(0xff808080);
                    sceGuDisable(GU_CULL_FACE);
                    drawRect();
                    sceGuEnable(GU_CULL_FACE);
                }
                break;
            }
        }

        // draw my bullet
        sceGuDisable(GU_CULL_FACE);
        for (int i = g.om5.objects.first(); -1 != i; i = g.om5.objects.next(i))
        {
            stge::Object const& o = g.om5.objects[i];

            if (!rc.ptInRect(sw2::POINT_t<int>((int)o.x,(int)o.y)))
                continue;

            sceGumLoadIdentity();
            ScePspFVector3 trans = {o.x,-o.y,-d};
            sceGumTranslate(&trans);
            ScePspFVector3 s = {2,2,2};
            sceGumScale(&s);
            sceGuColor(0xffff8080);
            drawRect();
        }
        sceGuEnable(GU_CULL_FACE);

        // draw hp
        if (g.mPlaying && 0 < g.player.mHP && !g.player.mFinish)
        {
            for (int i = 0; i < g.player.mHP; ++ i)
            {
                sceGumLoadIdentity();
                ScePspFVector3 trans = {-180 + 16.0f * i + bdOffsetX, -(190.0f + bdOffsetY), -d};
                sceGumTranslate(&trans);
                ScePspFVector3 s = {7,2,4};
                sceGumScale(&s);
                sceGuColor(0xff0000ff);
                fillRect();
            }
            if (5 > g.player.mHP)
            {
                float len = (12.0f * g.player.mHealTime / 10.0f) / 2.0f;
                sceGumLoadIdentity();
                ScePspFVector3 trans = {-180 + 16.0f * g.player.mHP - (7 - len) + bdOffsetX, -(190.0f + bdOffsetY), -d};
                sceGumTranslate(&trans);
                ScePspFVector3 s = {len,2,4};
                sceGumScale(&s);
                sceGuColor(0xff0000ff);
                fillRect();
            }
            for (int i = 0; i < 5; ++ i)
            {
                sceGumLoadIdentity();
                ScePspFVector3 trans = {-180 + 16.0f * i + bdOffsetX, -(190.0f + bdOffsetY), -d};
                sceGumTranslate(&trans);
                ScePspFVector3 s = {7,2,4};
                sceGumScale(&s);
                sceGuColor(0xffffffff);
                drawRect();
            }
        }

        // draw player
        if (g.mPlaying && 0 < g.player.mHP)
        {
            ScePspFVector3 trans = {g.player.x, -g.player.y,-d};
            ScePspFVector3 s = {6,6,6};
            sceGumLoadIdentity();
            sceGumTranslate(&trans);
            sceGumRotateY(g.mPlayerRot* (GU_PI/180.0f));
            sceGumScale(&s);
            sceGuColor(0xffffffff);
            fillTriangle();
        }

        // draw menu cursor
        if (-1 != g.mMenuItem)
        {
            ScePspFVector3 trans = {g.cursor.x, -g.cursor.y,-d};
            ScePspFVector3 s = {6,6,6};
            sceGumLoadIdentity();
            sceGumTranslate(&trans);
            sceGumRotateZ(-90* (GU_PI/180.0f));
            sceGumScale(&s);
            sceGuColor(0xff00ffff);
            sceGuDisable(GU_CULL_FACE);
            drawTriangle();
            sceGuEnable(GU_CULL_FACE);
        }

        sceGuFinish();
        sceGuSync(0,0);

        // draw menu/char
        sceGuStart(GU_DIRECT,list);

        sceGuEnable(GU_TEXTURE_2D);
        sceGuDisable(GU_DEPTH_TEST);

        for (int i = g.om3.objects.first(); -1 != i; i = g.om3.objects.next(i))
        {
            stge::Object const& o = g.om3.objects[i];

            if (!rc.ptInRect(sw2::POINT_t<int>((int)o.x,(int)o.y)))
                continue;

            ScePspFVector3 trans = {o.x,-o.y,-d};
            ScePspFVector3 s = {12/8.0f * o.user[2], o.user[2], 1};
            sceGumLoadIdentity();
            sceGumTranslate(&trans);
            sceGumRotateZ(-90* (GU_PI/180.0f));
            sceGumScale(&s);
            drawChar((int)o.user[1]);
        }

        sceGuFinish();
        sceGuSync(0,0);

        sceDisplayWaitVblankStart();
        sceGuSwapBuffers();
        // END DRAW
    }

    sceGuTerm();

    sceKernelExitGame();
    return 0;
}

// workaround for fixing link error.
extern "C" {
  int _isatty(int file) {
     (void)file; // avoid warning
     return 1;
  }
}