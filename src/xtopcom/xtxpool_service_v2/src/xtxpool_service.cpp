// Copyright (c) 2017-2020 Telos Foundation & contributors
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "xtxpool_service_v2/xtxpool_service.h"

#include "xcommon/xmessage_id.h"
#include "xdata/xblocktool.h"
#include "xdata/xtableblock.h"
#include "xmbus/xevent_behind.h"
#include "xmetrics/xmetrics.h"
#include "xtxpool_service_v2/xreceipt_strategy.h"
#include "xtxpool_v2/xtxpool.h"
#include "xtxpool_v2/xtxpool_error.h"
#include "xtxpool_v2/xtxpool_log.h"
#include "xutility/xhash.h"
#include "xverifier/xtx_verifier.h"
#include "xvledger/xvblock.h"
#include "xvnetwork/xvnetwork_error.h"

#include <cinttypes>

NS_BEG2(top, xtxpool_service_v2)

using xtxpool_v2::xtxpool_t;

enum enum_txpool_service_msg_type  // under pdu  enum_xpdu_type_consensus
{
    enum_txpool_service_msg_type_void = 0,  // reserved for old version

    enum_txpool_service_msg_type_send_receipt = 1,
    enum_txpool_service_msg_type_recv_receipt = 2,
};

#define load_blocks_for_missing_block_event_max (10)

class txpool_receipt_message_para_t : public top::base::xobject_t {
public:
    txpool_receipt_message_para_t(vnetwork::xvnode_address_t const & sender, vnetwork::xmessage_t const & message) : m_sender(sender), m_message(message) {
    }

private:
    ~txpool_receipt_message_para_t() override {
    }

public:
    vnetwork::xvnode_address_t m_sender;
    vnetwork::xmessage_t m_message;
};

xtxpool_service::xtxpool_service(const observer_ptr<router::xrouter_face_t> & router, const observer_ptr<xtxpool_svc_para_t> & para) : m_router(router), m_para(para) {
}

void xtxpool_service::set_params(const xvip2_t & xip, const std::shared_ptr<vnetwork::xvnetwork_driver_face_t> & vnet_driver) {
    if (xcons_utl::xip_equals(m_xip, xip)) {
        return;
    }
    m_xip = xip;
    m_vnet_driver = vnet_driver;
    m_vnetwork_str = vnet_driver->address().to_string();

    common::xnode_address_t node_addr = xcons_utl::to_address(m_xip, m_vnet_driver->address().version());

    m_node_id = static_cast<std::uint16_t>(get_node_id_from_xip2(m_xip));
    m_shard_size = static_cast<std::uint16_t>(get_group_nodes_count_from_xip2(m_xip));
    xassert(m_node_id < m_shard_size);

    auto type = node_addr.type();
    if (common::has<common::xnode_type_t::committee>(type)) {
        m_is_send_receipt_role = true;
        m_zone_index = base::enum_chain_zone_beacon_index;
    } else if (common::has<common::xnode_type_t::zec>(type)) {
        m_is_send_receipt_role = true;
        m_zone_index = base::enum_chain_zone_zec_index;
    } else if (common::has<common::xnode_type_t::auditor>(type)) {
        m_is_send_receipt_role = true;
        m_zone_index = base::enum_chain_zone_consensus_index;
    } else if (common::has<common::xnode_type_t::validator>(type)) {
        m_is_send_receipt_role = false;
        m_zone_index = base::enum_chain_zone_consensus_index;
    } else {
        xassert(0);
    }

    std::vector<uint16_t> tables = m_vnet_driver->table_ids();
    if (tables.empty()) {
        xerror("xtxpool_service::set_params xip low addr:%" PRIx64 "node:%s, load table failed", xip.low_addr, m_vnetwork_str.c_str());
        return;
    }
    m_cover_front_table_id = tables.front();
    m_cover_back_table_id = tables.back();
}

bool xtxpool_service::start(const xvip2_t & xip) {
    m_vnet_driver->register_message_ready_notify(xmessage_category_txpool, std::bind(&xtxpool_service::on_message_receipt, this, std::placeholders::_1, std::placeholders::_2));

    xinfo("xtxpool_service::start node:%s,xip:%llu,%llu zone:%d table:%d %d,is_send_receipt_role:%d",
          m_vnetwork_str.c_str(),
          xip.high_addr,
          xip.low_addr,
          m_zone_index,
          m_cover_front_table_id,
          m_cover_back_table_id,
          m_is_send_receipt_role);
    m_running = true;
    return m_running;
}
bool xtxpool_service::fade(const xvip2_t & xip) {
    xinfo("xtxpool_service::fade node:%s,xip:%llu,%llu zone:%d table:%d %d,is_send_receipt_role:%d",
          m_vnetwork_str.c_str(),
          xip.high_addr,
          xip.low_addr,
          m_zone_index,
          m_cover_front_table_id,
          m_cover_back_table_id,
          m_is_send_receipt_role);
    xassert(m_running);
    m_vnet_driver->unregister_message_ready_notify(xmessage_category_txpool);
    m_running = false;

    return !m_running;
}

bool xtxpool_service::is_running() const {
    return m_running;
}

void xtxpool_service::get_service_table_boundary(base::enum_xchain_zone_index & zone_id, uint32_t & fount_table_id, uint32_t & back_table_id) const {
    zone_id = m_zone_index;
    fount_table_id = m_cover_front_table_id;
    back_table_id = m_cover_back_table_id;
}

