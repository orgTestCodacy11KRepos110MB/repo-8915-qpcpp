//****************************************************************************
// Product: Product: "Fly'n'Shoot" game, BSP for 80x86, Open Watcom
// Last Updated for Version: 4.5.00
// Date of the Last Update:  May 20, 2012
//
//                    Q u a n t u m     L e a P s
//                    ---------------------------
//                    innovating embedded systems
//
// Copyright (C) 2002-2012 Quantum Leaps, LLC. All rights reserved.
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
#include "video.h"

#include <dos.h>                          // for _dos_setvect()/_dos_getvect()
#include <conio.h>                                         // for inp()/outp()
#include <stdlib.h>                                             // for _exit()

Q_DEFINE_THIS_FILE

// Local-scope objects -------------------------------------------------------
static void interrupt (*l_dosTmrISR)(void);
static void interrupt (*l_dosKbdISR)(void);
static void interrupt (*l_dosSpareISR)(void);

#ifdef Q_SPY
    static uint16_t l_uart_base;           // QS data uplink UART base address
    QSTimeCtr l_tickTime;                           // keeps timetsamp at tick
    static uint8_t   l_tmr;
    static uint8_t   l_kbd;

    #define UART_TXFIFO_DEPTH 16
#endif

#define TMR_VECTOR      0x08
#define KBD_VECTOR      0x09
#define SPARE_VECTOR    0x81

//............................................................................
static void interrupt ISR_tmr(void) {
    QF_ISR_ENTRY();

    QF::TICK(&l_tmr);                         // process all armed time events
    static QEvt const tickEvt = { TIME_TICK_SIG, 0 };
    QF::PUBLISH(&tickEvt, &l_tmr);                   // publish the tick event

#ifdef Q_SPY
    l_tickTime += 0x10000;
#endif


    QF_ISR_EXIT();
}
//............................................................................
static void interrupt ISR_kbd(void) {
    static uint8_t ship_pos = GAME_SHIP_Y;

    QF_ISR_ENTRY();

    uint8_t key = inp(0x60);         // key scan code from 8042 kbd controller
    uint8_t kcr = inp(0x61);                  // get keyboard control register
    outp(0x61, (uint8_t)(kcr | 0x80));          // toggle acknowledge bit high
    outp(0x61, kcr);                             // toggle acknowledge bit low

    switch (key) {
        case 200:                                                  // Up-arrow
        case 208: {                                              // Down-arrow
            ObjectPosEvt *ope = Q_NEW(ObjectPosEvt, PLAYER_SHIP_MOVE_SIG);
            if ((key == (uint8_t)200) && (ship_pos > 0x00)) {
                --ship_pos;
            }
            else if ((key == (uint8_t)208)
                     && (ship_pos < (GAME_SCREEN_HEIGHT - 3))) {
                ++ship_pos;
            }
            ope->x = (uint8_t)GAME_SHIP_X;              // x-position is fixed
            ope->y = (uint8_t)ship_pos;
            AO_Ship->POST(ope, &l_kbd);                    // post to the ship

            Video::printNumAt(24, 24, Video::FGND_YELLOW, ship_pos);
            break;
        }
        case 57: {                                                    // Space
            static uint16_t ntrig = 0;
            static QEvt const fireEvt = { PLAYER_TRIGGER_SIG, 0 };
            QF::PUBLISH(&fireEvt, &l_kbd);

            Video::printNumAt(47, 24, Video::FGND_YELLOW, ++ntrig);
            break;
        }                                                               // Esc
        case 129: {
            static QEvt const quitEvt = { PLAYER_QUIT_SIG, 0 };
            QF::PUBLISH(&quitEvt, &l_kbd);
            break;
        }
    }

    QF_ISR_EXIT();
}
//............................................................................
void BSP_init(int argc, char *argv[]) {
    char const *com = "COM1";

    com = com;                     // avoid compiler warning if QS is not used

    if (argc > 1) {
        com = argv[1];
    }
    if (!QS_INIT(com)) {                                      // initialize QS
        Q_ERROR();
    }

    QS_OBJ_DICTIONARY(&l_tmr);
    QS_OBJ_DICTIONARY(&l_kbd);

    Video::clearScreen(Video::BGND_LIGHT_GRAY);
    Video::clearRect( 0,  0, 80,   1, Video::BGND_RED | Video::BGND_BLINK);
    Video::clearRect( 0,  8, 80,  24, Video::BGND_BLACK | Video::FGND_WHITE);
    Video::clearRect( 0,  7, 80,   8, Video::BGND_BLUE);
    Video::clearRect( 0, 24, 80,  25, Video::BGND_BLUE);

    Video::clearRect(24, 24, 28,  25, Video::BGND_RED | Video::BGND_BLINK);
    Video::clearRect(24, 24, 28,  25, Video::BGND_RED | Video::BGND_BLINK);

    Video::printStrAt(35,  0, Video::FGND_WHITE, "FLY 'n' SHOOT");
    Video::printStrAt(15,  2, Video::FGND_BLACK,
                     "Press UP-arrow   to move the space ship up");
    Video::printStrAt(15,  3, Video::FGND_BLACK,
                     "Press DOWN-arrow to move the space ship down");
    Video::printStrAt(15,  4, Video::FGND_BLACK,
                     "Press SPACE      to fire the missile");
    Video::printStrAt(15,  5, Video::FGND_BLACK,
                     "Press ESC        to quit the game");
    Video::printStrAt( 8, 24, Video::FGND_WHITE, "Ship Position:");
    Video::printStrAt(37, 24, Video::FGND_WHITE, "Triggers:");
    Video::printStrAt(61, 24, Video::FGND_WHITE, "Score:");

    Video::clearRect(24, 24, 28,  25, Video::BGND_RED);
    Video::clearRect(47, 24, 51,  25, Video::BGND_RED);
    Video::clearRect(68, 24, 72,  25, Video::BGND_RED);
    Video::printNumAt(24, 24, Video::FGND_YELLOW, 0);
    Video::printNumAt(47, 24, Video::FGND_YELLOW, 0);
    Video::printNumAt(68, 24, Video::FGND_YELLOW, 0);
}
//............................................................................
void BSP_drawBitmap(uint8_t const *bitmap, uint8_t width, uint8_t height) {
    Video::drawBitmapAt(0, 8, bitmap, width, height);
}
//............................................................................
void BSP_drawNString(uint8_t x, uint8_t y, char const *str) {
    Video::drawStringAt(x, 8 + y*8, str);
}
//............................................................................
void BSP_updateScore(uint16_t score) {
    if (score == 0) {
        Video::clearRect(68, 24, 72,  25, Video::BGND_RED);
    }
    Video::printNumAt(68, 24, Video::FGND_YELLOW, score);
}
//............................................................................
void BSP_displayOn(void) {
}
//............................................................................
void BSP_displayOff(void) {
    Video::clearRect( 0,  8, 80,  24, Video::BGND_BLACK | Video::FGND_WHITE);
}

