//****************************************************************************
// Product: QF/C++ port to 80x86, uC/OS-II, Open Watcom
// Last Updated for Version: 5.1.0
// Date of the Last Update:  Sep 28, 2013
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
#ifndef qf_port_h
#define qf_port_h
                                      // uC/OS-II event queue and thread types
#define QF_EQUEUE_TYPE       OS_EVENT *
#define QF_THREAD_TYPE       INT8U

                    // The maximum number of active objects in the application
#define QF_MAX_ACTIVE        OS_MAX_TASKS

             // uC/OS-II critical section operations (critical section type 3)
#define QF_CRIT_STAT_TYPE    OS_CPU_SR
#define QF_CRIT_ENTRY(stat_) ((stat_) = OSCPUSaveSR())
#define QF_CRIT_EXIT(stat_)  OSCPURestoreSR(stat_)

#include "qep_port.h"        // QEP port, includes the master uC/OS-II include
#include "qequeue.h"             // native QF event queue for deferring events
#include "qf.h"                    // QF platform-independent public interface

//****************************************************************************
// interface used only inside QF, but not in applications
//
#ifdef qf_pkg_h

    namespace QP {

    class UCosMemPart {                 // uC/OS-II memory pool and block-size
        OS_MEM *m_pool;                                // uC/OS-II memory pool
        QEvtSize m_block_size;                   // the block size of the pool
        friend class QF;
    };

    }                                                          // namespace QP

                                             // uC/OS-II event pool operations
    #define QF_EPOOL_TYPE_   UCosMemPart
    #define QF_EPOOL_INIT_(p_, poolSto_, poolSize_, evtSize_) do { \
        INT8U err; \
        (p_).m_block_size = (evtSize_); \
        (p_).m_pool = OSMemCreate(poolSto_, \
                          static_cast<INT32U>((poolSize_)/(evtSize_)), \
                          static_cast<INT32U>(evtSize_), &err); \
        Q_ASSERT(err == OS_NO_ERR); \
    } while (false)

    #define QF_EPOOL_EVENT_SIZE_(p_)    ((p_).m_block_size)
    #define QF_EPOOL_GET_(p_, e_, m_) do { \
        QF_CRIT_STAT_ \
        QF_CRIT_ENTRY_(); \
        if ((p_).m_pool->OSMemNFree > (m_)) { \
            INT8U err; \
            (e_) = static_cast<QEvt *>(OSMemGet((p_).m_pool, &err)); \
            Q_ASSERT(err == OS_NO_ERR); \
        } \
        else { \
            (e_) = static_cast<QEvt *>(0); \
        } \
        QF_CRIT_EXIT_(); \
    } while (false)

    #define QF_EPOOL_PUT_(p_, e_) \
        OSMemPut((p_).m_pool, static_cast<void *>(e_))

#endif                                                       // ifdef qf_pkg_h

#endif                                                            // qf_port_h

//****************************************************************************
// NOTE01:
// The uC/OS critical section must be able to nest.
//
