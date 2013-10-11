//****************************************************************************
// Product:  QF/C++, Cortex-M, Vanilla port, ARM-KEIL compiler
// Last Updated for Version: 5.1.1
// Date of the Last Update:  Oct 10, 2013
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

        // The maximum number of active objects in the application, see NOTE01
#define QF_MAX_ACTIVE               32
                                      // The number of system clock tick rates
#define QF_MAX_TICK_RATE            2

// QF interrupt disable/enable and log2()...
#if (__TARGET_ARCH_THUMB == 3)    // Cortex-M0/M0+/M1(v6-M, v6S-M)?, see NOTE2

    #define QF_INT_DISABLE()        __disable_irq()
    #define QF_INT_ENABLE()         __enable_irq()

     // QF-aware ISR priority for CMSIS function NVIC_SetPriority(), see NOTE2
    #define QF_AWARE_ISR_CMSIS_PRI  0

                            // macro to put the CPU to sleep inside QF::idle()
    #define QF_CPU_SLEEP() do { \
        __wfi(); \
        QF_INT_ENABLE(); \
    } while (false)

#else                                           // Cortex-M3/M4/M4F, see NOTE3

    #define QF_SET_BASEPRI(val)     __asm("msr BASEPRI," #val)
    #define QF_INT_DISABLE()        QF_SET_BASEPRI(QF_BASEPRI)
    #define QF_INT_ENABLE()         QF_SET_BASEPRI(0U)

       // NOTE: keep in synch with the value defined in "qk_port.s", see NOTE4
    #define QF_BASEPRI              (0xFF >> 2)

     // QF-aware ISR priority for CMSIS function NVIC_SetPriority(), see NOTE5
    #define QF_AWARE_ISR_CMSIS_PRI  (QF_BASEPRI >> (8 - __NVIC_PRIO_BITS))

                            // macro to put the CPU to sleep inside QF::idle()
    #define QF_CPU_SLEEP() do { \
        __disable_irq(); \
        QF_INT_ENABLE(); \
        __wfi(); \
        __enable_irq(); \
    } while (false)

                 // Cortex-M3/M4/M4F provide the CLZ instruction for fast LOG2
    #define QF_LOG2(n_) ((uint8_t)(32U - __clz(n_)))

#endif
                                             // QF critical section entry/exit
// QF_CRIT_STAT_TYPE not defined: unconditional interrupt enabling" policy
#define QF_CRIT_ENTRY(dummy)        QF_INT_DISABLE()
#define QF_CRIT_EXIT(dummy)         QF_INT_ENABLE()
#define QF_CRIT_EXIT_NOP()          __nop()

#include "qep_port.h"                                              // QEP port
#include "qvanilla.h"                         // Vanilla cooperative scheduler
#include "qf.h"                    // QF platform-independent public interface

//****************************************************************************
// NOTE1:
// The maximum number of active objects QF_MAX_ACTIVE can be increased
// up to 63, if necessary. Here it is set to a lower level to save some RAM.
//
// NOTE2:
// On Cortex-M0/M0+/M1 (architecture v6-M, v6S-M), the interrupt disabling
// policy uses the PRIMASK register to disable interrupts globally. The
// QF_AWARE_ISR_CMSIS_PRI level is zero, meaning that all interrupts are
// "kernel-aware".
//
// NOTE3:
// On Cortex-M3/M4/M4F, the interrupt disable/enable policy uses the BASEPRI
// register (which is not implemented in Cortex-M0/M0+/M1) to disable
// interrupts only with priority lower than the level specified by the
// QF_BASEPRI macro. The interrupts with priorities above QF_BASEPRI (i.e.,
// with numerical priority values lower than QF_BASEPRI) are not disabled in
// this method. These free-running interrupts are not allowed to call any QF
// services, because QF is not aware of these interrupts. Coversely, only
// "QF-aware" interrupts, with numerical values of priorities eqal to or
// higher than QF_BASEPRI, can call QF services.
//
// NOTE4:
// For Cortex-M3/M4/M4F, the macro QF_BASEPRI leaves the top 2 priority bits
// empty for QF-aware interrupts. This is the highest-possible priority
// (lowest possible numerical value) for the guaranteed 3 priority bits
// implemented in the NVIC.
//
// NOTE5:
// The QF_AWARE_ISR_CMSIS_PRI macro is useful as an offset for enumerating
// the QF-aware interrupt priority levels in the applications, whereas the
// numerical values of the QF-aware interrupts must be greater or equal to
// QF_AWARE_ISR_CMSIS_PRI. The enumerated values based on
// QF_AWARE_ISR_CMSIS_PRI can be passed directly to the CMSIS function
// NVIC_SetPriority(), which shifts them by (8 - __NVIC_PRIO_BITS) into the
// correct bit position, while __NVIC_PRIO_BITS is the CMSIS macro defining
// the number of implemented priority bits in the NVIC. Please note that
// the macro QF_AWARE_ISR_CMSIS_PRI is intended only for applications and
// is not used inside the QF port, which remains generic and not dependent
// on the number of implemented priority bits in the NVIC.
//

#endif                                                            // qf_port_h
