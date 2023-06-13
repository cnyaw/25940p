
//
// Copyright (c) 2008 Waync Cheng.
// All Rights Reserved.
//
// 2008/03/09 Waync Cheng.
//

#pragma once

sw2::RECT_t<int> rcBound, rcCollision;

class MyObject : public stge::Object
{
public:

  enum OBJECT_TYPE
  {
    //
    // For om 1.
    //

    OBJ_BULLET = 0,                     // Enemy bullet.
    OBJ_MYBULLET = 100,                 // My bullet.
    OBJ_PARTICLE = 200,                 // Particle effect.
    OBJ_FRAGMENT = 201,                 // Destroy fragment.
    OBJ_OBJECT = 300,                   // General object.
    OBJ_CHAR = 400,                     // Character, for menu/title.
    OBJ_CURSOR = 500                    // Menu cursor, dummy.
  };

  float mRot;                           // For bullets.

  //
  // Return false if init failed, and clear this object.
  //

  template<class ObjectManagerT>
  bool init(ObjectManagerT& om, int idAction, int idNewObj)
  {
    mRot = sw2::Util::rangeRand(0.0f,360.0f);
    return true;
  }

  //
  // Return false, and clear this object.
  //

  template<typename PlayerT, class ObjectManagerT>
  bool update(float fElapsed, ObjectManagerT& om, PlayerT& player)
  {
    //
    // Move object.
    //

    stge::Object::update(fElapsed, om, player);
    mRot += 450 * fElapsed;

    //
    // Bounding check.
    //

    if (!rcBound.ptInRect(sw2::POINT_t<int>((int)x, (int)y))) {
      return false;
    }

    //
    // Collision check.
    //

    if (!rcCollision.ptInRect(sw2::POINT_t<int>((int)x, (int)y))) {
      return true;
    }

    int type = (int)user[0];
    if (OBJ_BULLET == type) {           // Enemy bullet.
      if (sw2::FloatRect(
                 player.x - 3,
                 player.y - 3,
                 player.x + 3,
                 player.y + 3).ptInRect(sw2::FloatPoint(x,y))) {

        if (!player.omFx.run("fx_hit", x, y)) {
          return true;
        }

        for (int i = om.objects.first(); -1 != i;) { // Clear bullets.
          int next = om.objects.next(i);
          if (OBJ_BULLET == (int)om.objects[i].user[0]) {
            om.freeObject(i);
          }
          i = next;
        }

        player.mHealTime = .0f;
        if (0 < player.mHP && !player.mFinish) {
          player.mHP -= 1;
          player.mShakeTime = .2f;
        }

        return false;
      }
    } else if (OBJ_OBJECT == type) {

      //
      // Check collision with my plane, boss only.
      //

      if (1000 == (int)user[2] &&
          sw2::FloatRect(
                 x - 6,
                 y - 6,
                 x + 6,
                 y + 6).ptInRect(sw2::FloatPoint(player.x,player.y))) {

        player.mHealTime = .0f;
        if (0 < player.mHP) {
          player.mHP -= 1;
          player.mShakeTime = .2f;
        }

        return true;
      }

      //
      // Check collision with my bullet.
      //

      for (int i = player.omBullet.objects.first(); -1 != i; i = player.omBullet.objects.next(i)) {

        stge::Object& o = player.omBullet.objects[i];

        if (!sw2::FloatRect(x - 6, y - 6, x + 6, y + 6).ptInRect(sw2::FloatPoint(o.x,o.y))) {
          continue;
        }

        player.omBullet.freeObject(i);

        user[1] -= 1;                   // HP.
        player.omFx.run("fx_hit", x, y); // Hit effect.

        if (.0f >= user[1]) {           // Destroyed.
          if (1000 == (int)user[2]) {
            player.omFx.run("fx_boss_destroy", x, y);
          } else {
            player.omFx.run("fx_destroy", x, y);
          }
          return false;
        }

        break;
      }
    }

    return true;
  }
};

template<class OMFx, class OMBullet>
class MyPlayer : public stge::Object
{
public:

