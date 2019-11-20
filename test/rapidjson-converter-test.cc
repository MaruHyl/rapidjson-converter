#include <iostream>
#include <algorithm>
#include <random>
#include <chrono>

#include "rapidjson/rapidjson.h"
#include "rapidjson/writer.h"
#include "rapidjson/document.h"
#include "rapidjson/prettywriter.h"

#include "rapidjson-converter/rapidjson-converter.h"

enum class TestEnum { kNone, kOne, kTwo };

bool TestEnumFromString(const rapidjson::Value& v, TestEnum* f) {
  if (v.IsString()) {
    const std::string str = v.GetString();
    if (str == "1") {
      *f = TestEnum::kOne;
      return true;
    } else if (str == "2" ) {
      *f = TestEnum::kTwo;
      return true;
    }
  }
  *f = TestEnum::kNone;
  return false;
}

void TestEnumToString(rapidjson::Writer<rapidjson::StringBuffer>& w, const TestEnum* f) {
  if (*f == TestEnum::kOne) {
    w.String("1");
  } else if (*f == TestEnum::kTwo) {
    w.String("2");
  } else {
    w.String("3");
  }
}

struct Test {
  int int_value;
  bool bool_value;
  double double_value;
  int64_t int64_value;
  std::string string_value;
  unsigned int uint_value;
  uint64_t uint64_value;
  TestEnum enum_value;

  static void RegisterRapidJsonConverter(rapidjson::converter::RapidJsonConverter<Test>& conv) {
    conv.RegisterIntField("int_value", &Test::int_value);
    conv.RegisterBoolField("bool_value", &Test::bool_value);
    conv.RegisterDoubleField("double_value", &Test::double_value);
    conv.RegisterInt64Field("int64_value", &Test::int64_value);
    conv.RegisterStringField("string_value", &Test::string_value);
    conv.RegisterUintField("uint_value", &Test::uint_value);
    conv.RegisterUint64Field("uint64_value", &Test::uint64_value);
    conv.RegisterCustomField("enum_value", &Test::enum_value, TestEnumFromString, TestEnumToString);
  }
};

struct CustomTest {
  int int_value;
};

bool CustomTestFromString(const rapidjson::Value& v, CustomTest* f) {
  if (v.IsString()) {
    f->int_value = std::atoi(v.GetString());
    return true;
  }
  return false;
}

void CustomTestToString(rapidjson::Writer<rapidjson::StringBuffer>& w, const CustomTest* f) {
  w.String(std::to_string(f->int_value).data());
}

struct NestedTest {
  Test t;
  std::vector<std::unique_ptr<int>> ints;
  std::vector<std::unique_ptr<bool>> bools;
  std::vector<std::unique_ptr<double>> doubles;
  std::vector<std::unique_ptr<int64_t>> int64_ts;
  std::vector<std::unique_ptr<std::string>> strings;
  std::vector<std::unique_ptr<unsigned int>> uints;
  std::vector<std::unique_ptr<uint64_t>> uint64_ts;
  std::vector<std::unique_ptr<Test>> tests;
  std::vector<std::unique_ptr<NestedTest>> nestedTests;
  std::vector<std::unique_ptr<CustomTest>> customTests;

  static void RegisterRapidJsonConverter(rapidjson::converter::RapidJsonConverter<NestedTest>& conv) {
    conv.RegisterNestedField("t", &NestedTest::t);
    conv.RegisterRepeatedField("ints", &NestedTest::ints);
    conv.RegisterRepeatedField("bools", &NestedTest::bools);
    conv.RegisterRepeatedField("doubles", &NestedTest::doubles);
    conv.RegisterRepeatedField("int64_ts", &NestedTest::int64_ts);
    conv.RegisterRepeatedField("strings", &NestedTest::strings);
    conv.RegisterRepeatedField("uints", &NestedTest::uints);
    conv.RegisterRepeatedField("uint64_ts", &NestedTest::uint64_ts);
    conv.RegisterRepeatedField("tests", &NestedTest::tests);
    conv.RegisterRepeatedField("nestedTests", &NestedTest::nestedTests);
    conv.RegisterRepeatedCustomField("customTests", &NestedTest::customTests, CustomTestFromString, CustomTestToString);
  }
};

static std::random_device r;
static std::default_random_engine re(r());

double RandomDouble(double low, double hi) {
  std::uniform_real_distribution<double> unif(low, hi);
  return unif(re);
}

