//
// main.cpp
// 25940p HTML5.
//
// 2021/6/19 Waync created.
//

#include <time.h>
#include <stdlib.h>

#include <string>

#include <GL/glfw.h>
#include "SDL/SDL.h"
#include "SDL/SDL_opengl.h"

#include <emscripten.h>

#include "swArchive.h"
#include "swStageStack.h"
#include "swGeometry.h"

#include "../stge/stge/stge.h"
#include "../stge/stge/par_yard.h"

#define STB_IMAGE_IMPLEMENTATION
#define GOOD_SUPPORT_STB_IMG
#include "gx/img.h"

#include "../p25940.c"                  // zipped 25940.stge bin data.
#include "font.c"                       // font.bmp

#include "../game.h"
#include "../app.h"

MyApp app;

bool initSDL(int w, int h)
{
  if (0 > SDL_Init(SDL_INIT_VIDEO)) {
    printf("init SDL failed");
    return false;
  }

  SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

  SDL_Surface* mScreen = SDL_SetVideoMode(w, h, 16, SDL_OPENGL);
  if (!mScreen) {
    printf("set video mode(%dx%d) failed", w, h);
    SDL_Quit();
    return false;
  }

  emscripten_set_canvas_size(w, h);
  return true;
}

bool pollSDL(Uint32& keys)
{
  SDL_Event event;

  while (SDL_PollEvent(&event)) {
    switch(event.type)
    {
    case SDL_KEYDOWN:
      switch (event.key.keysym.sym)
      {
      case SDLK_UP: keys |= MyGame::KEY_UP; break;
      case SDLK_DOWN: keys |= MyGame::KEY_DOWN; break;
      case SDLK_LEFT: keys |= MyGame::KEY_LEFT; break;
      case SDLK_RIGHT: keys |= MyGame::KEY_RIGHT; break;
      case SDLK_ESCAPE: keys |= MyGame::KEY_CANCEL; break;
      case SDLK_RETURN: keys |= MyGame::KEY_SELECT; break;
      case SDLK_z: keys |= MyGame::KEY_FIRE; break;
      }
      return false;
    case SDL_KEYUP:
      switch (event.key.keysym.sym)
      {
      case SDLK_UP: keys &= ~MyGame::KEY_UP; break;
      case SDLK_DOWN: keys &= ~MyGame::KEY_DOWN; break;
      case SDLK_LEFT: keys &= ~MyGame::KEY_LEFT; break;
      case SDLK_RIGHT: keys &= ~MyGame::KEY_RIGHT; break;
      case SDLK_ESCAPE: keys &= ~MyGame::KEY_CANCEL; break;
      case SDLK_RETURN: keys &= ~MyGame::KEY_SELECT; break;
      case SDLK_z: keys &= ~MyGame::KEY_FIRE; break;
      }
      return false;
    case SDL_QUIT:
      return true;
    }
  }
  return false;
}

void trigger()
{
  Uint32 keys = 0;
  pollSDL(keys);
  app.update(1/60.0f, keys);
  app.render();
}

int main(int argc, char* argv[])
{
  if (!initSDL(400, 400)) {
    return 1;
  }
  srand(time(0));
  app.init();
  std::string simg(FONT_BMP_BIN, FONT_BMP_BIN + sizeof(FONT_BMP_BIN));
  good::gx::GxImage img;
  if (img.loadFromStream(simg)) {
    img.convert32();
    img.flip();
  }
  app.initGL(img);
  app.resize(400, 400);
  emscripten_set_main_loop(trigger, 0, 1);
  emscripten_exit_with_live_runtime();
}