  int mHP;
  float mHealTime;

  float mShakeTime;                     // Shake effect, when player hit.

  bool mFinish;

  OMFx& omFx;
  OMBullet& omBullet;

  MyPlayer(OMFx& om4, OMBullet& om5) : omFx(om4), omBullet(om5)
  {
  }
};

template<class ObjT, int MAX_OBJ = 1024>
class MyObjectManager : public stge::ObjectManager<MyObjectManager<ObjT,MAX_OBJ>, ObjT, MAX_OBJ>
{
public:
  static int getWindowWidth()
  {
    return 400;
  }

  static int getWindowHeight()
  {
    return 400;
  }
};

class MyGame
{
public:

  enum KEYS
  {
    KEY_UP = 1,
    KEY_DOWN = 1 << 1,
    KEY_LEFT = 1 << 2,
    KEY_RIGHT = 1 << 3,
    KEY_FIRE = 1 << 4,
    KEY_SELECT = 1 << 5,
    KEY_CANCEL = 1 << 6
  };

  //
  // Property.
  //

  bool mQuit, mPlaying;

  int mScreenW, mScreenH;

  float mElapsedTime, mTimer;
  int mLastKeys, mKeys;

  //
  // stge.
  //

  stge::ScriptManager scm;
  MyObjectManager<MyObject> om;         // Game object.
  MyObjectManager<stge::Object,128> om2; // Star.
  MyObjectManager<stge::Object,384> om3; // Menu, char.

  typedef MyObjectManager<MyObject,384> OMgrFxT;
  typedef MyObjectManager<MyObject,32> OMgrBulletT;

  OMgrFxT om4;                          // Effect.
  OMgrBulletT om5;                      // My bullet.

  //
  // Player.
  //

  MyPlayer<OMgrFxT,OMgrBulletT> player;

  float mPlayerRot;
  int mWeapon, mJet;

  //
  // Cursor.
  //

  stge::Object cursor;

  int mMenuItem;

  float mCursorRot;
  float mCursorTimeX, mCursorTargetX;
  float mCursorLastY, mCursorTargetY;

  //
  // Star.
  //

  float starx, stary;

  //
  // Stage controller.
  //

  sw2::StageStack<MyGame> stage;

  //
  // Ctor.
  //

  MyGame() : mQuit(false), mPlaying(false), player(om4,om5)
  {
    om.init(&scm);
    om2.init(&scm);
    om3.init(&scm);
    om4.init(&scm);
    om5.init(&scm);
  }

  //
  // Init.
  //

  bool init(int w, int h)
  {
    //
    // Property.
    //

    mScreenW = w, mScreenH = h;
    mKeys = mLastKeys = 0;
    starx = stary = .0f;

    rcCollision = sw2::RECT_t<int>(0,0,w,h);
    rcCollision.offset(-w/2, -h/2);
    rcBound = rcCollision;
    rcBound.inflate(25,25);

    //
    // Load script.
    //

#ifndef LOAD_SCRIPT_FROM_SCRIPT_CODE
#ifdef _DEBUG
    if (stge::Parser::parse("25940.stge", scm)) {

      //
      // 1, load script from file system.
      //

    } else
#endif // _DEBUG
    {
      //
      // 2, load script from internal archive.
      //

      std::stringstream ssdata(std::string(P25940_MOD, P25940_MOD + sizeof(P25940_MOD)));
      sw2::Archive* ar = sw2::Archive::alloc();

      std::stringstream ss;
      bool result = ar->addStreamFileSystem(ssdata) && ar->loadFile("25940.stge", ss) && stge::Parser::parse(ss, scm);
      sw2::Archive::free(ar);
      if (!result) {
        return false;
      }
    }
#else
    stgeLoadScript(scm);
#endif

    //
    // Init stage.
    //

    stage.initialize(this, &MyGame::sInit);

    return true;
  }

  void update(float fElapsed, int keys)
  {
    mElapsedTime = fElapsed;
    mLastKeys = mKeys;
    mKeys = keys;

    stage.trigger();
  }