int RandomInt(int low, int hi) {
  std::uniform_int_distribution<int> unif(low, hi);
  return unif(re);
}

std::string RandomString() {
  std::string seed("0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz");
  std::shuffle(seed.begin(), seed.end(), re);
  return seed.substr(RandomInt(1, seed.size()));
}

Test RandomTest() {
  Test t;
  t.int_value = RandomInt(-1000000, 1000000);
  t.bool_value = RandomDouble(-1, 1) > 0;
  // t.double_value = RandomDouble(-1000000, 1000000);
  t.double_value = RandomInt(-1000000, 1000000);
  t.int64_value = RandomInt(-1000000, 1000000);
  t.string_value = RandomString();
  t.uint_value = RandomInt(-1000000, 1000000);
  t.uint64_value = RandomInt(-1000000, 1000000);
  t.enum_value = (RandomInt(1, 3)==1)?TestEnum::kOne:TestEnum::kTwo;
  return t;
}

NestedTest RandomNestedTest();
NestedTest RandomNestedTest() {
  NestedTest nt;
  nt.t = RandomTest();

  {
    int count = RandomInt(1, 10);
    for (int i = 0; i != count; ++i) {
      std::unique_ptr<int> ptr(new int());
      *ptr = RandomInt(-1000000, 1000000);
      nt.ints.push_back(std::move(ptr));
    }
  }

  {
    int count = RandomInt(1, 10);
    for (int i = 0; i != count; ++i) {
      std::unique_ptr<bool> ptr(new bool());
      *ptr = RandomDouble(-1, 1) > 0;
      nt.bools.push_back(std::move(ptr));
    }
  }

  {
    int count = RandomInt(1, 10);
    for (int i = 0; i != count; ++i) {
      std::unique_ptr<double> ptr(new double());
      // *ptr = RandomDouble(-1000000, 1000000);
      *ptr = RandomInt(-1000000, 1000000);
      nt.doubles.push_back(std::move(ptr));
    }
  }

  {
    int count = RandomInt(1, 10);
    for (int i = 0; i != count; ++i) {
      std::unique_ptr<int64_t> ptr(new int64_t());
      *ptr = RandomInt(-1000000, 1000000);
      nt.int64_ts.push_back(std::move(ptr));
    }
  }

  {
    int count = RandomInt(1, 10);
    for (int i = 0; i != count; ++i) {
      std::unique_ptr<unsigned int> ptr(new unsigned int());
      *ptr = RandomInt(-1000000, 1000000);
      nt.uints.push_back(std::move(ptr));
    }
  }

  {
    int count = RandomInt(1, 10);
    for (int i = 0; i != count; ++i) {
      std::unique_ptr<uint64_t> ptr(new uint64_t());
      *ptr = RandomInt(-1000000, 1000000);
      nt.uint64_ts.push_back(std::move(ptr));
    }
  }

  {
    int count = RandomInt(1, 10);
    for (int i = 0; i != count; ++i) {
      std::unique_ptr<std::string> ptr(new std::string());
      *ptr = RandomString();
      nt.strings.push_back(std::move(ptr));
    }
  }

  {
    int count = RandomInt(1, 10);
    for (int i = 0; i != count; ++i) {
      std::unique_ptr<Test> ptr(new Test());
      *ptr = RandomTest();
      nt.tests.push_back(std::move(ptr));
    }
  }

  {
    int count = RandomInt(0, 2);
    for (int i = 0; i != count; ++i) {
      std::unique_ptr<NestedTest> ptr(new NestedTest());
      *ptr = RandomNestedTest();
      nt.nestedTests.push_back(std::move(ptr));
    }
  }

  {
    int count = RandomInt(1, 10);
    for (int i = 0; i != count; ++i) {
      std::unique_ptr<CustomTest> ptr(new CustomTest());
      ptr->int_value = RandomInt(-1000000, 1000000);
      nt.customTests.push_back(std::move(ptr));
    }
  }

  return nt;
}

void ToJson(rapidjson::Writer<rapidjson::StringBuffer>& writer, const Test& t) {
  writer.StartObject();
  writer.Key("int_value"); writer.Int(t.int_value);
  writer.Key("bool_value"); writer.Bool(t.bool_value);
  writer.Key("double_value"); writer.Double(t.double_value);
  writer.Key("int64_value"); writer.Int64(t.int64_value);
  writer.Key("string_value"); writer.String(t.string_value.data());
  writer.Key("uint_value"); writer.Uint(t.uint_value);
  writer.Key("uint64_value"); writer.Uint64(t.uint64_value);
  writer.Key("enum_value"); TestEnumToString(writer, &(t.enum_value));
  writer.EndObject();
}

