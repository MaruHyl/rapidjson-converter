# rapidjson-converter
基于rapidjson的动态ToJson和FromJson，以简化rapidjson的使用

## 痛点
使用rapidjson手写ToJson和FromJson过于繁琐，重复体力劳动并且容易出错，涉及到嵌套和数组类型，就更加麻烦了，而且字段一多基本上满屏的json转换代码，难以阅读

## 使用示例
```c++

// 简单struct定义
struct Test {
    bool boolv = false;
    int intv = 0;
    unsigned uintv = 0;
    int64_t int64v = 0;
    uint64_t uint64v = 0;
    double doublev = 0;
    std::string stringv;
    std::vector<bool> bools;
    std::vector<int> ints;
    std::vector<unsigned> uints;
    std::vector<int64_t> int64s;
    std::vector<uint64_t> uint64s;
    std::vector<double> doubles;
    std::vector<std::string> strings;

    template <typename Reader, typename Writer>
    static void RegisterRapidJsonConverter(
        rapidjson::converter::Converter<Test, Reader, Writer> &conv) {
        conv.RegisterField("boolv", &Test::boolv);
        conv.RegisterField("intv", &Test::intv);
        conv.RegisterField("uintv", &Test::uintv);
        conv.RegisterField("int64v", &Test::int64v);
        conv.RegisterField("uint64v", &Test::uint64v);
        conv.RegisterField("doublev", &Test::doublev);
        conv.RegisterField("stringv", &Test::stringv);
        conv.RegisterField("bools", &Test::bools);
        conv.RegisterField("ints", &Test::ints);
        conv.RegisterField("uints", &Test::uints);
        conv.RegisterField("int64s", &Test::int64s);
        conv.RegisterField("uint64s", &Test::uint64s);
        conv.RegisterField("doubles", &Test::doubles);
        conv.RegisterField("strings", &Test::strings);
    }
};

// 复杂struct定义
struct CombTest {
    Test testv;
    std::vector<Test> tests;

    template <typename Reader, typename Writer>
    static void RegisterRapidJsonConverter(
        rapidjson::converter::Converter<CombTest, Reader, Writer> &conv) {
        conv.RegisterField("testv", &CombTest::testv);
        conv.RegisterField("tests", &CombTest::tests);
    }
};

// example
void example() {
    CombTest t1 = RandomCombTest();
    auto &conv =
        converter::Converter<CombTest, Value, PrettyWriter<StringBuffer>>::GetInstance();

    // ToJson
    StringBuffer sb;
    PrettyWriter<StringBuffer> writer(sb);
    conv.Write(writer, t1);
    string json = sb.GetString();

    // FromJson
    CombTest t2;
    Document doc;
    assert(!doc.Parse(json.data()).HasParseError());
    assert(conv.Read(doc, t2));
    assert(t1 == t2);
}

```

## 注意事项
- `没有严格测试和在严格场景下使用`. 目前在使用和测试中，由于还没有过于复杂的使用场景，所以暂未发现bug
- header-only lib