void xtxpool_service::resend_receipts(uint64_t now) {
    if (m_running && m_is_send_receipt_role) {
        for (uint32_t table_id = m_cover_front_table_id; table_id <= m_cover_back_table_id; table_id++) {
            if (!xreceipt_strategy_t::is_resend_node_for_talbe(now, table_id, m_shard_size, m_node_id)) {
                continue;
            }

            std::vector<xcons_transaction_ptr_t> recv_txs = m_para->get_txpool()->get_resend_txs(m_zone_index, table_id, now);
            for (auto recv_tx : recv_txs) {
                xassert(recv_tx->is_recv_tx());
                // filter out txs witch has already in txpool, just not consensused and committed.
                auto tx = m_para->get_txpool()->query_tx(recv_tx->get_source_addr(), recv_tx->get_transaction()->digest());
                if (tx != nullptr && tx->get_tx()->is_confirm_tx()) {
                    continue;
                }
                xinfo("xtxpool_service::resend_receipts recv_tx:%s", recv_tx->dump().c_str());
                send_receipt_retry(recv_tx);
            }
        }
    }
}

void xtxpool_service::pull_lacking_receipts(uint64_t now, xcovered_tables_t & covered_tables) {
    if (!m_running) {
        return;
    }
    for (uint32_t table_id = m_cover_front_table_id; table_id <= m_cover_back_table_id; table_id++) {
        if (!xreceipt_strategy_t::is_time_for_node_pull_lacking_receipts(now, table_id, m_node_id)) {
            continue;
        }

        if (covered_tables.is_covered(m_zone_index, table_id)) {
            continue;
        }
        if (!m_para->get_txpool()->need_sync_lacking_receipts(m_zone_index, table_id)) {
            continue;
        }

        std::string self_table_addr = data::xblocktool_t::make_address_table_account((base::enum_xchain_zone_index)m_zone_index, table_id);

        auto lacking_confirm_tx_hashs = m_para->get_txpool()->get_lacking_confirm_tx_hashs(m_zone_index, table_id, max_require_receipts);
        for (auto table_lacking_hashs : lacking_confirm_tx_hashs) {
            base::xtable_shortid_t peer_sid = table_lacking_hashs.get_peer_sid();
            uint16_t peer_zone_id = peer_sid >> 10;
            uint16_t peer_table_id = peer_sid - (peer_zone_id << 10);
            std::string peer_table_addr = data::xblocktool_t::make_address_table_account((base::enum_xchain_zone_index)peer_zone_id, peer_table_id);

            xreceipt_pull_confirm_receipt_t pulled_confirm_receipt;
            pulled_confirm_receipt.m_tx_from_account = self_table_addr;
            pulled_confirm_receipt.m_tx_to_account = peer_table_addr;
            pulled_confirm_receipt.m_req_node = m_vnet_driver->address();
            pulled_confirm_receipt.m_hash_of_receipts = table_lacking_hashs.get_receipt_hashs();
            for (auto hash : pulled_confirm_receipt.m_hash_of_receipts) {
                xinfo("xtxpool_service::pull_lacking_receipts confirm txs.reqnode:%s,table:%s:%s hash:%s",
                      m_vnetwork_str.c_str(),
                      self_table_addr.c_str(),
                      peer_table_addr.c_str(),
                      std::string(reinterpret_cast<char *>(hash.data()), hash.size()).c_str());
            }
            send_pull_receipts_of_confirm(pulled_confirm_receipt);
        }

        auto lacking_recv_tx_ids = m_para->get_txpool()->get_lacking_recv_tx_ids(m_zone_index, table_id, max_require_receipts);
        for (auto table_lacking_ids : lacking_recv_tx_ids) {
            base::xtable_shortid_t peer_sid = table_lacking_ids.get_peer_sid();
            uint16_t peer_zone_id = peer_sid >> 10;
            uint16_t peer_table_id = peer_sid - (peer_zone_id << 10);
            std::string peer_table_addr = data::xblocktool_t::make_address_table_account((base::enum_xchain_zone_index)peer_zone_id, peer_table_id);

            xreceipt_pull_recv_receipt_t pulled_recv_receipt;
            pulled_recv_receipt.m_tx_from_account = peer_table_addr;
            pulled_recv_receipt.m_tx_to_account = self_table_addr;
            pulled_recv_receipt.m_req_node = m_vnet_driver->address();
            pulled_recv_receipt.m_receipt_ids = table_lacking_ids.get_receipt_ids();
            for (auto receiptid : pulled_recv_receipt.m_receipt_ids) {
                xinfo("xtxpool_service::pull_lacking_receipts recv tx reqnode:%s,table:%s:%s,receiptid:%llu",
                     m_vnetwork_str.c_str(),
                     peer_table_addr.c_str(),
                     self_table_addr.c_str(),
                     receiptid);
            }
            send_pull_receipts_of_recv(pulled_recv_receipt);
        }
        covered_tables.add_covered_table(m_zone_index, table_id);
    }
}

bool xtxpool_service::is_belong_to_service(xtable_id_t tableid) const {
    if (tableid.get_zone_index() == m_zone_index && (tableid.get_subaddr() >= m_cover_front_table_id && tableid.get_subaddr() <= m_cover_back_table_id)) {
        return true;
    }
    return false;
}

bool xtxpool_service::table_boundary_equal_to(std::shared_ptr<xtxpool_service_face> & service) const {
    base::enum_xchain_zone_index zone_id;
    uint32_t fount_table_id;
    uint32_t back_table_id;
    service->get_service_table_boundary(zone_id, fount_table_id, back_table_id);
    if (zone_id == this->m_zone_index && fount_table_id == m_cover_front_table_id && back_table_id == m_cover_back_table_id) {
        return true;
    }
    return false;
}

