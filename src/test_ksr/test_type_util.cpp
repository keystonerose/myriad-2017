#include "ksr/type_util.hpp"

#include <QObject>
#include <QtTest/QtTest>

#include <cstdint>
#include <cstdlib>
#include <type_traits>

// TODO:HERE
/*
class narrow_cast_row {
public:

    template <typename T>
    explicit narrow_cast_row()
      : m_store{&erased_type::store<T>} {}

private:

    template <typename T>
    void store();

    using store_inst = void (erased_type::*)();
    store_inst m_store;

    std::array<unsigned char, 8> m_storage;
};

class erased_safe_narrowing {
};
*/

class test_type_util : public QObject {
    Q_OBJECT

private slots:
    void test_narrow_cast();
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
