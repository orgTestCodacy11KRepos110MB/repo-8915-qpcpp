//$file${.::table.cpp} vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
//
// Model: dpp.qm
// File:  ${.::table.cpp}
//
// This code has been generated by QM 5.2.3 <www.state-machine.com/qm>.
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
//$endhead${.::table.cpp} ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
#include "qpcpp.hpp"
#include "dpp.hpp"
#include "bsp.hpp"

Q_DEFINE_THIS_FILE

// Active object class -------------------------------------------------------
//$declare${AOs::Table} vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
namespace DPP {

//${AOs::Table} ..............................................................
class Table : public QP::GuiQActive {
private:
    uint8_t m_fork[N_PHILO];
    bool m_isHungry[N_PHILO];

public:
    Table();

protected:
    Q_STATE_DECL(initial);
    Q_STATE_DECL(active);
    Q_STATE_DECL(serving);
    Q_STATE_DECL(paused);
}; // class Table

} // namespace DPP
//$enddecl${AOs::Table} ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

namespace DPP {

// helper function to provide the RIGHT neighbour of a Philo[n]
inline uint8_t RIGHT(uint8_t const n) {
    return static_cast<uint8_t>((n + (N_PHILO - 1U)) % N_PHILO);
}

// helper function to provide the LEFT neighbour of a Philo[n]
inline uint8_t LEFT(uint8_t const n) {
    return static_cast<uint8_t>((n + 1U) % N_PHILO);
}

static uint8_t const FREE = static_cast<uint8_t>(0);
static uint8_t const USED = static_cast<uint8_t>(1);

static char const * const THINKING = &"thinking"[0];
static char const * const HUNGRY   = &"hungry  "[0];
static char const * const EATING   = &"eating  "[0];

// Local objects -------------------------------------------------------------
static Table l_table; // the single instance of the Table active object

// Global-scope objects ------------------------------------------------------
QP::QActive * const AO_Table = &l_table; // "opaque" AO pointer

} // namespace DPP

//............................................................................
//$skip${QP_VERSION} vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
// Check for the minimum required QP version
#if (QP_VERSION < 690U) || (QP_VERSION != ((QP_RELEASE^4294967295U) % 0x3E8U))
#error qpcpp version 6.9.0 or higher required
#endif
//$endskip${QP_VERSION} ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

