#include <EEPROM.h>
#include <Gamebuino.h>
#include <SPI.h>

const int INITIAL_SEG = 10;
const int MAX_SEG = 256;
const int SEG_ADDED = 3;
const int WWIDTH = LCDWIDTH / 2;
const int WHEIGHT = LCDHEIGHT / 2;

int num_seg;
int frame_delay = 3;
int STARTX;
int STARTY;
char snake[2 * MAX_SEG];
int vx, vy;
int fx, fy;
byte score;
byte max_score;

enum GAME_STATE_T { PLAY,
                    GAMEOVER };
GAME_STATE_T game_state;

Gamebuino gb;

const byte PROGMEM logo[] = {
    64, 30,
    0x0, 0x0, 0x0, 0x0, 0x0, 0x80, 0x0, 0x0,
    0x0, 0x0, 0x0, 0x0, 0x0, 0xc0, 0x0, 0x0,
    0x0, 0x0, 0x0, 0x0, 0x0, 0xa0, 0x0, 0x0,
    0x0, 0x0, 0x0, 0x0, 0x0, 0xa0, 0x0, 0x0,
    0x0, 0x0, 0x0, 0x0, 0x0, 0xc0, 0x0, 0x0,
    0x0, 0x0, 0x0, 0x0, 0x0, 0xf8, 0x0, 0x0,
    0x0, 0x0, 0x0, 0x0, 0x1, 0xf4, 0x0, 0x0,
    0x0, 0x0, 0x0, 0x0, 0x1, 0x9c, 0x0, 0x0,
    0x0, 0x0, 0x1c, 0x3d, 0xe3, 0x3, 0x0, 0x0,
    0x0, 0x8, 0x63, 0x42, 0x17, 0x7, 0x0, 0x0,
    0x0, 0xf, 0xe1, 0xc3, 0x9, 0xfe, 0x0, 0x0,
    0x0, 0x8, 0x10, 0xa1, 0x8f, 0xff, 0x80, 0x0,
    0x0, 0xc, 0x8, 0xa1, 0x8f, 0x7f, 0xc0, 0x0,
    0x0, 0xa, 0x18, 0xf1, 0x8b, 0xf2, 0xe0, 0x0,
    0x0, 0xb, 0xf8, 0xe1, 0x89, 0xc1, 0x80, 0x0,
    0x0, 0x9, 0x60, 0xa1, 0x88, 0xe0, 0x0, 0x0,
    0x0, 0x8, 0x60, 0x81, 0x88, 0x60, 0x0, 0x0,
    0x0, 0x7, 0xa1, 0x81, 0x89, 0xd8, 0x0, 0x0,
    0x0, 0x0, 0x13, 0x42, 0x89, 0x34, 0x0, 0x0,
    0x0, 0x0, 0x4, 0x3c, 0x8a, 0xbe, 0x0, 0x0,
    0x0, 0x0, 0x0, 0x0, 0x8a, 0xbf, 0x0, 0x0,
    0x0, 0x0, 0x0, 0x0, 0x87, 0x1, 0x0, 0x0,
    0x0, 0x0, 0x0, 0x0, 0x80, 0x1, 0x0, 0x0,
    0x0, 0x0, 0x0, 0x0, 0x80, 0x81, 0x0, 0x0,
    0x0, 0x0, 0x0, 0x0, 0x40, 0x42, 0x0, 0x0,
    0x0, 0x0, 0x0, 0x0, 0x30, 0xbc, 0x0, 0x0,
    0x0, 0x0, 0x0, 0x0, 0x1f, 0x6, 0x0, 0x0,
    0x0, 0x0, 0x0, 0x0, 0x0, 0x7, 0x0, 0x0,
    0x0, 0x0, 0x0, 0x0, 0x0, 0x3, 0x80, 0x0,
    0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0};

void setup() {
    gb.begin();
    gb.titleScreen(F("Snake"), logo);
    gb.pickRandomSeed();
    gamestart();
}

void gamestart() {
    game_state = PLAY;
    STARTX = WWIDTH / 2;
    STARTY = WHEIGHT / 2;
    fx = random(WWIDTH);
    fy = random(WHEIGHT);
    num_seg = INITIAL_SEG;
    for (int i = 0; i < num_seg; i++) {
        snake[2 * i] = STARTX - i;
        snake[2 * i + 1] = STARTY;
    }
    vx = 1;
    vy = 0;
    score = 0;
    gb.battery.show = false;
    max_score = EEPROM.read(0);
}

