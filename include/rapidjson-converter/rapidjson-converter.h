#ifndef RAPIDJSON_converter_H_
#define RAPIDJSON_converter_H_

#include <vector>
#include <functional>
#include <type_traits>
#include <memory>

#include "rapidjson/rapidjson.h"
#include "rapidjson/document.h"
#include "rapidjson/writer.h"

RAPIDJSON_NAMESPACE_BEGIN

namespace converter {

template< typename StructType >
class RapidJsonConverter;

template< typename FieldType >
using FromJsonValueConverter = std::function<bool(const rapidjson::Value&, FieldType*)>;

template< typename StructType >
using FromJsonConverter = std::function<bool(const rapidjson::Value&, StructType&)>;

template< typename FieldType >
using ToJsonValueConverter = std::function<void(rapidjson::Writer<rapidjson::StringBuffer>&, const FieldType*)>;

template< typename StructType >
using ToJsonConverter = std::function<void(rapidjson::Writer<rapidjson::StringBuffer>&, const StructType&)>;

namespace detail {

template< typename FieldType >
bool fromJsonValueConverter(const rapidjson::Value& v, FieldType* f) {
  if (v.IsObject()) {
    return RapidJsonConverter<FieldType>::GetSingleton().FromJsonValue(v, *f);
  }
  return false;
}

template< typename FieldType >
void toJsonValueConverter(rapidjson::Writer<rapidjson::StringBuffer>& w, const FieldType* f) {
  RapidJsonConverter<FieldType>::GetSingleton().ToJsonValue(w, *f);
}

template<>
bool fromJsonValueConverter<int>(const rapidjson::Value& v, int* f) {
  if (v.IsInt()) {
    *f = v.GetInt();
    return true;
  }
  return false;
}

template<>
void toJsonValueConverter<int>(rapidjson::Writer<rapidjson::StringBuffer>& w, const int* f) {
  w.Int(*f);
}

template<>
bool fromJsonValueConverter<bool>(const rapidjson::Value& v, bool* f) {
  if (v.IsBool()) {
    *f = v.GetBool();
    return true;
  }
  return false;
}

template<>
void toJsonValueConverter<bool>(rapidjson::Writer<rapidjson::StringBuffer>& w, const bool* f) {
  w.Bool(*f);
}

template<>
bool fromJsonValueConverter<double>(const rapidjson::Value& v, double* f) {
  if (v.IsDouble()) {
    *f = v.GetDouble();
    return true;
  }
  return false;
}

template<>
void toJsonValueConverter<double>(rapidjson::Writer<rapidjson::StringBuffer>& w, const double* f) {
  w.Double(*f);
}

template<>
bool fromJsonValueConverter<int64_t>(const rapidjson::Value& v, int64_t* f) {
  if (v.IsInt64()) {
    *f = v.GetInt64();
    return true;
  }
  return false;
}

template<>
void toJsonValueConverter<int64_t>(rapidjson::Writer<rapidjson::StringBuffer>& w, const int64_t* f) {
  w.Int64(*f);
}

template<>
bool fromJsonValueConverter<unsigned int>(const rapidjson::Value& v, unsigned int* f) {
  if (v.IsInt64()) {
    *f = v.GetInt64();
    return true;
  }
  return false;
}

template<>
void toJsonValueConverter<unsigned int>(rapidjson::Writer<rapidjson::StringBuffer>& w, const unsigned int* f) {
  w.Uint(*f);
}

template<>
bool fromJsonValueConverter<uint64_t>(const rapidjson::Value& v, uint64_t* f) {
  if (v.IsUint64()) {
    *f = v.GetUint64();
    return true;
  }
  return false;
}

template<>
void toJsonValueConverter<uint64_t>(rapidjson::Writer<rapidjson::StringBuffer>& w, const uint64_t* f) {
  w.Uint64(*f);
}

template<>
bool fromJsonValueConverter<std::string>(const rapidjson::Value& v, std::string* f) {
  if (v.IsString()) {
    (*f).assign(v.GetString(), v.GetStringLength());
    return true;
  }
  return false;
}

template<>
void toJsonValueConverter<std::string>(rapidjson::Writer<rapidjson::StringBuffer>& w, const std::string* f) {
  w.String(f->data(), f->size());
}

template< typename StructType, typename FieldType >
bool fromJsonFieldConverter(const rapidjson::Value& v, StructType& s, const std::string& field_name, FieldType StructType::* field) {
  const char* field_name_ch = field_name.data();
  if (v.IsObject()) {
    if (v.HasMember(field_name_ch)) {
      return fromJsonValueConverter<FieldType>(v[field_name_ch], &(s.*field));
    }
    return true;
  }
  return false;
}

template< typename StructType, typename FieldType >
void toJsonFieldConverter(rapidjson::Writer<rapidjson::StringBuffer>& w, const StructType& s, const std::string& field_name, FieldType StructType::* field) {
  w.Key(field_name.data());
  toJsonValueConverter<FieldType>(w, &(s.*field));
}

template< typename StructType, typename EleType >
bool fromJsonRepeatedCustomFieldConverter(
  const rapidjson::Value& v, StructType& s, const std::string& field_name, std::vector<std::unique_ptr<EleType>> StructType::* field,
  const FromJsonValueConverter<EleType>& conv) {
  const char* field_name_ch = field_name.data();
  if (v.IsObject()) {
    if (!v.HasMember(field_name_ch)) {
      return true;
    }
    const rapidjson::Value& __v = v[field_name_ch];
    if (__v.IsArray()) {
      auto __field = &(s.*field);
      auto array = __v.GetArray();
      __field->reserve(array.Size());
      for (const rapidjson::Value& ele : array) {
        std::unique_ptr<EleType> ptr(new EleType());
        if (!conv(ele, ptr.get())) {
          return false;
        }
        __field->push_back(std::move(ptr));
      }
      return true;
    }
  }
  return false;
}

template< typename StructType, typename EleType >
void toJsonRepeatedCustomFieldConverter(
  rapidjson::Writer<rapidjson::StringBuffer>& w, const StructType& s, 
  const std::string& field_name, std::vector<std::unique_ptr<EleType>> StructType::* field,
  const ToJsonValueConverter<EleType>& conv) {
  w.Key(field_name.data());
  w.StartArray();
  auto& vec = s.*field;
  for (auto& v : vec) {
    conv(w, v.get());
  }
  w.EndArray();
}

template< typename StructType, typename EleType >
bool fromJsonRepeatedFieldConverter(
  const rapidjson::Value& v, StructType& s, const std::string& field_name, std::vector<std::unique_ptr<EleType>> StructType::* field) {
  return fromJsonRepeatedCustomFieldConverter<StructType, EleType>(v, s, field_name, field, fromJsonValueConverter<EleType>);
}

template< typename StructType, typename EleType >
void toJsonRepeatedFieldConverter(
  rapidjson::Writer<rapidjson::StringBuffer>& w, const StructType& s, 
  const std::string& field_name, std::vector<std::unique_ptr<EleType>> StructType::* field) {
  toJsonRepeatedCustomFieldConverter<StructType, EleType>(w, s, field_name, field, toJsonValueConverter<EleType>);
}

template< typename StructType, typename FieldType >
bool fromJsonCustomFieldConverter(
  const rapidjson::Value& v, StructType& s, const std::string& field_name, FieldType StructType::* field, 
  const FromJsonValueConverter<FieldType>& conv) {
  const char* field_name_ch = field_name.data();
  if (v.IsObject()) {
    if (v.HasMember(field_name_ch)) {
      return conv(v[field_name_ch], &(s.*field));
    }
    return true;
  }
  return false;
}

template< typename StructType, typename FieldType >
void toJsonCustomFieldConverter(
  rapidjson::Writer<rapidjson::StringBuffer>& w, const StructType& s, const std::string& field_name, FieldType StructType::* field, 
  const ToJsonValueConverter<FieldType>& conv) {
  w.Key(field_name.data());
  conv(w, &(s.*field));
}

} // namespace detail

template< typename StructType >
class RapidJsonConverter {
public:

