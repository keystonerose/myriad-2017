#define KSR_THROW_ON_ASSERT
#include "ksr/type_util.hpp"

#include <QObject>
#include <QtTest/QtTest>

#include <cstdint>
#include <cstdlib>
#include <function>
#include <type_traits>

// TODO:HERE

template <typename From, typename To>
std::function<narrow_cast_result()> narrow_cast_case(const From from, const To to) {
    return [from] {

        try {
            ksr::narrow_cast<To>(from);
        } catch (const ksr::logic_error &) {
            return true;
        }

        return false;
    };
}

class narrow_cast_case {
public:

    template <typename From, typename To>
    explicit narrow_cast_row(const From from, const To to)
      : m_from{}, m_to{} {}

    bool throws() const {
        return this->*m_throws();
    }

private:

    template <typename From, typename To>
    bool throws_impl() const {

        const auto from = std::any_cast<From>(m_from);
        const auto to = std::any_cast<To>(m_to);


    }

    using throws_inst = bool (narrow_cast_case::*)();
    throws_inst m_throws;

    std::any m_from;
    std::any m_to;
};

class test_narrow_cast : public QObject {
    Q_OBJECT

private:

    struct result {
        bool expected_output;
        bool expected_throws;
    };

private slots:
    void test();
};

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

void test_type_util::test_narrow_cast() {
    QCOMPARE(ksr::narrow_cast<std::uint8_t>(std::int32_t{0xff}), std::uint8_t{0xff});
    QCOMPARE(ksr::narrow_cast<std::uint16_t>(std::int32_t{0xffff}), std::uint16_t{0xffff});
    QCOMPARE(ksr::narrow_cast<unscoped_enum_uint8>(scoped_enum::item), unscoped_item);
}

QTEST_MAIN(test_type_util)
#include "test_type_util.moc"
