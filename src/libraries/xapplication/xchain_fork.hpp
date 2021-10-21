// Copyright (c) 2017-2018 Telos Foundation & contributors
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#pragma once

#include "xbase/xns_macro.h"
#include "xvledger/xvstatestore.h"
#include "xvledger/xvblockstore.h"
#include "xbasic/xmemory.hpp"

NS_BEG2(top, application)
class xchain_fork
{
private:
    observer_ptr<base::xvblockstore_t> m_blockstore;
public:
    xchain_fork(xobject_ptr_t<base::xvblockstore_t> &blockstore);
    ~xchain_fork();
public:
    bool update_state();
};

xchain_fork::xchain_fork(xobject_ptr_t<base::xvblockstore_t> &blockstore)
  : m_blockstore(blockstore)
{
}

xchain_fork::~xchain_fork()
{
}

bool xchain_fork::update_state() {
    xvaccount_t udpate_account(R"T20000MVfDLsBKVcy1wMp4CoEHWxUeBEAVBL9ZEa@47");
    auto latest_vblock = m_blockstore->get_latest_cert_block(udpate_account);
    if (latest_vblock != nullptr) {
        base::xvblkstatestore_t* blkstatestore = base::xvchain_t::instance().get_xstatestore()->get_blkstate_store();
        xinfo("xchain_fork::update_state %s", latest_vblock->dump().c_str());
        return blkstatestore->delete_states_of_db(udpate_account, latest_vblock->get_height());
    }
    xwarn("T20000MVfDLsBKVcy1wMp4CoEHWxUeBEAVBL9ZEa@47 latest cert block not exist");
    return false;
}

NS_END2