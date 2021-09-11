//
// 25940p.js
// 25940p HTML5.
//
// 2021/6/19 Waync created.
//

var keyStates = [0,0,0,0,0,0,0]; // up/down/left/right/enter/esc/z.

function getKeyStates() {
  var mask = [1,2,4,8,32,64,16]; // up/down/left/right/enter/esc/z.
  var ks = 0;
  for (var i = 0; i < keyStates.length; i++) {
    if (0 != keyStates[i]) {
      ks += mask[i];
    }
  }
  return ks;
}

function keyDown(e) {
  updateKeyState(e, 1);
}

function keyUp(e) {
  updateKeyState(e, 0);
}

function updateKeyState(e, s) {
  switch (e.code)
  {
  case 'ArrowUp':
    keyStates[0] = s;
    break;
  case 'ArrowDown':
    keyStates[1] = s;
    break;
  case 'ArrowLeft':
    keyStates[2] = s;
    break;
  case 'ArrowRight':
    keyStates[3] = s;
    break;
  case 'Enter':
    keyStates[4] = s;
    break;
  case 'Escape':
  case 'KeyQ':
    keyStates[5] = s;
    break;
  case 'KeyZ':
    keyStates[6] = s;
    break;
  default:
    return;
  }
  var keys = getKeyStates();
  Module.ccall('cSetKeyStates', 'number', ['number'], [keys]);
}

window.onkeydown = keyDown;
window.onkeyup = keyUp;
