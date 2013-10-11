//****************************************************************************
// Product: "Fly 'n' Shoot" game example for Win32-GUI
// Last Updated for Version: 5.0.0
// Date of the Last Update:  Aug 23, 2013
//
//                    Q u a n t u m     L e a P s
//                    ---------------------------
//                    innovating embedded systems
//
// Copyright (C) 2002-2013 Quantum Leaps, LLC. All rights reserved.
//
// This program is open source software: you can redistribute it and/or
// modify it under the terms of the GNU General Public License as published
// by the Free Software Foundation, either version 2 of the License, or
// (at your option) any later version.
//
// Alternatively, this program may be distributed and modified under the
// terms of Quantum Leaps commercial licenses, which expressly supersede
// the GNU General Public License and are specifically designed for
// licensees interested in retaining the proprietary status of their code.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <http://www.gnu.org/licenses/>.
//
// Contact information:
// Quantum Leaps Web sites: http://www.quantum-leaps.com
//                          http://www.state-machine.com
// e-mail:                  info@quantum-leaps.com
//****************************************************************************
#include "qp_port.h"
#include "game.h"
#include "bsp.h"

#include "win32_gui.h"         // Win32 GUI elements for embedded front panels
#include "resource.h"    // GUI resource IDs generated by the resource editior

#include <stdio.h>                                        // for _snprintf_s()

//****************************************************************************
namespace GAME {

Q_DEFINE_THIS_FILE

#define LCD_WIDTH     BSP_SCREEN_WIDTH
#define LCD_HEIGHT    BSP_SCREEN_HEIGHT
#define LCD_X_SCALE   2U
#define LCD_Y_SCALE   2U

// local variables -----------------------------------------------------------
static HINSTANCE l_hInst;                         // this application instance
static HWND      l_hWnd;                                 // main window handle
static LPSTR     l_cmdLine;                         // the command line string

static GraphicDisplay   l_oled;    // the OLED display of the EK-LM3S811 board
static SegmentDisplay   l_userLED;         // USER LED of the EK-LM3S811 board
static SegmentDisplay   l_scoreBoard;         // segment display for the score
static OwnerDrawnButton l_userBtn;      // USER button of the EK-LM3S811 board

// (R,G,B) colors for the OLED display
static BYTE const c_onColor [3] = { 255U, 255U,   0U };              // yellow
static BYTE const c_offColor[3] = {  15U,  15U,  15U };      // very dark grey

static BYTE l_ship_pos = GAME_SHIP_Y;           // the current ship Y-position

#ifdef Q_SPY
    enum QSUserRecords {
        PLAYER_TRIGGER = QP::QS_USER
    };
    static uint8_t const l_clock_tick = 0U;
    static uint8_t const l_mouse      = 0U;
#endif

// Local functions -----------------------------------------------------------
static LRESULT CALLBACK WndProc(HWND hWnd, UINT iMsg,
                                WPARAM wParam, LPARAM lParam);

//............................................................................
extern "C" int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPrevInst,
                   LPSTR cmdLine, int iCmdShow)
{
    HWND hWnd;
    MSG  msg;

    (void)hPrevInst;          // avoid compiler warning about unused parameter

    l_hInst   = hInst;                        // save the application instance
    l_cmdLine = cmdLine;                       // save the command line string

    // create the main custom dialog window
    hWnd = CreateCustDialog(hInst, IDD_APPLICATION, NULL,
                            &WndProc, "QP_APP");
    ShowWindow(hWnd, iCmdShow);                        // show the main window

    // enter the message loop...
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    return msg.wParam;
}
//............................................................................
extern "C" int_t main_gui(void);                  // prototype for appThread()

