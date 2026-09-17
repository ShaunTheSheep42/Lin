#include "Basic/Char.h"
#include <QtTest/QtTest>

using namespace Lin;

class TestChar : public QObject {
  Q_OBJECT

private slots:
  void testSingleCharNormal() {
    Char c = MakeChar(u'A');
    QCOMPARE(GetChar(c), static_cast<char32_t>(u'A'));
    QVERIFY(!IsSurrogatePair(c));
    QVERIFY(!IsDoubleWidth(c));
    QVERIFY(!IsPlaceHolder(c));
    QVERIFY(!IsEmptyChar(c));
    QCOMPARE(CharToString(c), QString("A"));
  }

  void testSingleCharDoubleWidth() {
    Char c = MakeChar(u'字', true);
    QCOMPARE(GetChar(c), static_cast<char32_t>(u'字'));
    QVERIFY(IsDoubleWidth(c));
    QVERIFY(!IsSurrogatePair(c));
    QVERIFY(!IsPlaceHolder(c));
    QVERIFY(!IsEmptyChar(c));
    QCOMPARE(CharToString(c), QString("字"));
  }

  void testEmptyChar() {
    Char c = MakeChar(u'\0');
    QCOMPARE(GetChar(c), static_cast<char32_t>(0));
    QVERIFY(!IsSurrogatePair(c));
    QVERIFY(!IsDoubleWidth(c));
    QVERIFY(!IsPlaceHolder(c));
    QVERIFY(IsEmptyChar(c));
    QCOMPARE(CharToString(c), QString(QChar(0)));
  }

  void testPlaceHolder() {
    Char c = PlaceHolder;
    QCOMPARE(GetChar(c), static_cast<char32_t>(0));
    QVERIFY(IsPlaceHolder(c));
    QVERIFY(IsEmptyChar(c)); // 占位符也是空码点
    QCOMPARE(CharToString(c), QString(QChar(0)));
  }

  void testSurrogatePairNormal() {
    // 😀 emoji surrogate pair: U+1F600
    char16_t high = 0xD83D;
    char16_t low = 0xDE00;
    Char c = MakeChar(high, low);
    QCOMPARE(GetChar(c), static_cast<char32_t>(0x1F600));
    QVERIFY(IsSurrogatePair(c));
    QVERIFY(!IsEmptyChar(c));
    QVERIFY(!IsDoubleWidth(c));
    QVERIFY(!IsPlaceHolder(c));
    QCOMPARE(CharToString(c), QString("😀"));
  }

  void testSurrogatePairDoubleWidth() {
    // 󱍢  \udb84\udf62
    char16_t high = 0xDB84;
    char16_t low = 0xDF62;

    Char c = MakeChar(high, low, true);
    QCOMPARE(GetChar(c), static_cast<char32_t>(0xF1362));
    QVERIFY(IsSurrogatePair(c));
    QVERIFY(IsDoubleWidth(c));
    QVERIFY(!IsPlaceHolder(c));
    QVERIFY(!IsEmptyChar(c));
    QCOMPARE(CharToString(c), QString::fromUtf8("󱍢"));
  }

  void testStringToCharsAscii() {
    QString s = "ABC";
    Chars chars = StringToChars(s);

    QCOMPARE(chars.size(), 1);
    QCOMPARE(chars[0].size(), 3);
    QCOMPARE(GetChar(chars[0][0]), static_cast<char32_t>('A'));
    QCOMPARE(GetChar(chars[0][1]), static_cast<char32_t>('B'));
    QCOMPARE(GetChar(chars[0][2]), static_cast<char32_t>('C'));
  }
  void testStringToCharsChinese() {
    QString s = "汉字";
    Chars chars = StringToChars(s);

    QCOMPARE(chars.size(), 1);
    QCOMPARE(chars[0].size(), 4); // 汉 + 占位符 + 字 + 占位符
    QCOMPARE(CharToString(chars[0][0]), QString("汉"));
    QVERIFY(IsDoubleWidth(chars[0][0]));
    QVERIFY(IsPlaceHolder(chars[0][1]));
    QCOMPARE(CharToString(chars[0][2]), QString("字"));
    QVERIFY(IsDoubleWidth(chars[0][2]));
    QVERIFY(IsPlaceHolder(chars[0][3]));
  }

  void testStringToCharsEmoji() {
    QString s = "😀";
    Chars chars = StringToChars(s);

    QCOMPARE(chars.size(), 1);
    QCOMPARE(chars[0].size(), 2); // 😀 + 占位符
    QCOMPARE(GetChar(chars[0][0]), static_cast<char32_t>(0x1F600));
    QVERIFY(IsSurrogatePair(chars[0][0]));
    QVERIFY(IsDoubleWidth(chars[0][0]));
    QVERIFY(IsPlaceHolder(chars[0][1]));
  }

  void testStringToCharsMultiLine() {
    QString s = "AB\nCD";
    Chars chars = StringToChars(s);

    QCOMPARE(chars.size(), 2);
    QCOMPARE(chars[0].size(), 2);
    QCOMPARE(chars[1].size(), 2);
    QCOMPARE(CharToString(chars[0][0]), QString("A"));
    QCOMPARE(CharToString(chars[0][1]), QString("B"));
    QCOMPARE(CharToString(chars[1][0]), QString("C"));
    QCOMPARE(CharToString(chars[1][1]), QString("D"));
  }
};

QTEST_MAIN(TestChar)
#include "Char.moc"