//............................................................................
void QF::onStartup(void) {
    uint16_t count;
                                         // save the origingal DOS vectors ...
    l_dosTmrISR   = _dos_getvect(TMR_VECTOR);
    l_dosKbdISR   = _dos_getvect(KBD_VECTOR);
    l_dosSpareISR = _dos_getvect(SPARE_VECTOR);

    QF_INT_DISABLE();
    count = (uint16_t)(((1193180 * 2) / BSP_TICKS_PER_SEC + 1) >> 1);
    outp(0x43, 0x36);                    // use mode-3 for timer 0 in the 8254
    outp(0x40, count & 0xFF);                     // load low  byte of timer 0
    outp(0x40, (count >> 8) & 0xFF);              // load high byte of timer 0

    _dos_setvect(TMR_VECTOR, &ISR_tmr);
    _dos_setvect(KBD_VECTOR, &ISR_kbd);
    _dos_setvect(SPARE_VECTOR, l_dosTmrISR);
    QF_INT_ENABLE();
}
//............................................................................
void QF::onCleanup(void) {
                                  // restore the DOS system clock tick rate...
    QF_INT_DISABLE();
    outp(0x43, 0x36);                    // use mode-3 for timer 0 in the 8254
    outp(0x40, 0);                                // load low  byte of timer 0
    outp(0x40, 0);                                // load high byte of timer 0
                                       // restore the original DOS vectors ...
    _dos_setvect(TMR_VECTOR, l_dosTmrISR);
    _dos_setvect(KBD_VECTOR, l_dosKbdISR);
    _dos_setvect(SPARE_VECTOR, l_dosSpareISR);
    QF_INT_ENABLE();

    QS_EXIT();                                                      // exit QS
    _exit(0);                                                   // exit to DOS
}
//............................................................................
void QF::onIdle(void) {              // NOTE: entered with interrupts DISABLED
    QF_INT_ENABLE();                        // must at least enable interrupts
#ifdef Q_SPY
    if ((inp(l_uart_base + 5) & (1 << 5)) != 0) {            // Tx FIFO empty?
        uint16_t fifo = UART_TXFIFO_DEPTH;       // depth of the 15550 Tx FIFO
        uint8_t const *block;
        QF_INT_DISABLE();
        block = QS::getBlock(&fifo);      // try to get next block to transmit
        QF_INT_ENABLE();
        while (fifo-- != 0) {                       // any bytes in the block?
            outp(l_uart_base + 0, *block++);
        }
    }
#endif
}
//----------------------------------------------------------------------------
void Q_onAssert(char const Q_ROM * const Q_ROM_VAR file, int line) {
    Video::clearRect ( 0, 24, 80, 25, Video::BGND_RED);
    Video::printStrAt( 0, 24, Video::FGND_WHITE, "ASSERTION FAILED in file:");
    Video::printStrAt(26, 24, Video::FGND_YELLOW, file);
    Video::printStrAt(57, 24, Video::FGND_WHITE, "line:");
    Video::printNumAt(62, 24, Video::FGND_YELLOW, line);

    QF::stop();                                         // stop QF and cleanup
}

