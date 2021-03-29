#include "xelect_net/include/elect_vhost.h"

#include <memory>
#include <cinttypes>

#include "xconfig/xconfig_register.h"
#include "xconfig/xpredefined_configurations.h"
#include "xpbase/base/top_log.h"
#include "xpbase/base/top_utils.h"
#include "xkad/routing_table/routing_table.h"
#include "xwrouter/register_routing_table.h"
#include "xwrouter/register_message_handler.h"
#include "xwrouter/xwrouter.h"
#include "xelect_net/proto/elect_net.pb.h"
#include "xelect_net/include/elect_uitils.h"
#include "xgossip/include/gossip_utils.h"
#include "xmetrics/xmetrics.h"
#include "xtransport/udp_transport/transport_util.h"

#include "xcommon/xip.h"
#include "xvnetwork/xvnetwork_message.h"
#include "xvnetwork/xcodec/xmsgpack/xvnetwork_message_codec.hpp"
#include "xsyncbase/xmessage_ids.h"

static const uint32_t kBroadcastTypeNodesSize = 10;

namespace top {

using namespace vnetwork;
using namespace kadmlia;

namespace elect {

EcVHost::EcVHost(const uint32_t& xnetwork_id, const EcNetcardPtr& ec_netcard, common::xnode_id_t const & node_id)
    : network::xnetwork_driver_face_t(),
      xnetwork_id_(xnetwork_id),
      ec_netcard_(ec_netcard), m_node_id_{ node_id } {}

common::xnode_id_t const & EcVHost::host_node_id() const noexcept {
    return m_node_id_;
}

network::xnode_t EcVHost::host_node() const noexcept {
    return { {}, {"" , 999} };
}

base::KadmliaKeyPtr adapt_address(const vnetwork::xvnode_address_t & address) {
    // TODO(smaug) consider for kRoot
    base::XipParser xip;
    xip.set_xnetwork_id(static_cast<uint32_t>(address.network_id().value_or(common::xbroadcast_network_id_value)));
    xip.set_zone_id(static_cast<uint8_t>(address.zone_id().value_or(common::xbroadcast_zone_id_value)));
    xip.set_cluster_id(static_cast<uint8_t>(address.cluster_id().value_or(common::xbroadcast_cluster_id_value)));
    xip.set_group_id(static_cast<uint8_t>(address.group_id().value_or(common::xbroadcast_group_id_value)));
    //xip.set_node_id(static_cast<uint8_t>(address.slot_id().value_or(0)));
    //xip.set_network_type((uint8_t)(address.cluster_address().type()));

    base::KadmliaKeyPtr kad_key_ptr;
    if (((xip.xnetwork_id() & common::xbroadcast_network_id_value) == common::xbroadcast_network_id_value) ||
        ((xip.zone_id() & common::xbroadcast_zone_id_value) == common::xbroadcast_zone_id_value)) {
        kad_key_ptr = base::GetKadmliaKey(global_node_id, true);
    } else {
        kad_key_ptr = base::GetKadmliaKey(xip);
    }


    xdbg("adapt raw p2p_addr: %s platform_addr: %s\n",
            address.to_string().c_str(),
            HexEncode(kad_key_ptr->Get()).c_str());
    return kad_key_ptr;
}

base::KadmliaKeyPtr adapt_address(
        const vnetwork::xvnode_address_t & address,
        const std::string& account) {
    base::XipParser xip;
    xip.set_xnetwork_id(static_cast<uint32_t>(address.network_id().value_or(common::xbroadcast_network_id_value)));
    xip.set_zone_id(static_cast<uint8_t>(address.zone_id().value_or(common::xbroadcast_zone_id_value)));
    xip.set_cluster_id(static_cast<uint8_t>(address.cluster_id().value_or(common::xbroadcast_cluster_id_value)));
    xip.set_group_id(static_cast<uint8_t>(address.group_id().value_or(common::xbroadcast_group_id_value)));
    //xip.set_node_id(static_cast<uint8_t>(address.slot_id().value_or(0)));
    //xip.set_network_type((uint8_t)(address.cluster_address().type()));

    auto kad_key_ptr = base::GetKadmliaKey(xip, account);

    xdbg("adapt raw p2p_addr: %s platform_addr: %s\n",
            address.to_string().c_str(),
            HexEncode(kad_key_ptr->Get()).c_str());
    return kad_key_ptr;
}

base::KadmliaKeyPtr adapt_address(
        const common::xsharding_info_t & address,
        common::xnode_type_t node_type) {
    base::XipParser xip;
    xip.set_xnetwork_id(static_cast<uint32_t>(address.network_id().value_or(common::xbroadcast_network_id_value)));
    xip.set_zone_id(static_cast<uint8_t>(address.zone_id().value_or(common::xbroadcast_zone_id_value)));
    xip.set_cluster_id(static_cast<uint8_t>(address.cluster_id().value_or(common::xbroadcast_cluster_id_value)));
    xip.set_group_id(static_cast<uint8_t>(address.group_id().value_or(common::xbroadcast_group_id_value)));
    //xip.set_node_id(static_cast<uint8_t>(address.slot_id().value_or(0)));
    //xip.set_network_type((uint8_t)(node_type));

    base::KadmliaKeyPtr kad_key_ptr;
    if (((xip.xnetwork_id() & common::xbroadcast_network_id_value) == common::xbroadcast_network_id_value) ||
        ((xip.zone_id() & common::xbroadcast_zone_id_value) == common::xbroadcast_zone_id_value)) {
        kad_key_ptr = base::GetKadmliaKey(global_node_id, true);
    } else {
        kad_key_ptr = base::GetKadmliaKey(xip);
    }


    xdbg("adapt raw p2p_addr: %s platform_addr: %s\n",
            address.to_string().c_str(),
            HexEncode(kad_key_ptr->Get()).c_str());
    return kad_key_ptr;
}

base::KadmliaKeyPtr adapt_address(
        const common::xsharding_info_t & address,
        common::xnode_type_t node_type,
        const std::string& account) {
    base::XipParser xip;
    xip.set_xnetwork_id(static_cast<uint32_t>(address.network_id().value_or(common::xbroadcast_network_id_value)));
    xip.set_zone_id(static_cast<uint8_t>(address.zone_id().value_or(common::xbroadcast_zone_id_value)));
    xip.set_cluster_id(static_cast<uint8_t>(address.cluster_id().value_or(common::xbroadcast_cluster_id_value)));
    xip.set_group_id(static_cast<uint8_t>(address.group_id().value_or(common::xbroadcast_group_id_value)));
    //xip.set_node_id(static_cast<uint8_t>(address.slot_id().value_or(0)));
    //xip.set_network_type((uint8_t)(node_type));

    auto kad_key_ptr = base::GetKadmliaKey(xip, account);

    xdbg("adapt raw p2p_addr: %s platform_addr: %s\n",
            address.to_string().c_str(),
            HexEncode(kad_key_ptr->Get()).c_str());
    return kad_key_ptr;
}

bool EcVHost::SyncMessageWhenStart(
        const vnetwork::xvnode_address_t & send_address,
        const vnetwork::xvnode_address_t & recv_address,
        const common::xmessage_id_t& message_type) const {
    if ( send_address.network_id() == common::xnetwork_id_t{top::config::to_chainid(XGET_CONFIG(chain_name))}
            && send_address.zone_id() == common::xfrozen_zone_id
            && send_address.cluster_id() == common::xdefault_cluster_id
            && send_address.group_id() == common::xdefault_group_id) {
        if (message_type == syncbase::xmessage_id_sync_frozen_gossip 
            || message_type == syncbase::xmessage_id_sync_get_blocks
            || message_type == syncbase::xmessage_id_sync_blocks) {
            TOP_DEBUG("found static xip for sync");
            return true;
        }
    }
    return false;
}

void EcVHost::send_to(
        common::xnode_id_t const & node_id,
        xbyte_buffer_t const & bytes_message,
        network::xtransmission_property_t const & transmission_property) const {
    auto new_hash_val = base::xhash32_t::digest(
            std::string((char*)bytes_message.data(),bytes_message.size()));
    auto vnetwork_message = top::codec::msgpack_decode<vnetwork::xvnetwork_message_t>(bytes_message);


    xdbg("[kadbridge] send to %s [hash: %u] [hash x64 %" PRIx64 "] msgid:%d",
            node_id.to_string().c_str(),
            new_hash_val,
            vnetwork_message.message().hash(),
            vnetwork_message.message().id());

    base::KadmliaKeyPtr send_kad_key = nullptr;
    base::KadmliaKeyPtr recv_kad_key = nullptr;

    // specially for sync module when node start
    if (SyncMessageWhenStart(vnetwork_message.sender(), vnetwork_message.receiver(), vnetwork_message.message_id())) {
        auto kroot_rt = wrouter::GetRoutingTable(kRoot, true);
        if (!kroot_rt || kroot_rt->nodes_size() == 0) {
            TOP_WARN("network not joined, send failed, try again ...");
            return;
        }
        send_kad_key = kroot_rt->get_local_node_info()->kadmlia_key();
        if (node_id.length() == 0) {
            // usually sync request
            auto recv_node = kroot_rt->GetRandomNode();
            recv_kad_key = base::GetKadmliaKey(recv_node->node_id);
            TOP_DEBUG("static xip request");
        } else {
            // usually sync response
            recv_kad_key = base::GetKadmliaKey(node_id.to_string(), true);
            TOP_DEBUG("static xip response");
        }
    } else {
        send_kad_key = adapt_address(vnetwork_message.sender(), global_node_id);
        recv_kad_key = adapt_address(vnetwork_message.receiver(), node_id.to_string());
    }

    auto msg = elect::xelect_message_t(bytes_message, vnetwork_message.message_id());
    ec_netcard_->send(send_kad_key, recv_kad_key, msg, false);
}


void EcVHost::spread_rumor(xbyte_buffer_t const & rumor) const {
    auto vnetwork_message = top::codec::msgpack_decode<vnetwork::xvnetwork_message_t>(rumor);
    auto hash_val = base::xhash32_t::digest(std::string((char*)rumor.data(), rumor.size()));
    xdbg("[kadbridge] spread_rumor to all [hash: %u] [hash x64 %" PRIx64 "] msgid:%x",
            hash_val,
            vnetwork_message.message().hash(),
            vnetwork_message.message().id());

    // specially for sync module when node start
    if (SyncMessageWhenStart(vnetwork_message.sender(), vnetwork_message.receiver(), vnetwork_message.message_id())) {
        auto kroot_rt = wrouter::GetRoutingTable(kRoot, true);
        if (!kroot_rt || kroot_rt->nodes_size() == 0) {
            TOP_WARN("network not joined, send failed, try again ...");
            return;
        }
        auto send_kad_key = kroot_rt->get_local_node_info()->kadmlia_key();
        // usually sync request
        auto recv_node = kroot_rt->GetRandomNode();
        auto recv_kad_key = base::GetKadmliaKey(recv_node->node_id);
        TOP_DEBUG("static xip request");
        auto msg = elect::xelect_message_t(rumor, vnetwork_message.message_id());
        ec_netcard_->send(send_kad_key, recv_kad_key, msg, false); // using p2p
        return;
    }

    auto recv_kad_key = adapt_address(vnetwork_message.receiver());
    auto send_kad_key = adapt_address(vnetwork_message.sender(), global_node_id);
    auto msg = elect::xelect_message_t(rumor, vnetwork_message.message_id());
    ec_netcard_->send(send_kad_key, recv_kad_key, msg, true);
}

void EcVHost::forward_broadcast(
        const common::xsharding_info_t & shardInfo,
        common::xnode_type_t node_type,
        xbyte_buffer_t const & byte_msg) const {
    auto vnetwork_message = top::codec::msgpack_decode<vnetwork::xvnetwork_message_t>(byte_msg);
    auto new_hash_val = base::xhash32_t::digest(std::string((char*)byte_msg.data(), byte_msg.size()));
    auto recv_kad_key = adapt_address(shardInfo, node_type);
    auto send_kad_key = adapt_address(vnetwork_message.sender(), global_node_id);
    xdbg("[kadbridge] forward to:%s [hash:%u] [vnetwork hash:%" PRIx64 "] msgid:%x",
        (HexEncode(send_kad_key->Get())).c_str(),
        new_hash_val,
        vnetwork_message.hash(),
        vnetwork_message.message().id());

    auto msg = elect::xelect_message_t(byte_msg, vnetwork_message.message_id());
    ec_netcard_->send(send_kad_key, recv_kad_key, msg, true);
}

void EcVHost::spread_rumor(
        const common::xsharding_info_t & shardInfo,
        xbyte_buffer_t const & rumor) const {
    auto vnetwork_message = top::codec::msgpack_decode<vnetwork::xvnetwork_message_t>(rumor);
    auto hash_val = base::xhash32_t::digest(std::string((char*)rumor.data(), rumor.size()));
    xdbg("[kadbridge] spread_rumor to:%s [hash: %u] [vnetwork hash:%" PRIx64 "] msgid:%x",
        shardInfo.to_string().c_str(),
        hash_val,vnetwork_message.hash(),
        vnetwork_message.message().id());

    // specially for sync module when node start
    if (SyncMessageWhenStart(vnetwork_message.sender(), vnetwork_message.receiver(), vnetwork_message.message_id())) {
        auto kroot_rt = wrouter::GetRoutingTable(kRoot, true);
        if (!kroot_rt || kroot_rt->nodes_size() == 0) {
            TOP_WARN("network not joined, send failed, try again ...");
            return;
        }
        auto send_kad_key = kroot_rt->get_local_node_info()->kadmlia_key();
        // usually sync request
        auto recv_node = kroot_rt->GetRandomNode();
        auto recv_kad_key = base::GetKadmliaKey(recv_node->node_id);
        TOP_DEBUG("static xip request");
        auto msg = elect::xelect_message_t(rumor, vnetwork_message.message_id());
        ec_netcard_->send(send_kad_key, recv_kad_key, msg, false); // using p2p
        return;
    }


    auto recv_kad_key = adapt_address(shardInfo, common::xnode_type_t::cluster);
    auto send_kad_key = adapt_address(vnetwork_message.sender(), global_node_id);
    auto msg = elect::xelect_message_t(rumor, vnetwork_message.message_id());
    ec_netcard_->send(send_kad_key, recv_kad_key, msg, true);
}



bool EcVHost::p2p_bootstrap(std::vector<network::xdht_node_t> const & seeds) const {
    return true;
}

void EcVHost::direct_send_to(
        network::xnode_t const & to,
        xbyte_buffer_t verification_data,
        network::xtransmission_property_t const & transmission_property) {
    xdbg("[kadbridge] direct_send_to");
}

std::vector<common::xnode_id_t> EcVHost::neighbors() const {
    std::vector<common::xnode_id_t> empty;
    return empty;
}

std::size_t EcVHost::neighbor_size_upper_limit() const noexcept {
    return 256;
}

network::p2p::xdht_host_face_t const & EcVHost::dht_host() const noexcept {
    static network::p2p::xdht_host_t shost(m_node_id_, nullptr, nullptr);
    return shost;
}

void EcVHost::register_message_ready_notify(network::xnetwork_message_ready_callback_t cb) noexcept {
    assert(cb != nullptr);
    ec_netcard_->register_message_ready_notify(cb, xnetwork_id_);
}

void EcVHost::unregister_message_ready_notify() {
    ec_netcard_->unregister_message_ready_notify(xnetwork_id_);
}

}  // namespace elect

}  // namespace top