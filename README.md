# rapidjson-converter
基于rapidjson的动态ToJson和FromJson，以简化rapidjson的使用

## 痛点
手写ToJson和FromJson过于繁琐，重复体力劳动并且容易出错

```c++

... ToJson
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
...

... FromJson
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
...

```

涉及到嵌套和vector类型，就更加麻烦了，而且字段一多基本上满屏的json转换代码，难以阅读

```c++
... ToJson
  writer.Key("tests");
  writer.StartArray();
  for (const auto& v : t.tests) {
    ToJson(writer, *v); // 递归ToJson
  }
  writer.EndArray();

  writer.Key("nestedTests");
  writer.StartArray();
  for (const auto& v : t.nestedTests) {
    ToJson(writer, *v); // 递归ToJson
  }
  writer.EndArray();
...

... FromJson
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
      if (!FromJson(ele, *ptr)) {  // 递归FromJson
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
      if (!FromJson(ele, *ptr)) {  // 递归FromJson
        return false;
      }
      t.nestedTests.push_back(std::move(ptr));
    }
  }
...
```

## 解决办法
解决思路主要受到 `chromium/json-converter` 的启发, 由用户手动注册字段信息(keyname, 类型, 自定义转换函数等). 
但实现方式稍有不同, `chromium/json-converter` 的实现大量使用了虚函数, 我则是避免使用虚函数大量使用模版推导.
另外就是 `chromium/json-converter` 是基于 `chromium` 自己的json实现，而我则是基于`Tencent/rapidjson`做的.

## 使用示例
```c++
... 简单struct定义

enum class TestEnum;
bool TestEnumFromString(const rapidjson::Value& v, TestEnum* f);
void TestEnumToString(rapidjson::Writer<rapidjson::StringBuffer>& w, const TestEnum* f);

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
...

... 复杂struct定义

struct Test;
struct CustomTest;
bool CustomTestFromString(const rapidjson::Value& v, CustomTest* f);
void CustomTestToString(rapidjson::Writer<rapidjson::StringBuffer>& w, const CustomTest* f);

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
...

... ToJson
  NestedTest nt;
  std::string json = 
    rapidjson::converter::RapidJsonConverter<NestedTest>::GetSingleton().ToJson(nt);
...

... FromJson
  std::string json;
  NestedTest nt;
  rapidjson::converter::RapidJsonConverter<NestedTest>::GetSingleton().FromJson(json, nt);
...

```

## 注意事项
- `没有严格测试和在严格场景下使用`. 目前在使用和测试中，由于还没有过于复杂的使用场景，所以暂未发现bug，性能测试上在O2以上没有额外开销甚至比手写的转换代码更快
- 数组只支持`vector<unique_ptr>`的方式
- header-only lib
- 目前只能搭配`rapidjson`使用