  ~RapidJsonConverter() = default;

  static 
  const RapidJsonConverter& GetSingleton() {
    static RapidJsonConverter conv;
    return conv;
  }

  void RegisterIntField(const std::string& field_name, int StructType::* field) {
    from_json_converters_.push_back(
      std::bind(detail::fromJsonFieldConverter<StructType, int>, std::placeholders::_1, std::placeholders::_2, /*copy*/ field_name, field));
    to_json_converters_.push_back(
      std::bind(detail::toJsonFieldConverter<StructType, int>, std::placeholders::_1, std::placeholders::_2, /*copy*/ field_name, field));
  }

  void RegisterBoolField(const std::string& field_name, bool StructType::* field) {
    from_json_converters_.push_back(
      std::bind(detail::fromJsonFieldConverter<StructType, bool>, std::placeholders::_1, std::placeholders::_2, /*copy*/ field_name, field));
    to_json_converters_.push_back(
      std::bind(detail::toJsonFieldConverter<StructType, bool>, std::placeholders::_1, std::placeholders::_2, /*copy*/ field_name, field));
  }

  void RegisterDoubleField(const std::string& field_name, double StructType::* field) {
    from_json_converters_.push_back(
      std::bind(detail::fromJsonFieldConverter<StructType, double>, std::placeholders::_1, std::placeholders::_2, /*copy*/ field_name, field));
    to_json_converters_.push_back(
      std::bind(detail::toJsonFieldConverter<StructType, double>, std::placeholders::_1, std::placeholders::_2, /*copy*/ field_name, field));
  }