  //
  // Helper.
  //

  void getCursorPos()
  {
    mCursorLastY = mCursorTargetY;
    for (int i = om3.objects.first(); -1 != i; i = om3.objects.next(i)) {
      if (MyObject::OBJ_CURSOR == (int)om3.objects[i].user[0]) {
        mCursorTargetX = om3.objects[i].x;
        mCursorTargetY = om3.objects[i].y;
        om3.freeObject(i);
        break;
      }
    }
  }

  bool isKeyPressed(int keyCode) const
  {
    return (mKeys & keyCode) && !(mLastKeys & keyCode);
  }

  bool isKeyPushed(int keyCode) const
  {
    return !(mKeys & keyCode) && (mLastKeys & keyCode);
  }

  bool isKeyDown(int keyCode) const
  {
    return 0 != (mKeys & keyCode);
  }

  void menuItemDown(int last)
  {
    if (last <= mMenuItem) {
      return;
    }

    mMenuItem += 1;
    om3.run("cursor_down", mCursorTargetX, mCursorTargetY);
    om3.update(0, player);
    getCursorPos();
  }

  void menuItemUp()
  {
    if (0 >= mMenuItem) {
      return;
    }

    mMenuItem -= 1;
    om3.run("cursor_up", mCursorTargetX, mCursorTargetY);
    om3.update(0, player);
    getCursorPos();
  }

  void setCharObjUserdata0(float u)
  {
    for (int i = om3.objects.first(); -1 != i; i = om3.objects.next(i)) {
      if (MyObject::OBJ_CHAR == (int)om3.objects[i].user[0]) {
        stge::Action<stge::Object>& ac = om3.actions[om3.objects[i].idAction];
        ac.param[0] = u;
        ac.wait = .0f;
      }
    }
  }

  void updateMenuItem()
  {
    //
    // Rot cursor.
    //

    mCursorRot = ::fmod(mCursorRot + 6, 360.0f);

    //
    // Move horz.
    //

    mCursorTimeX += mElapsedTime;
    if (3.0f < mCursorTimeX) {
      mCursorTimeX -= 3.0f;
    }

    cursor.x = mCursorTargetX + 6.0f + 12.0f * ::sin(3.1415926f * mCursorTimeX / 3.0f);

    //
    // Move vert.
    //

    if (cursor.y > mCursorTargetY) {
      cursor.y -= mElapsedTime * (mCursorLastY - mCursorTargetY) / .15f;
      if (cursor.y < mCursorTargetY) {
        cursor.y = mCursorTargetY;
      }
    } else if (cursor.y < mCursorTargetY) {
      cursor.y += mElapsedTime * (mCursorTargetY - mCursorLastY) / .15f;
      if (cursor.y > mCursorTargetY) {
        cursor.y = mCursorTargetY;
      }
    }
  }

