//$file${src::qxk::qxk_xthr.cpp} vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
//
// Model: qpcpp.qm
// File:  ${src::qxk::qxk_xthr.cpp}
//
// This code has been generated by QM 5.2.1 <www.state-machine.com/qm>.
// DO NOT EDIT THIS FILE MANUALLY. All your changes will be lost.
//
// This code is covered by the following QP license:
// License #    : LicenseRef-QL-dual
// Issued to    : Any user of the QP/C++ real-time embedded framework
// Framework(s) : qpcpp
// Support ends : 2023-12-31
// License scope:
//
// Copyright (C) 2005 Quantum Leaps, LLC <state-machine.com>.
//
// SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-QL-commercial
//
// This software is dual-licensed under the terms of the open source GNU
// General Public License version 3 (or any later version), or alternatively,
// under the terms of one of the closed source Quantum Leaps commercial
// licenses.
//
// The terms of the open source GNU General Public License version 3
// can be found at: <www.gnu.org/licenses/gpl-3.0>
//
// The terms of the closed source Quantum Leaps commercial licenses
// can be found at: <www.state-machine.com/licensing>
//
// Redistributions in source code must retain this top-level comment block.
// Plagiarizing this software to sidestep the license obligations is illegal.
//
// Contact information:
// <www.state-machine.com/licensing>
// <info@state-machine.com>
//
//$endhead${src::qxk::qxk_xthr.cpp} ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
//! @file
//! @brief QXK/C++ preemptive kernel extended (blocking) thread implementation

#define QP_IMPL             // this is QP implementation
#include "qf_port.hpp"      // QF port
#include "qf_pkg.hpp"       // QF package-scope internal interface
#include "qassert.h"        // QP embedded systems-friendly assertions
#ifdef Q_SPY                // QS software tracing enabled?
    #include "qs_port.hpp"  // QS port
    #include "qs_pkg.hpp"   // QS facilities for pre-defined trace records
#else
    #include "qs_dummy.hpp" // disable the QS software tracing
#endif // Q_SPY

// protection against including this source file in a wrong project
#ifndef QXK_HPP
#error "Source file included in a project NOT based on the QXK kernel"
#endif // QXK_HPP

//============================================================================
namespace { // unnamed local namespace
Q_DEFINE_THIS_MODULE("qxk_xthr")
} // unnamed namespace

//============================================================================
//$skip${QP_VERSION} vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
// Check for the minimum required QP version
#if (QP_VERSION < 690U) || (QP_VERSION != ((QP_RELEASE^4294967295U) % 0x3E8U))
#error qpcpp version 6.9.0 or higher required
#endif
//$endskip${QP_VERSION} ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

