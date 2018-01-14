#include "x86.h"
#include "fb.h"
#include "kb.h"
#include "bochs.h"

void do_it_yourself() {
  disable();      // "please, dont disturb". (no interrupts while setting up)

  setup_x86();    // enables x86 32-bits things
  setup_fb();     // enables built-in video
  setup_kb();     // enables built-in keyboard
  fb_puts("DiyOS - do it yourself Operating System\n");

  enable();       // crossing fingers... interrupts are enabled :O
  while(1);
}
