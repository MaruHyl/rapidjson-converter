#ifndef RAPIDJSON_converter_H_
#define RAPIDJSON_converter_H_

#include <string>
#include <vector>
#include <functional>

#include "rapidjson/rapidjson.h"
#include "rapidjson/writer.h"
#include "rapidjson/document.h"

RAPIDJSON_NAMESPACE_BEGIN

namespace converter {

namespace internal {

template <typename ObjectType, typename Reader, typename Writer,
          typename FieldType>
bool read(Reader &reader, ObjectType &obj, const std::string &fieldName,
          FieldType ObjectType::*field);

template <typename ObjectType, typename Reader, typename Writer,
          typename FieldType>
void write(Writer &writer, ObjectType &obj, const std::string &fieldName,
           FieldType ObjectType::*field);

}  // namespace internal

template <typename ObjectType, typename Reader = rapidjson::Value,
          typename Writer = rapidjson::Writer<rapidjson::StringBuffer>>
class Converter {
   public:
    static const Converter &GetInstance() {
        static Converter conv;
        return conv;
    }

    template <typename FieldType>
    void RegisterField(const std::string &fieldName,
                       FieldType ObjectType::*field) {
        readCallbacks_.push_back(
            std::bind(internal::read<ObjectType, Reader, Writer, FieldType>,
                      std::placeholders::_1, std::placeholders::_2,
                      /*copy*/ fieldName, field));
        writeCallbacks_.push_back(
            std::bind(internal::write<ObjectType, Reader, Writer, FieldType>,
                      std::placeholders::_1, std::placeholders::_2,
                      /*copy*/ fieldName, field));
    }

    bool Read(Reader &reader, ObjectType &obj) const {
        for (auto &callback : readCallbacks_) {
            if (!callback(reader, obj)) {
                return false;
            }
        }
        return true;
    }

    void Write(Writer &writer, ObjectType &obj) const {
        writer.StartObject();
        for (auto &callback : writeCallbacks_) {
            callback(writer, obj);
        }
        writer.EndObject();
    }

   private:
    explicit Converter() { ObjectType::RegisterRapidJsonConverter(*this); }
    Converter(const Converter &) = delete;
    Converter &operator=(const Converter &) = delete;

