#define KSR_THROW_ON_ASSERT
#include "ksr/type_util.hpp"

#include <QObject>
#include <QtTest/QtTest>

#include <cstdint>
#include <cstdlib>
#include <functional>
#include <optional>
#include <type_traits>

class test_narrow_cast : public QObject {
    Q_OBJECT

private slots:

    /*
    void test_enum();
    void test_enum_data();

    void test_enum_class();
    void test_enum_class_data();
    */

    void test_mixed_signedness();
    void test_mixed_signedness_data();

    void test_signed();
    void test_signed_data();

    void test_unsigned();
    void test_unsigned_data();
};

namespace {

    using check = std::function<bool()>;

    template <typename T>
    constexpr auto min = std::numeric_limits<T>::min();

    template <typename T>
    constexpr auto max = std::numeric_limits<T>::max();

    constexpr auto min_u8 = min<std::uint8_t>;
    constexpr auto max_u8 = max<std::uint8_t>;
    constexpr auto min_i8 = min<std::int8_t>;
    constexpr auto max_i8 = max<std::int8_t>;

    constexpr auto min_u32 = min<std::uint32_t>;
    constexpr auto max_u32 = max<std::uint32_t>;
    constexpr auto min_i32 = min<std::int32_t>;
    constexpr auto max_i32 = max<std::int32_t>;

    // TODO:DOC why type-erasure

    template <typename OutputType, typename InputType>
    auto make_check(const InputType input, const OutputType expected_output) -> check {

        return [input, expected_output] {
            try {

                const auto output = ksr::narrow_cast<OutputType>(input);
                return output == expected_output;

            } catch (const ksr::logic_error&) {
                return false;
            }
        };
    }

    template <typename OutputType, typename InputType>
    auto make_check(const InputType input) -> check {

        return [input] {
            try {

                ksr::narrow_cast<OutputType>(input);
                return false;

            } catch (const ksr::logic_error&) {
                return true;
            }
        };
    }
}

Q_DECLARE_METATYPE(check)

// TODO static_assert a bunch of !can_narrows

enum class scoped_enum { item = 0xff };
static_assert(std::is_same_v<ksr::underlying_type_ext_t<scoped_enum>, int>);
static_assert(std::is_same_v<ksr::underlying_type_ext_t<const scoped_enum>, int>);

enum class scoped_enum_uint8 : std::uint8_t { item = 0xff };
enum unscoped_enum_uint8 : std::uint8_t { unscoped_item = 0xff };
static_assert(std::is_same_v<ksr::underlying_type_ext_t<scoped_enum_uint8>, std::uint8_t>);
static_assert(std::is_same_v<ksr::underlying_type_ext_t<const scoped_enum_uint8>, std::uint8_t>);
static_assert(std::is_same_v<ksr::underlying_type_ext_t<unscoped_enum_uint8>, std::uint8_t>);
static_assert(std::is_same_v<ksr::underlying_type_ext_t<const unscoped_enum_uint8>, std::uint8_t>);

static_assert(std::is_same_v<ksr::underlying_type_ext_t<int>, int>);
static_assert(std::is_same_v<ksr::underlying_type_ext_t<const int>, int>);
static_assert(std::is_same_v<ksr::underlying_type_ext_t<double>, double>);
static_assert(std::is_same_v<ksr::underlying_type_ext_t<const double>, double>);

// TODO:HERE

void test_narrow_cast::test_mixed_signedness() {
    QFETCH(check, check_row);
    QVERIFY(check_row());
}

void test_narrow_cast::test_mixed_signedness_data() {

    QTest::addColumn<check>("check_row");

    QTest::newRow("int8 -> uint8 (min good)") << make_check(std::int8_t{min_u8}, min_u8);
    QTest::newRow("int8 -> uint8 (max good)") << make_check(max_i8, std::uint8_t{max_i8});
    QTest::newRow("int8 -> uint8 (min too low)") << make_check();
    QTest::newRow("int8 -> uint8 (max too low)") << make_check();
}

void test_narrow_cast::test_signed() {
    QFETCH(check, check_row);
    QVERIFY(check_row());
}

template <typename T1, typename T2>
void check() {

    constexpr auto min1 = min<T1>;
    constexpr auto max1 = max<T1>;
    constexpr auto min2 = min<T2>;
    constexpr auto max2 = max<T2>;


}


void test_narrow_cast::test_signed_data() {

    QTest::addColumn<check>("check_row");

    QTest::newRow("int8 -> int8 (min)") << make_check(min_i8, min_i8);
    QTest::newRow("int8 -> int8 (max)") << make_check(max_i8, max_i8);

    QTest::newRow("int32 -> int32 (min)") << make_check(min_i32, min_i32);
    QTest::newRow("int32 -> int32 (max)") << make_check(max_i32, max_i32);

    QTest::newRow("int32 -> int8 (min good)") << make_check(std::int32_t{min_i8}, min_i8);
    QTest::newRow("int32 -> int8 (max good)") << make_check(std::int32_t{max_i8}, max_i8);

    QTest::newRow("int32 -> int8 (min too low)") << make_check<std::int8_t>(min_i32);
    QTest::newRow("int32 -> int8 (max too low)") << make_check<std::int8_t>(std::int32_t{min_i8 - 1});

    QTest::newRow("int32 -> int8 (min too high)") << make_check<std::int8_t>(std::int32_t{max_i8 + 1});
    QTest::newRow("int32 -> int8 (max too high)") << make_check<std::int8_t>(max_i32);
}

void test_narrow_cast::test_unsigned() {
    QFETCH(check, check_row);
    QVERIFY(check_row());
}

void test_narrow_cast::test_unsigned_data() {

    QTest::addColumn<check>("check_row");

    QTest::newRow("uint8 -> uint8 (min)") << make_check(min_u8, min_u8);
    QTest::newRow("uint8 -> uint8 (max)") << make_check(max_u8, max_u8);

    QTest::newRow("uint32 -> uint32 (min)") << make_check(min_u32, min_u32);
    QTest::newRow("uint32 -> uint32 (max)") << make_check(max_u32, max_u32);

    QTest::newRow("uint32 -> uint8 (min good)") << make_check(std::uint32_t{min_u8}, min_u8);
    QTest::newRow("uint32 -> uint8 (max good)") << make_check(std::uint32_t{max_u8}, max_u8);

    QTest::newRow("uint32 -> uint8 (min too high)") << make_check<std::uint8_t>(std::uint32_t{max_u8 + 1});
    QTest::newRow("uint32 -> uint8 (max too high)") << make_check<std::uint8_t>(max_u32);
}

QTEST_MAIN(test_narrow_cast)
#include "test_narrow_cast.moc"