void xtxpool_service::on_message_receipt(vnetwork::xvnode_address_t const & sender, vnetwork::xmessage_t const & message) {
    if (!m_running || m_para->get_fast_dispatcher() == nullptr) {
        return;
    }
    (void)sender;

    XMETRICS_TIME_RECORD("txpool_network_message_dispatch");

    if (message.id() == xtxpool_v2::xtxpool_msg_send_receipt || message.id() == xtxpool_v2::xtxpool_msg_recv_receipt || (message.id() == xtxpool_v2::xtxpool_msg_push_receipt)) {
        if (m_para->get_fast_dispatcher()->is_mailbox_over_limit()) {
            xwarn("xtxpool_service::on_message_receipt fast txpool mailbox limit,drop receipt");
            return;
        }

        base::xauto_ptr<txpool_receipt_message_para_t> para = new txpool_receipt_message_para_t(sender, message);
        if (message.id() == xtxpool_v2::xtxpool_msg_push_receipt) {
            auto handler = [this](base::xcall_t & call, const int32_t cur_thread_id, const uint64_t timenow_ms) -> bool {
                txpool_receipt_message_para_t * para = dynamic_cast<txpool_receipt_message_para_t *>(call.get_param1().get_object());
                this->on_message_push_receipt_received(para->m_sender, para->m_message);
                return true;
            };

            base::xcall_t asyn_call(handler, para.get());
            m_para->get_fast_dispatcher()->dispatch(asyn_call);
        } else {
            auto handler = [this](base::xcall_t & call, const int32_t cur_thread_id, const uint64_t timenow_ms) -> bool {
                txpool_receipt_message_para_t * para = dynamic_cast<txpool_receipt_message_para_t *>(call.get_param1().get_object());
                this->on_message_unit_receipt(para->m_sender, para->m_message);
                return true;
            };
            base::xcall_t asyn_call(handler, para.get());
            m_para->get_fast_dispatcher()->dispatch(asyn_call);
        }
    } else if ((message.id() == xtxpool_v2::xtxpool_msg_pull_recv_receipt) || (message.id() == xtxpool_v2::xtxpool_msg_pull_confirm_receipt)) {
        if (m_para->get_slow_dispatcher()->is_mailbox_over_limit()) {
            xwarn("xtxpool_service::on_message_receipt slow txpool mailbox limit,drop receipt");
            return;
        }

        auto handler = [this](base::xcall_t & call, const int32_t cur_thread_id, const uint64_t timenow_ms) -> bool {
            txpool_receipt_message_para_t * para = dynamic_cast<txpool_receipt_message_para_t *>(call.get_param1().get_object());
            if (para->m_message.id() == xtxpool_v2::xtxpool_msg_pull_recv_receipt) {
                this->on_message_pull_recv_receipt_received(para->m_sender, para->m_message);
            } else {
                this->on_message_pull_confirm_receipt_received(para->m_sender, para->m_message);
            }
            return true;
        };

        base::xauto_ptr<txpool_receipt_message_para_t> para = new txpool_receipt_message_para_t(sender, message);
        base::xcall_t asyn_call(handler, para.get());
        m_para->get_slow_dispatcher()->dispatch(asyn_call);
    } else {
        xassert(0);
    }
}

void xtxpool_service::on_message_unit_receipt(vnetwork::xvnode_address_t const & sender, vnetwork::xmessage_t const & message) {
    (void)sender;
    XMETRICS_TIME_RECORD("txpool_message_unit_receipt");
    base::xstream_t stream(top::base::xcontext_t::instance(), (uint8_t *)message.payload().data(), (uint32_t)message.payload().size());
    data::xcons_transaction_ptr_t receipt = make_object_ptr<data::xcons_transaction_t>();
    int32_t ret = receipt->serialize_from(stream);
    if (ret <= 0) {
        xerror("xtxpool_service::on_message_unit_receipt receipt serialize_from fail ret:%d", ret);
        return;
    }

    xinfo("xtxpool_service::on_message_unit_receipt receipt=%s,from_vnode:%s,at_node:%s", receipt->dump().c_str(), sender.to_string().c_str(), m_vnetwork_str.c_str());

    xtxpool_v2::xtx_para_t para;
    std::shared_ptr<xtxpool_v2::xtx_entry> tx_ent = std::make_shared<xtxpool_v2::xtx_entry>(receipt, para);

    bool is_self_send = (m_vnet_driver->address() == sender);
    if (is_self_send) {
        XMETRICS_COUNTER_INCREMENT("txpool_received_self_send_receipt_num", 1);
    } else {
        XMETRICS_COUNTER_INCREMENT("txpool_received_other_send_receipt_num", 1);
    }
    ret = m_para->get_txpool()->push_receipt(tx_ent, is_self_send, false);
    // push success means may be not consensused. duplicate means already committed, and need response recv receipt
    if (ret == xtxpool_v2::xtxpool_error_tx_duplicate) {
        check_and_response_recv_receipt(receipt);
    }
    if (receipt != nullptr) {
        if (common::has<common::xnode_type_t::consensus_validator>(sender.type()) || m_vnet_driver->address().cluster_address() == sender.cluster_address()) {
            return;
        }
        auditor_forward_receipt_to_shard(receipt, message);
    }
}

