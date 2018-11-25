
#include <gui.h>
#include <message.h>
#include <task.h>

void gui_draw();
void gui_desktop_create();
void gui_pointer_draw(uint32_t x, uint32_t y);
uint32_t pointerX, pointerY;

#define POINTER_H 7
#define POINTER_W 5

widget_t *desktop_window, *desktop_taskbar;
task_t* gui_task;

#define TASK_BAR_HEIGHT 16

uint8_t
paint(struct widget* widget) {
  // create a label as child at runtime, for testing...
  // widget_t* w = WIDGET(label_create("teste!",widget));
  // w->bgcolor = (color_t) {255,0, 0};

  return 1;
}

void
gui_main() {
  message_t* msg;
  window_t*  window;

  while(!gfx_is_ready());

  gui_desktop_create();

  task_listen(KEYBOARD); // listen for Keyboard events

  while(1) {
    disable();
    while( (msg = message()) != NULL ) { // read all messages
        switch(msg->from) {
            case KEYBOARD:
                // keyboard packet: byte0 = state keys; byte1 = key;
                // byte0: 0 = Press(1)/Release(0);

                // printf("GUI SERVER: key event (0x%x).\n", (uint8_t)msg->data);

                window = gui_get_active_window();
                if( window == NULL )
                  break;
                // widget_keydown(window->focus);
                break;
            case MOUSE:
                break;
            default:
                break;
          }
        message_destroy(msg);
      }

      gui_draw();
      enable();
      task_block();
    }

}

void
gui_desktop_create() {

    // the desktop area
    desktop_window = widget_create(0, 0, 0, gfx_width(), gfx_height()/* - TASK_BAR_HEIGHT - 1*/, NULL);
    // desktop_taskbar = widget_create(0, 0, gfx_height() - TASK_BAR_HEIGHT, gfx_width(), gfx_height(), NULL);
    // widget_set_padding(desktop_taskbar,2,2,2,2);
    // desktop_taskbar->bgcolor = (color_t){128,128,128};
    desktop_window->bgcolor = (color_t){255,255,255};

#if 1
    widget_t  *wnd, *wnd1, *lbl;
    wnd1 = WIDGET(window_create(250,100));
    lbl = WIDGET(label_create("Lorem ipsum dolor sit amet, consectetur adipiscing elit. Etiam venenatis viverra quam, id pretium mi maximus quis. Vestibulum vestibulum arcu in rutrum sagittis. Etiam interdum nec mi sit amet tincidunt.", wnd1));
    window_set_name(WINDOW(wnd1), "another window");
    // window_move(WINDOW(wnd1),10,10); // ERROR here!! (WTF)

    wnd = WIDGET(window_create(150,100));
    lbl = WIDGET(label_create("Welcome to the DiyOS basic GUI!", wnd));
    button_size( button_create("OK", wnd), (wnd->w - 40) / 2, (wnd->h - 12 - 10), 40, 12 );
    window_set_name(WINDOW(wnd), "Welcome Message!");
    gui_set_active_window(wnd);

    window_move(WINDOW(wnd),50,70);

#endif

    // the taskbar space
    widget_set_padding(desktop_window,0,0,0,/*TASK_BAR_HEIGHT*/0); // childs cannot use TaskBar area

    // taskbar_create_windows(desktop_taskbar);

    // mouse position
    pointerX = (gfx_width() - POINTER_W) / 2;
    pointerY = (gfx_height() - POINTER_H) / 2;
}

void
gui() {
    gui_task = task_create((uint32_t)gui_main, "gui", TS_READY );
    task_add(gui_task);
}

widget_t*
gui_widget_root() {
    return desktop_window;
}

void
gui_pointer_draw(uint32_t x, uint32_t y) {

  gfx_rect( x - POINTER_W/2 - 1, y - POINTER_H/2 - 1,
            x + POINTER_W/2 + 1, y + POINTER_H/2 + 1,
            (color_t) {64,64,64} );

  gfx_rect( x - POINTER_W/2, y - POINTER_H/2,
            x + POINTER_W/2, y + POINTER_H/2,
            (color_t) {255,255,0} );

}

void
taskbar_create_windows(widget_t* taskbar) {
  widget_t* desktop = gui_widget_root();
  widget_t* child;
  widget_t* lbl;
  uint32_t x = 0;

  if(!taskbar || !desktop )
    return;

  child = desktop->child;
  while( child ) {

    if(child->class == W_WINDOW) {
      lbl = WIDGET(label_create(WINDOW(child)->name, taskbar));
      lbl->x = x;
      x += lbl->w + lbl->padding_right;
    }

    child = child->next;
  }
}

void
gui_set_active_window(window_t* window) {
  widget_t* child, *root;

  root = gui_widget_root();
  if(root == NULL)
    return;

  child = root->child;
  while( child ) {
    if(child->class == W_WINDOW) {
      if(child == window)
        WINDOW(child)->active = 1;
      else
        WINDOW(child)->active = 0;
    }
    child = child->next;
  }
}

window_t*
gui_get_active_window() {
  //TODO: create GUI class struct with active_window member
  widget_t* child, *root;

  root = gui_widget_root();
  if(root == NULL)
    return;

  child = root->child;
  while( child ) {
    if(child->class == W_WINDOW) {
      if(WINDOW(child)->active == 1)
        return child;
    }
    child = child->next;
  }

  return NULL;
}

void
gui_draw() {
  widget_draw(desktop_window);

  // gui_pointer_draw(pointerX, pointerY);
}
