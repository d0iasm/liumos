#include "liumos.h"

#define KEYID_MASK_ID 0x007f
#define KEYID_MASK_BREAK 0x0080
#define KEYID_MASK_TENKEY 0x0100
#define KEYID_MASK_STATE_SHIFT 0x0200
#define KEYID_MASK_STATE_CTRL 0x0400
#define KEYID_MASK_STATE_ALT 0x0800
#define KEYID_MASK_STATE_LOCK_SCROOL 0x1000
#define KEYID_MASK_STATE_LOCK_NUM 0x2000
#define KEYID_MASK_STATE_LOCK_CAPS 0x4000
#define KEYID_MASK_EXTENDED 0x8000

#define KEYID_ASCII_NUL 0x0000
#define KEYID_ASCII_SOH 0x0001
#define KEYID_ASCII_STX 0x0002
#define KEYID_ASCII_ETX 0x0003
#define KEYID_ASCII_EOT 0x0004
#define KEYID_ASCII_ENQ 0x0005
#define KEYID_ASCII_ACK 0x0006
#define KEYID_ASCII_BEL 0x0007
#define KEYID_ASCII_BS 0x0008 /*0x0e*/
#define KEYID_ASCII_HT 0x0009 /*0x0f*/
#define KEYID_ASCII_LF 0x000a /*0x1c*/
#define KEYID_ASCII_VT 0x000b
#define KEYID_ASCII_FF 0x000c
#define KEYID_ASCII_CR 0x000d
#define KEYID_ASCII_SO 0x000e
#define KEYID_ASCII_SI 0x000f
#define KEYID_ASCII_DLE 0x0010
#define KEYID_ASCII_DC1 0x0011
#define KEYID_ASCII_DC2 0x0012
#define KEYID_ASCII_DC3 0x0013
#define KEYID_ASCII_DC4 0x0014
#define KEYID_ASCII_NAK 0x0015
#define KEYID_ASCII_SYN 0x0016
#define KEYID_ASCII_ETB 0x0017
#define KEYID_ASCII_CAN 0x0018
#define KEYID_ASCII_EM 0x0019
#define KEYID_ASCII_SUB 0x001a
#define KEYID_ASCII_ESC 0x001b
#define KEYID_ASCII_FS 0x001c
#define KEYID_ASCII_GS 0x001d
#define KEYID_ASCII_RS 0x001e
#define KEYID_ASCII_US 0x001f
#define KEYID_ASCII_CHAR_START 0x0020
#define KEYID_ASCII_CHAR_END 0x007e
#define KEYID_ASCII_DEL 0x007f

#define KEYID_ESC 0x0000
#define KEYID_F1 0x0001
#define KEYID_F2 0x0002
#define KEYID_F3 0x0003
#define KEYID_F4 0x0004
#define KEYID_F5 0x0005
#define KEYID_F6 0x0006
#define KEYID_F7 0x0007
#define KEYID_F8 0x0008
#define KEYID_F9 0x0009
#define KEYID_F10 0x000a
#define KEYID_F11 0x000b
#define KEYID_F12 0x000c
#define KEYID_LOCK_NUM 0x000d
#define KEYID_LOCK_SCROOL 0x000e
#define KEYID_LOCK_CAPS 0x000f
#define KEYID_SHIFT_L 0x0010
#define KEYID_SHIFT_R 0x0011
#define KEYID_CTRL_L 0x0012
#define KEYID_CTRL_R 0x0013
#define KEYID_ALT_L 0x0014
#define KEYID_ALT_R 0x0015
#define KEYID_DELETE 0x0016
#define KEYID_INSERT 0x0017
#define KEYID_PAUSE 0x0018
#define KEYID_BREAK 0x0019
#define KEYID_PRINT_SCREEN 0x001a
#define KEYID_SYS_RQ 0x001b
#define KEYID_CURSOR_U 0x001c
#define KEYID_CURSOR_D 0x001d
#define KEYID_CURSOR_L 0x001e
#define KEYID_CURSOR_R 0x001f
#define KEYID_PAGE_UP 0x0020
#define KEYID_PAGE_DOWN 0x0021
#define KEYID_HOME 0x0022
#define KEYID_END 0x0023
#define KEYID_ICON_L 0x0024
#define KEYID_ICON_R 0x0025
#define KEYID_MENU 0x0026
#define KEYID_KANJI 0x0027
#define KEYID_HIRAGANA 0x0028
#define KEYID_HENKAN 0x0029
#define KEYID_MUHENKAN 0x002a