void xtxpool_service::check_and_response_recv_receipt(const xcons_transaction_ptr_t & cons_tx) {
    if (!cons_tx->is_recv_tx() || !m_is_send_receipt_role) {
        return;
    }
    XMETRICS_TIME_RECORD("txpool_message_unit_receipt_check_receipt");
    xdbg("xtxpool_service::check_and_response_recv_receipt receipt=%s at_node:%ld", cons_tx->dump().c_str(), m_xip.low_addr);
    // if tx subtype is recv and is resend, need not select by function has_receipt_right, because sender is already selected by gmtime before here.

    uint32_t resend_time = xreceipt_strategy_t::calc_resend_time(cons_tx->get_receipt_gmtime(), xverifier::xtx_utl::get_gmttime_s());
    if (xreceipt_strategy_t::is_selected_resender(cons_tx, resend_time, m_node_id, m_shard_size)) {
        return;
    }

    auto confirm_tx = create_confirm_tx_by_hash(cons_tx->get_transaction()->digest());
    if (confirm_tx != nullptr) {
        send_receipt_retry(confirm_tx);
    } else {
        mbus::xevent_ptr_t ev = make_object_ptr<mbus::xevent_behind_on_demand_by_hash_t>(cons_tx->get_target_addr(), cons_tx->get_transaction()->get_digest_str(), "lack of unit");
        m_para->get_bus()->push_event(ev);
        xwarn("xtxpool_service::check_and_response_recv_receipt unit of recv tx not found, try sync unit for tx:%s", cons_tx->dump().c_str());
    }
}

xcons_transaction_ptr_t xtxpool_service::create_confirm_tx_by_hash(const uint256_t & hash) {
    auto str_hash(std::string(reinterpret_cast<char *>(hash.data()), hash.size()));
    base::xvtransaction_store_ptr_t tx_store = m_para->get_vblockstore()->query_tx(str_hash, base::enum_transaction_subtype_recv);
    // first time consensus transaction has been stored, so it can be found
    // in the second consensus, need check the m_recv_unit_height

    if (tx_store == nullptr || tx_store->get_raw_tx() == nullptr) {
        xtxpool_warn("xtxpool_service::create_confirm_tx_by_hash fail-query tx from store. txhash=%s", base::xstring_utl::to_hex(str_hash).c_str());
        return nullptr;
    }

    xtransaction_t * tx = dynamic_cast<xtransaction_t *>(tx_store->get_raw_tx());
    base::xvaccount_t _vaccount(tx->get_target_addr());
    uint64_t unit_height = tx_store->get_recv_unit_height();
    base::xauto_ptr<base::xvblock_t> commit_block = m_para->get_vblockstore()->load_block_object(_vaccount, unit_height, base::enum_xvblock_flag_committed, false);
    if (commit_block == nullptr) {
        xerror("xtxpool_service::create_confirm_tx_by_hash fail-commit unit not exist txhash=%s,account=%s,block_height:%ld",
               base::xstring_utl::to_hex(str_hash).c_str(),
               _vaccount.get_account().c_str(),
               unit_height);
        return nullptr;
    }
    m_para->get_vblockstore()->load_block_input(_vaccount, commit_block.get());
    base::xauto_ptr<base::xvblock_t> cert_block = m_para->get_vblockstore()->load_block_object(_vaccount, unit_height + 2, 0, false);
    if (commit_block == nullptr) {
        xerror("xtxpool_service::create_confirm_tx_by_hash fail-cert unit not exist txhash=%s,account=%s,block_height:%ld",
               base::xstring_utl::to_hex(str_hash).c_str(),
               _vaccount.get_account().c_str(),
               unit_height + 2);
        return nullptr;
    }
    // TODO(jimmy) return txreceipt with origin tx
    base::xfull_txreceipt_ptr_t txreceipt = base::xtxreceipt_build_t::create_one_txreceipt(commit_block.get(), cert_block.get(), str_hash);
    if (txreceipt == nullptr) {
        xerror("xtxpool_service::create_confirm_tx_by_hash fail-create one txreceipt txhash=%s,account=%s,block_height:%ld",
               base::xstring_utl::to_hex(str_hash).c_str(),
               _vaccount.get_account().c_str(),
               unit_height);
        return nullptr;
    }
    data::xcons_transaction_ptr_t contx = make_object_ptr<data::xcons_transaction_t>(*txreceipt.get());
    xassert(contx->is_confirm_tx());
    xdbg("xtxpool_service::create_confirm_tx_by_hash succ-txhash=%s,account=%s,block_height:%ld",
         base::xstring_utl::to_hex(str_hash).c_str(),
         _vaccount.get_account().c_str(),
         unit_height);
    return contx;
}

bool xtxpool_service::is_receipt_sender(const xtable_id_t & tableid) const {
    return m_running && m_is_send_receipt_role && is_belong_to_service(tableid);
}

void xtxpool_service::send_receipt_retry(data::xcons_transaction_ptr_t & cons_tx) {
    send_receipt_real(cons_tx);

    if (cons_tx->is_recv_tx()) {
        XMETRICS_COUNTER_INCREMENT("txpool_recv_tx_retry_send", 1);
    } else {
        XMETRICS_COUNTER_INCREMENT("txpool_confirm_tx_retry_send", 1);
    }
}

void xtxpool_service::send_receipt_first_time(data::xcons_transaction_ptr_t & cons_tx, xblock_t * cert_block) {
    xassert(cons_tx->is_receipt_valid());
    send_receipt_real(cons_tx);
    if (cons_tx->is_recv_tx()) {
        XMETRICS_COUNTER_INCREMENT("txpool_recv_tx_first_send", 1);
    } else {
        XMETRICS_COUNTER_INCREMENT("txpool_confirm_tx_first_send", 1);
    }
}