  void updatePlayer()
  {
    //
    // Move.
    //

    const float SPDTILT = 150.0f;
    const float TILE = 50.0f;
    const float vx = 110.0f, vy = 110.0f;
    float mx = .0f, my = .0f;

    if (isKeyDown(KEY_UP)) {
      my = -mElapsedTime * vy;
    } else if (isKeyDown(KEY_DOWN)) {
      my = mElapsedTime * vy;
    }

    if (isKeyDown(KEY_LEFT)) {
      mx = -mElapsedTime * vx;
      mPlayerRot += SPDTILT * mElapsedTime;
      if (TILE < mPlayerRot) {
        mPlayerRot = TILE;
      }
    } else if (isKeyDown(KEY_RIGHT)) {
      mx = mElapsedTime * vx;
      mPlayerRot -= SPDTILT * mElapsedTime;
      if (- TILE > mPlayerRot) {
        mPlayerRot = - TILE;
      }
    } else {
      if (.0f > mPlayerRot) {
        mPlayerRot += SPDTILT * mElapsedTime;
        if (.0f < mPlayerRot) {
          mPlayerRot = .0f;
        }
      } else if (.0f < mPlayerRot) {
        mPlayerRot -= SPDTILT * mElapsedTime;
        if (.0f > mPlayerRot) {
          mPlayerRot = .0f;
        }
      }
    }

    if (isKeyDown(KEY_LEFT|KEY_RIGHT) && isKeyDown(KEY_UP|KEY_DOWN)) {
      mx *= .707f;
      my *= .707f;
    }

    starx += mx, stary += my;
    player.x += mx, player.y += my;

    if (5.0f - mScreenW/2.0f > player.x) {
      player.x = 5.0f - mScreenW/2.0f;
    } else if (mScreenW/2.0f - 5.0f < player.x) {
      player.x = mScreenW/2.0f - 5.0f;
    }

    if (5.0f - mScreenH/2.0f > player.y) {
      player.y = 5.0f - mScreenH/2.0f;
    } else if (mScreenH/2 - 5.0f < player.y) {
      player.y = mScreenH/2.0f - 5.0f;
    }

    //
    // Fire.
    //

    if (isKeyDown(KEY_FIRE)) {
      if (-1 == mWeapon) {
        mWeapon = om5.run("weapon_1", player.x, player.y);
      }
      if (-1 != mWeapon) {
        stge::Action<MyObject>& ac = om5.actions[mWeapon];
        ac.x = player.x, ac.y = player.y;
      }
    } else if (-1 != mWeapon) {
      om5.freeAction(mWeapon);
      mWeapon = -1;
    }

    //
    // Moving.
    //

    if (isKeyDown(KEY_UP|KEY_DOWN|KEY_LEFT|KEY_RIGHT)) {

      if (-1 == mJet) {
        mJet = om4.run("pennn", player.x, player.y);
      }

      if (-1 != mJet) {

        float dir = .0f;
        if (isKeyDown(KEY_UP) && isKeyDown(KEY_LEFT)) {
          dir = 225.0f;
        } else if (isKeyDown(KEY_UP) && isKeyDown(KEY_RIGHT)) {
          dir = -45.0f;
        } else if (isKeyDown(KEY_DOWN) && isKeyDown(KEY_LEFT)) {
          dir = 135.0f;
        } else if (isKeyDown(KEY_DOWN) && isKeyDown(KEY_RIGHT)) {
          dir = 45.0f;
        } else if (isKeyDown(KEY_UP)) {
          dir = 270.0f;
        } else if (isKeyDown(KEY_DOWN)) {
          dir = 90.0f;
        } else if (isKeyDown(KEY_LEFT)) {
          dir = 180.0f;
        }

        stge::Action<MyObject>& ac = om4.actions[mJet];
        ac.x = player.x, ac.y = player.y;
        ac.direction = dir + 180.0f;
      }
    } else if (-1 != mJet) {
      om4.freeAction(mJet);
      mJet = -1;
    }

    //
    // Update HP.
    //

    player.mHealTime += mElapsedTime;
    if (10.0f < player.mHealTime) {
      player.mHealTime -= 10.0f;
      if (5 > player.mHP) {
        player.mHP += 1;
      }
    }

    if (0 == player.mHP) {

      if (-1 != mWeapon) {
        om5.freeAction(mWeapon);
      }

      if (-1 != mJet) {
        om4.freeAction(mJet);
      }

      stage.push(&MyGame::sGameOver);
    }

    //
    // Update misc timer.
    //

    if (.0f < player.mShakeTime) {
      player.mShakeTime -= mElapsedTime;
      if (.0f > player.mShakeTime) {
        player.mShakeTime = .0f;
      }
    }
  }

  //
  // Stage handler.
  //

  void sAllClear(int s, sw2::uint_ptr)
  {
    if (sw2::TRIGGER == s) {
      om3.update(mElapsedTime, player);
      if (0 == om3.actions.size() && 0 == om3.objects.size()) {
        stage.popAndPush(&MyGame::sMainMenu);
      }
    } else if (sw2::JOIN == s) {
      om3.reset();
      om3.run("all_clear", 0, 0);
    }
  }