  void RegisterInt64Field(const std::string& field_name, int64_t StructType::* field) {
    from_json_converters_.push_back(
      std::bind(detail::fromJsonFieldConverter<StructType, int64_t>, std::placeholders::_1, std::placeholders::_2, /*copy*/ field_name, field));
    to_json_converters_.push_back(
      std::bind(detail::toJsonFieldConverter<StructType, int64_t>, std::placeholders::_1, std::placeholders::_2, /*copy*/ field_name, field));
  }

  void RegisterUintField(const std::string& field_name, unsigned int StructType::* field) {
    from_json_converters_.push_back(
      std::bind(detail::fromJsonFieldConverter<StructType, unsigned int>, std::placeholders::_1, std::placeholders::_2, /*copy*/ field_name, field));
    to_json_converters_.push_back(
      std::bind(detail::toJsonFieldConverter<StructType, unsigned int>, std::placeholders::_1, std::placeholders::_2, /*copy*/ field_name, field));
  }

  void RegisterUint64Field(const std::string& field_name, uint64_t StructType::* field) {
    from_json_converters_.push_back(
      std::bind(detail::fromJsonFieldConverter<StructType, uint64_t>, std::placeholders::_1, std::placeholders::_2, /*copy*/ field_name, field));
    to_json_converters_.push_back(
      std::bind(detail::toJsonFieldConverter<StructType, uint64_t>, std::placeholders::_1, std::placeholders::_2, /*copy*/ field_name, field));
  }

  void RegisterStringField(const std::string& field_name, std::string StructType::* field) {
    from_json_converters_.push_back(
      std::bind(detail::fromJsonFieldConverter<StructType, std::string>, std::placeholders::_1, std::placeholders::_2, /*copy*/ field_name, field));
    to_json_converters_.push_back(
      std::bind(detail::toJsonFieldConverter<StructType, std::string>, std::placeholders::_1, std::placeholders::_2, /*copy*/ field_name, field));
  }