void ToJson(rapidjson::Writer<rapidjson::StringBuffer>& writer, const NestedTest& t);
void ToJson(rapidjson::Writer<rapidjson::StringBuffer>& writer, const NestedTest& t) {
  writer.StartObject();

  writer.Key("t"); ToJson(writer, t.t);
  
  writer.Key("ints");
  writer.StartArray();
  for (const auto& v : t.ints) {
    writer.Int(*v);
  }
  writer.EndArray();

  writer.Key("bools");
  writer.StartArray();
  for (const auto& v : t.bools) {
    writer.Bool(*v);
  }
  writer.EndArray();

  writer.Key("doubles");
  writer.StartArray();
  for (const auto& v : t.doubles) {
    writer.Double(*v);
  }
  writer.EndArray();

  writer.Key("int64_ts");
  writer.StartArray();
  for (const auto& v : t.int64_ts) {
    writer.Int64(*v);
  }
  writer.EndArray();

  writer.Key("strings");
  writer.StartArray();
  for (const auto& v : t.strings) {
    writer.String((*v).data());
  }
  writer.EndArray();

  writer.Key("uints");
  writer.StartArray();
  for (const auto& v : t.uints) {
    writer.Uint(*v);
  }
  writer.EndArray();

  writer.Key("uint64_ts");
  writer.StartArray();
  for (const auto& v : t.uint64_ts) {
    writer.Uint64(*v);
  }
  writer.EndArray();

  writer.Key("tests");
  writer.StartArray();
  for (const auto& v : t.tests) {
    ToJson(writer, *v);
  }
  writer.EndArray();

  writer.Key("nestedTests");
  writer.StartArray();
  for (const auto& v : t.nestedTests) {
    ToJson(writer, *v);
  }
  writer.EndArray();

  writer.Key("customTests");
  writer.StartArray();
  for (const auto& v : t.customTests) {
    CustomTestToString(writer, &*v);
  }
  writer.EndArray();

  writer.EndObject();
}

bool FromJson(const rapidjson::Value& v, Test& t) {
  if (v.IsObject()) {
    if (v.HasMember("int_value")) {
      if (v["int_value"].IsInt()) {
        t.int_value = v["int_value"].GetInt();
      } else {
        return false;
      }
    }
    if (v.HasMember("bool_value")) {
      if (v["bool_value"].IsBool()) {
        t.bool_value = v["bool_value"].GetBool();
      } else {
        return false;
      }
    }
    if (v.HasMember("double_value")) {
      if (v["double_value"].IsDouble()) {
        t.double_value = v["double_value"].GetDouble();
      } else {
        return false;
      }
    }
    if (v.HasMember("int64_value")) {
      if (v["int64_value"].IsInt64()) {
        t.int64_value = v["int64_value"].GetInt64();
      } else {
        return false;
      }
    }
    if (v.HasMember("string_value")) {
      if (v["string_value"].IsString()) {
        t.string_value.assign(v["string_value"].GetString(), v["string_value"].GetStringLength());
      } else {
        return false;
      }
    }
    if (v.HasMember("uint_value")) {
      if (v["uint_value"].IsUint()) {
        t.uint_value = v["uint_value"].GetUint();
      } else {
        return false;
      }
    }
    if (v.HasMember("uint64_value")) {
      if (v["uint64_value"].IsUint64()) {
        t.uint64_value = v["uint64_value"].GetUint64();
      } else {
        return false;
      }
    }
    if (v.HasMember("enum_value")) {
      if (!TestEnumFromString(v["enum_value"], &(t.enum_value))) {
        return false;
      }
    }
    return true;
  }
  return false;
}

