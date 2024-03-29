//$file${.::missile.cpp} vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
//
// Model: game.qm
// File:  ${.::missile.cpp}
//
// This code has been generated by QM 5.2.4 <www.state-machine.com/qm>.
// DO NOT EDIT THIS FILE MANUALLY. All your changes will be lost.
//
// SPDX-License-Identifier: GPL-3.0-or-later
//
// This generated code is open source software: you can redistribute it under
// the terms of the GNU General Public License as published by the Free
// Software Foundation.
//
// This code is distributed in the hope that it will be useful, but WITHOUT
// ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
// FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
// more details.
//
// NOTE:
// Alternatively, this generated code may be distributed under the terms
// of Quantum Leaps commercial licenses, which expressly supersede the GNU
// General Public License and are specifically designed for licensees
// interested in retaining the proprietary status of their code.
//
// Contact information:
// <www.state-machine.com/licensing>
// <info@state-machine.com>
//
//$endhead${.::missile.cpp} ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
#include "qpcpp.hpp"
#include "bsp.hpp"
#include "game.hpp"

//Q_DEFINE_THIS_FILE

// local objects -------------------------------------------------------------
//$declare${AOs::Missile} vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
namespace GAME {

//${AOs::Missile} ............................................................
class Missile : public QP::QActive {
private:
    uint8_t m_x;
    uint8_t m_y;
    uint8_t m_exp_ctr;

public:
    Missile();

protected:
    Q_STATE_DECL(initial);
    Q_STATE_DECL(armed);
    Q_STATE_DECL(flying);
    Q_STATE_DECL(exploding);
}; // class Missile

} // namespace GAME
//$enddecl${AOs::Missile} ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

namespace GAME {

static Missile l_missile; // the sole instance of the Missile active object

// Public-scope objects ------------------------------------------------------
QP::QActive * const AO_Missile = &l_missile; // opaque pointer

} // namespace GAME

// Active object definition --------------------------------------------------
//$skip${QP_VERSION} vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
// Check for the minimum required QP version
#if (QP_VERSION < 700U) || (QP_VERSION != ((QP_RELEASE^4294967295U) % 0x3E8U))
#error qpcpp version 7.0.0 or higher required
#endif
//$endskip${QP_VERSION} ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

//$define${AOs::Missile} vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
namespace GAME {

//${AOs::Missile} ............................................................

//${AOs::Missile::Missile} ...................................................
Missile::Missile()
 : QActive(Q_STATE_CAST(&Missile::initial))
{}

//${AOs::Missile::SM} ........................................................
Q_STATE_DEF(Missile, initial) {
    //${AOs::Missile::SM::initial}
    subscribe( TIME_TICK_SIG);

    QS_OBJ_DICTIONARY(&l_missile);  // object dictionary for Missile object

    QS_FUN_DICTIONARY(&Missile::initial); // dictionaries for Missile HSM
    QS_FUN_DICTIONARY(&Missile::armed);
    QS_FUN_DICTIONARY(&Missile::flying);
    QS_FUN_DICTIONARY(&Missile::exploding);

    QS_SIG_DICTIONARY(MISSILE_FIRE_SIG,   &l_missile);     // local signals
    QS_SIG_DICTIONARY(HIT_WALL_SIG,       &l_missile);
    QS_SIG_DICTIONARY(DESTROYED_MINE_SIG, &l_missile);

    QS_FUN_DICTIONARY(&Missile::armed);
    QS_FUN_DICTIONARY(&Missile::flying);
    QS_FUN_DICTIONARY(&Missile::exploding);

    return tran(&armed);
}

//${AOs::Missile::SM::armed} .................................................
Q_STATE_DEF(Missile, armed) {
    QP::QState status_;
    switch (e->sig) {
        //${AOs::Missile::SM::armed::MISSILE_FIRE}
        case MISSILE_FIRE_SIG: {
            m_x = Q_EVT_CAST(ObjectPosEvt)->x;
            m_y = Q_EVT_CAST(ObjectPosEvt)->y;
            status_ = tran(&flying);
            break;
        }
        default: {
            status_ = super(&top);
            break;
        }
    }
    return status_;
}

//${AOs::Missile::SM::flying} ................................................
Q_STATE_DEF(Missile, flying) {
    QP::QState status_;
    switch (e->sig) {
        //${AOs::Missile::SM::flying::TIME_TICK}
        case TIME_TICK_SIG: {
            //${AOs::Missile::SM::flying::TIME_TICK::[m_x+GAME_MISSILE_SPEED_X<GAME_S~}
            if (m_x + GAME_MISSILE_SPEED_X < GAME_SCREEN_WIDTH) {
                m_x += GAME_MISSILE_SPEED_X;
                // tell the Tunnel to draw the Missile and test for wall hits
                AO_Tunnel->POST(Q_NEW(ObjectImageEvt, MISSILE_IMG_SIG,
                                      m_x, m_y, MISSILE_BMP),
                                this);
                status_ = Q_RET_HANDLED;
            }
            //${AOs::Missile::SM::flying::TIME_TICK::[else]}
            else {
                status_ = tran(&armed);
            }
            break;
        }
        //${AOs::Missile::SM::flying::HIT_WALL}
        case HIT_WALL_SIG: {
            status_ = tran(&exploding);
            break;
        }
        //${AOs::Missile::SM::flying::DESTROYED_MINE}
        case DESTROYED_MINE_SIG: {
            AO_Ship->POST(e, this);
            status_ = tran(&armed);
            break;
        }
        default: {
            status_ = super(&top);
            break;
        }
    }
    return status_;
}

//${AOs::Missile::SM::exploding} .............................................
Q_STATE_DEF(Missile, exploding) {
    QP::QState status_;
    switch (e->sig) {
        //${AOs::Missile::SM::exploding}
        case Q_ENTRY_SIG: {
            m_exp_ctr = 0U;
            status_ = Q_RET_HANDLED;
            break;
        }
        //${AOs::Missile::SM::exploding::TIME_TICK}
        case TIME_TICK_SIG: {
            //${AOs::Missile::SM::exploding::TIME_TICK::[(m_x>=GAME_SPEED_X)&&(m_exp_ctr~}
            if ((m_x >= GAME_SPEED_X) && (m_exp_ctr < 15U)) {
                ++m_exp_ctr;           // advance the explosion counter
                m_x -= GAME_SPEED_X;   // move the explosion by one step

                // tell the Tunnel to render the current stage of Explosion
                AO_Tunnel->POST(Q_NEW(ObjectImageEvt, EXPLOSION_SIG,
                                      m_x + 3U,
                                      (int8_t)((int)m_y - 4U),
                                      EXPLOSION0_BMP + (m_exp_ctr >> 2)),
                                this);
                status_ = Q_RET_HANDLED;
            }
            //${AOs::Missile::SM::exploding::TIME_TICK::[else]}
            else {
                status_ = tran(&armed);
            }
            break;
        }
        default: {
            status_ = super(&top);
            break;
        }
    }
    return status_;
}

} // namespace GAME
//$enddef${AOs::Missile} ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