  void sGameMenu(int s, sw2::uint_ptr)
  {
    if (sw2::TRIGGER == s) {
      mTimer -= mElapsedTime;
      if (.0f > mTimer) {
        stage.popAndPush(&MyGame::sGameMenu2);
        return;
      }
      om3.update(mElapsedTime, player);
    } else if (sw2::JOIN == s) {
      mTimer = 0.7f;
      om3.reset();
      om3.run("game_menu", 0, 0);
    }
  }

  void sGameMenu2(int s, sw2::uint_ptr)
  {
    if (sw2::TRIGGER == s) {

      //
      // Handle keys.
      //

      if (isKeyPressed(KEY_SELECT)) {
        switch (mMenuItem)
        {
        case 0:                         // Resume game.
          stage.popAndPush(&MyGame::sGameMenu3);
          return;
        case 1:                         // New game.
          stage.popAndPush(&MyGame::sGamePlay, 2);
          return;
        case 2:                         // Main menu.
          stage.popAndPush(&MyGame::sMainMenu, 2);
          return;
        case 3:                         // Exit.
          stage.popAndPush(&MyGame::sQuitGame, 2);
          return;
        }
      } else if (isKeyPressed(KEY_UP)) {
        menuItemUp();
      } else if (isKeyPressed(KEY_DOWN)) {
        menuItemDown(3);
      } else if (isKeyPressed(KEY_CANCEL)) {
        stage.popAndPush(&MyGame::sGameMenu3);
        return;
      }

      updateMenuItem();

    } else if (sw2::LEAVE == s) {
      mMenuItem = -1;
    } else if (sw2::JOIN == s) {
      mMenuItem = 0;
      mCursorTimeX = .0f;
      getCursorPos();
      cursor.x = mCursorTargetX + 10.0f;
      cursor.y = mCursorTargetY;
    }
  }

  void sGameMenu3(int s, sw2::uint_ptr)
  {
    if (sw2::TRIGGER == s) {
      mTimer -= mElapsedTime;
      if (.0f > mTimer) {
        stage.pop();
        return;
      }
      om3.update(mElapsedTime, player);
    } else if (sw2::JOIN == s) {
      mTimer = 0.5f;
      setCharObjUserdata0(.0f);
    }
  }

  void sGameOver(int s, sw2::uint_ptr)
  {
    if (sw2::TRIGGER == s) {

      if (.0f < player.mShakeTime) {
        player.mShakeTime -= mElapsedTime;
        if (.0f > player.mShakeTime) {
          player.mShakeTime = .0f;
        }
      }

      mTimer -= mElapsedTime;
      if (.0f > mTimer) {
        stage.popAndPush(&MyGame::sGameOver2);
        return;
      }

      om.update(mElapsedTime, player);
      om4.update(mElapsedTime, player);
      om5.update(mElapsedTime, player);

    } else if (sw2::JOIN == s) {
      mTimer = 0.7f;
      om4.run("fx_hit", player.x, player.y);
      om4.run("fx_destroy", player.x, player.y);
      om4.run("fx_destroy", player.x, player.y);
    }
  }

  void sGameOver2(int s, sw2::uint_ptr)
  {
    if (sw2::TRIGGER == s) {
      mTimer -= mElapsedTime;
      if (.0f > mTimer) {
        stage.popAndPush(&MyGame::sGameOver3);
        return;
      }
      om3.update(mElapsedTime, player);
    } else if (sw2::JOIN == s) {
      mTimer = 0.6f;
      mMenuItem = -1;
      om3.reset();
      om3.run("game_over", 0, 0);
    }
  }