//----------------------------------------------------------------------------
#ifdef Q_SPY

//............................................................................
static uint8_t UART_config(char const *comName, uint32_t baud) {
    switch (comName[3]) {             // Set the base address of the COMx port
        case '1': l_uart_base = (uint16_t)0x03F8; break;               // COM1
        case '2': l_uart_base = (uint16_t)0x02F8; break;               // COM2
        case '3': l_uart_base = (uint16_t)0x03E8; break;               // COM3
        case '4': l_uart_base = (uint16_t)0x02E8; break;               // COM4
        default: return (uint8_t)0;           // COM port out of range failure
    }
    baud = (uint16_t)(115200UL / baud);               // divisor for baud rate
    outp(l_uart_base + 3, (1 << 7));          // Set divisor access bit (DLAB)
    outp(l_uart_base + 0, (uint8_t)baud);                      // Load divisor
    outp(l_uart_base + 1, (uint8_t)(baud >> 8));
    outp(l_uart_base + 3, (1 << 1) | (1 << 0));       // LCR:8-bits,no p,1stop
    outp(l_uart_base + 4, (1 << 3) | (1 << 1) | (1 << 0));      // DTR,RTS,Out
    outp(l_uart_base + 1, 0);           // Put UART into the polling FIFO mode
    outp(l_uart_base + 2, (1 << 2) | (1 << 0));       // FCR: enable, TX clear

    return (uint8_t)1;                                              // success
}
//............................................................................
bool QS::onStartup(void const *arg) {
    static uint8_t qsBuf[1*1024];                    // buffer for Quantum Spy
    initBuf(qsBuf, sizeof(qsBuf));
                                                    // setup the QS filters...
    QS_FILTER_ON(QS_ALL_RECORDS);

//    QS_FILTER_OFF(QS_QEP_STATE_EMPTY);
//    QS_FILTER_OFF(QS_QEP_STATE_ENTRY);
//    QS_FILTER_OFF(QS_QEP_STATE_EXIT);
//    QS_FILTER_OFF(QS_QEP_STATE_INIT);
//    QS_FILTER_OFF(QS_QEP_INIT_TRAN);
//    QS_FILTER_OFF(QS_QEP_INTERN_TRAN);
//    QS_FILTER_OFF(QS_QEP_TRAN);
//    QS_FILTER_OFF(QS_QEP_IGNORED);

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
    QS_FILTER_OFF(QS_QF_MPOOL_INIT);
    QS_FILTER_OFF(QS_QF_MPOOL_GET);
    QS_FILTER_OFF(QS_QF_MPOOL_PUT);
    QS_FILTER_OFF(QS_QF_PUBLISH);
    QS_FILTER_OFF(QS_QF_NEW);
    QS_FILTER_OFF(QS_QF_GC_ATTEMPT);
    QS_FILTER_OFF(QS_QF_GC);
//    QS_FILTER_OFF(QS_QF_TICK);
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

    return UART_config((char const *)arg, 115200UL);
}
//............................................................................
void QS::onCleanup(void) {
}
//............................................................................
QSTimeCtr QS::onGetTime(void) {              // invoked with interrupts locked
    static uint32_t l_lastTime;
    uint32_t now;
    uint16_t count16;                            // 16-bit count from the 8254

    outp(0x43, 0);                         // latch the 8254's counter-0 count
    count16 = (uint16_t)inp(0x40);           // read the low byte of counter-0
    count16 += ((uint16_t)inp(0x40) << 8);               // add on the hi byte

    now = l_tickTime + (0x10000 - count16);

    if (l_lastTime > now) {                    // are we going "back" in time?
        now += 0x10000;                  // assume that there was one rollover
    }
    l_lastTime = now;

    return (QSTimeCtr)now;
}
//............................................................................
void QS::onFlush(void) {
    uint16_t b;
    while ((b = getByte()) != QS_EOD) {       // next QS trace byte available?
        while ((inp(l_uart_base + 5) & (1 << 5)) == 0) {         // not empty?
        }
        outp(l_uart_base + 0, (uint8_t)b);          // put the byte to TX FIFO
    }
}
#endif                                                                // Q_SPY
//----------------------------------------------------------------------------