void xtxpool_service::send_receipt_real(const data::xcons_transaction_ptr_t & cons_tx) {
    try {
        std::string target_addr = cons_tx->get_receipt_target_account();

        top::base::xautostream_t<4096> stream(top::base::xcontext_t::instance());
        cons_tx->serialize_to(stream);
        vnetwork::xmessage_t msg =
            vnetwork::xmessage_t({stream.data(), stream.data() + stream.size()}, cons_tx->is_recv_tx() ? xtxpool_v2::xtxpool_msg_send_receipt : xtxpool_v2::xtxpool_msg_recv_receipt);

        // TODO(jimmy)  first get target account's table id, then get network addr by table id
        auto receiver_cluster_addr =
            m_router->sharding_address_from_account(common::xaccount_address_t{target_addr}, m_vnet_driver->network_id(), common::xnode_type_t::consensus_auditor);
        assert(common::has<common::xnode_type_t::consensus_auditor>(receiver_cluster_addr.type()) || common::has<common::xnode_type_t::committee>(receiver_cluster_addr.type()) ||
               common::has<common::xnode_type_t::zec>(receiver_cluster_addr.type()));

        xassert(!common::has<common::xnode_type_t::consensus_validator>(m_vnet_driver->type()));
        xassert(m_is_send_receipt_role);
        if (m_vnet_driver->address().cluster_address() == receiver_cluster_addr) {
            xinfo("xtxpool_service::send_receipt_real broadcast receipt=%s,size=%zu,vnode:%s", cons_tx->dump().c_str(), stream.size(), m_vnetwork_str.c_str());
            m_vnet_driver->broadcast(msg);
            on_message_unit_receipt(m_vnet_driver->address(), msg);
        } else {
            xinfo("xtxpool_service::send_receipt_real forward receipt=%s,size=%zu,from_vnode:%s,to_vnode:%s",
                  cons_tx->dump().c_str(),
                  stream.size(),
                  m_vnetwork_str.c_str(),
                  receiver_cluster_addr.to_string().c_str());
            m_vnet_driver->forward_broadcast_message(msg, vnetwork::xvnode_address_t{std::move(receiver_cluster_addr)});
        }
    } catch (top::error::xtop_error_t const & eh) {
        xwarn("xtxpool_service::send_receipt_real xvnetwork_error_t exception caught: %s; error code: %d", eh.what(), eh.code().value());
    } catch (const std::exception & eh) {
        xwarn("xtxpool_service::send_receipt_real std exception caught: %s;", eh.what());
    }
}

void xtxpool_service::auditor_forward_receipt_to_shard(const xcons_transaction_ptr_t & cons_tx, vnetwork::xmessage_t const & message) {
    common::xnode_address_t node_addr = xcons_utl::to_address(m_xip, m_vnet_driver->address().version());
    if (!common::has<common::xnode_type_t::auditor>(node_addr.type())) {
        return;
    }

    uint32_t resend_time = xreceipt_strategy_t::calc_resend_time(cons_tx->get_receipt_gmtime(), xverifier::xtx_utl::get_gmttime_s());
    bool has_right = xreceipt_strategy_t::is_selected_resender(cons_tx, resend_time, m_node_id, m_shard_size);
    if (has_right) {
        const std::string & target_address = cons_tx->get_receipt_target_account();

        auto cluster_addr =
            m_router->sharding_address_from_account(common::xaccount_address_t{target_address}, m_vnet_driver->network_id(), common::xnode_type_t::consensus_validator);
        xassert(common::has<common::xnode_type_t::consensus_validator>(cluster_addr.type()));

        vnetwork::xvnode_address_t vaddr{std::move(cluster_addr)};
        forward_broadcast_message(vaddr, message);
    }
}

void xtxpool_service::forward_broadcast_message(const vnetwork::xvnode_address_t & addr, const vnetwork::xmessage_t & message) {
    try {
        m_vnet_driver->forward_broadcast_message(message, addr);
    } catch (top::error::xtop_error_t const & eh) {
        xwarn("xtxpool_service::forward_broadcast_message xvnetwork_error_t exception caught: %s; error code: %d", eh.what(), eh.code().value());
    } catch (const std::exception & eh) {
        xwarn("xtxpool_service::forward_broadcast_message std exception caught: %s;", eh.what());
    }
}

int32_t xtxpool_service::request_transaction_consensus(const data::xtransaction_ptr_t & tx, bool local) {
    xdbg("xtxpool_service::request_transaction_consensus in, tx:source:%s target:%s hash:%s",
         tx->get_source_addr().c_str(),
         tx->get_target_addr().c_str(),
         tx->get_digest_hex_str().c_str());
    if (!m_running) {
        xwarn(
            "[xtxpool_service]not running, tx dropped:source:%s target:%s hash:%s", tx->get_source_addr().c_str(), tx->get_target_addr().c_str(), tx->get_digest_hex_str().c_str());
        return false;
    }

    int32_t ret = xverifier::xtx_verifier::verify_send_tx_source(tx.get(), local);
    if (ret) {
        xwarn("[global_trace][xtxpool_service]tx=%s,account=%s verify send tx source fail", tx->get_digest_hex_str().c_str(), tx->get_source_addr().c_str());
        return ret;
    }

    auto tableid = data::account_map_to_table_id(common::xaccount_address_t{tx->get_source_addr()});
    if (!is_belong_to_service(tableid)) {
        xerror("[global_trace][xtxpool_service]%s %s zone%d table%d not match this network driver",
               tx->get_digest_hex_str().c_str(),
               tx->get_source_addr().c_str(),
               tableid.get_zone_index(),
               tableid.get_subaddr());
        return xtxpool_v2::xtxpool_error_transaction_not_belong_to_this_service;
    }

    if (is_sys_sharding_contract_address(common::xaccount_address_t{tx->get_target_addr()})) {
        tx->adjust_target_address(tableid.get_subaddr());
    }

    xcons_transaction_ptr_t cons_tx = make_object_ptr<xcons_transaction_t>(tx.get());
    xtxpool_v2::xtx_para_t para;
    std::shared_ptr<xtxpool_v2::xtx_entry> tx_ent = std::make_shared<xtxpool_v2::xtx_entry>(cons_tx, para);
    return m_para->get_txpool()->push_send_tx(tx_ent);
}

