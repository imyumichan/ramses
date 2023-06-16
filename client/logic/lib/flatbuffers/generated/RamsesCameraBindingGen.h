// automatically generated by the FlatBuffers compiler, do not modify


#ifndef FLATBUFFERS_GENERATED_RAMSESCAMERABINDING_RLOGIC_SERIALIZATION_H_
#define FLATBUFFERS_GENERATED_RAMSESCAMERABINDING_RLOGIC_SERIALIZATION_H_

#include "flatbuffers/flatbuffers.h"

#include "LogicObjectGen.h"
#include "PropertyGen.h"
#include "RamsesBindingGen.h"
#include "RamsesReferenceGen.h"

namespace rlogic_serialization {

struct RamsesCameraBinding;
struct RamsesCameraBindingBuilder;

inline const flatbuffers::TypeTable *RamsesCameraBindingTypeTable();

struct RamsesCameraBinding FLATBUFFERS_FINAL_CLASS : private flatbuffers::Table {
  typedef RamsesCameraBindingBuilder Builder;
  struct Traits;
  static const flatbuffers::TypeTable *MiniReflectTypeTable() {
    return RamsesCameraBindingTypeTable();
  }
  enum FlatBuffersVTableOffset FLATBUFFERS_VTABLE_UNDERLYING_TYPE {
    VT_BASE = 4
  };
  const rlogic_serialization::RamsesBinding *base() const {
    return GetPointer<const rlogic_serialization::RamsesBinding *>(VT_BASE);
  }
  bool Verify(flatbuffers::Verifier &verifier) const {
    return VerifyTableStart(verifier) &&
           VerifyOffset(verifier, VT_BASE) &&
           verifier.VerifyTable(base()) &&
           verifier.EndTable();
  }
};

struct RamsesCameraBindingBuilder {
  typedef RamsesCameraBinding Table;
  flatbuffers::FlatBufferBuilder &fbb_;
  flatbuffers::uoffset_t start_;
  void add_base(flatbuffers::Offset<rlogic_serialization::RamsesBinding> base) {
    fbb_.AddOffset(RamsesCameraBinding::VT_BASE, base);
  }
  explicit RamsesCameraBindingBuilder(flatbuffers::FlatBufferBuilder &_fbb)
        : fbb_(_fbb) {
    start_ = fbb_.StartTable();
  }
  RamsesCameraBindingBuilder &operator=(const RamsesCameraBindingBuilder &);
  flatbuffers::Offset<RamsesCameraBinding> Finish() {
    const auto end = fbb_.EndTable(start_);
    auto o = flatbuffers::Offset<RamsesCameraBinding>(end);
    return o;
  }
};

inline flatbuffers::Offset<RamsesCameraBinding> CreateRamsesCameraBinding(
    flatbuffers::FlatBufferBuilder &_fbb,
    flatbuffers::Offset<rlogic_serialization::RamsesBinding> base = 0) {
  RamsesCameraBindingBuilder builder_(_fbb);
  builder_.add_base(base);
  return builder_.Finish();
}

struct RamsesCameraBinding::Traits {
  using type = RamsesCameraBinding;
  static auto constexpr Create = CreateRamsesCameraBinding;
};

inline const flatbuffers::TypeTable *RamsesCameraBindingTypeTable() {
  static const flatbuffers::TypeCode type_codes[] = {
    { flatbuffers::ET_SEQUENCE, 0, 0 }
  };
  static const flatbuffers::TypeFunction type_refs[] = {
    rlogic_serialization::RamsesBindingTypeTable
  };
  static const char * const names[] = {
    "base"
  };
  static const flatbuffers::TypeTable tt = {
    flatbuffers::ST_TABLE, 1, type_codes, type_refs, nullptr, names
  };
  return &tt;
}

}  // namespace rlogic_serialization

#endif  // FLATBUFFERS_GENERATED_RAMSESCAMERABINDING_RLOGIC_SERIALIZATION_H_