  void sGameOver3(int s, sw2::uint_ptr)
  {
    if (sw2::TRIGGER == s) {

      //
      // Handle keys.
      //

      if (isKeyPressed(KEY_SELECT)) {
        switch (mMenuItem)
        {
        case 0:                         // New game.
          stage.popAndPush(&MyGame::sGamePlay, 2);
          return;
        case 1:                         // Main menu.
          stage.popAndPush(&MyGame::sMainMenu, 2);
          return;
        case 2:                         // Exit.
          stage.popAndPush(&MyGame::sQuitGame, 2);
          return;
        }
      } else if (isKeyPressed(KEY_UP)) {
        menuItemUp();
      } else if (isKeyPressed(KEY_DOWN)) {
        menuItemDown(2);
      }

      updateMenuItem();

    } else if (sw2::LEAVE == s) {
      mMenuItem = -1;
    } else if (sw2::JOIN == s) {
      mMenuItem = 0;
      mCursorTimeX = .0f;
      getCursorPos();
      cursor.x = mCursorTargetX + 10.0f;
      cursor.y = mCursorTargetY;
    }
  }

  void sGamePlay(int s, sw2::uint_ptr)
  {
    if (sw2::TRIGGER == s) {
      mTimer -= mElapsedTime;
      if (.0f > mTimer) {
        stage.popAndPush(&MyGame::sGamePlay2);
        return;
      }
      om3.update(mElapsedTime, player);
    } else if (sw2::JOIN == s) {
      mTimer = 1.0f;
      setCharObjUserdata0(.0f);
      mPlaying = false;
      player.mFinish = true;
      om.reset();
      om4.reset();
      om5.reset();
    }
  }

  void sGamePlay2(int s, sw2::uint_ptr)
  {
    if (sw2::TRIGGER == s) {

      mTimer -= mElapsedTime;
      if (.0f > mTimer) {
        stage.popAndPush(&MyGame::sGamePlay3);
        return;
      }

      player.y = mScreenH / 2.0f - (1 - mTimer / .4f) * (mScreenH / 4.0f);

      om4.update(mElapsedTime, player);
      om4.actions[mJet].y = player.y;
      om4.actions[mJet].direction = 270.0f + 180.0f;

    } else if (sw2::JOIN == s) {

      mPlaying = true;
      player.x = 0;
      player.y = mScreenH / 2.0f;
      player.mShakeTime = .0f;
      player.mHealTime = .0f;
      player.mHP = 5;
      mPlayerRot = .0f;

      mTimer = .4f;

      om4.reset();
      mJet = om4.run("pennn", player.x, player.y);

    } else if (sw2::LEAVE == s) {
      om4.freeAction(mJet);
    }
  }

  void sGamePlay3(int s, sw2::uint_ptr)
  {
    if (sw2::TRIGGER == s) {

      //
      // Handle keys.
      //

      if (isKeyPressed(KEY_CANCEL)) {
        stage.push(&MyGame::sGameMenu);
        return;
      }

      //
      // Update game.
      //

      om.update(mElapsedTime, player);
      om4.update(mElapsedTime, player);
      om5.update(mElapsedTime, player);
      updatePlayer();

      //
      // Stage clear?
      //

      if (0 == om.actions.size()) {
        stage.popAndPush(&MyGame::sGamePlay4);
      }

    } else if (sw2::JOIN == s) {

      mWeapon = mJet = -1;
      player.mFinish = false;

      om.reset();
      om5.reset();
      om.run("stage1", 0, 0);

    } else if (sw2::LEAVE == s) {
      om5.reset();
    }
  }

  void sGamePlay4(int s, sw2::uint_ptr)
  {
    if (sw2::TRIGGER == s) {

      mTimer -= mElapsedTime;
      if (.0f > mTimer) {
        stage.popAndPush(&MyGame::sGamePlay5);
        return;
      }

      om.update(mElapsedTime, player);
      om4.update(mElapsedTime, player);

      const float SPDTILT = 150.0f;

      if (.0f > mPlayerRot) {
        mPlayerRot += SPDTILT * mElapsedTime;
        if (.0f < mPlayerRot) {
          mPlayerRot = .0f;
        }
      } else if (.0f < mPlayerRot) {
        mPlayerRot -= SPDTILT * mElapsedTime;
        if (.0f > mPlayerRot) {
          mPlayerRot = .0f;
        }
      }

    } else if (sw2::JOIN == s) {
      mTimer = 1.5f;
      player.mFinish = true;
      player.mShakeTime = .0f;
      if (-1 != mJet) {
        om4.freeAction(mJet);
        mJet = -1;
      }
    }
  }

