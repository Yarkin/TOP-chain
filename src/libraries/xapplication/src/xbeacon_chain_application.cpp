// Copyright (c) 2017-2018 Telos Foundation & contributors
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "xapplication/xbeacon_chain_application.h"

#include "xapplication/xapplication.h"
#include "xapplication/xerror/xerror.h"
#include "xbasic/xerror/xthrow_error.h"
#include "xcodec/xmsgpack_codec.hpp"
#include "xcommon/xip.h"
#include "xdata/xcodec/xmsgpack/xelection_result_store_codec.hpp"
#include "xdata/xelection/xelection_result_property.h"
#include "xdata/xelection/xelection_result_store.h"
#include "xdata/xblocktool.h"

NS_BEG2(top, application)

xtop_beacon_chain_application::xtop_beacon_chain_application(observer_ptr<xapplication_t> const & application,
                                                             xobject_ptr_t<base::xvblockstore_t> & blockstore,
                                                             xobject_ptr_t<base::xvnodesrv_t> & nodesvr_ptr,
                                                             xobject_ptr_t<base::xvcertauth_t> & cert_ptr,
                                                             observer_ptr<base::xiothread_t> const & grpc_thread,
                                                             observer_ptr<base::xiothread_t> const & sync_thread,
                                                             std::vector<observer_ptr<base::xiothread_t>> const & sync_account_thread_pool,
                                                             std::vector<observer_ptr<base::xiothread_t>> const & sync_handler_thread_pool)
  : xbase_t{application,
            common::xnetwork_id_t{top::config::to_chainid(XGET_CONFIG(chain_name))},
            blockstore,
            nodesvr_ptr,
            cert_ptr,
            grpc_thread,
            sync_thread,
            sync_account_thread_pool,
            sync_handler_thread_pool} {
}

static top::data::election::xelection_result_store_t load_election_data(observer_ptr<xvblockstore_t> blockstore,
                                                                        common::xaccount_address_t const & contract_address,
                                                                        uint64_t block_height,
                                                                        std::string const & property_name,
                                                                        uint64_t & returned_data_block_height) {
    data::xblocktool_t::get_committed_lightunit(blockstore.get(), contract_address.value(), block_height);
    auto latest_block = dynamic_xobject_ptr_cast<xblock_t>(latest_vblock);
    assert(latest_block);
    auto block_height = latest_block->get_height();
    while (latest_block->get_native_property().native_string_get(property, result) != 0) {
        //if (block_height == 0) {
        //    std::error_code ec = application::error::xerrc_t::load_election_data_failed;
        //    top::error::throw_error(ec);
        //}

        latest_vblock = data::xblocktool_t::get_committed_lightunit(blockstore.get(), addr, block_height - 1);
        latest_block = dynamic_xobject_ptr_cast<xblock_t>(latest_vblock);

        if (latest_block == nullptr) {
            break;
        }

        block_height = latest_block->get_height();
    }
}

void xtop_beacon_chain_application::load_last_election_data() {
    std::vector<std::string> sys_addr{sys_contract_rec_elect_rec_addr,
                                      sys_contract_rec_elect_zec_addr,
                                      sys_contract_zec_elect_consensus_addr,
                                      sys_contract_rec_elect_archive_addr,
                                      sys_contract_rec_elect_edge_addr};

    std::map<std::string, common::xzone_id_t> addr_to_zone_id{{sys_contract_rec_elect_rec_addr, common::xcommittee_zone_id},
                                                              {sys_contract_rec_elect_zec_addr, common::xzec_zone_id},
                                                              {sys_contract_zec_elect_consensus_addr, common::xdefault_zone_id},
                                                              {sys_contract_rec_elect_archive_addr, common::xarchive_zone_id},
                                                              {sys_contract_rec_elect_edge_addr, common::xedge_zone_id}};
    auto blockstore = m_application->blockstore();

    for (const auto & addr : sys_addr) {
        auto property_names = data::election::get_property_name_by_addr(common::xaccount_address_t{addr});
        for (auto const & property : property_names) {
            std::string result, result_next_to_last;
            common::xzone_id_t zone_id = addr_to_zone_id[addr];
            using top::data::election::xelection_result_store_t;

            // only use lightunit
            xobject_ptr_t<base::xvblock_t> latest_vblock = data::xblocktool_t::get_latest_committed_lightunit(blockstore.get(), addr);
            if (latest_vblock == nullptr) {
                xerror("xtop_beacon_chain_application::load_last_election_data has no latest lightunit. addr=%s", addr.c_str());
                continue;
            }

            auto latest_block = dynamic_xobject_ptr_cast<xblock_t>(latest_vblock);
            assert(latest_block);
            auto block_height = latest_block->get_height();
            while (latest_block->get_native_property().native_string_get(property, result) != 0) {
                //if (block_height == 0) {
                //    std::error_code ec = application::error::xerrc_t::load_election_data_failed;
                //    top::error::throw_error(ec);
                //}

                latest_vblock = data::xblocktool_t::get_committed_lightunit(blockstore.get(), addr, block_height - 1);
                latest_block = dynamic_xobject_ptr_cast<xblock_t>(latest_vblock);

                if (latest_block == nullptr) {
                    break;
                }

                block_height = latest_block->get_height();
            }

            if (latest_block == nullptr) {
                xerror("xbeacon_chain_application::load_latest_election_data has no lightunit. addr %s; prop", addr.c_str(), property.c_str());
                continue;
            }

            if (result.empty()) {
                xerror("xbeacon_chain_application::load_latest_election_data empty property. addr %s; prop %s", addr.c_str(), property.c_str());
                continue;
            }

            auto const & election_result_store = codec::msgpack_decode<xelection_result_store_t>({std::begin(result), std::end(result)});
            if ((addr == sys_contract_rec_elect_rec_addr || addr == sys_contract_rec_elect_zec_addr || addr == sys_contract_zec_elect_consensus_addr) && block_height != 0) {
                xobject_ptr_t<base::xvblock_t> prev_latest_vblock = data::xblocktool_t::get_committed_lightunit(m_application->blockstore().get(), addr, block_height - 1);
                if (prev_latest_vblock != nullptr) {
                    auto next_to_last_block = dynamic_xobject_ptr_cast<xblock_t>(prev_latest_vblock);
                    auto next_to_last_height = next_to_last_block->get_height();
                    if (!next_to_last_block->get_native_property().native_string_get(property, result_next_to_last) && !result_next_to_last.empty()) {
                        auto const & election_result_store_last =
                            codec::msgpack_decode<xelection_result_store_t>({std::begin(result_next_to_last), std::end(result_next_to_last)});
                        on_election_data_updated(election_result_store_last, zone_id, next_to_last_height);
                    } else {
                        xerror("xtop_beacon_chain_application::load_last_election_data fail-read property.addr=%s height=%ld", addr.c_str(), block_height - 1);
                    }
                } else {
                    xerror("xtop_beacon_chain_application::load_last_election_data has no prev lightunit. addr=%s height=%ld", addr.c_str(), block_height);
                }
            }
            on_election_data_updated(election_result_store, zone_id, block_height);

            xinfo("load new sys contract %s", addr.c_str());
        }
    }
}

NS_END2