void gameover() {
    game_state = GAMEOVER;
    if (score > max_score) {
        max_score = score;
        EEPROM.update(0, max_score);
    }
}

void input() {
    if (gb.buttons.pressed(BTN_C)) {
        gb.sound.playCancel();
        gb.titleScreen(F("Snake"), logo);
    }
    switch (game_state) {
        case PLAY: {
            bool up = gb.buttons.pressed(BTN_UP);
            bool down = gb.buttons.pressed(BTN_DOWN);
            bool left = gb.buttons.pressed(BTN_LEFT);
            bool right = gb.buttons.pressed(BTN_RIGHT);
            if (up && vy != 1) {
                vx = 0;
                vy = -1;
            } else if (down && vy != -1) {
                vx = 0;
                vy = 1;
            } else if (right && vx != -1) {
                vx = 1;
                vy = 0;
            } else if (left && vx != 1) {
                vx = -1;
                vy = 0;
            }
            frame_delay = gb.buttons.repeat(BTN_A, 1) ? 1 : 3;
        } break;
        case GAMEOVER: {
            if (gb.buttons.pressed(BTN_A) || gb.buttons.pressed(BTN_B)) {
                gb.sound.playOK();
                gamestart();
            }
        } break;
    }
}

void internal_update() {
    switch (game_state) {
        case PLAY:
            for (int i = num_seg - 1; i >= 0; i--) {
                auto &x = snake[2 * i];
                auto &y = snake[2 * i + 1];
                if (i == 0) {
                    x += vx;
                    y += vy;
                    x = (x + WWIDTH) % WWIDTH;
                    y = (y + WHEIGHT) % WHEIGHT;
                    for (int j = 1; j < num_seg; j++) {
                        if (x == snake[2 * j] && y == snake[2 * j + 1]) {
                            gameover();
                            return;
                        }
                    }
                    if (x == fx && y == fy) {
                        gb.sound.playTick();
                        if (num_seg + SEG_ADDED < MAX_SEG) {
                            for (int j = 0; j < SEG_ADDED; j++) {
                                snake[2 * (num_seg + j)] = fx;
                                snake[2 * (num_seg + j) + 1] = fy;
                            }
                            num_seg += SEG_ADDED;
                        }
                        fx = random(WWIDTH);
                        fy = random(WHEIGHT);
                        score++;
                    }
                } else {
                    x = snake[2 * (i - 1)];
                    y = snake[2 * (i - 1) + 1];
                    x = (x + WWIDTH) % WWIDTH;
                    y = (y + WHEIGHT) % WHEIGHT;
                }
            }
            break;
        case GAMEOVER:
            break;
    }
}

void update() {
    if (gb.frameCount % frame_delay == 0) {
        internal_update();
    }
}

void draw2x2(int x, int y) {
    gb.display.drawPixel(x, y);
    gb.display.drawPixel(x, y + 1);
    gb.display.drawPixel(x + 1, y);
    gb.display.drawPixel(x + 1, y + 1);
}

void draw() {
    switch (game_state) {
        case PLAY:
            for (int i = 0; i < num_seg; i++) {
                auto x = snake[i * 2];
                auto y = snake[i * 2 + 1];
                if (i == 0) {
                    draw2x2(2 * x, 2 * y);
                } else {
                    if (gb.frameCount & 1) {
                        gb.display.drawPixel(2 * x, 2 * y);
                        gb.display.drawPixel(2 * x + 1, 2 * y + 1);
                    } else {
                        gb.display.drawPixel(2 * x + 1, 2 * y);
                        gb.display.drawPixel(2 * x, 2 * y + 1);
                    }
                }
            }
            gb.display.setColor(BLACK);
            draw2x2(2 * fx, 2 * fy);
            gb.display.fontSize = 1;
            gb.display.print(String(score) + String(" | ") + max_score);
            break;
        case GAMEOVER:
            gb.display.fontSize = 3;
            gb.display.println("Game\nOver!");
            gb.display.fontSize = 2;
            gb.display.println(String("score: ") + score);
            break;
    }
}

void loop() {
    if (gb.update()) {
        input();
        update();
        draw();
    }
}