  void sGamePlay5(int s, sw2::uint_ptr)
  {
    if (sw2::TRIGGER == s) {

      mTimer -= mElapsedTime;
      if (.0f > mTimer) {
        stage.popAndPush(&MyGame::sAllClear);
        return;
      }

      om.update(mElapsedTime, player);
      om4.update(mElapsedTime, player);

      player.y -= mElapsedTime * mScreenH / 1.5f;
      om4.actions[mJet].y = player.y;
      om4.actions[mJet].direction = 270.0f + 180.0f;

    } else if (sw2::LEAVE == s) {
      mPlaying = false;
      om.reset();
      om4.reset();
    } else if (sw2::JOIN == s) {
      mTimer = 2.5f;
      mJet = om4.run("pennn", player.x, player.y);
    }
  }

  void sInit(int s, sw2::uint_ptr)
  {
    if (sw2::TRIGGER == s) {
      stage.popAndPush(&MyGame::sMainMenu);
    } else if (sw2::JOIN == s) {

      //
      // Create stars.
      //

      om2.run("star", 0, 0);
      om2.update(0, player);
    }
  }

  void sMainMenu(int s, sw2::uint_ptr)
  {
    if (sw2::TRIGGER == s) {

      mTimer -= mElapsedTime;
      if (.0f > mTimer) {
        stage.popAndPush(&MyGame::sMainMenu2);
        return;
      }

      om3.update(mElapsedTime, player);

    } else if (sw2::JOIN == s) {

      mPlaying = false;
      mTimer = 0.7f;
      mMenuItem = -1;

      om.reset();
      om3.reset();
      om4.reset();
      om5.reset();
      om3.run("main_menu", 0, 0);
    }
  }

  void sMainMenu2(int s, sw2::uint_ptr)
  {
    if (sw2::TRIGGER == s) {

      //
      // Handle keys.
      //

      if (isKeyPressed(KEY_SELECT)) {
        switch (mMenuItem)
        {
        case 0:                         // Play game.
          stage.popAndPush(&MyGame::sGamePlay);
          return;
        case 1:                         // Homepage, DANDANDAN.
#ifdef WIN32
          {
            SHELLEXECUTEINFO shExeInfo = {sizeof(SHELLEXECUTEINFO), 0, 0, TEXT("open"), TEXT("http://www.smallworld.idv.tw/"), 0, 0, SW_SHOWNORMAL, 0, 0, 0, 0, 0, 0, 0 };
            ::ShellExecuteEx(&shExeInfo);
          }
#endif
          return;
        case 2:                         // Exit.
          stage.popAndPush(&MyGame::sQuitGame);
          return;
        }
      } else if (isKeyPressed(KEY_UP)) {
        menuItemUp();
      } else if (isKeyPressed(KEY_DOWN)) {
        menuItemDown(2);
      }

      updateMenuItem();

    } else if (sw2::LEAVE == s) {
      mMenuItem = -1;
    } else if (sw2::JOIN == s) {
      mMenuItem = 0;
      mCursorTimeX = .0f;
      getCursorPos();
      cursor.x = mCursorTargetX + 6.0f;
      cursor.y = mCursorTargetY;
    }
  }

  void sQuitGame(int s, sw2::uint_ptr)
  {
    if (sw2::TRIGGER == s) {
      mTimer -= mElapsedTime;
      if (.0f > mTimer) {
        stage.popAndPush(&MyGame::sQuitGame2);
        return;
      }
      om3.update(mElapsedTime, player);
    } else if (sw2::JOIN == s) {
      mTimer = 1.2f;
      setCharObjUserdata0(1.0f);
    }
  }

  void sQuitGame2(int s, sw2::uint_ptr)
  {
    if (sw2::JOIN == s) {
      mQuit = true;
    }
  }
};

// end of game.h
