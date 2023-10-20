// automatically generated by the FlatBuffers compiler, do not modify


#ifndef FLATBUFFERS_GENERATED_APIOBJECTS_RLOGIC_SERIALIZATION_H_
#define FLATBUFFERS_GENERATED_APIOBJECTS_RLOGIC_SERIALIZATION_H_

#include "flatbuffers/flatbuffers.h"

// Ensure the included flatbuffers.h is the same version as when this file was
// generated, otherwise it may not be compatible.
static_assert(FLATBUFFERS_VERSION_MAJOR == 23 &&
              FLATBUFFERS_VERSION_MINOR == 5 &&
              FLATBUFFERS_VERSION_REVISION == 9,
             "Non-compatible flatbuffers version included");

#include "AnchorPointGen.h"
#include "AnimationNodeGen.h"
#include "AppearanceBindingGen.h"
#include "CameraBindingGen.h"
#include "DataArrayGen.h"
#include "LinkGen.h"
#include "LuaInterfaceGen.h"
#include "LuaModuleGen.h"
#include "LuaScriptGen.h"
#include "MeshNodeBindingGen.h"
#include "NodeBindingGen.h"
#include "RenderGroupBindingGen.h"
#include "RenderPassBindingGen.h"
#include "SkinBindingGen.h"
#include "TimerNodeGen.h"

