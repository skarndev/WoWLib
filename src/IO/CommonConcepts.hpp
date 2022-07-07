#pragma once
#include <IO/ByteBuffer.hpp>
#include <type_traits>
#include <vector>

/* These concepts are created in order to ensure in compiletime
   the consistency of chunk-based interfaces. Make sure when adding a chunk
   to assert if it satisfies at least minimal, or common requirements.
   Make sure to coceptualize template arguments for generic code accepting chunks.
 */
namespace IO::Common::Concepts
{
  template<typename T, std::uint32_t fourcc_req>
  struct CheckFourCCEqual
  {
    static_assert(fourcc_req ? T::magic == fourcc_req : true);
  };

  template<typename T, std::uint32_t fourcc_req = 0>
  concept ChunkProtocolCommon = requires (T t)
  {
    { static_cast<void(T::*)()>(&T::Initialize) };
    { t.Read(Common::ByteBuffer(), std::size_t())} -> std::same_as<void>;
    // { static_cast<void(T::*)(Common::ByteBuffer const&, std::size_t)>(&T::Read) };
    { static_cast<void(T::*)(Common::ByteBuffer&) const>(&T::Write) };
    { static_cast<std::size_t(T::*)() const>(&T::ByteSize) };
    { static_cast<bool(T::*)() const>(&T::IsInitialized) };
    { CheckFourCCEqual<T, fourcc_req>() };
  };

  template<typename T, std::uint32_t fourcc_req = 0>
  concept DataChunkProtocol = ChunkProtocolCommon<T, fourcc_req>
    && std::is_convertible_v<T, typename T::ValueType>
    && requires (T t) { { static_cast<void(T::*)(typename T::InterfaceType)>(&T::Initialize)}; };

  template<typename T, std::uint32_t fourcc_req = 0>
  concept DataArrayChunkProtocol = ChunkProtocolCommon<T, fourcc_req>
    && requires (T t)
    {
      { static_cast<void(T::*)(typename T::ValueType const&, std::size_t)>(&T::Initialize)};
      { static_cast<void(T::*)(typename T::ArrayImplT const&)>(&T::Initialize)};
      { static_cast<std::size_t(T::*)() const>(&T::Size)};
      { static_cast<typename T::ArrayImplT::const_iterator(T::*)() const>(&T::begin)};
      { static_cast<typename T::ArrayImplT::const_iterator(T::*)() const>(&T::end)};
      { static_cast<typename T::ArrayImplT::const_iterator(T::*)() const>(&T::cbegin)};
      { static_cast<typename T::ArrayImplT::const_iterator(T::*)() const>(&T::cend)};
      { static_cast<typename T::ArrayImplT::iterator(T::*)()>(&T::begin)};
      { static_cast<typename T::ArrayImplT::iterator(T::*)()>(&T::end)};
      { static_cast<typename T::ValueType&(T::*)(std::size_t)>(&T::operator[])};
      { static_cast<typename T::ValueType const&(T::*)(std::size_t) const>(&T::operator[])};
      { static_cast<typename T::ValueType&(T::*)(std::size_t)>(&T::At)};
      { static_cast<typename T::ValueType const&(T::*)(std::size_t) const>(&T::At)};
    }
    // dynamic array-specific interface
    && (!std::is_same_v<typename T::ArrayImplT, std::vector<typename T::ValueType>>
    || requires(T t )
    {
      { static_cast<typename T::ValueType&(T::*)()>(&T::Add) };
      { static_cast<void(T::*)(std::size_t)>(&T::Remove) };
      { static_cast<void(T::*)(typename T::ArrayImplT::iterator)>(&T::Remove) };
      { static_cast<void(T::*)()>(&T::Clear) };
    });

  template<typename T, std::uint32_t fourcc_req = 0>
  concept StringBlockChunkProtocol = ChunkProtocolCommon<T, fourcc_req>
   && requires (T t)
   {
     { t.Initialize(std::vector<std::string>{}) } -> std::same_as<void>;
     // { static_cast<void(T::*)(std::vector<std::string> const&)>(&T::Initialize)};
     { static_cast<std::size_t(T::*)() const>(&T::Size)};
     { static_cast<void(T::*)(std::string const&)>(&T::Add)};
     { static_cast<void(T::*)(std::size_t)>(&T::Remove)};
     { static_cast<void(T::*)(typename T::ArrayImplT::iterator)>(&T::Remove)};
     { static_cast<void(T::*)(typename T::ArrayImplT::const_iterator)>(&T::Remove)};
     { static_cast<void(T::*)()>(&T::Clear)};
     { static_cast<typename T::ArrayImplT::value_type const&(T::*)(std::size_t) const>(&T::At)};
     { static_cast<typename T::ArrayImplT::value_type&(T::*)(std::size_t)>(&T::At)};
     { static_cast<typename T::ArrayImplT::value_type&(T::*)(std::size_t)>(&T::operator[])};
     { static_cast<typename T::ArrayImplT::value_type const&(T::*)(std::size_t) const>(&T::operator[])};
     { static_cast<typename T::ArrayImplT::const_iterator(T::*)() const>(&T::begin)};
     { static_cast<typename T::ArrayImplT::const_iterator(T::*)() const>(&T::end)};
     { static_cast<typename T::ArrayImplT::const_iterator(T::*)() const>(&T::cbegin)};
     { static_cast<typename T::ArrayImplT::const_iterator(T::*)() const>(&T::cend)};
     { static_cast<typename T::ArrayImplT::iterator(T::*)()>(&T::begin)};
     { static_cast<typename T::ArrayImplT::iterator(T::*)()>(&T::end)};
   };
}