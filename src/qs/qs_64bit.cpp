//$file${src::qs::qs_64bit.cpp} vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
//
// Model: qpcpp.qm
// File:  ${src::qs::qs_64bit.cpp}
//
// This code has been generated by QM 5.2.3 <www.state-machine.com/qm>.
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
//$endhead${src::qs::qs_64bit.cpp} ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
//! @file
//! @brief QS long-long (64-bit) output

#define QP_IMPL           // this is QF/QK implementation
#include "qs_port.hpp"    // QS port
#include "qs_pkg.hpp"     // QS package-scope internal interface

//$skip${QP_VERSION} vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
// Check for the minimum required QP version
#if (QP_VERSION < 690U) || (QP_VERSION != ((QP_RELEASE^4294967295U) % 0x3E8U))
#error qpcpp version 6.9.0 or higher required
#endif
//$endskip${QP_VERSION} ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

//$define${QS::QS-tx-64bit} vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
namespace QP {
namespace QS {

//${QS::QS-tx-64bit::u64_raw_} ...............................................
void u64_raw_(std::uint64_t d) noexcept {
    std::uint8_t chksum_ = priv_.chksum;
    std::uint8_t * const buf_ = priv_.buf;
    QSCtr head_      = priv_.head;
    QSCtr const end_ = priv_.end;

    priv_.used = (priv_.used + 8U); // 8 bytes are about to be added
    for (std::int_fast8_t i = 8U; i != 0U; --i) {
        std::uint8_t const b = static_cast<std::uint8_t>(d);
        QS_INSERT_ESC_BYTE_(b)
        d >>= 8U;
    }

    priv_.head   = head_;    // save the head
    priv_.chksum = chksum_;  // save the checksum
}

//${QS::QS-tx-64bit::u64_fmt_} ...............................................
void u64_fmt_(
    std::uint8_t format,
    std::uint64_t d) noexcept
{
    std::uint8_t chksum_ = priv_.chksum;
    std::uint8_t * const buf_ = priv_.buf;
    QSCtr head_      = priv_.head;
    QSCtr const end_ = priv_.end;

    priv_.used = (priv_.used + 9U); // 9 bytes are about to be added
    QS_INSERT_ESC_BYTE_(format)  // insert the format byte

    for (std::int_fast8_t i = 8U; i != 0U; --i) {
        format = static_cast<std::uint8_t>(d);
        QS_INSERT_ESC_BYTE_(format)
        d >>= 8U;
    }

    priv_.head   = head_;    // save the head
    priv_.chksum = chksum_;  // save the checksum
}

} // namespace QS
} // namespace QP
//$enddef${QS::QS-tx-64bit} ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
