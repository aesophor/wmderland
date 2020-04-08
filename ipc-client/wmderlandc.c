// Copyright (c) 2018-2019 Marco Wang <m.aesophor@gmail.com>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <X11/Xlib.h>
#include <X11/Xatom.h>

#define WMDERLAND_CLIENT_EVENT "WMDERLAND_CLIENT_EVENT"
#define CMD_ID 0
#define HAS_ARGUMENT 1
#define ARGUMENT 2

typedef enum arg_type {
  ARG_TYPE_DEC, // Decimal
  ARG_TYPE_HEX, // Hexidecimal
  ARG_TYPE_NONE // Dummy
} ArgType;

typedef struct command_t {
  const char *cmd;
  int argc;
  ArgType arg_type;
} Command;

static Command cmd_table[] = {
  {"navigate_left",            0, ARG_TYPE_NONE},
  {"navigate_right",           0, ARG_TYPE_NONE},
  {"navigate_up",              0, ARG_TYPE_NONE},
  {"navigate_down",            0, ARG_TYPE_NONE},
  {"float_move_left",          0, ARG_TYPE_NONE},
  {"float_move_right",         0, ARG_TYPE_NONE},
  {"float_move_up",            0, ARG_TYPE_NONE},
  {"float_move_down",          0, ARG_TYPE_NONE},
  {"float_resize_left",        0, ARG_TYPE_NONE},
  {"float_resize_right",       0, ARG_TYPE_NONE},
  {"float_resize_up",          0, ARG_TYPE_NONE},
  {"float_resize_down",        0, ARG_TYPE_NONE},
  {"tile_h",                   0, ARG_TYPE_NONE},
  {"tile_v",                   0, ARG_TYPE_NONE},
  {"toggle_floating",          0, ARG_TYPE_NONE},
  {"toggle_fullscreen",        0, ARG_TYPE_NONE},
  {"goto_workspace",           1, ARG_TYPE_DEC },
  {"workspace",                1, ARG_TYPE_DEC },
  {"move_window_to_workspace", 1, ARG_TYPE_DEC },
  {"swap",                     1, ARG_TYPE_DEC },
  {"kill",                     0, ARG_TYPE_NONE},
  {"exit",                     0, ARG_TYPE_NONE},
  {"reload",                   0, ARG_TYPE_NONE},
  {"debug_crash",              0, ARG_TYPE_NONE},
  {NULL,                       0, ARG_TYPE_NONE}
};


int main(int argc, char *args[]) {
  int ret = EXIT_FAILURE;
  int cmd_id = 0;
  char err_msg[64] = {0};
  Command *cmd = NULL;
  Display *dpy = XOpenDisplay(None);
  Window root_window = DefaultRootWindow(dpy);
  XEvent msg;

  if (argc < 2) {
    printf("usage: %s <command> [args...]\n", args[0]);
    return EXIT_SUCCESS;
  }
 
  for (cmd_id = 0; cmd_table[cmd_id].cmd; cmd_id++) {
    if (!strcmp(cmd_table[cmd_id].cmd, args[1])) {
      cmd = &cmd_table[cmd_id];
      break;
    }
  }

  if (!cmd) {
    sprintf(err_msg, "No such command: %s\n", args[1]);
    goto end;
  }

  if (argc - 2 < cmd->argc) {
    sprintf(err_msg, "Too few arguments, expected %d\n", cmd->argc);
    goto end;
  }

  if (!dpy) {
    sprintf(err_msg, "Failed to open display");
    goto end;
  }

  memset(&msg, 0, sizeof(msg));
  msg.xclient.type = ClientMessage;
  msg.xclient.message_type = XInternAtom(dpy, WMDERLAND_CLIENT_EVENT, False);
  msg.xclient.window = root_window;
  msg.xclient.format = 32;
  msg.xclient.data.l[CMD_ID] = cmd_id;
  msg.xclient.data.l[HAS_ARGUMENT] = True;

  switch (cmd->arg_type) {
    case ARG_TYPE_DEC:
      msg.xclient.data.l[ARGUMENT] = strtoul(args[2], NULL, 10);
      break;
    case ARG_TYPE_HEX:
      msg.xclient.data.l[ARGUMENT] = strtoul(args[2], NULL, 16);
      break;
    default:
      msg.xclient.data.l[HAS_ARGUMENT] = False;
      break;
  }
  XSendEvent(dpy, root_window, False, SubstructureRedirectMask, &msg);

end:
  if (dpy) {
    XCloseDisplay(dpy);
  }
  fputs(err_msg, stderr);
  return ret;
}