// thread function for running the application main()
static DWORD WINAPI appThread(LPVOID par) {
    (void)par;                                             // unused parameter
    return main_gui();                               // run the QF application
}
//............................................................................
static LRESULT CALLBACK WndProc(HWND hWnd, UINT iMsg,
                                WPARAM wParam, LPARAM lParam)
{
    switch (iMsg) {

        // Perform initialization upon cration of the main dialog window
        // NOTE: Any child-windows are NOT created yet at this time, so
        // the GetDlgItem() function can't be used (it will return NULL).
        //
        case WM_CREATE: {
            l_hWnd = hWnd;                           // save the window handle

            // initialize the owner-drawn buttons...
            // NOTE: must be done *before* the first drawing of the buttons,
            // so WM_INITDIALOG is too late.
            //
            l_userBtn.init(LoadBitmap(l_hInst, MAKEINTRESOURCE(IDB_BTN_UP)),
                           LoadBitmap(l_hInst, MAKEINTRESOURCE(IDB_BTN_DWN)),
                           LoadCursor(NULL, IDC_HAND));
            return 0;
        }

        // Perform initialization after all child windows have been created
        case WM_INITDIALOG: {
            l_oled.init(BSP_SCREEN_WIDTH,  2U,       // scale horizontally by 2
                        BSP_SCREEN_HEIGHT, 2U,         // scale vertically by 2
                        GetDlgItem(hWnd, IDC_LCD), c_offColor);

            l_userLED.init(1U,                  // 1 "segment" (the LED itself)
                           2U);            // 2 bitmaps (for LED OFF/ON states)
            l_userLED.initSegment(0U, GetDlgItem(hWnd, IDC_LED));
            l_userLED.initBitmap(0U,
                LoadBitmap(l_hInst, MAKEINTRESOURCE(IDB_LED_OFF)));
            l_userLED.initBitmap(1U,
                LoadBitmap(l_hInst, MAKEINTRESOURCE(IDB_LED_ON)));

            l_scoreBoard.init(4U,                  // 4 "segments" (digits 0-3)
                              10U);           // 10 bitmaps (for 0-9 states)
            l_scoreBoard.initSegment(0U, GetDlgItem(hWnd, IDC_SEG0));
            l_scoreBoard.initSegment(1U, GetDlgItem(hWnd, IDC_SEG1));
            l_scoreBoard.initSegment(2U, GetDlgItem(hWnd, IDC_SEG2));
            l_scoreBoard.initSegment(3U, GetDlgItem(hWnd, IDC_SEG3));
            l_scoreBoard.initBitmap(0U,
                LoadBitmap(l_hInst, MAKEINTRESOURCE(IDB_SEG0)));
            l_scoreBoard.initBitmap(1U,
                LoadBitmap(l_hInst, MAKEINTRESOURCE(IDB_SEG1)));
            l_scoreBoard.initBitmap(2U,
                LoadBitmap(l_hInst, MAKEINTRESOURCE(IDB_SEG2)));
            l_scoreBoard.initBitmap(3U,
                LoadBitmap(l_hInst, MAKEINTRESOURCE(IDB_SEG3)));
            l_scoreBoard.initBitmap(4U,
                LoadBitmap(l_hInst, MAKEINTRESOURCE(IDB_SEG4)));
            l_scoreBoard.initBitmap(5U,
                LoadBitmap(l_hInst, MAKEINTRESOURCE(IDB_SEG5)));
            l_scoreBoard.initBitmap(6U,
                LoadBitmap(l_hInst, MAKEINTRESOURCE(IDB_SEG6)));
            l_scoreBoard.initBitmap(7U,
                LoadBitmap(l_hInst, MAKEINTRESOURCE(IDB_SEG7)));
            l_scoreBoard.initBitmap(8U,
                LoadBitmap(l_hInst, MAKEINTRESOURCE(IDB_SEG8)));
            l_scoreBoard.initBitmap(9U,
                LoadBitmap(l_hInst, MAKEINTRESOURCE(IDB_SEG9)));

            BSP_updateScore(0U);

            // --> QP: spawn the application thread to run main()
            Q_ALLEGE(CreateThread(NULL, 0, &appThread, NULL, 0, NULL)
                     != (HANDLE)0);
            return 0;
        }

        case WM_DESTROY: {
            BSP_terminate(0);
            return 0;
        }

        // commands from regular buttons and menus...
        case WM_COMMAND: {
            SetFocus(hWnd);
            switch (wParam) {
                case IDOK:
                case IDCANCEL: {
                    BSP_terminate(0);
                    break;
                }
            }
            return 0;
        }

        // owner-drawn buttons...
        case WM_DRAWITEM: {
            LPDRAWITEMSTRUCT pdis = (LPDRAWITEMSTRUCT)lParam;
            switch (pdis->CtlID) {
                case IDC_USER: {                    // USER owner-drawn button
                    switch (l_userBtn.draw(pdis)) {
                        case OwnerDrawnButton::BTN_DEPRESSED: {
                            BSP_playerTrigger();
                            l_userLED.setSegment(0U, 1U);
                            break;
                        }
                        case OwnerDrawnButton::BTN_RELEASED: {
                            l_userLED.setSegment(0U, 0U);
                            break;
                        }
                        default: {
                            break;
                        }
                    }
                    break;
                }
            }
            return 0;
        }

        // mouse input...
        case WM_MOUSEWHEEL: {
            if ((HIWORD(wParam) & 0x8000U) == 0U) {   // wheel turned forward?
                BSP_moveShipUp();
            }
            else {                           // the wheel was turned backwards
                BSP_moveShipDown();
            }
            return 0;
        }

        // keyboard input...
        case WM_KEYDOWN: {
            switch (wParam) {
                case VK_UP:
                    BSP_moveShipUp();
                    break;
                case VK_DOWN:
                    BSP_moveShipDown();
                    break;
                case VK_SPACE:
                    BSP_playerTrigger();
                    break;
                default:
                    break;
            }
            return 0;
        }

    }
    return DefWindowProc(hWnd, iMsg, wParam, lParam) ;
}
//............................................................................
void BSP_init(void) {
    Q_ALLEGE(QS_INIT(l_cmdLine));
    QS_RESET();
    QS_OBJ_DICTIONARY(&l_clock_tick);
    QS_USR_DICTIONARY(PLAYER_TRIGGER);
}
//............................................................................
void BSP_terminate(int result) {
    QP::QF::stop();
    PostQuitMessage(result);               // post the Quit message to the GUI
}
//............................................................................
void BSP_drawBitmap(uint8_t const *bitmap) {
    uint16_t x, y;
    for (y = 0; y < LCD_HEIGHT; ++y) {
        for (x = 0; x < LCD_WIDTH; ++x) {
            uint8_t bits = bitmap[x + (y/8U)*LCD_WIDTH];
            if ((bits & (1U << (y & 0x07U))) != 0U) {
                l_oled.setPixel(x, y, c_onColor);
            }
            else {
                l_oled.clearPixel(x, y);
            }
        }
    }
    l_oled.redraw();
}
//............................................................................
void BSP_drawNString(uint8_t x, uint8_t y, char const *str) {
    static uint8_t const font5x7[95][5] = {
        { 0x00, 0x00, 0x00, 0x00, 0x00 },                               // ' '
        { 0x00, 0x00, 0x4F, 0x00, 0x00 },                                 // !
        { 0x00, 0x07, 0x00, 0x07, 0x00 },                                 // "
        { 0x14, 0x7F, 0x14, 0x7F, 0x14 },                                 // #
        { 0x24, 0x2A, 0x7F, 0x2A, 0x12 },                                 // $
        { 0x23, 0x13, 0x08, 0x64, 0x62 },                                 // %
        { 0x36, 0x49, 0x55, 0x22, 0x50 },                                 // &
        { 0x00, 0x05, 0x03, 0x00, 0x00 },                                 // '
        { 0x00, 0x1C, 0x22, 0x41, 0x00 },                                 // (
        { 0x00, 0x41, 0x22, 0x1C, 0x00 },                                 // )
        { 0x14, 0x08, 0x3E, 0x08, 0x14 },                                 // *
        { 0x08, 0x08, 0x3E, 0x08, 0x08 },                                 // +
        { 0x00, 0x50, 0x30, 0x00, 0x00 },                                 // ,
        { 0x08, 0x08, 0x08, 0x08, 0x08 },                                 // -
        { 0x00, 0x60, 0x60, 0x00, 0x00 },                                 // .
        { 0x20, 0x10, 0x08, 0x04, 0x02 },                                 // /
        { 0x3E, 0x51, 0x49, 0x45, 0x3E },                                 // 0
        { 0x00, 0x42, 0x7F, 0x40, 0x00 },                                 // 1
        { 0x42, 0x61, 0x51, 0x49, 0x46 },                                 // 2
        { 0x21, 0x41, 0x45, 0x4B, 0x31 },                                 // 3
        { 0x18, 0x14, 0x12, 0x7F, 0x10 },                                 // 4
        { 0x27, 0x45, 0x45, 0x45, 0x39 },                                 // 5
        { 0x3C, 0x4A, 0x49, 0x49, 0x30 },                                 // 6
        { 0x01, 0x71, 0x09, 0x05, 0x03 },                                 // 7
        { 0x36, 0x49, 0x49, 0x49, 0x36 },                                 // 8
        { 0x06, 0x49, 0x49, 0x29, 0x1E },                                 // 9
        { 0x00, 0x36, 0x36, 0x00, 0x00 },                                 // :
        { 0x00, 0x56, 0x36, 0x00, 0x00 },                                 // ;
        { 0x08, 0x14, 0x22, 0x41, 0x00 },                                 // <
        { 0x14, 0x14, 0x14, 0x14, 0x14 },                                 // =
        { 0x00, 0x41, 0x22, 0x14, 0x08 },                                 // >
        { 0x02, 0x01, 0x51, 0x09, 0x06 },                                 // ?
        { 0x32, 0x49, 0x79, 0x41, 0x3E },                                 // @
        { 0x7E, 0x11, 0x11, 0x11, 0x7E },                                 // A
        { 0x7F, 0x49, 0x49, 0x49, 0x36 },                                 // B
        { 0x3E, 0x41, 0x41, 0x41, 0x22 },                                 // C
        { 0x7F, 0x41, 0x41, 0x22, 0x1C },                                 // D
        { 0x7F, 0x49, 0x49, 0x49, 0x41 },                                 // E
        { 0x7F, 0x09, 0x09, 0x09, 0x01 },                                 // F
        { 0x3E, 0x41, 0x49, 0x49, 0x7A },                                 // G
        { 0x7F, 0x08, 0x08, 0x08, 0x7F },                                 // H
        { 0x00, 0x41, 0x7F, 0x41, 0x00 },                                 // I
        { 0x20, 0x40, 0x41, 0x3F, 0x01 },                                 // J
        { 0x7F, 0x08, 0x14, 0x22, 0x41 },                                 // K
        { 0x7F, 0x40, 0x40, 0x40, 0x40 },                                 // L
        { 0x7F, 0x02, 0x0C, 0x02, 0x7F },                                 // M
        { 0x7F, 0x04, 0x08, 0x10, 0x7F },                                 // N
        { 0x3E, 0x41, 0x41, 0x41, 0x3E },                                 // O
        { 0x7F, 0x09, 0x09, 0x09, 0x06 },                                 // P
        { 0x3E, 0x41, 0x51, 0x21, 0x5E },                                 // Q
        { 0x7F, 0x09, 0x19, 0x29, 0x46 },                                 // R
        { 0x46, 0x49, 0x49, 0x49, 0x31 },                                 // S
        { 0x01, 0x01, 0x7F, 0x01, 0x01 },                                 // T
        { 0x3F, 0x40, 0x40, 0x40, 0x3F },                                 // U
        { 0x1F, 0x20, 0x40, 0x20, 0x1F },                                 // V
        { 0x3F, 0x40, 0x38, 0x40, 0x3F },                                 // W
        { 0x63, 0x14, 0x08, 0x14, 0x63 },                                 // X
        { 0x07, 0x08, 0x70, 0x08, 0x07 },                                 // Y
        { 0x61, 0x51, 0x49, 0x45, 0x43 },                                 // Z
        { 0x00, 0x7F, 0x41, 0x41, 0x00 },                                 // [
        { 0x02, 0x04, 0x08, 0x10, 0x20 },                               // '\'
        { 0x00, 0x41, 0x41, 0x7F, 0x00 },                                 // ]
        { 0x04, 0x02, 0x01, 0x02, 0x04 },                                 // ^
        { 0x40, 0x40, 0x40, 0x40, 0x40 },                                 // _
        { 0x00, 0x01, 0x02, 0x04, 0x00 },                                 // `
        { 0x20, 0x54, 0x54, 0x54, 0x78 },                                 // a
        { 0x7F, 0x48, 0x44, 0x44, 0x38 },                                 // b
        { 0x38, 0x44, 0x44, 0x44, 0x20 },                                 // c
        { 0x38, 0x44, 0x44, 0x48, 0x7F },                                 // d
        { 0x38, 0x54, 0x54, 0x54, 0x18 },                                 // e
        { 0x08, 0x7E, 0x09, 0x01, 0x02 },                                 // f
        { 0x0C, 0x52, 0x52, 0x52, 0x3E },                                 // g
        { 0x7F, 0x08, 0x04, 0x04, 0x78 },                                 // h
        { 0x00, 0x44, 0x7D, 0x40, 0x00 },                                 // i
        { 0x20, 0x40, 0x44, 0x3D, 0x00 },                                 // j
        { 0x7F, 0x10, 0x28, 0x44, 0x00 },                                 // k
        { 0x00, 0x41, 0x7F, 0x40, 0x00 },                                 // l
        { 0x7C, 0x04, 0x18, 0x04, 0x78 },                                 // m
        { 0x7C, 0x08, 0x04, 0x04, 0x78 },                                 // n
        { 0x38, 0x44, 0x44, 0x44, 0x38 },                                 // o
        { 0x7C, 0x14, 0x14, 0x14, 0x08 },                                 // p
        { 0x08, 0x14, 0x14, 0x18, 0x7C },                                 // q
        { 0x7C, 0x08, 0x04, 0x04, 0x08 },                                 // r
        { 0x48, 0x54, 0x54, 0x54, 0x20 },                                 // s
        { 0x04, 0x3F, 0x44, 0x40, 0x20 },                                 // t
        { 0x3C, 0x40, 0x40, 0x20, 0x7C },                                 // u
        { 0x1C, 0x20, 0x40, 0x20, 0x1C },                                 // v
        { 0x3C, 0x40, 0x30, 0x40, 0x3C },                                 // w
        { 0x44, 0x28, 0x10, 0x28, 0x44 },                                 // x
        { 0x0C, 0x50, 0x50, 0x50, 0x3C },                                 // y
        { 0x44, 0x64, 0x54, 0x4C, 0x44 },                                 // z
        { 0x00, 0x08, 0x36, 0x41, 0x00 },                                 // {
        { 0x00, 0x00, 0x7F, 0x00, 0x00 },                                 // |
        { 0x00, 0x41, 0x36, 0x08, 0x00 },                                 // }
        { 0x02, 0x01, 0x02, 0x04, 0x02 },                                 // ~
    };
    UINT dx, dy;

    while (*str != '\0') {
        uint8_t const *ch = &font5x7[*str - ' '][0];
        for (dx = 0U; dx < 5U; ++dx) {
            for (dy = 0U; dy < 8U; ++dy) {
                if ((ch[dx] & (1U << dy)) != 0U) {
                    l_oled.setPixel((UINT)(x + dx),
                                    (UINT)(y*8U + dy), c_onColor);
                }
                else {
                    l_oled.clearPixel((UINT)(x + dx),
                                      (UINT)(y*8U + dy));
                }
            }
        }
        ++str;
        x += 6U;
    }
    l_oled.redraw();                // draw the updated display on the screen
}
//............................................................................
void BSP_updateScore(uint16_t score) {
    // update the score in the l_scoreBoard SegmentDisplay
    l_scoreBoard.setSegment(0U, (UINT)(score % 10U));
    score /= 10U;
    l_scoreBoard.setSegment(1U, (UINT)(score % 10U));
    score /= 10U;
    l_scoreBoard.setSegment(2U, (UINT)(score % 10U));
    score /= 10U;
    l_scoreBoard.setSegment(3U, (UINT)(score % 10U));
}
//............................................................................
void BSP_displayOn(void) {
    l_userLED.setSegment(0U, 1U);
}
//............................................................................
void BSP_displayOff(void) {
    l_userLED.setSegment(0U, 0U);
    l_oled.clear();
    l_oled.redraw();
}
//............................................................................
void BSP_playerTrigger(void) {
    static QP::QEvt const fireEvt = QEVT_INITIALIZER(PLAYER_TRIGGER_SIG);
    QP::QF::PUBLISH(&fireEvt, (void*)0);
}
//............................................................................
void BSP_moveShipUp(void) {
    if (l_ship_pos > 0U) {
        --l_ship_pos;
    }
    ObjectPosEvt *ope = Q_NEW(ObjectPosEvt, PLAYER_SHIP_MOVE_SIG);
    ope->x = (uint8_t)GAME_SHIP_X;
    ope->y = (uint8_t)l_ship_pos;
    AO_Ship->POST(ope, &l_mouse);
}
//............................................................................
void BSP_moveShipDown(void) {
    if (l_ship_pos < (GAME_SCREEN_HEIGHT - 3U)) {
        ++l_ship_pos;
    }
    ObjectPosEvt *ope = Q_NEW(ObjectPosEvt, PLAYER_SHIP_MOVE_SIG);
    ope->x = (uint8_t)GAME_SHIP_X;
    ope->y = (uint8_t)l_ship_pos;
    AO_Ship->POST(ope, &l_mouse);
}

}                                                             // namespace DPP
//****************************************************************************

