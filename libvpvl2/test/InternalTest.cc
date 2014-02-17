#include "Common.h"
#include "vpvl2/extensions/icu4c/String.h"
#include "vpvl2/internal/MotionHelper.h"
#include "vpvl2/internal/util.h"
#include <limits>

using namespace ::testing;
using namespace vpvl2;
using namespace vpvl2::extensions::icu4c;
using namespace vpvl2::internal;

TEST(InternalTest, Lerp)
{
    ASSERT_EQ(4.0, vpvl2::internal::MotionHelper::lerp(4, 2, 0));
    ASSERT_EQ(2.0, vpvl2::internal::MotionHelper::lerp(4, 2, 1));
    ASSERT_EQ(3.0, vpvl2::internal::MotionHelper::lerp(4, 2, 0.5));
}

TEST(InternalTest, Size32)
{
    QByteArray bytes;
    QBuffer buffer(&bytes);
    QDataStream stream(&buffer);
    quint32 expected = INT_MAX;
    buffer.open(QBuffer::WriteOnly);
    stream.setByteOrder(QDataStream::LittleEndian);
    stream << expected;
    uint8_t *ptr = reinterpret_cast<uint8_t *>(bytes.data());
    size_t rest = 2;
    int actual = 0;
    // rest is not enough to read (2 < 4)
    ASSERT_FALSE(vpvl2::internal::getTyped<int>(ptr, rest, actual));
    ASSERT_EQ(size_t(0), actual);
    ASSERT_EQ(size_t(2), rest);
    rest = sizeof(quint32);
    // rest is now enough to read (4 = 4)
    ASSERT_TRUE(vpvl2::internal::getTyped<int>(ptr, rest, actual));
    ASSERT_EQ(size_t(expected), actual);
    ASSERT_EQ(size_t(0), rest);
}

TEST(InternalTest, ClearAll)
{
    Array<int *> array;
    array.append(new int(1));
    array.append(new int(2));
    array.append(new int(3));
    ASSERT_EQ(3, array.count());
    array.releaseAll();
    ASSERT_EQ(0, array.count());
    Hash<HashString, int*> hash;
    hash.insert(HashString("foo"), new int(1));
    hash.insert(HashString("bar"), new int(2));
    hash.insert(HashString("baz"), new int(3));
    ASSERT_EQ(3, hash.count());
    hash.releaseAll();
    ASSERT_EQ(0, hash.count());
}

TEST(InternalTest, Version)
{
    ASSERT_TRUE(isLibraryVersionCorrect(VPVL2_VERSION));
    ASSERT_FALSE(isLibraryVersionCorrect(
                     VPVL2_MAKE_VERSION(VPVL2_VERSION_MAJOR - 1,
                                        VPVL2_VERSION_COMPAT,
                                        VPVL2_VERSION_MINOR)));
    ASSERT_STREQ(VPVL2_VERSION_STRING, libraryVersionString());
}

TEST(InternalTest, SizeText)
{
    QByteArray bytes;
    QBuffer buffer(&bytes);
    QDataStream stream(&buffer);
    int expected = 4;
    buffer.open(QBuffer::WriteOnly);
    stream.setByteOrder(QDataStream::LittleEndian);
    stream << expected;
    const char textData[] = "test";
    stream.writeRawData(textData, sizeof(textData));
    uint8_t *ptr = reinterpret_cast<uint8_t *>(bytes.data()), *text = 0;
    size_t rest = sizeof(expected) + expected;
    int size = 0;
    ASSERT_TRUE(vpvl2::internal::getText(ptr, rest, text, size));
    ASSERT_EQ(size_t(0), rest);
    ASSERT_EQ(size_t(expected), size);
    ASSERT_TRUE(qstrncmp("test", reinterpret_cast<const char *>(text), expected) == 0);
}

TEST(InternalTest, ReadWriteSignedIndex8)
{
    int8_t expected = std::numeric_limits<int8_t>::min();
    QByteArray bytes;
    bytes.resize(sizeof(expected));
    uint8_t *data = reinterpret_cast<uint8_t *>(bytes.data());
    vpvl2::internal::writeSignedIndex(expected, sizeof(expected), data);
    uint8_t *ptr = reinterpret_cast<uint8_t *>(bytes.data());
    ASSERT_EQ(expected, vpvl2::internal::readSignedIndex(ptr, sizeof(expected)));
}

TEST(InternalTest, ReadWriteSignedIndex16)
{
    int16_t expected = std::numeric_limits<int16_t>::min();
    QByteArray bytes;
    bytes.resize(sizeof(expected));
    uint8_t *data = reinterpret_cast<uint8_t *>(bytes.data());
    vpvl2::internal::writeSignedIndex(expected, sizeof(expected), data);
    uint8_t *ptr = reinterpret_cast<uint8_t *>(bytes.data());
    ASSERT_EQ(expected, vpvl2::internal::readSignedIndex(ptr, sizeof(expected)));
}

TEST(InternalTest, ReadWriteSignedIndex32)
{
    int expected = std::numeric_limits<int>::min();
    QByteArray bytes;
    bytes.resize(sizeof(expected));
    uint8_t *data = reinterpret_cast<uint8_t *>(bytes.data());
    vpvl2::internal::writeSignedIndex(expected, sizeof(expected), data);
    uint8_t *ptr = reinterpret_cast<uint8_t *>(bytes.data());
    ASSERT_EQ(expected, vpvl2::internal::readSignedIndex(ptr, sizeof(expected)));
}