bool FromJson(const rapidjson::Value& v, NestedTest& t) {
  if (v.IsObject()) {
    if (v.HasMember("t")) {
      if (!FromJson(v["t"], t.t)) {
        return false;
      }
    }

    if (v.HasMember("ints")) {
      if (!v["ints"].IsArray()) {
        return false;
      }
      const auto& array = v["ints"].GetArray();
      t.ints.reserve(array.Size());
      for (const auto& ele : array) {
        if (!ele.IsInt()) {
          return false;
        }
        std::unique_ptr<int> ptr(new int());
        *ptr = ele.GetInt();
        t.ints.push_back(std::move(ptr));
      }
    }

    if (v.HasMember("bools")) {
      if (!v["bools"].IsArray()) {
        return false;
      }
      const auto& array = v["bools"].GetArray();
      t.bools.reserve(array.Size());
      for (const auto& ele : array) {
        if (!ele.IsBool()) {
          return false;
        }
        std::unique_ptr<bool> ptr(new bool());
        *ptr = ele.GetBool();
        t.bools.push_back(std::move(ptr));
      }
    }

    if (v.HasMember("doubles")) {
      if (!v["doubles"].IsArray()) {
        return false;
      }
      const auto& array = v["doubles"].GetArray();
      t.doubles.reserve(array.Size());
      for (const auto& ele : array) {
        if (!ele.IsDouble()) {
          return false;
        }
        std::unique_ptr<double> ptr(new double());
        *ptr = ele.GetDouble();
        t.doubles.push_back(std::move(ptr));
      }
    }

    if (v.HasMember("int64_ts")) {
      if (!v["int64_ts"].IsArray()) {
        return false;
      }
      const auto& array = v["int64_ts"].GetArray();
      t.int64_ts.reserve(array.Size());
      for (const auto& ele : array) {
        if (!ele.IsInt64()) {
          return false;
        }
        std::unique_ptr<int64_t> ptr(new int64_t());
        *ptr = ele.GetInt64();
        t.int64_ts.push_back(std::move(ptr));
      }
    }

    if (v.HasMember("strings")) {
      if (!v["strings"].IsArray()) {
        return false;
      }
      const auto& array = v["strings"].GetArray();
      t.strings.reserve(array.Size());
      for (const auto& ele : array) {
        if (!ele.IsString()) {
          return false;
        }
        std::unique_ptr<std::string> ptr(new std::string());
        ptr->assign(ele.GetString(), ele.GetStringLength());
        t.strings.push_back(std::move(ptr));
      }
    }

    if (v.HasMember("uints")) {
      if (!v["uints"].IsArray()) {
        return false;
      }
      const auto& array = v["uints"].GetArray();
      t.uints.reserve(array.Size());
      for (const auto& ele : array) {
        if (!ele.IsUint()) {
          return false;
        }
        std::unique_ptr<unsigned int> ptr(new unsigned int());
        *ptr = ele.GetUint();
        t.uints.push_back(std::move(ptr));
      }
    }

    if (v.HasMember("uint64_ts")) {
      if (!v["uint64_ts"].IsArray()) {
        return false;
      }
      const auto& array = v["uint64_ts"].GetArray();
      t.uint64_ts.reserve(array.Size());
      for (const auto& ele : array) {
        if (!ele.IsUint64()) {
          return false;
        }
        std::unique_ptr<uint64_t> ptr(new uint64_t());
        *ptr = ele.GetUint64();
        t.uint64_ts.push_back(std::move(ptr));
      }
    }

    if (v.HasMember("tests")) {
      if (!v["tests"].IsArray()) {
        return false;
      }
      const auto& array = v["tests"].GetArray();
      t.tests.reserve(array.Size());
      for (const auto& ele : array) {
        if (!ele.IsObject()) {
          return false;
        }
        std::unique_ptr<Test> ptr(new Test());
        if (!FromJson(ele, *ptr)) {
          return false;
        }
        t.tests.push_back(std::move(ptr));
      }
    }

    if (v.HasMember("nestedTests")) {
      if (!v["nestedTests"].IsArray()) {
        return false;
      }
      const auto& array = v["nestedTests"].GetArray();
      t.nestedTests.reserve(array.Size());
      for (const auto& ele : array) {
        if (!ele.IsObject()) {
          return false;
        }
        std::unique_ptr<NestedTest> ptr(new NestedTest());
        if (!FromJson(ele, *ptr)) {
          return false;
        }
        t.nestedTests.push_back(std::move(ptr));
      }
    }

    if (v.HasMember("customTests")) {
      if (!v["customTests"].IsArray()) {
        return false;
      }
      const auto& array = v["customTests"].GetArray();
      t.customTests.reserve(array.Size());
      for (const auto& ele : array) {
        if (!ele.IsString()) {
          return false;
        }
        std::unique_ptr<CustomTest> ptr(new CustomTest());
        if (!CustomTestFromString(ele, &*ptr)) {
          return false;
        }
        t.customTests.push_back(std::move(ptr));
      }
    }
    return true;
  }
  return false;
}

namespace stdc = std::chrono;