namespace QP {

//............................................................................
void QF::onStartup(void) {
    QF_setTickRate(GAME::BSP_TICKS_PER_SEC);     // set the desired tick rate
}
//............................................................................
void QF::onCleanup(void) {
}
//............................................................................
void QF_onClockTick(void) {
    static QP::QEvt const tickEvt = QEVT_INITIALIZER(GAME::TIME_TICK_SIG);
    QP::QF::TICK(&GAME::l_clock_tick); //perform the QF clock tick processing
    QP::QF::PUBLISH(&tickEvt, &GAME::l_clock_tick); // publish the tick event
}
//............................................................................
extern "C" void Q_onAssert(char const Q_ROM * const file, int line) {
    char message[80];
    snprintf(message, Q_DIM(message) - 1,
             "Assertion failed in module %s line %d", file, line);
    MessageBox(GAME::l_hWnd, message, "!!! ASSERTION !!!",
               MB_OK | MB_ICONEXCLAMATION | MB_APPLMODAL);
    GAME::BSP_terminate(-1);
}

//----------------------------------------------------------------------------
#ifdef Q_SPY                                            // define QS callbacks

#include "qspy.h"
#include <time.h>

static uint8_t l_running;

//............................................................................
static DWORD WINAPI idleThread(LPVOID par) {   // signature for CreateThread()
    (void)par;

    SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_IDLE);
    l_running = (uint8_t)1;
    while (l_running) {
        uint16_t nBytes = 256;
        uint8_t const *block;
        QF_CRIT_ENTRY(dummy);
        block = QP::QS::getBlock(&nBytes);
        QF_CRIT_EXIT(dummy);
        if (block != (uint8_t *)0) {
            QSPY_parse(block, nBytes);
        }
        Sleep(50);                                         // wait for a while
    }
    return 0;                                                // return success
}
//............................................................................
static int custParserFun(QSpyRecord * const qrec) {
    static uint32_t mpool1;
    int ret = 1;                              // perform standard QSPY parsing
    switch (qrec->rec) {
        case QS_QF_MPOOL_INIT: {                    // example record to parse
            if (mpool1 == 1U) {                                   // MPool[1]?
                mpool1 = (uint32_t)QSpyRecord_getUint64(qrec,QS_OBJ_PTR_SIZE);
            }
            else {
                ++mpool1;
            }
            break;
        }
        case QS_QF_MPOOL_GET: {                     // example record to parse
            int nFree;
            uint32_t mpool;
            (void)QSpyRecord_getUint32(qrec, QS_TIME_SIZE);
            mpool = (uint32_t)QSpyRecord_getUint64(qrec, QS_OBJ_PTR_SIZE);
            nFree = (int)QSpyRecord_getUint32(qrec, QF_MPOOL_CTR_SIZE);
            (void)QSpyRecord_getUint32(qrec, QF_MPOOL_CTR_SIZE);       // nMin
            if (QSpyRecord_OK(qrec) && (mpool == mpool1)) {       // MPool[1]?
                SetDlgItemInt(GAME::l_hWnd, IDC_MPOOL0, nFree, FALSE);
                ret = 0;                // don't perform standard QSPY parsing
            }
            break;
        }
    }
    return ret;
}
//............................................................................
bool QP::QS::onStartup(void const *arg) {
    static uint8_t qsBuf[4*1024];                 // 4K buffer for Quantum Spy
    initBuf(qsBuf, sizeof(qsBuf));

    QSPY_config(QP_VERSION,         // version
                QS_OBJ_PTR_SIZE,    // objPtrSize
                QS_FUN_PTR_SIZE,    // funPtrSize
                QS_TIME_SIZE,       // tstampSize
                Q_SIGNAL_SIZE,      // sigSize,
                QF_EVENT_SIZ_SIZE,  // evtSize
                QF_EQUEUE_CTR_SIZE, // queueCtrSize
                QF_MPOOL_CTR_SIZE,  // poolCtrSize
                QF_MPOOL_SIZ_SIZE,  // poolBlkSize
                QF_TIMEEVT_CTR_SIZE,// tevtCtrSize
                (void *)0,          // matFile,
                (void *)0,          // mscFile
                &custParserFun);    // customized parser function

    QS_FILTER_ON(QS_ALL_RECORDS);

//    QS_FILTER_OFF(QS_QEP_STATE_EMPTY);
//    QS_FILTER_OFF(QS_QEP_STATE_ENTRY);
//    QS_FILTER_OFF(QS_QEP_STATE_EXIT);
//    QS_FILTER_OFF(QS_QEP_STATE_INIT);
//    QS_FILTER_OFF(QS_QEP_INIT_TRAN);
//    QS_FILTER_OFF(QS_QEP_INTERN_TRAN);
//    QS_FILTER_OFF(QS_QEP_TRAN);
//    QS_FILTER_OFF(QS_QEP_IGNORED);
//    QS_FILTER_OFF(QS_QEP_DISPATCH);
//    QS_FILTER_OFF(QS_QEP_UNHANDLED);

    QS_FILTER_OFF(QS_QF_ACTIVE_ADD);
    QS_FILTER_OFF(QS_QF_ACTIVE_REMOVE);
    QS_FILTER_OFF(QS_QF_ACTIVE_SUBSCRIBE);
    QS_FILTER_OFF(QS_QF_ACTIVE_UNSUBSCRIBE);
    QS_FILTER_OFF(QS_QF_ACTIVE_POST_FIFO);
    QS_FILTER_OFF(QS_QF_ACTIVE_POST_LIFO);
    QS_FILTER_OFF(QS_QF_ACTIVE_GET);
    QS_FILTER_OFF(QS_QF_ACTIVE_GET_LAST);
    QS_FILTER_OFF(QS_QF_EQUEUE_INIT);
    QS_FILTER_OFF(QS_QF_EQUEUE_POST_FIFO);
    QS_FILTER_OFF(QS_QF_EQUEUE_POST_LIFO);
    QS_FILTER_OFF(QS_QF_EQUEUE_GET);
    QS_FILTER_OFF(QS_QF_EQUEUE_GET_LAST);
//    QS_FILTER_OFF(QS_QF_MPOOL_INIT);
//    QS_FILTER_OFF(QS_QF_MPOOL_GET);
    QS_FILTER_OFF(QS_QF_MPOOL_PUT);
    QS_FILTER_OFF(QS_QF_PUBLISH);
    QS_FILTER_OFF(QS_QF_NEW);
    QS_FILTER_OFF(QS_QF_GC_ATTEMPT);
    QS_FILTER_OFF(QS_QF_GC);
    QS_FILTER_OFF(QS_QF_TICK);
    QS_FILTER_OFF(QS_QF_TIMEEVT_ARM);
    QS_FILTER_OFF(QS_QF_TIMEEVT_AUTO_DISARM);
    QS_FILTER_OFF(QS_QF_TIMEEVT_DISARM_ATTEMPT);
    QS_FILTER_OFF(QS_QF_TIMEEVT_DISARM);
    QS_FILTER_OFF(QS_QF_TIMEEVT_REARM);
    QS_FILTER_OFF(QS_QF_TIMEEVT_POST);
    QS_FILTER_OFF(QS_QF_CRIT_ENTRY);
    QS_FILTER_OFF(QS_QF_CRIT_EXIT);
    QS_FILTER_OFF(QS_QF_ISR_ENTRY);
    QS_FILTER_OFF(QS_QF_ISR_EXIT);

    return CreateThread(NULL, 1024, &idleThread, (void *)0, 0, NULL)
             != (HANDLE)0;    // return the status of creating the idle thread
}
//............................................................................
void QP::QS::onCleanup(void) {
    QSPY_stop();
}
//............................................................................
void QP::QS::onFlush(void) {
    uint16_t nBytes = 1000;
    uint8_t const *block;
    while ((block = getBlock(&nBytes)) != (uint8_t *)0) {
        QSPY_parse(block, nBytes);
        nBytes = 1024;
    }
}
//............................................................................
QP::QSTimeCtr QP::QS::onGetTime(void) {
    return (QSTimeCtr)clock();
}
//............................................................................
extern "C" void QSPY_onPrintLn(void) {
    OutputDebugString(QSPY_line);
    OutputDebugString("\n");
}
#endif                                                                // Q_SPY
//----------------------------------------------------------------------------

}                                                              // namespace QP