    std::vector<std::function<bool(Reader &, ObjectType &)>> readCallbacks_;
    std::vector<std::function<void(Writer &, ObjectType &)>> writeCallbacks_;
};

namespace internal {

template <typename Reader, typename Writer, typename FieldType>
struct helper {
    static bool read(Reader &reader, FieldType *field) {
        if (!reader.IsObject()) {
            return false;
        }
        return Converter<FieldType, Reader, Writer>::GetInstance().Read(reader,
                                                                        *field);
    }
    static void write(Writer &writer, FieldType *field) {
        Converter<FieldType, Reader, Writer>::GetInstance().Write(writer,
                                                                  *field);
    }
};

template <typename Reader, typename Writer>
struct helper<Reader, Writer, bool> {
    static bool read(Reader &reader, bool *field) {
        if (reader.IsBool()) {
            *field = reader.GetBool();
            return true;
        }
        return false;
    }
    static void write(Writer &writer, bool *field) { writer.Bool(*field); }
};

template <typename Reader, typename Writer>
struct helper<Reader, Writer, int> {
    static bool read(Reader &reader, int *field) {
        if (reader.IsInt()) {
            *field = reader.GetInt();
            return true;
        }
        return false;
    }
    static void write(Writer &writer, int *field) { writer.Int(*field); }
};

template <typename Reader, typename Writer>
struct helper<Reader, Writer, unsigned> {
    static bool read(Reader &reader, unsigned *field) {
        if (reader.IsUint()) {
            *field = reader.GetUint();
            return true;
        }
        return false;
    }
    static void write(Writer &writer, unsigned *field) { writer.Uint(*field); }
};

template <typename Reader, typename Writer>
struct helper<Reader, Writer, int64_t> {
    static bool read(Reader &reader, int64_t *field) {
        if (reader.IsInt64()) {
            *field = reader.GetInt64();
            return true;
        }
        return false;
    }
    static void write(Writer &writer, int64_t *field) { writer.Int64(*field); }
};

template <typename Reader, typename Writer>
struct helper<Reader, Writer, uint64_t> {
    static bool read(Reader &reader, uint64_t *field) {
        if (reader.IsUint64()) {
            *field = reader.GetUint64();
            return true;
        }
        return false;
    }
    static void write(Writer &writer, uint64_t *field) {
        writer.Uint64(*field);
    }
};

template <typename Reader, typename Writer>
struct helper<Reader, Writer, double> {
    static bool read(Reader &reader, double *field) {
        if (reader.IsDouble()) {
            *field = reader.GetDouble();
            return true;
        }
        return false;
    }
    static void write(Writer &writer, double *field) { writer.Double(*field); }
};

template <typename Reader, typename Writer>
struct helper<Reader, Writer, std::string> {
    static bool read(Reader &reader, std::string *field) {
        if (reader.IsString()) {
            field->assign(reader.GetString(), reader.GetStringLength());
            return true;
        }
        return false;
    }
    static void write(Writer &writer, std::string *field) {
        writer.String(field->data(), field->size());
    }
};

template <typename Reader, typename Writer, typename ElemType>
struct helper<Reader, Writer, std::vector<ElemType>> {
    static bool read(Reader &reader, std::vector<ElemType> *field) {
        if (reader.IsArray()) {
            field->resize(reader.Size());
            for (rapidjson::SizeType i = 0; i != reader.Size(); ++i) {
                if (!helper<Reader, Writer, ElemType>::read(
                        reader[i], &static_cast<ElemType &>(field->at(i)))) {
                    return false;
                }
            }
            return true;
        }
        return false;
    }
    static void write(Writer &writer, std::vector<ElemType> *field) {
        writer.StartArray();
        for (size_t i = 0; i != field->size(); ++i) {
            helper<Reader, Writer, ElemType>::write(
                writer, &static_cast<ElemType &>(field->at(i)));
        }
        writer.EndArray();
    }
};

template <typename Reader, typename Writer>
struct helper<Reader, Writer, std::vector<bool>> {
    static bool read(Reader &reader, std::vector<bool> *field) {
        if (reader.IsArray()) {
            field->reserve(reader.Size());
            for (rapidjson::SizeType i = 0; i != reader.Size(); ++i) {
                bool v;
                if (!helper<Reader, Writer, bool>::read(reader[i], &v)) {
                    return false;
                }
                field->push_back(v);
            }
            return true;
        }
        return false;
    }
    static void write(Writer &writer, std::vector<bool> *field) {
        writer.StartArray();
        for (size_t i = 0; i != field->size(); ++i) {
            bool v = field->at(i);
            helper<Reader, Writer, bool>::write(writer, &v);
        }
        writer.EndArray();
    }
};

template <typename ObjectType, typename Reader, typename Writer,
          typename FieldType>
bool read(Reader &reader, ObjectType &obj, const std::string &fieldName,
          FieldType ObjectType::*field) {
    const char *fieldNameCh = fieldName.data();
    if (reader.IsObject() && reader.HasMember(fieldNameCh)) {
        return helper<Reader, Writer, FieldType>::read(reader[fieldNameCh],
                                                       &(obj.*field));
    }
    return false;
}

template <typename ObjectType, typename Reader, typename Writer,
          typename FieldType>
void write(Writer &writer, ObjectType &obj, const std::string &fieldName,
           FieldType ObjectType::*field) {
    writer.Key(fieldName.data());
    helper<Reader, Writer, FieldType>::write(writer, &(obj.*field));
}

}  // namespace internal

}  // namespace converter

RAPIDJSON_NAMESPACE_END

#endif
