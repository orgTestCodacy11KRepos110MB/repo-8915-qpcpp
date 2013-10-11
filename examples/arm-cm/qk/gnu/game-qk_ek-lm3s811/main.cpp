//////////////////////////////////////////////////////////////////////////////
// Model: game.qm
// File:  ./main.cpp
//
// This code has been generated by QM tool (see state-machine.com/qm).
// DO NOT EDIT THIS FILE MANUALLY. All your changes will be lost.
//
// This program is open source software: you can redistribute it and/or
// modify it under the terms of the GNU General Public License as published
// by the Free Software Foundation.
//
// This program is distributed in the hope that it will be useful, but
// WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
// or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License
// for more details.
//////////////////////////////////////////////////////////////////////////////
// @(/3/6) ...................................................................
#include "qp_port.h"
#include "bsp.h"
#include "game.h"

namespace GAME {

//............................................................................
extern "C" int_t main() {
    static QP::QEvt const * missileQueueSto[2];
    static QP::QEvt const * shipQueueSto[3];
    static QP::QEvt const * tunnelQueueSto[GAME_MINES_MAX + 5];

    static QF_MPOOL_EL(QP::QEvt)       smlPoolSto[10];
    static QF_MPOOL_EL(ObjectImageEvt) medPoolSto[2*GAME_MINES_MAX + 10];

    static QP::QSubscrList    subscrSto[MAX_PUB_SIG];


    QP::QF::init();   // initialize the framework and the underlying RT kernel

    BSP_init();                        // initialize the Board Support Package

                                              // initialize the event pools...
    QP::QF::poolInit(smlPoolSto, sizeof(smlPoolSto), sizeof(smlPoolSto[0]));
    QP::QF::poolInit(medPoolSto, sizeof(medPoolSto), sizeof(medPoolSto[0]));

    QP::QF::psInit(subscrSto, Q_DIM(subscrSto));     // init publish-subscribe

                               // send object dictionaries for event queues...
    QS_OBJ_DICTIONARY(missileQueueSto);
    QS_OBJ_DICTIONARY(shipQueueSto);
    QS_OBJ_DICTIONARY(tunnelQueueSto);

                                // send object dictionaries for event pools...
    QS_OBJ_DICTIONARY(smlPoolSto);
    QS_OBJ_DICTIONARY(medPoolSto);

                  // send signal dictionaries for globally published events...
    QS_SIG_DICTIONARY(TIME_TICK_SIG,      static_cast<void *>(0));
    QS_SIG_DICTIONARY(PLAYER_TRIGGER_SIG, static_cast<void *>(0));
    QS_SIG_DICTIONARY(PLAYER_QUIT_SIG,    static_cast<void *>(0));
    QS_SIG_DICTIONARY(GAME_OVER_SIG,      static_cast<void *>(0));

                                                // start the active objects...
    AO_Tunnel ->start(1U,                                          // priority
                      tunnelQueueSto, Q_DIM(tunnelQueueSto),      // evt queue
                      static_cast<void *>(0), 0U);      // no per-thread stack
    AO_Missile->start(2U,                                          // priority
                      missileQueueSto, Q_DIM(missileQueueSto),    // evt queue
                      static_cast<void *>(0), 0U);      // no per-thread stack
    AO_Ship   ->start(3U,                                          // priority
                      shipQueueSto, Q_DIM(shipQueueSto),          // evt queue
                      static_cast<void *>(0), 0U);      // no per-thread stack

    return QP::QF::run();                            // run the QF application
}

} // namespace GAME