TEST(InternalTest, ReadWriteUnsignedIndex8)
{
    uint8_t expected = std::numeric_limits<uint8_t>::max();
    QByteArray bytes;
    bytes.resize(sizeof(expected));
    uint8_t *data = reinterpret_cast<uint8_t *>(bytes.data());
    vpvl2::internal::writeSignedIndex(expected, sizeof(expected), data);
    uint8_t *ptr = reinterpret_cast<uint8_t *>(bytes.data());
    ASSERT_EQ(expected, vpvl2::internal::readUnsignedIndex(ptr, sizeof(expected)));
}

TEST(InternalTest, ReadWriteUnsignedIndex16)
{
    uint16_t expected = std::numeric_limits<uint16_t>::max();
    QByteArray bytes;
    bytes.resize(sizeof(expected));
    uint8_t *data = reinterpret_cast<uint8_t *>(bytes.data());
    vpvl2::internal::writeSignedIndex(expected, sizeof(expected), data);
    uint8_t *ptr = reinterpret_cast<uint8_t *>(bytes.data());
    ASSERT_EQ(expected, vpvl2::internal::readUnsignedIndex(ptr, sizeof(expected)));
}

TEST(InternalTest, ReadWriteUnsignedIndex32)
{
    int expected = std::numeric_limits<int>::max();
    QByteArray bytes;
    bytes.resize(sizeof(expected));
    uint8_t *data = reinterpret_cast<uint8_t *>(bytes.data());
    vpvl2::internal::writeSignedIndex(expected, sizeof(expected), data);
    uint8_t *ptr = reinterpret_cast<uint8_t *>(bytes.data());
    ASSERT_EQ(expected, vpvl2::internal::readUnsignedIndex(ptr, sizeof(expected)));
}

TEST(InternalTest, SetAndGetPosition)
{
    Vector3 v(0.1, 0.2, 0.3);
    float f[3];
    vpvl2::internal::getPosition(v, f);
    vpvl2::internal::setPosition(f, v);
    ASSERT_EQ(0.1f, v.x());
    ASSERT_EQ(0.2f, v.y());
    ASSERT_EQ(0.3f, btFabs(v.z()));
}

TEST(InternalTest, SetAndGetRotation)
{
    Quaternion q(0.1, 0.2, 0.3, 0.4);
    float f[4];
    vpvl2::internal::getRotation(q, f);
    vpvl2::internal::setRotation(f, q);
    ASSERT_EQ(0.1f, q.x());
    ASSERT_EQ(0.2f, btFabs(q.y()));
    ASSERT_EQ(0.3f, btFabs(q.z()));
    ASSERT_EQ(0.4f, q.w());
}

TEST(InternalTest, WriteNullString)
{
    QByteArray bytes;
    size_t size = vpvl2::internal::estimateSize(0, IString::kUTF8);
    bytes.resize(size);
    uint8_t *data = reinterpret_cast<uint8_t *>(bytes.data());
    vpvl2::internal::writeString(0, IString::kUTF8, data);
    uint8_t *ptr = reinterpret_cast<uint8_t *>(bytes.data());
    ASSERT_EQ(0, vpvl2::internal::readSignedIndex(ptr, sizeof(int)));
}

TEST(InternalTest, WriteNotNullString)
{
    QByteArray bytes;
    String str("Hello World");
    bytes.resize(vpvl2::internal::estimateSize(&str, IString::kUTF8));
    uint8_t *data = reinterpret_cast<uint8_t *>(bytes.data());
    vpvl2::internal::writeString(&str, IString::kUTF8, data);
    uint8_t *ptr = reinterpret_cast<uint8_t *>(bytes.data());
    size_t length = str.length(IString::kUTF8);
    ASSERT_EQ(length, size_t(vpvl2::internal::readSignedIndex(ptr, sizeof(int))));
    ASSERT_EQ(0, qstrncmp(reinterpret_cast<const char *>(str.toByteArray()), reinterpret_cast<const char *>(ptr), length));
}

TEST(InternalTest, EstimateSize)
{
    String str("Hello World");
    ASSERT_EQ(size_t(4), vpvl2::internal::estimateSize(0, IString::kUTF8));
    ASSERT_EQ(size_t(4) + str.length(IString::kUTF8), vpvl2::internal::estimateSize(&str, IString::kUTF8));
}

TEST(InternalTest, SetString)
{
    IString *value = 0;
    vpvl2::internal::setString(0, value);
    ASSERT_EQ(static_cast<IString*>(0), value);
    String str("Hello World");
    vpvl2::internal::setString(&str, value);
    ASSERT_TRUE(value != &str);
    ASSERT_TRUE(value->equals(&str));
    delete value;
}

TEST(InternalTest, SetStringDirect)
{
    IString *value = 0;
    vpvl2::internal::setStringDirect(0, value);
    ASSERT_EQ(static_cast<IString*>(0), value);
    String str("Hello World");
    vpvl2::internal::setStringDirect(&str, value);
    ASSERT_TRUE(value == &str);
    ASSERT_TRUE(value->equals(&str));
}

TEST(InternalTest, ToggleFlag)
{
    uint16_t flag = 0;
    vpvl2::internal::toggleFlag(0x0002, true, flag);
    vpvl2::internal::toggleFlag(0x0010, true, flag);
    vpvl2::internal::toggleFlag(0x0400, true, flag);
    ASSERT_EQ(0x0412, int(flag));
    vpvl2::internal::toggleFlag(0x0002, false, flag);
    ASSERT_EQ(0x0410, int(flag));
    vpvl2::internal::toggleFlag(0x0010, false, flag);
    ASSERT_EQ(0x0400, int(flag));
    vpvl2::internal::toggleFlag(0x0400, false, flag);
    ASSERT_EQ(0x0000, int(flag));
}