void xtxpool_service::deal_table_block(xblock_t * block, uint64_t now_clock) {
    const std::string & account_addr = block->get_account();
    base::xvaccount_t _vaddress(account_addr);
    auto tableid = data::account_map_to_table_id(common::xaccount_address_t{account_addr});
    if (block->get_clock() + block_clock_height_fall_behind_max > now_clock) {
        auto table_height_pair = m_table_db_event_height_map.find(tableid.get_subaddr());
        if (table_height_pair != m_table_db_event_height_map.end()) {
            uint64_t min_height = (block->get_height() > table_height_pair->second + load_blocks_for_missing_block_event_max) ?
                                      (block->get_height() - load_blocks_for_missing_block_event_max) :
                                      (table_height_pair->second + 1);
            for (uint64_t height = block->get_height() - 1; height >= min_height; height--) {
                if (!xreceipt_strategy_t::is_selected_sender(account_addr, height, m_node_id, m_shard_size)) {
                    continue;
                }
                base::xauto_ptr<base::xvblock_t> blockobj =
                    m_para->get_vblockstore()->load_block_object(_vaddress, height, base::enum_xvblock_flag_committed, false);
                if (blockobj != nullptr) {
                    m_para->get_vblockstore()->load_block_input(_vaddress, blockobj.get());
                    xblock_t * missing_block = dynamic_cast<xblock_t *>(blockobj.get());
                    if (missing_block->get_clock() + block_clock_height_fall_behind_max <= now_clock) {
                        break;
                    }
                    xinfo("xtxpool_service::deal_table_block block:%s", missing_block->dump().c_str());
                    make_receipts_and_send(missing_block);
                }
            }
        }

        if (xreceipt_strategy_t::is_selected_sender(account_addr, block->get_height(), m_node_id, m_shard_size)) {
            make_receipts_and_send(block);
        }
    }
    m_table_db_event_height_map[tableid.get_subaddr()] = block->get_height();
}

void xtxpool_service::make_receipts_and_send(xblock_t * block) {
    if (!block->is_lighttable()) {
        return;
    }
    uint64_t cert_height = block->get_height() + 2;
    base::xauto_ptr<base::xvblock_t> cert_block = m_para->get_vblockstore()->load_block_object(block->get_account(), cert_height, base::enum_xvblock_flag_authenticated, false);
    if (cert_block == nullptr) {
        xerror("xtxpool_service::make_receipts_and_send load cert block fail table:%s,height:%llu", block->get_account().c_str(), cert_height);
        return;
    }
    xblock_t * raw_cert_block = dynamic_cast<xblock_t *>(cert_block.get());
    auto receipts = xreceipt_strategy_t::make_receipts(block, cert_block.get());
    for (auto & receipt : receipts) {
        xinfo("xtxpool_service::make_receipts_and_send tx=%s,block:%s", receipt->dump().c_str(), block->dump().c_str());
        send_receipt_first_time(receipt, raw_cert_block);
    }
}

xcons_transaction_ptr_t xtxpool_service::get_confirmed_tx(const uint256_t & hash) {
    return create_confirm_tx_by_hash(hash);
}

void xtxpool_service::send_pull_receipts_of_recv(xreceipt_pull_recv_receipt_t & pulled_receipt) {
    base::xstream_t stream(base::xcontext_t::instance());
    vnetwork::xmessage_t msg;
    pulled_receipt.serialize_to(stream);
    msg = vnetwork::xmessage_t({stream.data(), stream.data() + stream.size()}, xtxpool_v2::xtxpool_msg_pull_recv_receipt);
    send_receipt_sync_msg(msg, pulled_receipt.m_tx_from_account);
}

void xtxpool_service::send_pull_receipts_of_confirm(xreceipt_pull_confirm_receipt_t & pulled_receipt) {
    base::xstream_t stream(base::xcontext_t::instance());
    vnetwork::xmessage_t msg;
    pulled_receipt.serialize_to(stream);
    msg = vnetwork::xmessage_t({stream.data(), stream.data() + stream.size()}, xtxpool_v2::xtxpool_msg_pull_confirm_receipt);
    send_receipt_sync_msg(msg, pulled_receipt.m_tx_to_account);
}

void xtxpool_service::send_push_receipts(xreceipt_push_t & pushed_receipt, vnetwork::xvnode_address_t const & target) {
    base::xstream_t stream(base::xcontext_t::instance());
    vnetwork::xmessage_t msg;
    pushed_receipt.serialize_to(stream);
    msg = vnetwork::xmessage_t({stream.data(), stream.data() + stream.size()}, xtxpool_v2::xtxpool_msg_push_receipt);

    if (target == pushed_receipt.m_req_node) {
        m_vnet_driver->send_to(target, msg);
    } else {
        std::string target_table_addr = pushed_receipt.m_receipt_type == enum_transaction_subtype_recv ? pushed_receipt.m_tx_to_account : pushed_receipt.m_tx_from_account;
        send_receipt_sync_msg(msg, target_table_addr);
    }
}