#define KEYID_BACKSPACE 0x0040
#define KEYID_TAB 0x0041
#define KEYID_ENTER 0x0042

#define KEYID_KBD_ERROR 0x007e
#define KEYID_UNKNOWN 0x007f

static const uint16_t kKeyCodeTable[0x80] = {
    KEYID_MASK_EXTENDED | KEYID_KBD_ERROR, KEYID_MASK_EXTENDED | KEYID_ESC, '1',
    '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '^',
    KEYID_MASK_EXTENDED | KEYID_BACKSPACE, KEYID_MASK_EXTENDED | KEYID_TAB,
    /*0x10*/
    'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '@', '[',
    KEYID_MASK_EXTENDED | KEYID_ENTER, KEYID_MASK_EXTENDED | KEYID_CTRL_L, 'A',
    'S',
    /*0x20*/
    'D', 'F', 'G', 'H', 'J', 'K', 'L', ';', ':',
    KEYID_MASK_EXTENDED | KEYID_KANJI, KEYID_MASK_EXTENDED | KEYID_SHIFT_L, ']',
    'Z', 'X', 'C', 'V',
    /*0x30*/
    'B', 'N', 'M', ',', '.', '/', KEYID_MASK_EXTENDED | KEYID_SHIFT_R,
    KEYID_MASK_TENKEY | '*', KEYID_MASK_EXTENDED | KEYID_ALT_L, ' ',
    KEYID_MASK_EXTENDED | KEYID_LOCK_CAPS, KEYID_MASK_EXTENDED | KEYID_F1,
    KEYID_MASK_EXTENDED | KEYID_F2, KEYID_MASK_EXTENDED | KEYID_F3,
    KEYID_MASK_EXTENDED | KEYID_F4, KEYID_MASK_EXTENDED | KEYID_F5,
    /*0x40*/
    KEYID_MASK_EXTENDED | KEYID_F6, KEYID_MASK_EXTENDED | KEYID_F7,
    KEYID_MASK_EXTENDED | KEYID_F8, KEYID_MASK_EXTENDED | KEYID_F9,
    KEYID_MASK_EXTENDED | KEYID_F10, KEYID_MASK_EXTENDED | KEYID_LOCK_NUM,
    KEYID_MASK_EXTENDED | KEYID_LOCK_SCROOL, KEYID_MASK_TENKEY | '7',
    KEYID_MASK_TENKEY | '8', KEYID_MASK_TENKEY | '9', KEYID_MASK_TENKEY | '-',
    KEYID_MASK_TENKEY | '4', KEYID_MASK_TENKEY | '5', KEYID_MASK_TENKEY | '6',
    KEYID_MASK_TENKEY | '+', KEYID_MASK_TENKEY | '1',
    /*0x50*/
    KEYID_MASK_TENKEY | '2', KEYID_MASK_TENKEY | '3', KEYID_MASK_TENKEY | '0',
    KEYID_MASK_TENKEY | '.', KEYID_MASK_EXTENDED | KEYID_SYS_RQ, 0x0000, 0x0000,
    KEYID_MASK_EXTENDED | KEYID_F11, KEYID_MASK_EXTENDED | KEYID_F12, 0x0000,
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    /*0x60*/
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    /*0x70*/
    KEYID_MASK_EXTENDED | KEYID_HIRAGANA, 0x0000, 0x0000, '_', 0x0000, 0x0000,
    0x0000, 0x0000, 0x0000, KEYID_MASK_EXTENDED | KEYID_HENKAN, 0x0000,
    KEYID_MASK_EXTENDED | KEYID_MUHENKAN, 0x0000, '\\',
    //  0x5c, // ='\' for mikan-trap.
    0x0000, 0x0000};

uint16_t ParseKeyCode(uint8_t keycode) {
  if (keycode & 0x80) {
    PutStringAndHex("break:", keycode);
    return 0;
  }
  uint16_t keyid = kKeyCodeTable[keycode];
  char s[2];
  s[0] = keyid;
  s[1] = 0;
  PutStringAndHex(s, keyid);
  return 0;
}