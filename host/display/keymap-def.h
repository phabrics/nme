static int keycode_min = 8;
static int keycode_max = 255;

static int keycode_to_modifier[] = {
  TME_KEYBOARD_MODIFIER_NONE,
  TME_KEYBOARD_MODIFIER_NONE,
  TME_KEYBOARD_MODIFIER_NONE,
  TME_KEYBOARD_MODIFIER_NONE,
  TME_KEYBOARD_MODIFIER_NONE,
  TME_KEYBOARD_MODIFIER_NONE,
  TME_KEYBOARD_MODIFIER_NONE,
  TME_KEYBOARD_MODIFIER_NONE,
  TME_KEYBOARD_MODIFIER_MOD5,
  TME_KEYBOARD_MODIFIER_NONE,
  TME_KEYBOARD_MODIFIER_NONE,
  TME_KEYBOARD_MODIFIER_NONE,
  TME_KEYBOARD_MODIFIER_NONE,
  TME_KEYBOARD_MODIFIER_NONE,
  TME_KEYBOARD_MODIFIER_NONE,
  TME_KEYBOARD_MODIFIER_NONE,
  TME_KEYBOARD_MODIFIER_NONE,
  TME_KEYBOARD_MODIFIER_NONE,
  TME_KEYBOARD_MODIFIER_NONE,
  TME_KEYBOARD_MODIFIER_NONE,
  TME_KEYBOARD_MODIFIER_NONE,
  TME_KEYBOARD_MODIFIER_NONE,
  TME_KEYBOARD_MODIFIER_NONE,
  TME_KEYBOARD_MODIFIER_NONE,
  TME_KEYBOARD_MODIFIER_NONE,
  TME_KEYBOARD_MODIFIER_NONE,
  TME_KEYBOARD_MODIFIER_NONE,
  TME_KEYBOARD_MODIFIER_NONE,
  TME_KEYBOARD_MODIFIER_NONE,
  TME_KEYBOARD_MODIFIER_NONE,
  TME_KEYBOARD_MODIFIER_NONE,
  TME_KEYBOARD_MODIFIER_NONE,
  TME_KEYBOARD_MODIFIER_NONE,
  TME_KEYBOARD_MODIFIER_NONE,
  TME_KEYBOARD_MODIFIER_NONE,
  TME_KEYBOARD_MODIFIER_NONE,
  TME_KEYBOARD_MODIFIER_NONE,
  TME_KEYBOARD_MODIFIER_CONTROL,
  TME_KEYBOARD_MODIFIER_NONE,
  TME_KEYBOARD_MODIFIER_NONE,
  TME_KEYBOARD_MODIFIER_NONE,
  TME_KEYBOARD_MODIFIER_NONE,
  TME_KEYBOARD_MODIFIER_NONE,
  TME_KEYBOARD_MODIFIER_NONE,
  TME_KEYBOARD_MODIFIER_NONE,
  TME_KEYBOARD_MODIFIER_NONE,
  TME_KEYBOARD_MODIFIER_NONE,
  TME_KEYBOARD_MODIFIER_NONE,
  TME_KEYBOARD_MODIFIER_NONE,
  TME_KEYBOARD_MODIFIER_NONE,
  TME_KEYBOARD_MODIFIER_SHIFT,
  TME_KEYBOARD_MODIFIER_NONE,
  TME_KEYBOARD_MODIFIER_NONE,
  TME_KEYBOARD_MODIFIER_NONE,
  TME_KEYBOARD_MODIFIER_NONE,
  TME_KEYBOARD_MODIFIER_NONE,
  TME_KEYBOARD_MODIFIER_NONE,
  TME_KEYBOARD_MODIFIER_NONE,
  TME_KEYBOARD_MODIFIER_NONE,
  TME_KEYBOARD_MODIFIER_NONE,
  TME_KEYBOARD_MODIFIER_NONE,
  TME_KEYBOARD_MODIFIER_NONE,
  TME_KEYBOARD_MODIFIER_SHIFT,
  TME_KEYBOARD_MODIFIER_NONE,
  TME_KEYBOARD_MODIFIER_MOD1,
  TME_KEYBOARD_MODIFIER_NONE,
  TME_KEYBOARD_MODIFIER_LOCK,
  TME_KEYBOARD_MODIFIER_NONE,
  TME_KEYBOARD_MODIFIER_NONE,
  TME_KEYBOARD_MODIFIER_NONE,
  TME_KEYBOARD_MODIFIER_NONE,
  TME_KEYBOARD_MODIFIER_NONE,
  TME_KEYBOARD_MODIFIER_NONE,
  TME_KEYBOARD_MODIFIER_NONE,
  TME_KEYBOARD_MODIFIER_NONE,
  TME_KEYBOARD_MODIFIER_NONE,
  TME_KEYBOARD_MODIFIER_NONE,
  TME_KEYBOARD_MODIFIER_MOD2,
  TME_KEYBOARD_MODIFIER_NONE,
  TME_KEYBOARD_MODIFIER_NONE,
  TME_KEYBOARD_MODIFIER_NONE,
  TME_KEYBOARD_MODIFIER_NONE,
  TME_KEYBOARD_MODIFIER_NONE,
  TME_KEYBOARD_MODIFIER_NONE,
  TME_KEYBOARD_MODIFIER_NONE,
  TME_KEYBOARD_MODIFIER_NONE,
  TME_KEYBOARD_MODIFIER_NONE,
  TME_KEYBOARD_MODIFIER_NONE,
  TME_KEYBOARD_MODIFIER_NONE,
  TME_KEYBOARD_MODIFIER_NONE,
  TME_KEYBOARD_MODIFIER_NONE,
  TME_KEYBOARD_MODIFIER_NONE,
  TME_KEYBOARD_MODIFIER_NONE,
  TME_KEYBOARD_MODIFIER_NONE,
  TME_KEYBOARD_MODIFIER_NONE,
  TME_KEYBOARD_MODIFIER_NONE,
  TME_KEYBOARD_MODIFIER_NONE,
  TME_KEYBOARD_MODIFIER_NONE,
  TME_KEYBOARD_MODIFIER_NONE,
  TME_KEYBOARD_MODIFIER_NONE,
  TME_KEYBOARD_MODIFIER_NONE,
  TME_KEYBOARD_MODIFIER_NONE,
  TME_KEYBOARD_MODIFIER_NONE,
  TME_KEYBOARD_MODIFIER_NONE,
  TME_KEYBOARD_MODIFIER_NONE,
  TME_KEYBOARD_MODIFIER_NONE,
  TME_KEYBOARD_MODIFIER_NONE,
  TME_KEYBOARD_MODIFIER_NONE,
  TME_KEYBOARD_MODIFIER_NONE,
  TME_KEYBOARD_MODIFIER_CONTROL,
  TME_KEYBOARD_MODIFIER_NONE,
  TME_KEYBOARD_MODIFIER_NONE,
  TME_KEYBOARD_MODIFIER_NONE,
  TME_KEYBOARD_MODIFIER_MOD1,
  TME_KEYBOARD_MODIFIER_NONE,
  TME_KEYBOARD_MODIFIER_MOD4,
  TME_KEYBOARD_MODIFIER_MOD4,
  TME_KEYBOARD_MODIFIER_NONE,
  TME_KEYBOARD_MODIFIER_NONE,
  TME_KEYBOARD_MODIFIER_NONE,
  TME_KEYBOARD_MODIFIER_NONE,
  TME_KEYBOARD_MODIFIER_NONE,
  TME_KEYBOARD_MODIFIER_NONE,
  TME_KEYBOARD_MODIFIER_NONE,
  TME_KEYBOARD_MODIFIER_MOD5,
  TME_KEYBOARD_MODIFIER_NONE,
  TME_KEYBOARD_MODIFIER_NONE,
  TME_KEYBOARD_MODIFIER_MOD4,
  TME_KEYBOARD_MODIFIER_MOD4,
  TME_KEYBOARD_MODIFIER_NONE,
  TME_KEYBOARD_MODIFIER_NONE,
  TME_KEYBOARD_MODIFIER_NONE,
  TME_KEYBOARD_MODIFIER_NONE,
  TME_KEYBOARD_MODIFIER_NONE,
  TME_KEYBOARD_MODIFIER_NONE,
  TME_KEYBOARD_MODIFIER_NONE,
  TME_KEYBOARD_MODIFIER_NONE,
  TME_KEYBOARD_MODIFIER_NONE,
  TME_KEYBOARD_MODIFIER_NONE,
  TME_KEYBOARD_MODIFIER_NONE,
  TME_KEYBOARD_MODIFIER_NONE,
  TME_KEYBOARD_MODIFIER_NONE,
  TME_KEYBOARD_MODIFIER_NONE,
  TME_KEYBOARD_MODIFIER_NONE,
  TME_KEYBOARD_MODIFIER_NONE,
  TME_KEYBOARD_MODIFIER_NONE,
  TME_KEYBOARD_MODIFIER_NONE,
  TME_KEYBOARD_MODIFIER_NONE,
  TME_KEYBOARD_MODIFIER_NONE,
  TME_KEYBOARD_MODIFIER_NONE,
  TME_KEYBOARD_MODIFIER_NONE,
  TME_KEYBOARD_MODIFIER_NONE,
  TME_KEYBOARD_MODIFIER_NONE,
  TME_KEYBOARD_MODIFIER_NONE,
  TME_KEYBOARD_MODIFIER_NONE,
  TME_KEYBOARD_MODIFIER_NONE,
  TME_KEYBOARD_MODIFIER_MOD1,
  TME_KEYBOARD_MODIFIER_NONE,
  TME_KEYBOARD_MODIFIER_NONE,
  TME_KEYBOARD_MODIFIER_NONE,
  TME_KEYBOARD_MODIFIER_NONE,
  TME_KEYBOARD_MODIFIER_NONE,
  TME_KEYBOARD_MODIFIER_NONE,
  TME_KEYBOARD_MODIFIER_NONE,
  TME_KEYBOARD_MODIFIER_NONE,
  TME_KEYBOARD_MODIFIER_NONE,
  TME_KEYBOARD_MODIFIER_NONE,
  TME_KEYBOARD_MODIFIER_NONE,
  TME_KEYBOARD_MODIFIER_NONE,
  TME_KEYBOARD_MODIFIER_NONE,
  TME_KEYBOARD_MODIFIER_NONE,
  TME_KEYBOARD_MODIFIER_NONE,
  TME_KEYBOARD_MODIFIER_NONE,
  TME_KEYBOARD_MODIFIER_NONE,
  TME_KEYBOARD_MODIFIER_NONE,
  TME_KEYBOARD_MODIFIER_NONE,
  TME_KEYBOARD_MODIFIER_NONE,
  TME_KEYBOARD_MODIFIER_NONE,
  TME_KEYBOARD_MODIFIER_NONE,
  TME_KEYBOARD_MODIFIER_NONE,
  TME_KEYBOARD_MODIFIER_NONE,
  TME_KEYBOARD_MODIFIER_NONE,
  TME_KEYBOARD_MODIFIER_NONE,
  TME_KEYBOARD_MODIFIER_NONE,
  TME_KEYBOARD_MODIFIER_NONE,
  TME_KEYBOARD_MODIFIER_NONE,
  TME_KEYBOARD_MODIFIER_NONE,
  TME_KEYBOARD_MODIFIER_NONE,
  TME_KEYBOARD_MODIFIER_NONE,
  TME_KEYBOARD_MODIFIER_NONE,
  TME_KEYBOARD_MODIFIER_NONE,
  TME_KEYBOARD_MODIFIER_NONE,
  TME_KEYBOARD_MODIFIER_NONE,
  TME_KEYBOARD_MODIFIER_NONE,
  TME_KEYBOARD_MODIFIER_NONE,
  TME_KEYBOARD_MODIFIER_NONE,
  TME_KEYBOARD_MODIFIER_NONE,
  TME_KEYBOARD_MODIFIER_NONE,
  TME_KEYBOARD_MODIFIER_NONE,
  TME_KEYBOARD_MODIFIER_NONE,
  TME_KEYBOARD_MODIFIER_NONE,
  TME_KEYBOARD_MODIFIER_NONE,
  TME_KEYBOARD_MODIFIER_NONE,
  TME_KEYBOARD_MODIFIER_NONE,
  TME_KEYBOARD_MODIFIER_NONE,
  TME_KEYBOARD_MODIFIER_NONE,
  TME_KEYBOARD_MODIFIER_NONE,
  TME_KEYBOARD_MODIFIER_NONE,
  TME_KEYBOARD_MODIFIER_NONE,
  TME_KEYBOARD_MODIFIER_NONE,
  TME_KEYBOARD_MODIFIER_NONE,
  TME_KEYBOARD_MODIFIER_NONE,
  TME_KEYBOARD_MODIFIER_NONE,
  TME_KEYBOARD_MODIFIER_NONE,
  TME_KEYBOARD_MODIFIER_NONE,
  TME_KEYBOARD_MODIFIER_NONE,
  TME_KEYBOARD_MODIFIER_NONE,
  TME_KEYBOARD_MODIFIER_NONE,
  TME_KEYBOARD_MODIFIER_NONE,
  TME_KEYBOARD_MODIFIER_NONE,
  TME_KEYBOARD_MODIFIER_NONE,
  TME_KEYBOARD_MODIFIER_NONE,
  TME_KEYBOARD_MODIFIER_NONE,
  TME_KEYBOARD_MODIFIER_NONE,
  TME_KEYBOARD_MODIFIER_NONE,
  TME_KEYBOARD_MODIFIER_NONE,
  TME_KEYBOARD_MODIFIER_NONE,
  TME_KEYBOARD_MODIFIER_NONE,
  TME_KEYBOARD_MODIFIER_NONE,
  TME_KEYBOARD_MODIFIER_NONE,
  TME_KEYBOARD_MODIFIER_NONE,
  TME_KEYBOARD_MODIFIER_NONE,
  TME_KEYBOARD_MODIFIER_NONE,
  TME_KEYBOARD_MODIFIER_NONE,
  TME_KEYBOARD_MODIFIER_NONE,
  TME_KEYBOARD_MODIFIER_NONE,
  TME_KEYBOARD_MODIFIER_NONE,
  TME_KEYBOARD_MODIFIER_NONE,
  TME_KEYBOARD_MODIFIER_NONE,
  TME_KEYBOARD_MODIFIER_NONE,
  TME_KEYBOARD_MODIFIER_NONE,
  TME_KEYBOARD_MODIFIER_NONE,
  TME_KEYBOARD_MODIFIER_NONE,
  TME_KEYBOARD_MODIFIER_NONE,
  TME_KEYBOARD_MODIFIER_NONE,
  TME_KEYBOARD_MODIFIER_NONE,
  TME_KEYBOARD_MODIFIER_NONE,
  TME_KEYBOARD_MODIFIER_NONE,
  TME_KEYBOARD_MODIFIER_NONE,
  TME_KEYBOARD_MODIFIER_NONE,
  TME_KEYBOARD_MODIFIER_NONE,
  TME_KEYBOARD_MODIFIER_NONE,
  TME_KEYBOARD_MODIFIER_NONE,
  TME_KEYBOARD_MODIFIER_NONE,
  TME_KEYBOARD_MODIFIER_NONE,
  TME_KEYBOARD_MODIFIER_NONE
};