namespace rlogic_serialization {

struct ApiObjects;
struct ApiObjectsBuilder;

inline const ::flatbuffers::TypeTable *ApiObjectsTypeTable();

struct ApiObjects FLATBUFFERS_FINAL_CLASS : private ::flatbuffers::Table {
  typedef ApiObjectsBuilder Builder;
  struct Traits;
  static const ::flatbuffers::TypeTable *MiniReflectTypeTable() {
    return ApiObjectsTypeTable();
  }
  enum FlatBuffersVTableOffset FLATBUFFERS_VTABLE_UNDERLYING_TYPE {
    VT_LUAMODULES = 4,
    VT_LUASCRIPTS = 6,
    VT_LUAINTERFACES = 8,
    VT_NODEBINDINGS = 10,
    VT_APPEARANCEBINDINGS = 12,
    VT_CAMERABINDINGS = 14,
    VT_DATAARRAYS = 16,
    VT_ANIMATIONNODES = 18,
    VT_TIMERNODES = 20,
    VT_LINKS = 22,
    VT_RENDERPASSBINDINGS = 24,
    VT_ANCHORPOINTS = 26,
    VT_RENDERGROUPBINDINGS = 28,
    VT_SKINBINDINGS = 30,
    VT_MESHNODEBINDINGS = 32
  };
  const ::flatbuffers::Vector<::flatbuffers::Offset<rlogic_serialization::LuaModule>> *luaModules() const {
    return GetPointer<const ::flatbuffers::Vector<::flatbuffers::Offset<rlogic_serialization::LuaModule>> *>(VT_LUAMODULES);
  }
  const ::flatbuffers::Vector<::flatbuffers::Offset<rlogic_serialization::LuaScript>> *luaScripts() const {
    return GetPointer<const ::flatbuffers::Vector<::flatbuffers::Offset<rlogic_serialization::LuaScript>> *>(VT_LUASCRIPTS);
  }
  const ::flatbuffers::Vector<::flatbuffers::Offset<rlogic_serialization::LuaInterface>> *luaInterfaces() const {
    return GetPointer<const ::flatbuffers::Vector<::flatbuffers::Offset<rlogic_serialization::LuaInterface>> *>(VT_LUAINTERFACES);
  }
  const ::flatbuffers::Vector<::flatbuffers::Offset<rlogic_serialization::NodeBinding>> *nodeBindings() const {
    return GetPointer<const ::flatbuffers::Vector<::flatbuffers::Offset<rlogic_serialization::NodeBinding>> *>(VT_NODEBINDINGS);
  }
  const ::flatbuffers::Vector<::flatbuffers::Offset<rlogic_serialization::AppearanceBinding>> *appearanceBindings() const {
    return GetPointer<const ::flatbuffers::Vector<::flatbuffers::Offset<rlogic_serialization::AppearanceBinding>> *>(VT_APPEARANCEBINDINGS);
  }
  const ::flatbuffers::Vector<::flatbuffers::Offset<rlogic_serialization::CameraBinding>> *cameraBindings() const {
    return GetPointer<const ::flatbuffers::Vector<::flatbuffers::Offset<rlogic_serialization::CameraBinding>> *>(VT_CAMERABINDINGS);
  }
  const ::flatbuffers::Vector<::flatbuffers::Offset<rlogic_serialization::DataArray>> *dataArrays() const {
    return GetPointer<const ::flatbuffers::Vector<::flatbuffers::Offset<rlogic_serialization::DataArray>> *>(VT_DATAARRAYS);
  }
  const ::flatbuffers::Vector<::flatbuffers::Offset<rlogic_serialization::AnimationNode>> *animationNodes() const {
    return GetPointer<const ::flatbuffers::Vector<::flatbuffers::Offset<rlogic_serialization::AnimationNode>> *>(VT_ANIMATIONNODES);
  }
  const ::flatbuffers::Vector<::flatbuffers::Offset<rlogic_serialization::TimerNode>> *timerNodes() const {
    return GetPointer<const ::flatbuffers::Vector<::flatbuffers::Offset<rlogic_serialization::TimerNode>> *>(VT_TIMERNODES);
  }
  const ::flatbuffers::Vector<::flatbuffers::Offset<rlogic_serialization::Link>> *links() const {
    return GetPointer<const ::flatbuffers::Vector<::flatbuffers::Offset<rlogic_serialization::Link>> *>(VT_LINKS);
  }
  const ::flatbuffers::Vector<::flatbuffers::Offset<rlogic_serialization::RenderPassBinding>> *renderPassBindings() const {
    return GetPointer<const ::flatbuffers::Vector<::flatbuffers::Offset<rlogic_serialization::RenderPassBinding>> *>(VT_RENDERPASSBINDINGS);
  }
  const ::flatbuffers::Vector<::flatbuffers::Offset<rlogic_serialization::AnchorPoint>> *anchorPoints() const {
    return GetPointer<const ::flatbuffers::Vector<::flatbuffers::Offset<rlogic_serialization::AnchorPoint>> *>(VT_ANCHORPOINTS);
  }
  const ::flatbuffers::Vector<::flatbuffers::Offset<rlogic_serialization::RenderGroupBinding>> *renderGroupBindings() const {
    return GetPointer<const ::flatbuffers::Vector<::flatbuffers::Offset<rlogic_serialization::RenderGroupBinding>> *>(VT_RENDERGROUPBINDINGS);
  }
  const ::flatbuffers::Vector<::flatbuffers::Offset<rlogic_serialization::SkinBinding>> *skinBindings() const {
    return GetPointer<const ::flatbuffers::Vector<::flatbuffers::Offset<rlogic_serialization::SkinBinding>> *>(VT_SKINBINDINGS);
  }
  const ::flatbuffers::Vector<::flatbuffers::Offset<rlogic_serialization::MeshNodeBinding>> *meshNodeBindings() const {
    return GetPointer<const ::flatbuffers::Vector<::flatbuffers::Offset<rlogic_serialization::MeshNodeBinding>> *>(VT_MESHNODEBINDINGS);
  }
  bool Verify(::flatbuffers::Verifier &verifier) const {
    return VerifyTableStart(verifier) &&
           VerifyOffset(verifier, VT_LUAMODULES) &&
           verifier.VerifyVector(luaModules()) &&
           verifier.VerifyVectorOfTables(luaModules()) &&
           VerifyOffset(verifier, VT_LUASCRIPTS) &&
           verifier.VerifyVector(luaScripts()) &&
           verifier.VerifyVectorOfTables(luaScripts()) &&
           VerifyOffset(verifier, VT_LUAINTERFACES) &&
           verifier.VerifyVector(luaInterfaces()) &&
           verifier.VerifyVectorOfTables(luaInterfaces()) &&
           VerifyOffset(verifier, VT_NODEBINDINGS) &&
           verifier.VerifyVector(nodeBindings()) &&
           verifier.VerifyVectorOfTables(nodeBindings()) &&
           VerifyOffset(verifier, VT_APPEARANCEBINDINGS) &&
           verifier.VerifyVector(appearanceBindings()) &&
           verifier.VerifyVectorOfTables(appearanceBindings()) &&
           VerifyOffset(verifier, VT_CAMERABINDINGS) &&
           verifier.VerifyVector(cameraBindings()) &&
           verifier.VerifyVectorOfTables(cameraBindings()) &&
           VerifyOffset(verifier, VT_DATAARRAYS) &&
           verifier.VerifyVector(dataArrays()) &&
           verifier.VerifyVectorOfTables(dataArrays()) &&
           VerifyOffset(verifier, VT_ANIMATIONNODES) &&
           verifier.VerifyVector(animationNodes()) &&
           verifier.VerifyVectorOfTables(animationNodes()) &&
           VerifyOffset(verifier, VT_TIMERNODES) &&
           verifier.VerifyVector(timerNodes()) &&
           verifier.VerifyVectorOfTables(timerNodes()) &&
           VerifyOffset(verifier, VT_LINKS) &&
           verifier.VerifyVector(links()) &&
           verifier.VerifyVectorOfTables(links()) &&
           VerifyOffset(verifier, VT_RENDERPASSBINDINGS) &&
           verifier.VerifyVector(renderPassBindings()) &&
           verifier.VerifyVectorOfTables(renderPassBindings()) &&
           VerifyOffset(verifier, VT_ANCHORPOINTS) &&
           verifier.VerifyVector(anchorPoints()) &&
           verifier.VerifyVectorOfTables(anchorPoints()) &&
           VerifyOffset(verifier, VT_RENDERGROUPBINDINGS) &&
           verifier.VerifyVector(renderGroupBindings()) &&
           verifier.VerifyVectorOfTables(renderGroupBindings()) &&
           VerifyOffset(verifier, VT_SKINBINDINGS) &&
           verifier.VerifyVector(skinBindings()) &&
           verifier.VerifyVectorOfTables(skinBindings()) &&
           VerifyOffset(verifier, VT_MESHNODEBINDINGS) &&
           verifier.VerifyVector(meshNodeBindings()) &&
           verifier.VerifyVectorOfTables(meshNodeBindings()) &&
           verifier.EndTable();
  }
};

struct ApiObjectsBuilder {
  typedef ApiObjects Table;
  ::flatbuffers::FlatBufferBuilder &fbb_;
  ::flatbuffers::uoffset_t start_;
  void add_luaModules(::flatbuffers::Offset<::flatbuffers::Vector<::flatbuffers::Offset<rlogic_serialization::LuaModule>>> luaModules) {
    fbb_.AddOffset(ApiObjects::VT_LUAMODULES, luaModules);
  }
  void add_luaScripts(::flatbuffers::Offset<::flatbuffers::Vector<::flatbuffers::Offset<rlogic_serialization::LuaScript>>> luaScripts) {
    fbb_.AddOffset(ApiObjects::VT_LUASCRIPTS, luaScripts);
  }
  void add_luaInterfaces(::flatbuffers::Offset<::flatbuffers::Vector<::flatbuffers::Offset<rlogic_serialization::LuaInterface>>> luaInterfaces) {
    fbb_.AddOffset(ApiObjects::VT_LUAINTERFACES, luaInterfaces);
  }
  void add_nodeBindings(::flatbuffers::Offset<::flatbuffers::Vector<::flatbuffers::Offset<rlogic_serialization::NodeBinding>>> nodeBindings) {
    fbb_.AddOffset(ApiObjects::VT_NODEBINDINGS, nodeBindings);
  }
  void add_appearanceBindings(::flatbuffers::Offset<::flatbuffers::Vector<::flatbuffers::Offset<rlogic_serialization::AppearanceBinding>>> appearanceBindings) {
    fbb_.AddOffset(ApiObjects::VT_APPEARANCEBINDINGS, appearanceBindings);
  }
  void add_cameraBindings(::flatbuffers::Offset<::flatbuffers::Vector<::flatbuffers::Offset<rlogic_serialization::CameraBinding>>> cameraBindings) {
    fbb_.AddOffset(ApiObjects::VT_CAMERABINDINGS, cameraBindings);
  }
  void add_dataArrays(::flatbuffers::Offset<::flatbuffers::Vector<::flatbuffers::Offset<rlogic_serialization::DataArray>>> dataArrays) {
    fbb_.AddOffset(ApiObjects::VT_DATAARRAYS, dataArrays);
  }
  void add_animationNodes(::flatbuffers::Offset<::flatbuffers::Vector<::flatbuffers::Offset<rlogic_serialization::AnimationNode>>> animationNodes) {
    fbb_.AddOffset(ApiObjects::VT_ANIMATIONNODES, animationNodes);
  }
  void add_timerNodes(::flatbuffers::Offset<::flatbuffers::Vector<::flatbuffers::Offset<rlogic_serialization::TimerNode>>> timerNodes) {
    fbb_.AddOffset(ApiObjects::VT_TIMERNODES, timerNodes);
  }
  void add_links(::flatbuffers::Offset<::flatbuffers::Vector<::flatbuffers::Offset<rlogic_serialization::Link>>> links) {
    fbb_.AddOffset(ApiObjects::VT_LINKS, links);
  }
  void add_renderPassBindings(::flatbuffers::Offset<::flatbuffers::Vector<::flatbuffers::Offset<rlogic_serialization::RenderPassBinding>>> renderPassBindings) {
    fbb_.AddOffset(ApiObjects::VT_RENDERPASSBINDINGS, renderPassBindings);
  }
  void add_anchorPoints(::flatbuffers::Offset<::flatbuffers::Vector<::flatbuffers::Offset<rlogic_serialization::AnchorPoint>>> anchorPoints) {
    fbb_.AddOffset(ApiObjects::VT_ANCHORPOINTS, anchorPoints);
  }
  void add_renderGroupBindings(::flatbuffers::Offset<::flatbuffers::Vector<::flatbuffers::Offset<rlogic_serialization::RenderGroupBinding>>> renderGroupBindings) {
    fbb_.AddOffset(ApiObjects::VT_RENDERGROUPBINDINGS, renderGroupBindings);
  }
  void add_skinBindings(::flatbuffers::Offset<::flatbuffers::Vector<::flatbuffers::Offset<rlogic_serialization::SkinBinding>>> skinBindings) {
    fbb_.AddOffset(ApiObjects::VT_SKINBINDINGS, skinBindings);
  }
  void add_meshNodeBindings(::flatbuffers::Offset<::flatbuffers::Vector<::flatbuffers::Offset<rlogic_serialization::MeshNodeBinding>>> meshNodeBindings) {
    fbb_.AddOffset(ApiObjects::VT_MESHNODEBINDINGS, meshNodeBindings);
  }
  explicit ApiObjectsBuilder(::flatbuffers::FlatBufferBuilder &_fbb)
        : fbb_(_fbb) {
    start_ = fbb_.StartTable();
  }
  ::flatbuffers::Offset<ApiObjects> Finish() {
    const auto end = fbb_.EndTable(start_);
    auto o = ::flatbuffers::Offset<ApiObjects>(end);
    return o;
  }
};

inline ::flatbuffers::Offset<ApiObjects> CreateApiObjects(
    ::flatbuffers::FlatBufferBuilder &_fbb,
    ::flatbuffers::Offset<::flatbuffers::Vector<::flatbuffers::Offset<rlogic_serialization::LuaModule>>> luaModules = 0,
    ::flatbuffers::Offset<::flatbuffers::Vector<::flatbuffers::Offset<rlogic_serialization::LuaScript>>> luaScripts = 0,
    ::flatbuffers::Offset<::flatbuffers::Vector<::flatbuffers::Offset<rlogic_serialization::LuaInterface>>> luaInterfaces = 0,
    ::flatbuffers::Offset<::flatbuffers::Vector<::flatbuffers::Offset<rlogic_serialization::NodeBinding>>> nodeBindings = 0,
    ::flatbuffers::Offset<::flatbuffers::Vector<::flatbuffers::Offset<rlogic_serialization::AppearanceBinding>>> appearanceBindings = 0,
    ::flatbuffers::Offset<::flatbuffers::Vector<::flatbuffers::Offset<rlogic_serialization::CameraBinding>>> cameraBindings = 0,
    ::flatbuffers::Offset<::flatbuffers::Vector<::flatbuffers::Offset<rlogic_serialization::DataArray>>> dataArrays = 0,
    ::flatbuffers::Offset<::flatbuffers::Vector<::flatbuffers::Offset<rlogic_serialization::AnimationNode>>> animationNodes = 0,
    ::flatbuffers::Offset<::flatbuffers::Vector<::flatbuffers::Offset<rlogic_serialization::TimerNode>>> timerNodes = 0,
    ::flatbuffers::Offset<::flatbuffers::Vector<::flatbuffers::Offset<rlogic_serialization::Link>>> links = 0,
    ::flatbuffers::Offset<::flatbuffers::Vector<::flatbuffers::Offset<rlogic_serialization::RenderPassBinding>>> renderPassBindings = 0,
    ::flatbuffers::Offset<::flatbuffers::Vector<::flatbuffers::Offset<rlogic_serialization::AnchorPoint>>> anchorPoints = 0,
    ::flatbuffers::Offset<::flatbuffers::Vector<::flatbuffers::Offset<rlogic_serialization::RenderGroupBinding>>> renderGroupBindings = 0,
    ::flatbuffers::Offset<::flatbuffers::Vector<::flatbuffers::Offset<rlogic_serialization::SkinBinding>>> skinBindings = 0,
    ::flatbuffers::Offset<::flatbuffers::Vector<::flatbuffers::Offset<rlogic_serialization::MeshNodeBinding>>> meshNodeBindings = 0) {
  ApiObjectsBuilder builder_(_fbb);
  builder_.add_meshNodeBindings(meshNodeBindings);
  builder_.add_skinBindings(skinBindings);
  builder_.add_renderGroupBindings(renderGroupBindings);
  builder_.add_anchorPoints(anchorPoints);
  builder_.add_renderPassBindings(renderPassBindings);
  builder_.add_links(links);
  builder_.add_timerNodes(timerNodes);
  builder_.add_animationNodes(animationNodes);
  builder_.add_dataArrays(dataArrays);
  builder_.add_cameraBindings(cameraBindings);
  builder_.add_appearanceBindings(appearanceBindings);
  builder_.add_nodeBindings(nodeBindings);
  builder_.add_luaInterfaces(luaInterfaces);
  builder_.add_luaScripts(luaScripts);
  builder_.add_luaModules(luaModules);
  return builder_.Finish();
}

struct ApiObjects::Traits {
  using type = ApiObjects;
  static auto constexpr Create = CreateApiObjects;
};

inline ::flatbuffers::Offset<ApiObjects> CreateApiObjectsDirect(
    ::flatbuffers::FlatBufferBuilder &_fbb,
    const std::vector<::flatbuffers::Offset<rlogic_serialization::LuaModule>> *luaModules = nullptr,
    const std::vector<::flatbuffers::Offset<rlogic_serialization::LuaScript>> *luaScripts = nullptr,
    const std::vector<::flatbuffers::Offset<rlogic_serialization::LuaInterface>> *luaInterfaces = nullptr,
    const std::vector<::flatbuffers::Offset<rlogic_serialization::NodeBinding>> *nodeBindings = nullptr,
    const std::vector<::flatbuffers::Offset<rlogic_serialization::AppearanceBinding>> *appearanceBindings = nullptr,
    const std::vector<::flatbuffers::Offset<rlogic_serialization::CameraBinding>> *cameraBindings = nullptr,
    const std::vector<::flatbuffers::Offset<rlogic_serialization::DataArray>> *dataArrays = nullptr,
    const std::vector<::flatbuffers::Offset<rlogic_serialization::AnimationNode>> *animationNodes = nullptr,
    const std::vector<::flatbuffers::Offset<rlogic_serialization::TimerNode>> *timerNodes = nullptr,
    const std::vector<::flatbuffers::Offset<rlogic_serialization::Link>> *links = nullptr,
    const std::vector<::flatbuffers::Offset<rlogic_serialization::RenderPassBinding>> *renderPassBindings = nullptr,
    const std::vector<::flatbuffers::Offset<rlogic_serialization::AnchorPoint>> *anchorPoints = nullptr,
    const std::vector<::flatbuffers::Offset<rlogic_serialization::RenderGroupBinding>> *renderGroupBindings = nullptr,
    const std::vector<::flatbuffers::Offset<rlogic_serialization::SkinBinding>> *skinBindings = nullptr,
    const std::vector<::flatbuffers::Offset<rlogic_serialization::MeshNodeBinding>> *meshNodeBindings = nullptr) {
  auto luaModules__ = luaModules ? _fbb.CreateVector<::flatbuffers::Offset<rlogic_serialization::LuaModule>>(*luaModules) : 0;
  auto luaScripts__ = luaScripts ? _fbb.CreateVector<::flatbuffers::Offset<rlogic_serialization::LuaScript>>(*luaScripts) : 0;
  auto luaInterfaces__ = luaInterfaces ? _fbb.CreateVector<::flatbuffers::Offset<rlogic_serialization::LuaInterface>>(*luaInterfaces) : 0;
  auto nodeBindings__ = nodeBindings ? _fbb.CreateVector<::flatbuffers::Offset<rlogic_serialization::NodeBinding>>(*nodeBindings) : 0;
  auto appearanceBindings__ = appearanceBindings ? _fbb.CreateVector<::flatbuffers::Offset<rlogic_serialization::AppearanceBinding>>(*appearanceBindings) : 0;
  auto cameraBindings__ = cameraBindings ? _fbb.CreateVector<::flatbuffers::Offset<rlogic_serialization::CameraBinding>>(*cameraBindings) : 0;
  auto dataArrays__ = dataArrays ? _fbb.CreateVector<::flatbuffers::Offset<rlogic_serialization::DataArray>>(*dataArrays) : 0;
  auto animationNodes__ = animationNodes ? _fbb.CreateVector<::flatbuffers::Offset<rlogic_serialization::AnimationNode>>(*animationNodes) : 0;
  auto timerNodes__ = timerNodes ? _fbb.CreateVector<::flatbuffers::Offset<rlogic_serialization::TimerNode>>(*timerNodes) : 0;
  auto links__ = links ? _fbb.CreateVector<::flatbuffers::Offset<rlogic_serialization::Link>>(*links) : 0;
  auto renderPassBindings__ = renderPassBindings ? _fbb.CreateVector<::flatbuffers::Offset<rlogic_serialization::RenderPassBinding>>(*renderPassBindings) : 0;
  auto anchorPoints__ = anchorPoints ? _fbb.CreateVector<::flatbuffers::Offset<rlogic_serialization::AnchorPoint>>(*anchorPoints) : 0;
  auto renderGroupBindings__ = renderGroupBindings ? _fbb.CreateVector<::flatbuffers::Offset<rlogic_serialization::RenderGroupBinding>>(*renderGroupBindings) : 0;
  auto skinBindings__ = skinBindings ? _fbb.CreateVector<::flatbuffers::Offset<rlogic_serialization::SkinBinding>>(*skinBindings) : 0;
  auto meshNodeBindings__ = meshNodeBindings ? _fbb.CreateVector<::flatbuffers::Offset<rlogic_serialization::MeshNodeBinding>>(*meshNodeBindings) : 0;
  return rlogic_serialization::CreateApiObjects(
      _fbb,
      luaModules__,
      luaScripts__,
      luaInterfaces__,
      nodeBindings__,
      appearanceBindings__,
      cameraBindings__,
      dataArrays__,
      animationNodes__,
      timerNodes__,
      links__,
      renderPassBindings__,
      anchorPoints__,
      renderGroupBindings__,
      skinBindings__,
      meshNodeBindings__);
}

inline const ::flatbuffers::TypeTable *ApiObjectsTypeTable() {
  static const ::flatbuffers::TypeCode type_codes[] = {
    { ::flatbuffers::ET_SEQUENCE, 1, 0 },
    { ::flatbuffers::ET_SEQUENCE, 1, 1 },
    { ::flatbuffers::ET_SEQUENCE, 1, 2 },
    { ::flatbuffers::ET_SEQUENCE, 1, 3 },
    { ::flatbuffers::ET_SEQUENCE, 1, 4 },
    { ::flatbuffers::ET_SEQUENCE, 1, 5 },
    { ::flatbuffers::ET_SEQUENCE, 1, 6 },
    { ::flatbuffers::ET_SEQUENCE, 1, 7 },
    { ::flatbuffers::ET_SEQUENCE, 1, 8 },
    { ::flatbuffers::ET_SEQUENCE, 1, 9 },
    { ::flatbuffers::ET_SEQUENCE, 1, 10 },
    { ::flatbuffers::ET_SEQUENCE, 1, 11 },
    { ::flatbuffers::ET_SEQUENCE, 1, 12 },
    { ::flatbuffers::ET_SEQUENCE, 1, 13 },
    { ::flatbuffers::ET_SEQUENCE, 1, 14 }
  };
  static const ::flatbuffers::TypeFunction type_refs[] = {
    rlogic_serialization::LuaModuleTypeTable,
    rlogic_serialization::LuaScriptTypeTable,
    rlogic_serialization::LuaInterfaceTypeTable,
    rlogic_serialization::NodeBindingTypeTable,
    rlogic_serialization::AppearanceBindingTypeTable,
    rlogic_serialization::CameraBindingTypeTable,
    rlogic_serialization::DataArrayTypeTable,
    rlogic_serialization::AnimationNodeTypeTable,
    rlogic_serialization::TimerNodeTypeTable,
    rlogic_serialization::LinkTypeTable,
    rlogic_serialization::RenderPassBindingTypeTable,
    rlogic_serialization::AnchorPointTypeTable,
    rlogic_serialization::RenderGroupBindingTypeTable,
    rlogic_serialization::SkinBindingTypeTable,
    rlogic_serialization::MeshNodeBindingTypeTable
  };
  static const char * const names[] = {
    "luaModules",
    "luaScripts",
    "luaInterfaces",
    "nodeBindings",
    "appearanceBindings",
    "cameraBindings",
    "dataArrays",
    "animationNodes",
    "timerNodes",
    "links",
    "renderPassBindings",
    "anchorPoints",
    "renderGroupBindings",
    "skinBindings",
    "meshNodeBindings"
  };
  static const ::flatbuffers::TypeTable tt = {
    ::flatbuffers::ST_TABLE, 15, type_codes, type_refs, nullptr, nullptr, names
  };
  return &tt;
}

}  // namespace rlogic_serialization

#endif  // FLATBUFFERS_GENERATED_APIOBJECTS_RLOGIC_SERIALIZATION_H_
