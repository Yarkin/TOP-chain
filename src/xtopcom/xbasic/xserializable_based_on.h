// Copyright (c) 2017-2018 Telos Foundation & contributors
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#pragma once

#include "xbase/xdata.h"
#include "xbase/xmem.h"
#include "xbase/xcontext.h"
#include "xbasic/xns_macro.h"

#include <cstdint>
#include <string>

NS_BEG1(top)

/**
 * @brief Prototype for serialization.
 *
 * @tparam BasedOnT Serialization based on type.
 */
template <typename BasedOnT>
class xtop_serializable_based_on;

/**
 * @brief Specialization for serialization based on base::xdataunit_t
 *
 */
template <>
class xtop_serializable_based_on<base::xdataunit_t> : public base::xdataunit_t {
protected:
    xtop_serializable_based_on(xtop_serializable_based_on const &)             = default;
    xtop_serializable_based_on & operator=(xtop_serializable_based_on const &) = default;
    xtop_serializable_based_on(xtop_serializable_based_on &&)                  = default;
    xtop_serializable_based_on & operator=(xtop_serializable_based_on &&)      = default;
    virtual ~xtop_serializable_based_on()                                      = default;

    xtop_serializable_based_on() : base::xdataunit_t(base::xdataunit_t::enum_xdata_type_unknow) {
    }
};

/**
 * @brief Specialization for serialization based on nothing. Manually implement the serialization process.
 *
 */
template <>
class xtop_serializable_based_on<void> {
public:
    xtop_serializable_based_on()                                               = default;
    xtop_serializable_based_on(xtop_serializable_based_on const &)             = default;
    xtop_serializable_based_on & operator=(xtop_serializable_based_on const &) = default;
    xtop_serializable_based_on(xtop_serializable_based_on &&)                  = default;
    xtop_serializable_based_on & operator=(xtop_serializable_based_on &&)      = default;
    virtual ~xtop_serializable_based_on()                                      = default;

    /**
     * @brief Serialize the object into the stream.  The serialization operation
     *        will serialize the object into a single internal stream and then
     *        serialize this internal stream into the 'stream' parameter object.
     */
    virtual
    std::int32_t
    serialize_to(base::xstream_t & stream) const;

    /**
     * @brief Deserialize the object from the stream.  The deserialization operation
     *        will first read out an stream object form the 'stream' parameter and
     *        then deserialze based on the internal stream object.
     */
    virtual
    std::int32_t
    serialize_from(base::xstream_t & stream);

    /**
     * @brief Serialize the object into the steam directly.
     */
    virtual
    std::int32_t
    do_write(base::xstream_t & stream) const = 0;

    /**
     * @brief Deserialize the object from the stream directly.
     */
    virtual
    std::int32_t
    do_read(base::xstream_t & stream) = 0;

    std::string serialize_to_string() const {
        base::xstream_t stream(base::xcontext_t::instance());
        serialize_to(stream);
        return std::string((const char*)stream.data(), stream.size());
    }
    int32_t serialize_from_string(const std::string & str) {
        base::xstream_t _stream(base::xcontext_t::instance(), (uint8_t*)str.data(), (int32_t)str.size());
        int32_t ret = serialize_from(_stream);
        if (ret <= 0) {
            xerror("serialize_from_string fail. ret=%d,bin_data_size=%d", ret, str.size());
        }
        return ret;
    }
};

template <typename BasedOnT>
using xserializable_based_on = xtop_serializable_based_on<BasedOnT>;

NS_END1

