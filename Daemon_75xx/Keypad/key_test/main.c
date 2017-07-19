#include <stdio.h>

static int keymap_num[] = {
    KEY(0, 0, KEY_RESERVED),    KEY(0, 1, KEY_LEFTALT),
    KEY(0, 2, KEY_F5),  KEY(0, 3, KEY_INFO),
    KEY(0, 4, KEY_UP),  KEY(0, 5, KEY_HOME),
    KEY(0, 6, KEY_TAB), KEY(0, 7, KEY_F1),

    KEY(1, 0, KEY_ESC), KEY(1, 1, KEY_DOWN),
    KEY(1, 2, KEY_RIGHT),       KEY(1, 3, KEY_F6),
    KEY(1, 4, KEY_F2),  KEY(1, 5, KEY_1),
    KEY(1, 6, KEY_2),   KEY(1, 7, KEY_3),

    KEY(2, 0, KEY_LEFT),        KEY(2, 1, KEY_CAPSLOCK),
    KEY(2, 2, KEY_F3),  KEY(2, 3, KEY_4),
    KEY(2, 4, KEY_5),   KEY(2, 5, KEY_6),
    KEY(2, 6, KEY_SPACE),       KEY(2, 7, KEY_F4),

    KEY(3, 0, KEY_RESERVED),    KEY(3, 1, KEY_7),
    KEY(3, 2, KEY_8),   KEY(3, 3, KEY_9),
    KEY(3, 4, KEY_BACKSPACE),   KEY(3, 5, KEY_DOT),
    KEY(3, 6, KEY_0),   KEY(3, 7, KEY_ENTER),
};

static int keymap_alpha[] = {
    KEY(0, 0, KEY_F),   KEY(0, 1, KEY_LEFTALT),
    KEY(0, 2, KEY_A),  KEY(0, 3, KEY_B),
    KEY(0, 4, KEY_C),  KEY(0, 5, KEY_HOME),
    KEY(0, 6, KEY_D),  KEY(0, 7, KEY_E),

    KEY(1, 0, KEY_ESC), KEY(1, 1, KEY_G),
    KEY(1, 2, KEY_H),  KEY(1, 3, KEY_I),
    KEY(1, 4, KEY_J),  KEY(1, 5, KEY_K),
    KEY(1, 6, KEY_L),  KEY(1, 7, KEY_M),

    KEY(2, 0, KEY_RESERVED),    KEY(2, 1, KEY_N),
    KEY(2, 2, KEY_O),  KEY(2, 3, KEY_P),
    KEY(2, 4, KEY_Q),  KEY(2, 5, KEY_R),
    KEY(2, 6, KEY_S),  KEY(2, 7, KEY_T),

    KEY(3, 0, KEY_RESERVED),    KEY(3, 1, KEY_U),
    KEY(3, 2, KEY_V),  KEY(3, 3, KEY_W),
    KEY(3, 4, KEY_X),  KEY(3, 5, KEY_Y),
};

int main(void)
{
    printf("Hello World!\n");
    return 0;
}

