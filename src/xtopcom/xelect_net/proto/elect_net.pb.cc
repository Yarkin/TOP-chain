// Generated by the protocol buffer compiler.  DO NOT EDIT!
// source: elect_net.proto

#include "elect_net.pb.h"

#include <algorithm>

#include <google/protobuf/stubs/common.h>
#include <google/protobuf/stubs/port.h>
#include <google/protobuf/io/coded_stream.h>
#include <google/protobuf/wire_format_lite_inl.h>
#include <google/protobuf/descriptor.h>
#include <google/protobuf/generated_message_reflection.h>
#include <google/protobuf/reflection_ops.h>
#include <google/protobuf/wire_format.h>
// This is a temporary google only hack
#ifdef GOOGLE_PROTOBUF_ENFORCE_UNIQUENESS
#include "third_party/protobuf/version.h"
#endif
// @@protoc_insertion_point(includes)

namespace top {
namespace elect {
namespace protobuf {
class VhostMessageDefaultTypeInternal {
 public:
  ::google::protobuf::internal::ExplicitlyConstructed<VhostMessage>
      _instance;
} _VhostMessage_default_instance_;
}  // namespace protobuf
}  // namespace elect
}  // namespace top
namespace protobuf_elect_5fnet_2eproto {
static void InitDefaultsVhostMessage() {
  GOOGLE_PROTOBUF_VERIFY_VERSION;

  {
    void* ptr = &::top::elect::protobuf::_VhostMessage_default_instance_;
    new (ptr) ::top::elect::protobuf::VhostMessage();
    ::google::protobuf::internal::OnShutdownDestroyMessage(ptr);
  }
  ::top::elect::protobuf::VhostMessage::InitAsDefaultInstance();
}

::google::protobuf::internal::SCCInfo<0> scc_info_VhostMessage =
    {{ATOMIC_VAR_INIT(::google::protobuf::internal::SCCInfoBase::kUninitialized), 0, InitDefaultsVhostMessage}, {}};

void InitDefaults() {
  ::google::protobuf::internal::InitSCC(&scc_info_VhostMessage.base);
}

::google::protobuf::Metadata file_level_metadata[1];

const ::google::protobuf::uint32 TableStruct::offsets[] GOOGLE_PROTOBUF_ATTRIBUTE_SECTION_VARIABLE(protodesc_cold) = {
  GOOGLE_PROTOBUF_GENERATED_MESSAGE_FIELD_OFFSET(::top::elect::protobuf::VhostMessage, _has_bits_),
  GOOGLE_PROTOBUF_GENERATED_MESSAGE_FIELD_OFFSET(::top::elect::protobuf::VhostMessage, _internal_metadata_),
  ~0u,  // no _extensions_
  ~0u,  // no _oneof_case_
  ~0u,  // no _weak_field_map_
  GOOGLE_PROTOBUF_GENERATED_MESSAGE_FIELD_OFFSET(::top::elect::protobuf::VhostMessage, type_),
  GOOGLE_PROTOBUF_GENERATED_MESSAGE_FIELD_OFFSET(::top::elect::protobuf::VhostMessage, data_),
  GOOGLE_PROTOBUF_GENERATED_MESSAGE_FIELD_OFFSET(::top::elect::protobuf::VhostMessage, cb_type_),
  GOOGLE_PROTOBUF_GENERATED_MESSAGE_FIELD_OFFSET(::top::elect::protobuf::VhostMessage, node_id_),
  2,
  0,
  3,
  1,
};
static const ::google::protobuf::internal::MigrationSchema schemas[] GOOGLE_PROTOBUF_ATTRIBUTE_SECTION_VARIABLE(protodesc_cold) = {
  { 0, 9, sizeof(::top::elect::protobuf::VhostMessage)},
};

static ::google::protobuf::Message const * const file_default_instances[] = {
  reinterpret_cast<const ::google::protobuf::Message*>(&::top::elect::protobuf::_VhostMessage_default_instance_),
};

void protobuf_AssignDescriptors() {
  AddDescriptors();
  AssignDescriptors(
      "elect_net.proto", schemas, file_default_instances, TableStruct::offsets,
      file_level_metadata, NULL, NULL);
}

void protobuf_AssignDescriptorsOnce() {
  static ::google::protobuf::internal::once_flag once;
  ::google::protobuf::internal::call_once(once, protobuf_AssignDescriptors);
}

void protobuf_RegisterTypes(const ::std::string&) GOOGLE_PROTOBUF_ATTRIBUTE_COLD;
void protobuf_RegisterTypes(const ::std::string&) {
  protobuf_AssignDescriptorsOnce();
  ::google::protobuf::internal::RegisterAllTypes(file_level_metadata, 1);
}

void AddDescriptorsImpl() {
  InitDefaults();
  static const char descriptor[] GOOGLE_PROTOBUF_ATTRIBUTE_SECTION_VARIABLE(protodesc_cold) = {
      "\n\017elect_net.proto\022\022top.elect.protobuf\"L\n"
      "\014VhostMessage\022\014\n\004type\030\001 \001(\r\022\014\n\004data\030\002 \001("
      "\014\022\017\n\007cb_type\030\003 \001(\r\022\017\n\007node_id\030\004 \001(\014"
  };
  ::google::protobuf::DescriptorPool::InternalAddGeneratedFile(
      descriptor, 115);
  ::google::protobuf::MessageFactory::InternalRegisterGeneratedFile(
    "elect_net.proto", &protobuf_RegisterTypes);
}

void AddDescriptors() {
  static ::google::protobuf::internal::once_flag once;
  ::google::protobuf::internal::call_once(once, AddDescriptorsImpl);
}
// Force AddDescriptors() to be called at dynamic initialization time.
struct StaticDescriptorInitializer {
  StaticDescriptorInitializer() {
    AddDescriptors();
  }
} static_descriptor_initializer;
}  // namespace protobuf_elect_5fnet_2eproto
namespace top {
namespace elect {
namespace protobuf {

// ===================================================================

void VhostMessage::InitAsDefaultInstance() {
}
#if !defined(_MSC_VER) || _MSC_VER >= 1900
const int VhostMessage::kTypeFieldNumber;
const int VhostMessage::kDataFieldNumber;
const int VhostMessage::kCbTypeFieldNumber;
const int VhostMessage::kNodeIdFieldNumber;
#endif  // !defined(_MSC_VER) || _MSC_VER >= 1900

VhostMessage::VhostMessage()
  : ::google::protobuf::Message(), _internal_metadata_(NULL) {
  ::google::protobuf::internal::InitSCC(
      &protobuf_elect_5fnet_2eproto::scc_info_VhostMessage.base);
  SharedCtor();
  // @@protoc_insertion_point(constructor:top.elect.protobuf.VhostMessage)
}
VhostMessage::VhostMessage(const VhostMessage& from)
  : ::google::protobuf::Message(),
      _internal_metadata_(NULL),
      _has_bits_(from._has_bits_) {
  _internal_metadata_.MergeFrom(from._internal_metadata_);
  data_.UnsafeSetDefault(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
  if (from.has_data()) {
    data_.AssignWithDefault(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), from.data_);
  }
  node_id_.UnsafeSetDefault(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
  if (from.has_node_id()) {
    node_id_.AssignWithDefault(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), from.node_id_);
  }
  ::memcpy(&type_, &from.type_,
    static_cast<size_t>(reinterpret_cast<char*>(&cb_type_) -
    reinterpret_cast<char*>(&type_)) + sizeof(cb_type_));
  // @@protoc_insertion_point(copy_constructor:top.elect.protobuf.VhostMessage)
}

void VhostMessage::SharedCtor() {
  data_.UnsafeSetDefault(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
  node_id_.UnsafeSetDefault(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
  ::memset(&type_, 0, static_cast<size_t>(
      reinterpret_cast<char*>(&cb_type_) -
      reinterpret_cast<char*>(&type_)) + sizeof(cb_type_));
}

VhostMessage::~VhostMessage() {
  // @@protoc_insertion_point(destructor:top.elect.protobuf.VhostMessage)
  SharedDtor();
}

void VhostMessage::SharedDtor() {
  data_.DestroyNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
  node_id_.DestroyNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}

void VhostMessage::SetCachedSize(int size) const {
  _cached_size_.Set(size);
}
const ::google::protobuf::Descriptor* VhostMessage::descriptor() {
  ::protobuf_elect_5fnet_2eproto::protobuf_AssignDescriptorsOnce();
  return ::protobuf_elect_5fnet_2eproto::file_level_metadata[kIndexInFileMessages].descriptor;
}

const VhostMessage& VhostMessage::default_instance() {
  ::google::protobuf::internal::InitSCC(&protobuf_elect_5fnet_2eproto::scc_info_VhostMessage.base);
  return *internal_default_instance();
}


void VhostMessage::Clear() {
// @@protoc_insertion_point(message_clear_start:top.elect.protobuf.VhostMessage)
  ::google::protobuf::uint32 cached_has_bits = 0;
  // Prevent compiler warnings about cached_has_bits being unused
  (void) cached_has_bits;

  cached_has_bits = _has_bits_[0];
  if (cached_has_bits & 3u) {
    if (cached_has_bits & 0x00000001u) {
      data_.ClearNonDefaultToEmptyNoArena();
    }
    if (cached_has_bits & 0x00000002u) {
      node_id_.ClearNonDefaultToEmptyNoArena();
    }
  }
  if (cached_has_bits & 12u) {
    ::memset(&type_, 0, static_cast<size_t>(
        reinterpret_cast<char*>(&cb_type_) -
        reinterpret_cast<char*>(&type_)) + sizeof(cb_type_));
  }
  _has_bits_.Clear();
  _internal_metadata_.Clear();
}

bool VhostMessage::MergePartialFromCodedStream(
    ::google::protobuf::io::CodedInputStream* input) {
#define DO_(EXPRESSION) if (!GOOGLE_PREDICT_TRUE(EXPRESSION)) goto failure
  ::google::protobuf::uint32 tag;
  // @@protoc_insertion_point(parse_start:top.elect.protobuf.VhostMessage)
  for (;;) {
    ::std::pair<::google::protobuf::uint32, bool> p = input->ReadTagWithCutoffNoLastTag(127u);
    tag = p.first;
    if (!p.second) goto handle_unusual;
    switch (::google::protobuf::internal::WireFormatLite::GetTagFieldNumber(tag)) {
      // optional uint32 type = 1;
      case 1: {
        if (static_cast< ::google::protobuf::uint8>(tag) ==
            static_cast< ::google::protobuf::uint8>(8u /* 8 & 0xFF */)) {
          set_has_type();
          DO_((::google::protobuf::internal::WireFormatLite::ReadPrimitive<
                   ::google::protobuf::uint32, ::google::protobuf::internal::WireFormatLite::TYPE_UINT32>(
                 input, &type_)));
        } else {
          goto handle_unusual;
        }
        break;
      }

      // optional bytes data = 2;
      case 2: {
        if (static_cast< ::google::protobuf::uint8>(tag) ==
            static_cast< ::google::protobuf::uint8>(18u /* 18 & 0xFF */)) {
          DO_(::google::protobuf::internal::WireFormatLite::ReadBytes(
                input, this->mutable_data()));
        } else {
          goto handle_unusual;
        }
        break;
      }

      // optional uint32 cb_type = 3;
      case 3: {
        if (static_cast< ::google::protobuf::uint8>(tag) ==
            static_cast< ::google::protobuf::uint8>(24u /* 24 & 0xFF */)) {
          set_has_cb_type();
          DO_((::google::protobuf::internal::WireFormatLite::ReadPrimitive<
                   ::google::protobuf::uint32, ::google::protobuf::internal::WireFormatLite::TYPE_UINT32>(
                 input, &cb_type_)));
        } else {
          goto handle_unusual;
        }
        break;
      }

      // optional bytes node_id = 4;
      case 4: {
        if (static_cast< ::google::protobuf::uint8>(tag) ==
            static_cast< ::google::protobuf::uint8>(34u /* 34 & 0xFF */)) {
          DO_(::google::protobuf::internal::WireFormatLite::ReadBytes(
                input, this->mutable_node_id()));
        } else {
          goto handle_unusual;
        }
        break;
      }

      default: {
      handle_unusual:
        if (tag == 0) {
          goto success;
        }
        DO_(::google::protobuf::internal::WireFormat::SkipField(
              input, tag, _internal_metadata_.mutable_unknown_fields()));
        break;
      }
    }
  }
success:
  // @@protoc_insertion_point(parse_success:top.elect.protobuf.VhostMessage)
  return true;
failure:
  // @@protoc_insertion_point(parse_failure:top.elect.protobuf.VhostMessage)
  return false;
#undef DO_
}

void VhostMessage::SerializeWithCachedSizes(
    ::google::protobuf::io::CodedOutputStream* output) const {
  // @@protoc_insertion_point(serialize_start:top.elect.protobuf.VhostMessage)
  ::google::protobuf::uint32 cached_has_bits = 0;
  (void) cached_has_bits;

  cached_has_bits = _has_bits_[0];
  // optional uint32 type = 1;
  if (cached_has_bits & 0x00000004u) {
    ::google::protobuf::internal::WireFormatLite::WriteUInt32(1, this->type(), output);
  }

  // optional bytes data = 2;
  if (cached_has_bits & 0x00000001u) {
    ::google::protobuf::internal::WireFormatLite::WriteBytesMaybeAliased(
      2, this->data(), output);
  }

  // optional uint32 cb_type = 3;
  if (cached_has_bits & 0x00000008u) {
    ::google::protobuf::internal::WireFormatLite::WriteUInt32(3, this->cb_type(), output);
  }

  // optional bytes node_id = 4;
  if (cached_has_bits & 0x00000002u) {
    ::google::protobuf::internal::WireFormatLite::WriteBytesMaybeAliased(
      4, this->node_id(), output);
  }

  if (_internal_metadata_.have_unknown_fields()) {
    ::google::protobuf::internal::WireFormat::SerializeUnknownFields(
        _internal_metadata_.unknown_fields(), output);
  }
  // @@protoc_insertion_point(serialize_end:top.elect.protobuf.VhostMessage)
}

::google::protobuf::uint8* VhostMessage::InternalSerializeWithCachedSizesToArray(
    bool deterministic, ::google::protobuf::uint8* target) const {
  (void)deterministic; // Unused
  // @@protoc_insertion_point(serialize_to_array_start:top.elect.protobuf.VhostMessage)
  ::google::protobuf::uint32 cached_has_bits = 0;
  (void) cached_has_bits;

  cached_has_bits = _has_bits_[0];
  // optional uint32 type = 1;
  if (cached_has_bits & 0x00000004u) {
    target = ::google::protobuf::internal::WireFormatLite::WriteUInt32ToArray(1, this->type(), target);
  }

  // optional bytes data = 2;
  if (cached_has_bits & 0x00000001u) {
    target =
      ::google::protobuf::internal::WireFormatLite::WriteBytesToArray(
        2, this->data(), target);
  }

  // optional uint32 cb_type = 3;
  if (cached_has_bits & 0x00000008u) {
    target = ::google::protobuf::internal::WireFormatLite::WriteUInt32ToArray(3, this->cb_type(), target);
  }

  // optional bytes node_id = 4;
  if (cached_has_bits & 0x00000002u) {
    target =
      ::google::protobuf::internal::WireFormatLite::WriteBytesToArray(
        4, this->node_id(), target);
  }

  if (_internal_metadata_.have_unknown_fields()) {
    target = ::google::protobuf::internal::WireFormat::SerializeUnknownFieldsToArray(
        _internal_metadata_.unknown_fields(), target);
  }
  // @@protoc_insertion_point(serialize_to_array_end:top.elect.protobuf.VhostMessage)
  return target;
}

size_t VhostMessage::ByteSizeLong() const {
// @@protoc_insertion_point(message_byte_size_start:top.elect.protobuf.VhostMessage)
  size_t total_size = 0;

  if (_internal_metadata_.have_unknown_fields()) {
    total_size +=
      ::google::protobuf::internal::WireFormat::ComputeUnknownFieldsSize(
        _internal_metadata_.unknown_fields());
  }
  if (_has_bits_[0 / 32] & 15u) {
    // optional bytes data = 2;
    if (has_data()) {
      total_size += 1 +
        ::google::protobuf::internal::WireFormatLite::BytesSize(
          this->data());
    }

    // optional bytes node_id = 4;
    if (has_node_id()) {
      total_size += 1 +
        ::google::protobuf::internal::WireFormatLite::BytesSize(
          this->node_id());
    }

    // optional uint32 type = 1;
    if (has_type()) {
      total_size += 1 +
        ::google::protobuf::internal::WireFormatLite::UInt32Size(
          this->type());
    }

    // optional uint32 cb_type = 3;
    if (has_cb_type()) {
      total_size += 1 +
        ::google::protobuf::internal::WireFormatLite::UInt32Size(
          this->cb_type());
    }

  }
  int cached_size = ::google::protobuf::internal::ToCachedSize(total_size);
  SetCachedSize(cached_size);
  return total_size;
}

void VhostMessage::MergeFrom(const ::google::protobuf::Message& from) {
// @@protoc_insertion_point(generalized_merge_from_start:top.elect.protobuf.VhostMessage)
  GOOGLE_DCHECK_NE(&from, this);
  const VhostMessage* source =
      ::google::protobuf::internal::DynamicCastToGenerated<const VhostMessage>(
          &from);
  if (source == NULL) {
  // @@protoc_insertion_point(generalized_merge_from_cast_fail:top.elect.protobuf.VhostMessage)
    ::google::protobuf::internal::ReflectionOps::Merge(from, this);
  } else {
  // @@protoc_insertion_point(generalized_merge_from_cast_success:top.elect.protobuf.VhostMessage)
    MergeFrom(*source);
  }
}

void VhostMessage::MergeFrom(const VhostMessage& from) {
// @@protoc_insertion_point(class_specific_merge_from_start:top.elect.protobuf.VhostMessage)
  GOOGLE_DCHECK_NE(&from, this);
  _internal_metadata_.MergeFrom(from._internal_metadata_);
  ::google::protobuf::uint32 cached_has_bits = 0;
  (void) cached_has_bits;

  cached_has_bits = from._has_bits_[0];
  if (cached_has_bits & 15u) {
    if (cached_has_bits & 0x00000001u) {
      set_has_data();
      data_.AssignWithDefault(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), from.data_);
    }
    if (cached_has_bits & 0x00000002u) {
      set_has_node_id();
      node_id_.AssignWithDefault(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), from.node_id_);
    }
    if (cached_has_bits & 0x00000004u) {
      type_ = from.type_;
    }
    if (cached_has_bits & 0x00000008u) {
      cb_type_ = from.cb_type_;
    }
    _has_bits_[0] |= cached_has_bits;
  }
}

void VhostMessage::CopyFrom(const ::google::protobuf::Message& from) {
// @@protoc_insertion_point(generalized_copy_from_start:top.elect.protobuf.VhostMessage)
  if (&from == this) return;
  Clear();
  MergeFrom(from);
}

void VhostMessage::CopyFrom(const VhostMessage& from) {
// @@protoc_insertion_point(class_specific_copy_from_start:top.elect.protobuf.VhostMessage)
  if (&from == this) return;
  Clear();
  MergeFrom(from);
}

bool VhostMessage::IsInitialized() const {
  return true;
}

void VhostMessage::Swap(VhostMessage* other) {
  if (other == this) return;
  InternalSwap(other);
}
void VhostMessage::InternalSwap(VhostMessage* other) {
  using std::swap;
  data_.Swap(&other->data_, &::google::protobuf::internal::GetEmptyStringAlreadyInited(),
    GetArenaNoVirtual());
  node_id_.Swap(&other->node_id_, &::google::protobuf::internal::GetEmptyStringAlreadyInited(),
    GetArenaNoVirtual());
  swap(type_, other->type_);
  swap(cb_type_, other->cb_type_);
  swap(_has_bits_[0], other->_has_bits_[0]);
  _internal_metadata_.Swap(&other->_internal_metadata_);
}

::google::protobuf::Metadata VhostMessage::GetMetadata() const {
  protobuf_elect_5fnet_2eproto::protobuf_AssignDescriptorsOnce();
  return ::protobuf_elect_5fnet_2eproto::file_level_metadata[kIndexInFileMessages];
}


// @@protoc_insertion_point(namespace_scope)
}  // namespace protobuf
}  // namespace elect
}  // namespace top
namespace google {
namespace protobuf {
template<> GOOGLE_PROTOBUF_ATTRIBUTE_NOINLINE ::top::elect::protobuf::VhostMessage* Arena::CreateMaybeMessage< ::top::elect::protobuf::VhostMessage >(Arena* arena) {
  return Arena::CreateInternal< ::top::elect::protobuf::VhostMessage >(arena);
}
}  // namespace protobuf
}  // namespace google

// @@protoc_insertion_point(global_scope)
