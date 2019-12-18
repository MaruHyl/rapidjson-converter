#include <algorithm>
#include <cassert>
#include <iostream>
#include <random>

#include "rapidjson/prettywriter.h"

#include "rapidjson-converter/rapidjson-converter.h"

static std::random_device r;
static std::mt19937_64 re(r());

double RandomDouble() {
    std::uniform_real_distribution<double> unif(
        std::numeric_limits<double>::min(), std::numeric_limits<double>::max());
    return unif(re);
}

template <typename IntType>
IntType RandomInt() {
    std::uniform_int_distribution<IntType> unif(
        std::numeric_limits<IntType>::min(),
        std::numeric_limits<IntType>::max());
    return unif(re);
}

bool RandomBool() {
    std::uniform_int_distribution<int> unif(0, 1);
    return unif(re);
}

template <typename IntType>
IntType RandomInt(IntType min, IntType max) {
    std::uniform_int_distribution<IntType> unif(min, max);
    return unif(re);
}

std::string RandomString() {
    std::string seed(
        "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz");
    std::shuffle(seed.begin(), seed.end(), re);
    return seed;
}

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

bool operator==(const Test &lhs, const Test &rhs) {
    return lhs.boolv == rhs.boolv && lhs.intv == rhs.intv &&
           lhs.uintv == rhs.uintv && lhs.int64v == rhs.int64v &&
           lhs.uint64v == rhs.uint64v &&
           //    (lhs.doublev - rhs.doublev) > -0.01 && (lhs.doublev -
           //    rhs.doublev) < 0.01 &&
           lhs.stringv == rhs.stringv && lhs.bools == rhs.bools &&
           lhs.ints == rhs.ints && lhs.uints == rhs.uints &&
           lhs.int64s == rhs.int64s && lhs.uint64s == rhs.uint64s &&
           //    std::distance(lhs.doubles.cbegin(), lhs.doubles.cend()) ==
           //        std::distance(rhs.doubles.cbegin(), rhs.doubles.cend()) &&
           //    std::equal(lhs.doubles.cbegin(), lhs.doubles.cend(),
           //               rhs.doubles.cbegin(),
           //               [](const double &a, const double &b) {
           //                   return (a - b) < 0.01 && (a - b) > -0.01;
           //               }) &&
           lhs.strings == rhs.strings;
}

bool operator!=(const Test &lhs, const Test &rhs) { return !(lhs == rhs); }

Test RandomTest() {
    Test t;
    t.boolv = RandomBool();
    t.intv = RandomInt<int>();
    t.uintv = RandomInt<unsigned>();
    t.int64v = RandomInt<int64_t>();
    t.uint64v = RandomInt<uint64_t>();
    t.doublev = RandomDouble();
    t.stringv = RandomString();
    size_t len = 0;
    size_t maxLen = 10;
    len = RandomInt<size_t>(1, maxLen);
    t.bools.reserve(len);
    for (size_t i = 0; i != len; ++i) {
        t.bools.push_back(RandomBool());
    }
    len = RandomInt<size_t>(1, maxLen);
    t.ints.reserve(len);
    for (size_t i = 0; i != len; ++i) {
        t.ints.push_back(RandomInt<int>());
    }
    len = RandomInt<size_t>(1, maxLen);
    t.uints.reserve(len);
    for (size_t i = 0; i != len; ++i) {
        t.uints.push_back(RandomInt<unsigned>());
    }
    len = RandomInt<size_t>(1, maxLen);
    t.int64s.reserve(len);
    for (size_t i = 0; i != len; ++i) {
        t.int64s.push_back(RandomInt<int64_t>());
    }
    len = RandomInt<size_t>(1, maxLen);
    t.uint64s.reserve(len);
    for (size_t i = 0; i != len; ++i) {
        t.uint64s.push_back(RandomInt<uint64_t>());
    }
    len = RandomInt<size_t>(1, maxLen);
    t.doubles.reserve(len);
    for (size_t i = 0; i != len; ++i) {
        t.doubles.push_back(RandomDouble());
    }
    len = RandomInt<size_t>(1, maxLen);
    t.strings.reserve(len);
    for (size_t i = 0; i != len; ++i) {
        t.strings.push_back(RandomString());
    }
    return t;
}

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

bool operator==(const CombTest &lhs, const CombTest &rhs) {
    return lhs.testv == rhs.testv && lhs.tests == rhs.tests;
}

bool operator!=(const CombTest &lhs, const CombTest &rhs) {
    return !(lhs == rhs);
}

CombTest RandomCombTest() {
    CombTest t;
    t.testv = RandomTest();
    size_t len = 0;
    size_t maxLen = 3;
    len = RandomInt<size_t>(1, maxLen);
    t.tests.reserve(len);
    for (size_t i = 0; i != len; ++i) {
        t.tests.push_back(RandomTest());
    }
    return t;
}

int main() {
    using namespace std;
    using namespace rapidjson;

    CombTest t1 = RandomCombTest();
    auto &conv =
        converter::Converter<CombTest, Value, PrettyWriter<StringBuffer>>::GetInstance();

    // ToJson
    StringBuffer sb;
    PrettyWriter<StringBuffer> writer(sb);
    conv.Write(writer, t1);
    string json1 = sb.GetString();
    cout << json1 << endl;

    // FromJson
    CombTest t2;
    Document doc;
    assert(!doc.Parse(json1.data()).HasParseError());
    assert(conv.Read(doc, t2));
    assert(t1 == t2);
}