static int keymap_width = 7;

static tme_keyboard_keyval_t keymap[] = {
  0xff7e, 0x0, 0xff7e, 0x0, 0x0, 0x0, 0x0,
  0xff1b, 0x0, 0xff1b, 0x0, 0x0, 0x0, 0x0,
  0x31, 0x21, 0x31, 0x21, 0x0, 0x0, 0x0,
  0x32, 0x40, 0x32, 0x40, 0x0, 0x0, 0x0,
  0x33, 0x23, 0x33, 0x23, 0x0, 0x0, 0x0,
  0x34, 0x24, 0x34, 0x24, 0x0, 0x0, 0x0,
  0x35, 0x25, 0x35, 0x25, 0x0, 0x0, 0x0,
  0x36, 0x5e, 0x36, 0x5e, 0x0, 0x0, 0x0,
  0x37, 0x26, 0x37, 0x26, 0x0, 0x0, 0x0,
  0x38, 0x2a, 0x38, 0x2a, 0x0, 0x0, 0x0,
  0x39, 0x28, 0x39, 0x28, 0x0, 0x0, 0x0,
  0x30, 0x29, 0x30, 0x29, 0x0, 0x0, 0x0,
  0x2d, 0x5f, 0x2d, 0x5f, 0x0, 0x0, 0x0,
  0x3d, 0x2b, 0x3d, 0x2b, 0x0, 0x0, 0x0,
  0xff08, 0xff08, 0xff08, 0xff08, 0x0, 0x0, 0x0,
  0xff09, 0xfe20, 0xff09, 0xfe20, 0x0, 0x0, 0x0,
  0x71, 0x51, 0x71, 0x51, 0x0, 0x0, 0x0,
  0x77, 0x57, 0x77, 0x57, 0x0, 0x0, 0x0,
  0x65, 0x45, 0x65, 0x45, 0x0, 0x0, 0x0,
  0x72, 0x52, 0x72, 0x52, 0x0, 0x0, 0x0,
  0x74, 0x54, 0x74, 0x54, 0x0, 0x0, 0x0,
  0x79, 0x59, 0x79, 0x59, 0x0, 0x0, 0x0,
  0x75, 0x55, 0x75, 0x55, 0x0, 0x0, 0x0,
  0x69, 0x49, 0x69, 0x49, 0x0, 0x0, 0x0,
  0x6f, 0x4f, 0x6f, 0x4f, 0x0, 0x0, 0x0,
  0x70, 0x50, 0x70, 0x50, 0x0, 0x0, 0x0,
  0x5b, 0x7b, 0x5b, 0x7b, 0x0, 0x0, 0x0,
  0x5d, 0x7d, 0x5d, 0x7d, 0x0, 0x0, 0x0,
  0xff0d, 0x0, 0xff0d, 0x0, 0x0, 0x0, 0x0,
  0xffe3, 0x0, 0xffe3, 0x0, 0x0, 0x0, 0x0,
  0x61, 0x41, 0x61, 0x41, 0x0, 0x0, 0x0,
  0x73, 0x53, 0x73, 0x53, 0x0, 0x0, 0x0,
  0x64, 0x44, 0x64, 0x44, 0x0, 0x0, 0x0,
  0x66, 0x46, 0x66, 0x46, 0x0, 0x0, 0x0,
  0x67, 0x47, 0x67, 0x47, 0x0, 0x0, 0x0,
  0x68, 0x48, 0x68, 0x48, 0x0, 0x0, 0x0,
  0x6a, 0x4a, 0x6a, 0x4a, 0x0, 0x0, 0x0,
  0x6b, 0x4b, 0x6b, 0x4b, 0x0, 0x0, 0x0,
  0x6c, 0x4c, 0x6c, 0x4c, 0x0, 0x0, 0x0,
  0x3b, 0x3a, 0x3b, 0x3a, 0x0, 0x0, 0x0,
  0x27, 0x22, 0x27, 0x22, 0x0, 0x0, 0x0,
  0x60, 0x7e, 0x60, 0x7e, 0x0, 0x0, 0x0,
  0xffe1, 0x0, 0xffe1, 0x0, 0x0, 0x0, 0x0,
  0x5c, 0x7c, 0x5c, 0x7c, 0x0, 0x0, 0x0,
  0x7a, 0x5a, 0x7a, 0x5a, 0x0, 0x0, 0x0,
  0x78, 0x58, 0x78, 0x58, 0x0, 0x0, 0x0,
  0x63, 0x43, 0x63, 0x43, 0x0, 0x0, 0x0,
  0x76, 0x56, 0x76, 0x56, 0x0, 0x0, 0x0,
  0x62, 0x42, 0x62, 0x42, 0x0, 0x0, 0x0,
  0x6e, 0x4e, 0x6e, 0x4e, 0x0, 0x0, 0x0,
  0x6d, 0x4d, 0x6d, 0x4d, 0x0, 0x0, 0x0,
  0x2c, 0x3c, 0x2c, 0x3c, 0x0, 0x0, 0x0,
  0x2e, 0x3e, 0x2e, 0x3e, 0x0, 0x0, 0x0,
  0x2f, 0x3f, 0x2f, 0x3f, 0x0, 0x0, 0x0,
  0xffe2, 0x0, 0xffe2, 0x0, 0x0, 0x0, 0x0,
  0xffaa, 0xffaa, 0xffaa, 0xffaa, 0xffaa, 0xffaa, 0x1008fe21,
  0xffe9, 0xffe7, 0xffe9, 0xffe7, 0x0, 0x0, 0x0,
  0x20, 0x0, 0x20, 0x0, 0x0, 0x0, 0x0,
  0xffe5, 0x0, 0xffe5, 0x0, 0x0, 0x0, 0x0,
  0xffbe, 0xffbe, 0xffbe, 0xffbe, 0xffbe, 0xffbe, 0x1008fe01,
  0xffbf, 0xffbf, 0xffbf, 0xffbf, 0xffbf, 0xffbf, 0x1008fe02,
  0xffc0, 0xffc0, 0xffc0, 0xffc0, 0xffc0, 0xffc0, 0x1008fe03,
  0xffc1, 0xffc1, 0xffc1, 0xffc1, 0xffc1, 0xffc1, 0x1008fe04,
  0xffc2, 0xffc2, 0xffc2, 0xffc2, 0xffc2, 0xffc2, 0x1008fe05,
  0xffc3, 0xffc3, 0xffc3, 0xffc3, 0xffc3, 0xffc3, 0x1008fe06,
  0xffc4, 0xffc4, 0xffc4, 0xffc4, 0xffc4, 0xffc4, 0x1008fe07,
  0xffc5, 0xffc5, 0xffc5, 0xffc5, 0xffc5, 0xffc5, 0x1008fe08,
  0xffc6, 0xffc6, 0xffc6, 0xffc6, 0xffc6, 0xffc6, 0x1008fe09,
  0xffc7, 0xffc7, 0xffc7, 0xffc7, 0xffc7, 0xffc7, 0x1008fe0a,
  0xff7f, 0x0, 0xff7f, 0x0, 0x0, 0x0, 0x0,
  0xff14, 0x0, 0xff14, 0x0, 0x0, 0x0, 0x0,
  0xff95, 0xffb7, 0xff95, 0xffb7, 0x0, 0x0, 0x0,
  0xff97, 0xffb8, 0xff97, 0xffb8, 0x0, 0x0, 0x0,
  0xff9a, 0xffb9, 0xff9a, 0xffb9, 0x0, 0x0, 0x0,
  0xffad, 0xffad, 0xffad, 0xffad, 0xffad, 0xffad, 0x1008fe23,
  0xff96, 0xffb4, 0xff96, 0xffb4, 0x0, 0x0, 0x0,
  0xff9d, 0xffb5, 0xff9d, 0xffb5, 0x0, 0x0, 0x0,
  0xff98, 0xffb6, 0xff98, 0xffb6, 0x0, 0x0, 0x0,
  0xffab, 0xffab, 0xffab, 0xffab, 0xffab, 0xffab, 0x1008fe22,
  0xff9c, 0xffb1, 0xff9c, 0xffb1, 0x0, 0x0, 0x0,
  0xff99, 0xffb2, 0xff99, 0xffb2, 0x0, 0x0, 0x0,
  0xff9b, 0xffb3, 0xff9b, 0xffb3, 0x0, 0x0, 0x0,
  0xff9e, 0xffb0, 0xff9e, 0xffb0, 0x0, 0x0, 0x0,
  0xff9f, 0xffae, 0xff9f, 0xffae, 0x0, 0x0, 0x0,
  0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
  0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
  0x3c, 0x3e, 0x3c, 0x3e, 0x7c, 0xa6, 0x7c,
  0xffc8, 0xffc8, 0xffc8, 0xffc8, 0xffc8, 0xffc8, 0x1008fe0b,
  0xffc9, 0xffc9, 0xffc9, 0xffc9, 0xffc9, 0xffc9, 0x1008fe0c,
  0xff50, 0x0, 0xff50, 0x0, 0x0, 0x0, 0x0,
  0xff52, 0x0, 0xff52, 0x0, 0x0, 0x0, 0x0,
  0xff55, 0x0, 0xff55, 0x0, 0x0, 0x0, 0x0,
  0xff51, 0x0, 0xff51, 0x0, 0x0, 0x0, 0x0,
  0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
  0xff53, 0x0, 0xff53, 0x0, 0x0, 0x0, 0x0,
  0xff57, 0x0, 0xff57, 0x0, 0x0, 0x0, 0x0,
  0xff54, 0x0, 0xff54, 0x0, 0x0, 0x0, 0x0,
  0xff56, 0x0, 0xff56, 0x0, 0x0, 0x0, 0x0,
  0xff63, 0x0, 0xff63, 0x0, 0x0, 0x0, 0x0,
  0xffff, 0x0, 0xffff, 0x0, 0x0, 0x0, 0x0,
  0xff8d, 0x0, 0xff8d, 0x0, 0x0, 0x0, 0x0,
  0xffe4, 0x0, 0xffe4, 0x0, 0x0, 0x0, 0x0,
  0xff13, 0xff6b, 0xff13, 0xff6b, 0x0, 0x0, 0x0,
  0xff61, 0xff15, 0xff61, 0xff15, 0x0, 0x0, 0x0,
  0xffaf, 0xffaf, 0xffaf, 0xffaf, 0xffaf, 0xffaf, 0x1008fe20,
  0xffea, 0xffe8, 0xffea, 0xffe8, 0x0, 0x0, 0x0,
  0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
  0xffeb, 0x0, 0xffeb, 0x0, 0x0, 0x0, 0x0,
  0xffec, 0x0, 0xffec, 0x0, 0x0, 0x0, 0x0,
  0xff67, 0x0, 0xff67, 0x0, 0x0, 0x0, 0x0,
  0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
  0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
  0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
  0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
  0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
  0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
  0xfe03, 0x0, 0xfe03, 0x0, 0x0, 0x0, 0x0,
  0x0, 0xffe9, 0x0, 0xffe9, 0x0, 0x0, 0x0,
  0xffbd, 0x0, 0xffbd, 0x0, 0x0, 0x0, 0x0,
  0x0, 0xffeb, 0x0, 0xffeb, 0x0, 0x0, 0x0,
  0x0, 0xffed, 0x0, 0xffed, 0x0, 0x0, 0x0,
  0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
  0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
  0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
  0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
  0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
  0xffae, 0xffae, 0xffae, 0xffae, 0x0, 0x0, 0x0,
  0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
  0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
  0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
  0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
  0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
  0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
  0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
  0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
  0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
  0x1008ff16, 0x0, 0x1008ff16, 0x0, 0x0, 0x0, 0x0,
  0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
  0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
  0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
  0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
  0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
  0x1008ff2f, 0x0, 0x1008ff2f, 0x0, 0x0, 0x0, 0x0,
  0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
  0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
  0x1008ff17, 0x0, 0x1008ff17, 0x0, 0x0, 0x0, 0x0,
  0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
  0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
  0x0, 0xffe7, 0x0, 0xffe7, 0x0, 0x0, 0x0,
  0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
  0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
  0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
  0x1008ff12, 0x0, 0x1008ff12, 0x0, 0x0, 0x0, 0x0,
  0x1008ff1d, 0x0, 0x1008ff1d, 0x0, 0x0, 0x0, 0x0,
  0x1008ff14, 0x1008ff31, 0x1008ff14, 0x1008ff31, 0x0, 0x0, 0x0,
  0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
  0x1008ff15, 0x1008ff2c, 0x1008ff15, 0x1008ff2c, 0x0, 0x0, 0x0,
  0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
  0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
  0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
  0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
  0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
  0x1008ff2c, 0x0, 0x1008ff2c, 0x0, 0x0, 0x0, 0x0,
  0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
  0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
  0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
  0x1008ff11, 0x0, 0x1008ff11, 0x0, 0x0, 0x0, 0x0,
  0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
  0x1008ff13, 0x0, 0x1008ff13, 0x0, 0x0, 0x0, 0x0,
  0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
  0x1008ff2e, 0x0, 0x1008ff2e, 0x0, 0x0, 0x0, 0x0,
  0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
  0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
  0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
  0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
  0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
  0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
  0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
  0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
  0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
  0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
  0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
  0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
  0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
  0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
  0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
  0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
  0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
  0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
  0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
  0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
  0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
  0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
  0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
  0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
  0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
  0x1008ff2c, 0x0, 0x1008ff2c, 0x0, 0x0, 0x0, 0x0,
  0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
  0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
  0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
  0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
  0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
  0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
  0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
  0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
  0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
  0x1008ff59, 0x0, 0x1008ff59, 0x0, 0x0, 0x0, 0x0,
  0x1008ff04, 0x0, 0x1008ff04, 0x0, 0x0, 0x0, 0x0,
  0x1008ff06, 0x0, 0x1008ff06, 0x0, 0x0, 0x0, 0x0,
  0x1008ff05, 0x0, 0x1008ff05, 0x0, 0x0, 0x0, 0x0,
  0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
  0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
  0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
  0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
  0x1008ff2a, 0x0, 0x1008ff2a, 0x0, 0x0, 0x0, 0x0,
  0x1008ff10, 0x0, 0x1008ff10, 0x0, 0x0, 0x0, 0x0,
  0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
  0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
  0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
  0x1008ff2b, 0x0, 0x1008ff2b, 0x0, 0x0, 0x0, 0x0,
  0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
  0x1008ff1b, 0x0, 0x1008ff1b, 0x0, 0x0, 0x0, 0x0,
  0x1008ff30, 0x0, 0x1008ff30, 0x0, 0x0, 0x0, 0x0,
  0x1008ff73, 0x0, 0x1008ff73, 0x0, 0x0, 0x0, 0x0,
  0x1008ff28, 0x0, 0x1008ff28, 0x0, 0x0, 0x0, 0x0,
  0x1008ff27, 0x0, 0x1008ff27, 0x0, 0x0, 0x0, 0x0,
  0x1008ff26, 0x0, 0x1008ff26, 0x0, 0x0, 0x0, 0x0,
  0x1008ff33, 0x0, 0x1008ff33, 0x0, 0x0, 0x0, 0x0,
  0x1008ff19, 0x0, 0x1008ff19, 0x0, 0x0, 0x0, 0x0,
  0x1008ff32, 0x0, 0x1008ff32, 0x0, 0x0, 0x0, 0x0,
  0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
  0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
  0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
  0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
  0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
  0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
  0x1008ff93, 0x0, 0x1008ff93, 0x0, 0x0, 0x0, 0x0,
  0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
  0x1008ff95, 0x0, 0x1008ff95, 0x0, 0x0, 0x0, 0x0,
  0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
  0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
  0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
  0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
  0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
  0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
  0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
  0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
  0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0
};