//$define${AOs::Table} vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
namespace DPP {

//${AOs::Table} ..............................................................

//${AOs::Table::Table} .......................................................
Table::Table()
 : GuiQActive(Q_STATE_CAST(&Table::initial))
{
    for (uint8_t n = 0U; n < N_PHILO; ++n) {
        m_fork[n] = FREE;
        m_isHungry[n] = false;
    }
}

//${AOs::Table::SM} ..........................................................
Q_STATE_DEF(Table, initial) {
    //${AOs::Table::SM::initial}
    (void)e; // suppress the compiler warning about unused parameter

    QS_OBJ_DICTIONARY(&l_table);
    QS_FUN_DICTIONARY(&QP::QHsm::top);
    QS_FUN_DICTIONARY(&Table::initial);
    QS_FUN_DICTIONARY(&Table::active);
    QS_FUN_DICTIONARY(&Table::serving);
    QS_FUN_DICTIONARY(&Table::paused);

    QS_SIG_DICTIONARY(DONE_SIG,      nullptr); // global signals
    QS_SIG_DICTIONARY(EAT_SIG,       nullptr);
    QS_SIG_DICTIONARY(PAUSE_SIG,     nullptr);
    QS_SIG_DICTIONARY(TERMINATE_SIG, nullptr);

    QS_SIG_DICTIONARY(HUNGRY_SIG,    this); // signal just for Table

    subscribe(DONE_SIG);
    subscribe(PAUSE_SIG);
    subscribe(TERMINATE_SIG);

    for (uint8_t n = 0U; n < N_PHILO; ++n) {
        m_fork[n] = FREE;
        m_isHungry[n] = false;
        BSP_displayPhilStat(n, THINKING);
    }

    QS_FUN_DICTIONARY(&Table::active);
    QS_FUN_DICTIONARY(&Table::serving);
    QS_FUN_DICTIONARY(&Table::paused);

    return tran(&serving);
}

//${AOs::Table::SM::active} ..................................................
Q_STATE_DEF(Table, active) {
    QP::QState status_;
    switch (e->sig) {
        //${AOs::Table::SM::active::TERMINATE}
        case TERMINATE_SIG: {
            BSP_terminate(0);
            status_ = Q_RET_HANDLED;
            break;
        }
        //${AOs::Table::SM::active::EAT}
        case EAT_SIG: {
            Q_ERROR();
            status_ = Q_RET_HANDLED;
            break;
        }
        default: {
            status_ = super(&top);
            break;
        }
    }
    return status_;
}

//${AOs::Table::SM::active::serving} .........................................
Q_STATE_DEF(Table, serving) {
    QP::QState status_;
    switch (e->sig) {
        //${AOs::Table::SM::active::serving}
        case Q_ENTRY_SIG: {
            for (uint8_t n = 0U; n < N_PHILO; ++n) { // give permissions to eat...
                if (m_isHungry[n]
                    && (m_fork[LEFT(n)] == FREE)
                    && (m_fork[n] == FREE))
                {
                    m_fork[LEFT(n)] = USED;
                    m_fork[n] = USED;
                    QP::QActive::PUBLISH(Q_NEW(TableEvt, EAT_SIG, n), this);
                    m_isHungry[n] = false;
                    BSP_displayPhilStat(n, EATING);
                }
            }
            status_ = Q_RET_HANDLED;
            break;
        }
        //${AOs::Table::SM::active::serving::HUNGRY}
        case HUNGRY_SIG: {
            uint8_t n = Q_EVT_CAST(TableEvt)->philoNum;
            // phil ID must be in range and he must be not hungry
            Q_ASSERT((n < N_PHILO) && (!m_isHungry[n]));

            BSP_displayPhilStat(n, HUNGRY);
            uint8_t m = LEFT(n);
            //${AOs::Table::SM::active::serving::HUNGRY::[bothfree]}
            if ((m_fork[m] == FREE) && (m_fork[n] == FREE)) {
                m_fork[m] = USED;
                m_fork[n] = USED;
                QP::QActive::PUBLISH(Q_NEW(TableEvt, EAT_SIG, n), this);
                BSP_displayPhilStat(n, EATING);
                status_ = Q_RET_HANDLED;
            }
            //${AOs::Table::SM::active::serving::HUNGRY::[else]}
            else {
                m_isHungry[n] = true;
                status_ = Q_RET_HANDLED;
            }
            break;
        }
        //${AOs::Table::SM::active::serving::DONE}
        case DONE_SIG: {
            uint8_t n = Q_EVT_CAST(TableEvt)->philoNum;
            // phil ID must be in range and he must be not hungry
            Q_ASSERT((n < N_PHILO) && (!m_isHungry[n]));

            BSP_displayPhilStat(n, THINKING);
            uint8_t m = LEFT(n);
            // both forks of Phil[n] must be used
            Q_ASSERT((m_fork[n] == USED) && (m_fork[m] == USED));

            m_fork[m] = FREE;
            m_fork[n] = FREE;
            m = RIGHT(n); // check the right neighbor

            if (m_isHungry[m] && (m_fork[m] == FREE)) {
                m_fork[n] = USED;
                m_fork[m] = USED;
                m_isHungry[m] = false;
                QP::QActive::PUBLISH(Q_NEW(TableEvt, EAT_SIG, m), this);
                BSP_displayPhilStat(m, EATING);
            }
            m = LEFT(n); // check the left neighbor
            n = LEFT(m); // left fork of the left neighbor
            if (m_isHungry[m] && (m_fork[n] == FREE)) {
                m_fork[m] = USED;
                m_fork[n] = USED;
                m_isHungry[m] = false;
                QP::QActive::PUBLISH(Q_NEW(TableEvt, EAT_SIG, m), this);
                BSP_displayPhilStat(m, EATING);
            }
            status_ = Q_RET_HANDLED;
            break;
        }
        //${AOs::Table::SM::active::serving::EAT}
        case EAT_SIG: {
            Q_ERROR();
            status_ = Q_RET_HANDLED;
            break;
        }
        //${AOs::Table::SM::active::serving::PAUSE}
        case PAUSE_SIG: {
            status_ = tran(&paused);
            break;
        }
        default: {
            status_ = super(&active);
            break;
        }
    }
    return status_;
}

//${AOs::Table::SM::active::paused} ..........................................
Q_STATE_DEF(Table, paused) {
    QP::QState status_;
    switch (e->sig) {
        //${AOs::Table::SM::active::paused}
        case Q_ENTRY_SIG: {
            BSP_displayPaused(1U);
            status_ = Q_RET_HANDLED;
            break;
        }
        //${AOs::Table::SM::active::paused}
        case Q_EXIT_SIG: {
            BSP_displayPaused(0U);
            status_ = Q_RET_HANDLED;
            break;
        }
        //${AOs::Table::SM::active::paused::PAUSE}
        case PAUSE_SIG: {
            status_ = tran(&serving);
            break;
        }
        //${AOs::Table::SM::active::paused::HUNGRY}
        case HUNGRY_SIG: {
            uint8_t n = Q_EVT_CAST(TableEvt)->philoNum;
            // philo ID must be in range and he must be not hungry
            Q_ASSERT((n < N_PHILO) && (!m_isHungry[n]));
            m_isHungry[n] = true;
            BSP_displayPhilStat(n, HUNGRY);
            status_ = Q_RET_HANDLED;
            break;
        }
        //${AOs::Table::SM::active::paused::DONE}
        case DONE_SIG: {
            uint8_t n = Q_EVT_CAST(TableEvt)->philoNum;
            // phil ID must be in range and he must be not hungry
            Q_ASSERT((n < N_PHILO) && (!m_isHungry[n]));

            BSP_displayPhilStat(n, THINKING);
            uint8_t m = LEFT(n);
            /* both forks of Phil[n] must be used */
            Q_ASSERT((m_fork[n] == USED) && (m_fork[m] == USED));

            m_fork[m] = FREE;
            m_fork[n] = FREE;
            status_ = Q_RET_HANDLED;
            break;
        }
        default: {
            status_ = super(&active);
            break;
        }
    }
    return status_;
}

} // namespace DPP
//$enddef${AOs::Table} ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