void xtxpool_service::send_receipt_sync_msg(const vnetwork::xmessage_t & msg, const std::string & target_table_addr) {
    auto cluster_addr =
        m_router->sharding_address_from_account(common::xaccount_address_t{target_table_addr}, m_vnet_driver->network_id(), common::xnode_type_t::consensus_auditor);
    if (cluster_addr == m_vnet_driver->address().cluster_address()) {
        m_vnet_driver->broadcast(msg);
    } else {
        m_vnet_driver->forward_broadcast_message(msg, common::xnode_address_t{std::move(cluster_addr)});
    }
}

void xtxpool_service::on_message_push_receipt_received(vnetwork::xvnode_address_t const & sender, vnetwork::xmessage_t const & message) {
    xinfo("xtxpool_service::on_message_push_receipt_received from_vnode:%s,at_node:%s,msg id:%x", sender.to_string().c_str(), m_vnetwork_str.c_str(), message.id());
    uint64_t now = xverifier::xtx_utl::get_gmttime_s();

    xreceipt_push_t pushed_receipt;
    base::xstream_t stream(top::base::xcontext_t::instance(), (uint8_t *)message.payload().data(), (uint32_t)message.payload().size());
    pushed_receipt.serialize_from(stream);

    if (pushed_receipt.m_req_node != m_vnet_driver->address()) {
        xdbg("xtxpool_service::on_message_receipts_received xtxpool_msg_push_receipt this node(%s) is not req node(%s).table:%s:%s",
             m_vnetwork_str.c_str(),
             pushed_receipt.m_req_node.to_string().c_str(),
             pushed_receipt.m_tx_from_account.c_str(),
             pushed_receipt.m_tx_to_account.c_str());
        auto type = pushed_receipt.m_req_node.type();
        if (!common::has<common::xnode_type_t::auditor>(type) &&
            xreceipt_strategy_t::is_selected_receipt_pull_msg_sender(pushed_receipt.m_tx_from_account, now, m_node_id, m_shard_size)) {
            xdbg("xtxpool_service::on_message_receipts_received xtxpool_msg_push_receipt transmit to reqnode:%s.table:%s:%s",
                 pushed_receipt.m_req_node.to_string().c_str(),
                 pushed_receipt.m_tx_from_account.c_str(),
                 pushed_receipt.m_tx_to_account.c_str());
            m_vnet_driver->send_to(pushed_receipt.m_req_node, message);
        }
        return;
    }

    xinfo("xtxpool_service::on_message_receipts_received push receipts.reqnode:%s,table:%s:%s,receipts num=%u",
          pushed_receipt.m_req_node.to_string().c_str(),
          pushed_receipt.m_tx_from_account.c_str(),
          pushed_receipt.m_tx_to_account.c_str(),
          pushed_receipt.m_receipts.size());
    for (auto tx : pushed_receipt.m_receipts) {
        xdbg("xtxpool_service::on_message_receipts_received xtxpool_msg_push_receipt at node:%s,table:%s:%s receipt:%s",
             m_vnetwork_str.c_str(),
             pushed_receipt.m_tx_from_account.c_str(),
             pushed_receipt.m_tx_to_account.c_str(),
             tx->dump().c_str());
        xtxpool_v2::xtx_para_t para;
        std::shared_ptr<xtxpool_v2::xtx_entry> tx_ent = std::make_shared<xtxpool_v2::xtx_entry>(tx, para);
        m_para->get_txpool()->push_receipt(tx_ent, false, true);
    }
}

void xtxpool_service::on_message_pull_recv_receipt_received(vnetwork::xvnode_address_t const & sender, vnetwork::xmessage_t const & message) {
    xinfo("xtxpool_service::on_message_push_receipt_received from_vnode:%s,at_node:%s,msg id:%x", sender.to_string().c_str(), m_vnetwork_str.c_str(), message.id());
    uint64_t now = xverifier::xtx_utl::get_gmttime_s();

    if (!m_is_send_receipt_role) {
        xinfo("xtxpool_service::on_message_receipts_received xtxpool_msg_pull_recv_receipt droped from_vnode:%s,at_node:%s,msg id:%x",
              sender.to_string().c_str(),
              m_vnetwork_str.c_str(),
              message.id());
        return;
    }
    xreceipt_pull_recv_receipt_t pulled_receipt;
    base::xstream_t stream(top::base::xcontext_t::instance(), (uint8_t *)message.payload().data(), (uint32_t)message.payload().size());
    pulled_receipt.serialize_from(stream);

    if (!xreceipt_strategy_t::is_selected_receipt_pull_msg_sender(pulled_receipt.m_tx_to_account, now, m_node_id, m_shard_size)) {
        return;
    }

    if (pulled_receipt.m_receipt_ids.size() > max_require_receipts) {
        xerror("xtxpool_service::on_message_receipts_received receipts in one request");
        return;
    }

    auto cluster_addr =
        m_router->sharding_address_from_account(common::xaccount_address_t{pulled_receipt.m_tx_from_account}, m_vnet_driver->network_id(), common::xnode_type_t::consensus_auditor);
    if (cluster_addr != m_vnet_driver->address().cluster_address()) {
        xdbg("xtxpool_service::on_message_receipts_received forward broadcast message, cluster address is %s", cluster_addr.to_string().c_str());
        m_vnet_driver->forward_broadcast_message(message, vnetwork::xvnode_address_t{std::move(cluster_addr)});
    } else {
        xinfo("xtxpool_service::on_message_receipts_received recv txs.reqnode:%s,table:%s:%s,receiptid num=%u",
              pulled_receipt.m_req_node.to_string().c_str(),
              pulled_receipt.m_tx_from_account.c_str(),
              pulled_receipt.m_tx_to_account.c_str(),
              pulled_receipt.m_receipt_ids.size());
        xreceipt_push_t pushed_receipt;
        pushed_receipt.m_receipt_type = enum_transaction_subtype_recv;
        pushed_receipt.m_tx_from_account = pulled_receipt.m_tx_from_account;
        pushed_receipt.m_tx_to_account = pulled_receipt.m_tx_to_account;
        pushed_receipt.m_req_node = pulled_receipt.m_req_node;
        for (auto receiptid : pulled_receipt.m_receipt_ids) {
            xdbg("xtxpool_service::on_message_receipts_received reqnode:%s,table:%s:%s recv_tx receiptid:%llu",
                 pulled_receipt.m_req_node.to_string().c_str(),
                 pulled_receipt.m_tx_from_account.c_str(),
                 pulled_receipt.m_tx_to_account.c_str(),
                 receiptid);
            auto tranx = m_para->get_txpool()->get_unconfirmed_tx(pulled_receipt.m_tx_from_account, pulled_receipt.m_tx_to_account, receiptid);
            if (tranx != nullptr) {
                xdbg("xtxpool_service::on_message_receipts_received reqnode:%s,table:%s:%s recv_tx:%s",
                     pulled_receipt.m_req_node.to_string().c_str(),
                     pulled_receipt.m_tx_from_account.c_str(),
                     pulled_receipt.m_tx_to_account.c_str(),
                     tranx->dump().c_str());
                pushed_receipt.m_receipts.push_back(tranx);
            }
        }
        send_push_receipts(pushed_receipt, sender);
    }
}