//$define${QXK::QXThread} vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
namespace QP {

//${QXK::QXThread} ...........................................................

//${QXK::QXThread::QXThread} .................................................
QXThread::QXThread(
    QXThreadHandler const handler,
    std::uint_fast8_t const tickRate) noexcept
  : QActive(Q_STATE_CAST(handler)),
    m_timeEvt(this, static_cast<enum_t>(QXK::DELAY_SIG),
                    static_cast<std::uint_fast8_t>(tickRate))
{
    m_state.act = nullptr; // mark as extended thread
}

//${QXK::QXThread::delay} ....................................................
bool QXThread::delay(std::uint_fast16_t const nTicks) noexcept {
    QF_CRIT_STAT_
    QF_CRIT_E_();

    QXThread * const thr = QXK_PTR_CAST_(QXThread*, QXK_attr_.curr);

    //! @pre this function must:
    //! - NOT be called from an ISR;
    //! - number of ticks cannot be zero
    //! - be called from an extended thread;
    //! - the thread must NOT be already blocked on any object.
    Q_REQUIRE_ID(800, (!QXK_ISR_CONTEXT_())
        && (nTicks != 0U)
        && (thr != nullptr)
        && (thr->m_temp.obj == nullptr));
    //! @pre also: the thread must NOT be holding a scheduler lock
    Q_REQUIRE_ID(801, QXK_attr_.lockHolder != thr->m_prio);

    // remember the blocking object
    thr->m_temp.obj = QXK_PTR_CAST_(QMState const*, &thr->m_timeEvt);
    thr->teArm_(static_cast<enum_t>(QXK::DELAY_SIG), nTicks);
    thr->block_();
    QF_CRIT_X_();
    QF_CRIT_EXIT_NOP(); // BLOCK here

    QF_CRIT_E_();
    // the blocking object must be the time event
    Q_ENSURE_ID(890, thr->m_temp.obj
                     == QXK_PTR_CAST_(QMState*, &thr->m_timeEvt));
    thr->m_temp.obj = nullptr; // clear
    QF_CRIT_X_();

    // signal of zero means that the time event was posted without
    // being canceled.
    return (thr->m_timeEvt.sig == 0U);
}

//${QXK::QXThread::delayCancel} ..............................................
bool QXThread::delayCancel() noexcept {
    QF_CRIT_STAT_
    QF_CRIT_E_();

    bool wasArmed;
    if (m_temp.obj == QXK_PTR_CAST_(QMState*, &m_timeEvt)) {
        wasArmed = teDisarm_();
        unblock_();
    }
    else {
        wasArmed = false;
    }
    QF_CRIT_X_();

    return wasArmed;
}

//${QXK::QXThread::queueGet} .................................................
QEvt const * QXThread::queueGet(std::uint_fast16_t const nTicks) noexcept {
    QF_CRIT_STAT_
    QF_CRIT_E_();

    QXThread * const thr = QXTHREAD_CAST_(QXK_attr_.curr);

    //! @pre this function must:
    //! - NOT be called from an ISR;
    //! - be called from an extended thread;
    //! - the thread must NOT be already blocked on any object.
    Q_REQUIRE_ID(500, (!QXK_ISR_CONTEXT_())
        && (thr != nullptr)
        && (thr->m_temp.obj == nullptr));
    //! @pre also: the thread must NOT be holding a scheduler lock.
    Q_REQUIRE_ID(501, QXK_attr_.lockHolder != thr->m_prio);

    // is the queue empty? -- block and wait for event(s)
    if (thr->m_eQueue.m_frontEvt == nullptr) {

        // remember the blocking object (the thread's queue)
        thr->m_temp.obj = QXK_PTR_CAST_(QMState*, &thr->m_eQueue);

        thr->teArm_(static_cast<enum_t>(QXK::QUEUE_SIG), nTicks);
        QF::readySet_.remove(
                           static_cast<std::uint_fast8_t>(thr->m_prio));
        static_cast<void>(QXK_sched_(0U)); // synchronous scheduling
        QF_CRIT_X_();
        QF_CRIT_EXIT_NOP(); // BLOCK here

        QF_CRIT_E_();
        // the blocking object must be this queue
        Q_ASSERT_ID(510, thr->m_temp.obj ==
                         QXK_PTR_CAST_(QMState *, &thr->m_eQueue));
        thr->m_temp.obj = nullptr; // clear
    }

    // is the queue not empty?
    QEvt const *e;
    if (thr->m_eQueue.m_frontEvt != nullptr) {
        e = thr->m_eQueue.m_frontEvt; // remove from the front
        // volatile into tmp
        QEQueueCtr const nFree = thr->m_eQueue.m_nFree + 1U;
        thr->m_eQueue.m_nFree = nFree; // update the number of free

        // any events in the ring buffer?
        if (nFree <= thr->m_eQueue.m_end) {

            // remove event from the tail
            thr->m_eQueue.m_frontEvt =
                thr->m_eQueue.m_ring[thr->m_eQueue.m_tail];
            if (thr->m_eQueue.m_tail == 0U) {
                thr->m_eQueue.m_tail = thr->m_eQueue.m_end; // wrap
            }
            // advance the tail (counter clockwise)
            thr->m_eQueue.m_tail = (thr->m_eQueue.m_tail - 1U);

            QS_BEGIN_NOCRIT_PRE_(QS_QF_ACTIVE_GET, thr->m_prio)
                QS_TIME_PRE_();      // timestamp
                QS_SIG_PRE_(e->sig); // the signal of this event
                QS_OBJ_PRE_(&thr);   // this active object
                QS_2U8_PRE_(e->poolId_, e->refCtr_); // poolID & ref Count
                QS_EQC_PRE_(nFree);  // number of free entries
            QS_END_NOCRIT_PRE_()
        }
        else {
            thr->m_eQueue.m_frontEvt = nullptr; // the queue becomes empty

            // all entries in the queue must be free (+1 for fronEvt)
            Q_ASSERT_ID(520, nFree == (thr->m_eQueue.m_end + 1U));

            QS_BEGIN_NOCRIT_PRE_(QS_QF_ACTIVE_GET_LAST, thr->m_prio)
                QS_TIME_PRE_();      // timestamp
                QS_SIG_PRE_(e->sig); // the signal of this event
                QS_OBJ_PRE_(&thr);   // this active object
                QS_2U8_PRE_(e->poolId_, e->refCtr_); // poolID & ref Count
            QS_END_NOCRIT_PRE_()
        }
    }
    else { // the queue is still empty -- the timeout must have fired
         e = nullptr;
    }
    QF_CRIT_X_();

    return e;

}

//${QXK::QXThread::init} .....................................................
void QXThread::init(
    void const * const e,
    std::uint_fast8_t const qs_id)
{
    Q_UNUSED_PAR(e);
    Q_UNUSED_PAR(qs_id);
    Q_ERROR_ID(110);
}

//${QXK::QXThread::init} .....................................................
void QXThread::init(std::uint_fast8_t const qs_id) {
    Q_UNUSED_PAR(qs_id);
    Q_ERROR_ID(111);
}

//${QXK::QXThread::dispatch} .................................................
void QXThread::dispatch(
    QEvt const * const e,
    std::uint_fast8_t const qs_id)
{
    Q_UNUSED_PAR(e);
    Q_UNUSED_PAR(qs_id);
    Q_ERROR_ID(120);
}

//${QXK::QXThread::start} ....................................................
void QXThread::start(
    QPrioSpec const prioSpec,
    QEvt const * * const qSto,
    std::uint_fast16_t const qLen,
    void * const stkSto,
    std::uint_fast16_t const stkSize,
    void const * const par)
{
    Q_UNUSED_PAR(par);

    //! @pre this function must:
    //! - NOT be called from an ISR;
    //! - the stack storage must be provided;
    //! - the thread must be instantiated (see #QXThread).
    Q_REQUIRE_ID(200, (!QXK_ISR_CONTEXT_())
        && (stkSto != nullptr)
        && (stkSize != 0U)
        && (m_state.act == nullptr));

    // is storage for the queue buffer provided?
    if (qSto != nullptr) {
        m_eQueue.init(qSto, qLen);
    }

    // extended threads provide their thread function in place of
    // the top-most initial transition 'm_temp.act'
    QXK_stackInit_(this, m_temp.thr, stkSto, stkSize);

    m_prio  = static_cast<std::uint8_t>(prioSpec & 0xFFU); //  QF-prio.
    m_pthre = static_cast<std::uint8_t>(prioSpec >> 8U); // preemption-thre.
    register_(); // make QF aware of this AO

    // the new thread is not blocked on any object
    m_temp.obj = nullptr;

    QF_CRIT_STAT_
    QF_CRIT_E_();
    // extended-thread becomes ready immediately
    QF::readySet_.insert(static_cast<std::uint_fast8_t>(m_prio));

    // see if this thread needs to be scheduled in case QXK is running
    static_cast<void>(QXK_sched_(0U)); // synchronous scheduling
    QF_CRIT_X_();
}

//${QXK::QXThread::post_} ....................................................
bool QXThread::post_(
    QEvt const * const e,
    std::uint_fast16_t const margin,
    void const * const sender) noexcept
{
    QF_CRIT_STAT_
    QS_TEST_PROBE_DEF(&QXThread::post_)

    // is it the private time event?
    bool status;
    if (e == &m_timeEvt) {
        QF_CRIT_E_();
        // the private time event is disarmed and not in any queue,
        // so it is safe to change its signal. The signal of 0 means
        // that the time event has expired.
        m_timeEvt.sig = 0U;

        unblock_();
        QF_CRIT_X_();

        status = true;
    }
    // is the event queue provided?
    else if (m_eQueue.m_end != 0U) {

        //! @pre event pointer must be valid
        Q_REQUIRE_ID(300, e != nullptr);

        QF_CRIT_E_();
        QEQueueCtr nFree = m_eQueue.m_nFree; // get volatile into temporary

        // test-probe#1 for faking queue overflow
        QS_TEST_PROBE_ID(1,
            nFree = 0U;
        )

        if (margin == QF_NO_MARGIN) {
            if (nFree > 0U) {
                status = true; // can post
            }
            else {
                status = false; // cannot post
                Q_ERROR_CRIT_(310); // must be able to post the event
            }
        }
        else if (nFree > static_cast<QEQueueCtr>(margin)) {
            status = true; // can post
        }
        else {
            status = false; // cannot post, but don't assert
        }

        // is it a dynamic event?
        if (e->poolId_ != 0U) {
            QF_EVT_REF_CTR_INC_(e); // increment the reference counter
        }

        if (status) { // can post the event?

            --nFree;  // one free entry just used up
            m_eQueue.m_nFree = nFree; // update the volatile
            if (m_eQueue.m_nMin > nFree) {
                m_eQueue.m_nMin = nFree; // update minimum so far
            }

            QS_BEGIN_NOCRIT_PRE_(QS_QF_ACTIVE_POST, m_prio)
                QS_TIME_PRE_();        // timestamp
                QS_OBJ_PRE_(sender);   // the sender object
                QS_SIG_PRE_(e->sig);   // the signal of the event
                QS_OBJ_PRE_(this);     // this active object
                QS_2U8_PRE_(e->poolId_, e->refCtr_); // poolID & refCtr
                QS_EQC_PRE_(nFree);    // number of free entries
                QS_EQC_PRE_(m_eQueue.m_nMin); // min number of free entries
            QS_END_NOCRIT_PRE_()

            // queue empty?
            if (m_eQueue.m_frontEvt == nullptr) {
                m_eQueue.m_frontEvt = e;  // deliver event directly

                // is this thread blocked on the queue?
                if (m_temp.obj == QXK_PTR_CAST_(QMState*, &m_eQueue)) {
                    static_cast<void>(teDisarm_());
                    QF::readySet_.insert(
                        static_cast<std::uint_fast8_t>(m_prio));
                    if (!QXK_ISR_CONTEXT_()) {
                        static_cast<void>(QXK_sched_(0U));
                    }
                }
            }
            // queue is not empty, insert event into the ring-buffer
            else {
                // insert event into the ring buffer (FIFO)
                m_eQueue.m_ring[m_eQueue.m_head] = e;

                // need to wrap the head couner?
                if (m_eQueue.m_head == 0U) {
                    m_eQueue.m_head = m_eQueue.m_end; // wrap around
                }
                // advance the head (counter clockwise)
                m_eQueue.m_head = (m_eQueue.m_head - 1U);
            }

            QF_CRIT_X_();
        }
        else { // cannot post the event

            QS_BEGIN_NOCRIT_PRE_(QS_QF_ACTIVE_POST_ATTEMPT, m_prio)
                QS_TIME_PRE_();        // timestamp
                QS_OBJ_PRE_(sender);   // the sender object
                QS_SIG_PRE_(e->sig);   // the signal of the event
                QS_OBJ_PRE_(this);     // this active object (recipient)
                QS_2U8_PRE_(e->poolId_, e->refCtr_); // poolID & ref Count
                QS_EQC_PRE_(nFree);    // number of free entries
                QS_EQC_PRE_(margin);   // margin
            QS_END_NOCRIT_PRE_()

            QF_CRIT_X_();

            QF::gc(e); // recycle the event to avoid a leak
        }
    }
    else { // the queue is not available
         QF::gc(e); // make sure the event is not leaked
         status = false;
         Q_ERROR_ID(320); // this extended thread cannot accept events
    }

    return status;
}

//${QXK::QXThread::postLIFO} .................................................
void QXThread::postLIFO(QEvt const * const e) noexcept {
    Q_UNUSED_PAR(e);
    Q_ERROR_ID(410);
}

//${QXK::QXThread::block_} ...................................................
void QXThread::block_() const noexcept {
    //! @pre the thread holding the lock cannot block!
    Q_REQUIRE_ID(600, (QXK_attr_.lockHolder != m_prio));
    QF::readySet_.remove(static_cast<std::uint_fast8_t>(m_prio));
    static_cast<void>(QXK_sched_(0U)); // synchronous scheduling
}

//${QXK::QXThread::unblock_} .................................................
void QXThread::unblock_() const noexcept {
    QF::readySet_.insert(static_cast<std::uint_fast8_t>(m_prio));

    if ((!QXK_ISR_CONTEXT_()) // not inside ISR?
        && (QActive::registry_[0] != nullptr)) // kernel started?
    {
        static_cast<void>(QXK_sched_(0U)); // synchronous scheduling
    }
}

//${QXK::QXThread::teArm_} ...................................................
void QXThread::teArm_(
    enum_t const sig,
    std::uint_fast16_t const nTicks) noexcept
{
    //! @pre the time event must be unused
    Q_REQUIRE_ID(700, m_timeEvt.m_ctr == 0U);

    m_timeEvt.sig = static_cast<QSignal>(sig);

    if (nTicks != QXTHREAD_NO_TIMEOUT) {
        m_timeEvt.m_ctr = static_cast<QTimeEvtCtr>(nTicks);
        m_timeEvt.m_interval = 0U;

        // is the time event unlinked?
        // NOTE: For the duration of a single clock tick of the specified tick
        // rate a time event can be disarmed and yet still linked in the list,
        // because un-linking is performed exclusively in QTimeEvt::tickX().
        if (static_cast<std::uint8_t>(m_timeEvt.refCtr_ & TE_IS_LINKED) == 0U)
        {
            std::uint_fast8_t const tickRate =
                static_cast<std::uint_fast8_t>(m_timeEvt.refCtr_);

            // mark as linked
            m_timeEvt.refCtr_ = static_cast<std::uint8_t>(
                m_timeEvt.refCtr_ | TE_IS_LINKED);

            // The time event is initially inserted into the separate
            // "freshly armed" list based on timeEvtHead_[tickRate].act.
            // Only later, inside QTimeEvt::tick_(), the "freshly armed"
            // list is appended to the main list of armed time events based on
            // timeEvtHead_[tickRate].next. Again, this is to keep any
            // changes to the main list exclusively inside QTimeEvt::tick_().
            //
            m_timeEvt.m_next
                = QXK_PTR_CAST_(QTimeEvt*,
                                QTimeEvt::timeEvtHead_[tickRate].m_act);
            QTimeEvt::timeEvtHead_[tickRate].m_act = &m_timeEvt;
        }
    }
}

//${QXK::QXThread::teDisarm_} ................................................
bool QXThread::teDisarm_() noexcept {
    bool wasArmed;
    // is the time evt running?
    if (m_timeEvt.m_ctr != 0U) {
        wasArmed = true;
        // schedule removal from list
        m_timeEvt.m_ctr = 0U;
    }
    // the time event was already automatically disarmed
    else {
        wasArmed = false;
    }
    return wasArmed;
}

} // namespace QP
//$enddef${QXK::QXThread} ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
