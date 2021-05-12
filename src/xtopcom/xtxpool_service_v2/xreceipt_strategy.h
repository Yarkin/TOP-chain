// Copyright (c) 2017-2020 Telos Foundation & contributors
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#pragma once

#include "xbasic/xmemory.hpp"
#include "xdata/xblock.h"
#include "xdata/xcons_transaction.h"
NS_BEG2(top, xtxpool_service_v2)

class xreceipt_strategy_t {
public:
    static bool is_time_for_recover_unconfirmed_txs(uint64_t now);
    static std::vector<data::xcons_transaction_ptr_t> make_receipts(data::xblock_t * block);
    static bool is_resend_node_for_talbe(uint64_t now, uint32_t table_id, uint16_t shard_size, uint16_t self_node_id);
    static bool is_need_select_sender(base::enum_transaction_subtype subtype, uint32_t resend_time);
    static bool is_selected_sender(const data::xcons_transaction_ptr_t & cons_tx, uint32_t resend_time, uint16_t node_id, uint16_t shard_size);
    static uint32_t calc_resend_time(uint64_t tx_cert_time, uint64_t now);
    static bool is_selected_pos(uint32_t pos, uint32_t rand_pos, uint32_t select_num, uint32_t size);
};

NS_END2