void xtxpool_service::on_message_pull_confirm_receipt_received(vnetwork::xvnode_address_t const & sender, vnetwork::xmessage_t const & message) {
    xinfo("xtxpool_service::on_message_push_receipt_received from_vnode:%s,at_node:%s,msg id:%x", sender.to_string().c_str(), m_vnetwork_str.c_str(), message.id());
    uint64_t now = xverifier::xtx_utl::get_gmttime_s();

    if (!m_is_send_receipt_role) {
        xinfo("xtxpool_service::on_message_receipts_received xtxpool_msg_pull_recv_receipt droped from_vnode:%s,at_node:%s,msg id:%x",
              sender.to_string().c_str(),
              m_vnetwork_str.c_str(),
              message.id());
        return;
    }
    xreceipt_pull_confirm_receipt_t pulled_receipt;
    base::xstream_t stream(top::base::xcontext_t::instance(), (uint8_t *)message.payload().data(), (uint32_t)message.payload().size());
    pulled_receipt.serialize_from(stream);

    if (!xreceipt_strategy_t::is_selected_receipt_pull_msg_sender(pulled_receipt.m_tx_from_account, now, m_node_id, m_shard_size)) {
        return;
    }

    if (pulled_receipt.m_hash_of_receipts.size() > max_require_receipts) {
        xerror("xtxpool_service::on_message_receipts_received receipts in one request");
        return;
    }
    auto cluster_addr =
        m_router->sharding_address_from_account(common::xaccount_address_t{pulled_receipt.m_tx_to_account}, m_vnet_driver->network_id(), common::xnode_type_t::consensus_auditor);
    if (cluster_addr != m_vnet_driver->address().cluster_address()) {
        xdbg("xtxpool_service::on_message_receipts_received forward broadcast message, cluster address is %s", cluster_addr.to_string().c_str());
        m_vnet_driver->forward_broadcast_message(message, vnetwork::xvnode_address_t{std::move(cluster_addr)});
    } else {
        xinfo("xtxpool_service::on_message_receipts_received confirm txs.reqnode:%s,table:%s:%s,receipt hash num=%u",
              pulled_receipt.m_req_node.to_string().c_str(),
              pulled_receipt.m_tx_from_account.c_str(),
              pulled_receipt.m_tx_to_account.c_str(),
              pulled_receipt.m_hash_of_receipts.size());
        xreceipt_push_t pushed_receipt;
        pushed_receipt.m_receipt_type = enum_transaction_subtype_confirm;
        pushed_receipt.m_tx_from_account = pulled_receipt.m_tx_from_account;
        pushed_receipt.m_tx_to_account = pulled_receipt.m_tx_to_account;
        pushed_receipt.m_req_node = pulled_receipt.m_req_node;
        for (auto tx_hash : pulled_receipt.m_hash_of_receipts) {
            xdbg("xtxpool_service::on_message_receipts_received reqnode:%s,table:%s:%s confirm_tx hash:%s",
                 pulled_receipt.m_req_node.to_string().c_str(),
                 pulled_receipt.m_tx_from_account.c_str(),
                 pulled_receipt.m_tx_to_account.c_str(),
                 std::string(reinterpret_cast<char *>(tx_hash.data()), tx_hash.size()).c_str());
            auto tranx = get_confirmed_tx(tx_hash);
            if (tranx != nullptr) {
                xdbg("xtxpool_service::on_message_receipts_received reqnode:%s,table:%s:%s confirm_tx:%s",
                     pulled_receipt.m_req_node.to_string().c_str(),
                     pulled_receipt.m_tx_from_account.c_str(),
                     pulled_receipt.m_tx_to_account.c_str(),
                     tranx->dump().c_str());
                pushed_receipt.m_receipts.push_back(tranx);
            }
        }
        send_push_receipts(pushed_receipt, sender);
    }
}

NS_END2