  template< typename NestedType >
  void RegisterNestedField(
    const std::string& field_name, NestedType StructType::* field) {
    from_json_converters_.push_back(
      std::bind(detail::fromJsonFieldConverter<StructType, NestedType>, std::placeholders::_1, std::placeholders::_2, /*copy*/ field_name, field));
    to_json_converters_.push_back(
      std::bind(detail::toJsonFieldConverter<StructType, NestedType>, std::placeholders::_1, std::placeholders::_2, /*copy*/ field_name, field));
  }

  template< typename EleType >
  void RegisterRepeatedField(
    const std::string& field_name, std::vector<std::unique_ptr<EleType>> StructType::* field) {
    from_json_converters_.push_back(
      std::bind(detail::fromJsonRepeatedFieldConverter<StructType, EleType>, std::placeholders::_1, std::placeholders::_2, /*copy*/ field_name, field));
    to_json_converters_.push_back(
      std::bind(detail::toJsonRepeatedFieldConverter<StructType, EleType>, std::placeholders::_1, std::placeholders::_2, /*copy*/ field_name, field));
  }

  template< typename EleType >
  void RegisterRepeatedCustomField(
    const std::string& field_name, std::vector<std::unique_ptr<EleType>> StructType::* field, 
    bool(*from_json_conv)(const rapidjson::Value&, EleType*), void(*to_json_conv)(rapidjson::Writer<rapidjson::StringBuffer>& writer, const EleType*)) {
    from_json_converters_.push_back(
      std::bind(detail::fromJsonRepeatedCustomFieldConverter<StructType, EleType>, std::placeholders::_1, std::placeholders::_2, /*copy*/ field_name, field, from_json_conv));
    to_json_converters_.push_back(
      std::bind(detail::toJsonRepeatedCustomFieldConverter<StructType, EleType>, std::placeholders::_1, std::placeholders::_2, /*copy*/ field_name, field, to_json_conv));
  }

  template< typename FieldType >
  void RegisterCustomField(
    const std::string& field_name, FieldType StructType::* field, 
    bool(*from_json_conv)(const rapidjson::Value&, FieldType*), void(*to_json_conv)(rapidjson::Writer<rapidjson::StringBuffer>& writer, const FieldType*)) {
    from_json_converters_.push_back(
      std::bind(detail::fromJsonCustomFieldConverter<StructType, FieldType>, std::placeholders::_1, std::placeholders::_2, /*copy*/ field_name, field, from_json_conv));
    to_json_converters_.push_back(
      std::bind(detail::toJsonCustomFieldConverter<StructType, FieldType>, std::placeholders::_1, std::placeholders::_2, /*copy*/ field_name, field, to_json_conv));
  }

  bool FromJson(const std::string& json, StructType& obj) const {
    rapidjson::Document doc;
    if (doc.Parse(json.data()).HasParseError()) {
      return false;
    }
    return FromJsonValue(doc, obj);
  }

  bool FromJsonValue(const rapidjson::Value& value, StructType& obj) const {
    for (const auto& c : from_json_converters_) {
      if (!c(value, obj)) {
        return false;
      }
    }
    return true;
  }

  std::string ToJson(const StructType& obj) const {
    rapidjson::StringBuffer sb;
    rapidjson::Writer<rapidjson::StringBuffer> writer(sb);
    ToJsonValue(writer, obj);
    return std::string(sb.GetString(), sb.GetSize());
  }

  void ToJsonValue(rapidjson::Writer<rapidjson::StringBuffer>& writer, const StructType& obj) const {
    writer.StartObject();
    for (const auto& c : to_json_converters_) {
      c(writer, obj);
    }
    writer.EndObject();
  }

private:
  RapidJsonConverter() {
    StructType::RegisterRapidJsonConverter(*this);
  }
  RapidJsonConverter(const RapidJsonConverter&) = delete;
  RapidJsonConverter& operator=(const RapidJsonConverter&) = delete;

  std::vector<FromJsonConverter<StructType>> from_json_converters_;
  std::vector<ToJsonConverter<StructType>> to_json_converters_;
};

} // namespace converter

RAPIDJSON_NAMESPACE_END

#endif