stdc::nanoseconds ToJsonNormal(const NestedTest& nt, std::string& nt_json_normal) {
  auto start_time = stdc::high_resolution_clock::now();
  rapidjson::StringBuffer sb;
  rapidjson::Writer<rapidjson::StringBuffer> writer(sb);
  ToJson(writer, nt);
  nt_json_normal.assign(sb.GetString(), sb.GetSize());
  return stdc::duration_cast<stdc::nanoseconds>(stdc::high_resolution_clock::now()-start_time);
}

stdc::nanoseconds FromJsonNormal(NestedTest& nt, const std::string& nt_json_normal) {
  auto start_time = stdc::high_resolution_clock::now();
  rapidjson::Document doc;
  if (doc.Parse(nt_json_normal.data()).HasParseError()) {
    std::cout << "FromJsonNormal::Parse error" << std::endl;
    exit(0);
  }
  if (!FromJson(doc, nt)) {
    std::cout << "FromJsonNormal::FromJson error" << std::endl;
    exit(0);
  }
  return stdc::duration_cast<stdc::nanoseconds>(stdc::high_resolution_clock::now()-start_time);
}

stdc::nanoseconds ToJsonConverter(const NestedTest& nt, std::string& nt_json_converter) {
  auto start_time = stdc::high_resolution_clock::now();
  auto& conv_nested_test = rapidjson::converter::RapidJsonConverter<NestedTest>::GetSingleton();
  std::string json = conv_nested_test.ToJson(nt);
  nt_json_converter.assign(json.data(), json.size());
  return stdc::duration_cast<stdc::nanoseconds>(stdc::high_resolution_clock::now()-start_time);
}

stdc::nanoseconds FromJsonConverter(NestedTest& nt, const std::string& nt_json_converter) {
  auto start_time = stdc::high_resolution_clock::now();
  auto& conv_nested_test = rapidjson::converter::RapidJsonConverter<NestedTest>::GetSingleton();
  if (!conv_nested_test.FromJson(nt_json_converter, nt)) {
    std::cout << "FromJsonConverter::FromJson error" << std::endl;
    exit(0);
  }
  return stdc::duration_cast<stdc::nanoseconds>(stdc::high_resolution_clock::now()-start_time);
}

int main() {

  stdc::nanoseconds to_json_nano_normal(0);
  stdc::nanoseconds from_json_nano_normal(0);
  stdc::nanoseconds to_json_nano_converter(0);
  stdc::nanoseconds from_json_nano_converter(0);

  auto& conv_test = rapidjson::converter::RapidJsonConverter<Test>::GetSingleton();
  auto& conv_nested_test = rapidjson::converter::RapidJsonConverter<NestedTest>::GetSingleton();

  for (size_t i = 0; i != 10; ++i) {
    NestedTest nt = RandomNestedTest();
    std::string nt_json_normal;
    std::string nt_json_converter;
    // tojson nomal
    {
      to_json_nano_normal += ToJsonNormal(nt, nt_json_normal);
      NestedTest nt_;
      from_json_nano_normal += FromJsonNormal(nt_, nt_json_normal);
      std::string nt_json_normal_;
      to_json_nano_normal += ToJsonNormal(nt_, nt_json_normal_);
      if (nt_json_normal != nt_json_normal_) {
        std::cout << "to_from_json_normal error" << std::endl;
        return 0;
      }
    }
    // tojson converter
    {
      to_json_nano_converter += ToJsonConverter(nt, nt_json_converter);
      NestedTest nt_;
      from_json_nano_converter += FromJsonConverter(nt_, nt_json_converter);
      std::string nt_json_converter_;
      to_json_nano_converter += ToJsonConverter(nt_, nt_json_converter_);
      if (nt_json_converter != nt_json_converter_) {
        std::cout << "to_from_json_converter error" << std::endl;
        return 0;
      }
    }
    // check tojson
    if (nt_json_normal != nt_json_converter) {
      std::cout << "tojson error" << std::endl;
      return 0;
    }
  }

  std::cout << "to_json_nano_normal: " << to_json_nano_normal.count() << ", "
       << "from_json_nano_normal: " << from_json_nano_normal.count() << ", "
       << "to_json_nano_converter: " << to_json_nano_converter.count() << ", "
       << "from_json_nano_converter: " << from_json_nano_converter.count() << ", "
       << "to_json_diff: " << (to_json_nano_normal-to_json_nano_converter).count() << ", "
       << "from_json_diff: " << (from_json_nano_normal-from_json_nano_converter).count() << ", "
       << std::endl;

  return 0;
}
