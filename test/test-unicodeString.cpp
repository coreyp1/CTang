/**
 * @file
 *
 * Contains tests for the Tang::UnicodeString class.
 */

#include <gtest/gtest.h>
#include <cutil/memory.h>
#include <unicode/uclean.h>
#include <iostream>
#include <tang/unicodeString.h>

using namespace std;

/*
 * Helper Macro for testing the same type of Unicode String Render procedures
 * using different string pairs.
 */
#define DO_ALL_TEST(SOURCE, EXPECTED, TYPE) \
  { \
    gcu_memory_reset_counts(); \
    auto s = gta_unicode_string_create(SOURCE, strlen(SOURCE), TYPE); \
    EXPECT_NE(nullptr, s); \
    GTA_Unicode_Rendered_String rendered = gta_unicode_string_render(s); \
    EXPECT_TRUE(rendered.buffer); \
    EXPECT_EQ(EXPECTED, string{rendered.buffer}); \
    EXPECT_EQ(strlen(EXPECTED), rendered.length); \
    gcu_free(rendered.buffer); \
    gta_unicode_string_destroy(s); \
    ASSERT_EQ(gcu_get_alloc_count(), gcu_get_free_count()); \
  }

/*
TEST(Core, Unescape) {
  EXPECT_EQ(unescape(""), R"()");
  EXPECT_EQ(unescape("bacdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789!@#$%^&*()`~[]{}|;:',.<>? \""), R"(bacdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789!@#$%^&*()`~[]{}|;:',.<>? \")");
  EXPECT_EQ(unescape("\""), R"(\")");
  EXPECT_EQ(unescape("\a"), R"(\a)");
  EXPECT_EQ(unescape("\b"), R"(\b)");
  EXPECT_EQ(unescape("\f"), R"(\f)");
  EXPECT_EQ(unescape("\n"), R"(\n)");
  EXPECT_EQ(unescape("\r"), R"(\r)");
  EXPECT_EQ(unescape("\t"), R"(\t)");
  EXPECT_EQ(unescape("\v"), R"(\v)");
  EXPECT_EQ(unescape("\xAA"), R"(\xAA)");
  EXPECT_EQ(unescape("\xFF"), R"(\xFF)");
  EXPECT_EQ(unescape(string("\x00", 1)), R"(\x00)");
  EXPECT_EQ(unescape("\x01"), R"(\x01)");
}

TEST(Core, HtmlEscape) {
  UnicodeString s{"$\xF0\x9F\x8F\xB4\xF3\xA0\x81\xA7\xF3\xA0\x81\xA2\xF3\xA0\x81\xB3\xF3\xA0\x81\xA3\xF3\xA0\x81\xB4\xF3\xA0\x81\xBF."};
  EXPECT_EQ(htmlEscape(""), R"()");
  EXPECT_EQ(htmlEscape("<"), "&lt;");
  EXPECT_EQ(htmlEscape(">"), "&gt;");
  EXPECT_EQ(htmlEscape("&"), "&amp;");
  EXPECT_EQ(htmlEscape("\""), "&quot;");
  EXPECT_EQ(htmlEscape("'"), "&apos;");
  EXPECT_EQ(htmlEscape("bacdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789!@#$%^&*()`~[]{}|;:',.<>? \""), R"(bacdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789!@#$%^&amp;*()`~[]{}|;:&apos;,.&lt;&gt;? &quot;)");
  EXPECT_EQ(htmlEscape("\a"), "\a");
  EXPECT_EQ(htmlEscape("\b"), "\b");
  EXPECT_EQ(htmlEscape("\f"), "\f");
  EXPECT_EQ(htmlEscape("\n"), "\n");
  EXPECT_EQ(htmlEscape("\r"), "\r");
  EXPECT_EQ(htmlEscape("\t"), "\t");
  EXPECT_EQ(htmlEscape("\v"), "\v");
  EXPECT_EQ(htmlEscape("\xAA"), "\xAA");
  EXPECT_EQ(htmlEscape("\xFF"), "\xFF");
  EXPECT_EQ(htmlEscape(string("\x00", 1)), "\x00");
  EXPECT_EQ(htmlEscape("\x01"), "\x01");
  EXPECT_EQ(htmlEscape(s), "$\xF0\x9F\x8F\xB4\xF3\xA0\x81\xA7\xF3\xA0\x81\xA2\xF3\xA0\x81\xB3\xF3\xA0\x81\xA3\xF3\xA0\x81\xB4\xF3\xA0\x81\xBF.");
}

TEST(Core, HtmlEscapeAscii) {
  UnicodeString s{"$\xF0\x9F\x8F\xB4\xF3\xA0\x81\xA7\xF3\xA0\x81\xA2\xF3\xA0\x81\xB3\xF3\xA0\x81\xA3\xF3\xA0\x81\xB4\xF3\xA0\x81\xBF."};
  EXPECT_EQ(htmlEscapeAscii(""), R"()");
  EXPECT_EQ(htmlEscapeAscii("<"), "&lt;");
  EXPECT_EQ(htmlEscapeAscii(">"), "&gt;");
  EXPECT_EQ(htmlEscapeAscii("&"), "&amp;");
  EXPECT_EQ(htmlEscapeAscii("\""), "&quot;");
  EXPECT_EQ(htmlEscapeAscii("'"), "&apos;");
  EXPECT_EQ(htmlEscapeAscii("bacdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789!@#$%^&*()`~[]{}|;:',.<>? \""), R"(bacdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789&excl;&commat;&num;&dollar;&percnt;&Hat;&amp;&ast;&lpar;&rpar;&grave;~&lsqb;&rsqb;&lcub;&rcub;&verbar;&semi;&colon;&apos;&comma;&period;&lt;&gt;&quest; &quot;)");
  EXPECT_EQ(htmlEscapeAscii("\a"), "&#x7;");
  EXPECT_EQ(htmlEscapeAscii("\b"), "&#x8;");
  EXPECT_EQ(htmlEscapeAscii("\f"), "&#xC;");
  EXPECT_EQ(htmlEscapeAscii("\n"), "&NewLine;");
  EXPECT_EQ(htmlEscapeAscii("\r"), "&#xD;");
  EXPECT_EQ(htmlEscapeAscii("\t"), "&Tab;");
  EXPECT_EQ(htmlEscapeAscii("\v"), "&#xB;");
  EXPECT_EQ(htmlEscapeAscii("\xAA"), "&xFFFD;");
  EXPECT_EQ(htmlEscapeAscii("\xFF"), "&xFFFD;");
  EXPECT_EQ(htmlEscapeAscii(string("\x00", 1)), "&#x0;");
  EXPECT_EQ(htmlEscapeAscii("\x01"), "&#x1;");
  EXPECT_EQ(htmlEscapeAscii(s), "&dollar;&#x1F3F4;&#xE0067;&#xE0062;&#xE0073;&#xE0063;&#xE0074;&#xE007F;&period;");
  // 2-byte Unicode character, Pound Sign (currency)
  EXPECT_EQ(htmlEscapeAscii("\xc2\xa3"), "&pound;");
  // 3-byte Unicode character, Japanese word "yen" (currency)
  EXPECT_EQ(htmlEscapeAscii("\xe5\x86\x86"), "&#x5186;");
}

TEST(UnicodeString, SubString) {
  {
    // Testing an empty string.
    UnicodeString s{""};
    EXPECT_EQ(s.substr(0, 0), "");
    EXPECT_EQ(s.substr(0, 1), "");
  }
  {
    // Testing a string with one character.
    UnicodeString s{"a"};
    EXPECT_EQ(s.substr(0, 0), "");
    EXPECT_EQ(s.substr(0, 1), "a");
    EXPECT_EQ(s.substr(3, 1), "");
  }
  {
    // Testing multiple graphemes, each made of a single unicode codepoint.
    UnicodeString s{"$\u00A3\u5186"};
    EXPECT_EQ(s.substr(0, 1), "$");
    EXPECT_EQ(s.substr(1, 1), "\u00A3");
    EXPECT_EQ(s.substr(0, 2), "$\u00A3");
    EXPECT_EQ(s.substr(2, 1), "\u5186");
    EXPECT_EQ(s.substr(1, 2), "\u00A3\u5186");
    EXPECT_EQ(s.substr(0, 3), "$\u00A3\u5186");
  }
  {
    // Testing multiple graphemes, one of which is is made of multiple
    // codepoints.
    // The long string of hex values is a UTF-8 encoding of the Scottish Flag.
    UnicodeString s{"$\xF0\x9F\x8F\xB4\xF3\xA0\x81\xA7\xF3\xA0\x81\xA2\xF3\xA0\x81\xB3\xF3\xA0\x81\xA3\xF3\xA0\x81\xB4\xF3\xA0\x81\xBF."};
    EXPECT_EQ(s.substr(0, 0), "");
    EXPECT_EQ(s.substr(0, 1), "$");
    EXPECT_EQ(s.substr(1, 1), "\xF0\x9F\x8F\xB4\xF3\xA0\x81\xA7\xF3\xA0\x81\xA2\xF3\xA0\x81\xB3\xF3\xA0\x81\xA3\xF3\xA0\x81\xB4\xF3\xA0\x81\xBF");
    EXPECT_EQ(s.substr(2, 1), ".");
    EXPECT_EQ(s.substr(0, 2), "$\xF0\x9F\x8F\xB4\xF3\xA0\x81\xA7\xF3\xA0\x81\xA2\xF3\xA0\x81\xB3\xF3\xA0\x81\xA3\xF3\xA0\x81\xB4\xF3\xA0\x81\xBF");
    EXPECT_EQ(s.substr(1, 2), "\xF0\x9F\x8F\xB4\xF3\xA0\x81\xA7\xF3\xA0\x81\xA2\xF3\xA0\x81\xB3\xF3\xA0\x81\xA3\xF3\xA0\x81\xB4\xF3\xA0\x81\xBF.");
    EXPECT_EQ(s.substr(0, 3), "$\xF0\x9F\x8F\xB4\xF3\xA0\x81\xA7\xF3\xA0\x81\xA2\xF3\xA0\x81\xB3\xF3\xA0\x81\xA3\xF3\xA0\x81\xB4\xF3\xA0\x81\xBF.");
    EXPECT_EQ(s.substr(0, 30), "$\xF0\x9F\x8F\xB4\xF3\xA0\x81\xA7\xF3\xA0\x81\xA2\xF3\xA0\x81\xB3\xF3\xA0\x81\xA3\xF3\xA0\x81\xB4\xF3\xA0\x81\xBF.");
    EXPECT_EQ(s.substr(15, 0), "");
  }
}

TEST(UnicodeString, Types) {
  {
    // Default (trusted) string.
    UnicodeString s{"&"};
    EXPECT_EQ(s.render(), "&");
  }
  {
    // Untrusted string.
    UnicodeString s{"&"};
    s.setUntrusted();
    EXPECT_EQ(s.render(), "&amp;");
  }
  {
    // Trusted string.
    UnicodeString s{"<h1>"};
    EXPECT_EQ(s.render(), "<h1>");
  }
  {
    // Untrusted string.
    UnicodeString s{"<h1>"};
    s.setUntrusted();
    EXPECT_EQ(s.render(), "&lt;h1&gt;");
  }
  {
    // Trusted string.
    UnicodeString s{"<h1>Hello\nWorld!</h1>"};
    EXPECT_EQ(s.renderAscii(), "<h1>Hello&NewLine;World!</h1>");
  }
  {
    // Untrusted string.
    UnicodeString s{"<h1>Hello\nWorld!</h1>"};
    s.setUntrusted();
    EXPECT_EQ(s.renderAscii(), "&lt;h1&gt;Hello&NewLine;World&excl;&lt;&sol;h1&gt;");
  }
}

TEST(Core, PercentEncode) {
  {
    // Space becomes a plus.
    UnicodeString s{"abc def"};
    EXPECT_EQ(percentEncode(s), "abc+def");
  }
  {
    // Plus is percent encoded.
    UnicodeString s{"abc+def"};
    EXPECT_EQ(percentEncode(s), "abc%2Bdef");
  }
  {
    // Plus is percent encoded.
    UnicodeString s{"ABC\nDEF"};
    EXPECT_EQ(percentEncode(s), "ABC%0ADEF");
  }
  {
    // Unicode Character is percent encoded.
    // Notice the "$" is also encoded, but the "." did not need to be encoded.
    UnicodeString s{"$\xF0\x9F\x8F\xB4\xF3\xA0\x81\xA7\xF3\xA0\x81\xA2\xF3\xA0\x81\xB3\xF3\xA0\x81\xA3\xF3\xA0\x81\xB4\xF3\xA0\x81\xBF."};
    EXPECT_EQ(percentEncode(s), "%24%F0%9F%8F%B4%F3%A0%81%A7%F3%A0%81%A2%F3%A0%81%B3%F3%A0%81%A3%F3%A0%81%B4%F3%A0%81%BF.");
  }
}
*/

TEST(UnicodeString, ByteAndGraphemeCount) {
  {
    // Testing an empty string.
    gcu_memory_reset_counts();
    auto s = gta_unicode_string_create("", 0, GTA_UNICODE_STRING_TYPE_TRUSTED);
    EXPECT_NE(nullptr, s);
    EXPECT_EQ(0, s->byte_length);
    EXPECT_EQ(0, s->grapheme_length);
    gta_unicode_string_destroy(s);
    ASSERT_EQ(gcu_get_alloc_count(), gcu_get_free_count());
  }
  {
    // Testing a string with ascii characters.
    gcu_memory_reset_counts();
    auto s = gta_unicode_string_create("abc", 3, GTA_UNICODE_STRING_TYPE_TRUSTED);
    EXPECT_NE(nullptr, s);
    EXPECT_EQ(3, s->byte_length);
    EXPECT_EQ(3, s->grapheme_length);
    gta_unicode_string_destroy(s);
    ASSERT_EQ(gcu_get_alloc_count(), gcu_get_free_count());
  }
  {
    // Testing a string with a single unicode character.
    gcu_memory_reset_counts();
    auto s = gta_unicode_string_create("\u00A3", 2, GTA_UNICODE_STRING_TYPE_TRUSTED);
    EXPECT_NE(nullptr, s);
    EXPECT_EQ(2, s->byte_length);
    EXPECT_EQ(1, s->grapheme_length);
    gta_unicode_string_destroy(s);
    ASSERT_EQ(gcu_get_alloc_count(), gcu_get_free_count());
  }
  {
    // Testing a string with multiple unicode characters.
    gcu_memory_reset_counts();
    const char * str = "\u00A3\u5186";
    auto s = gta_unicode_string_create(str, strlen(str), GTA_UNICODE_STRING_TYPE_TRUSTED);
    EXPECT_NE(nullptr, s);
    EXPECT_EQ(strlen(str), s->byte_length);
    EXPECT_EQ(2, s->grapheme_length);
    gta_unicode_string_destroy(s);
    ASSERT_EQ(gcu_get_alloc_count(), gcu_get_free_count());
  }
  {
    gcu_memory_reset_counts();
    // Testing a string with multiple graphemes, one of which is is made of multiple
    // codepoints.
    // The long string of hex values is a UTF-8 encoding of the Scottish Flag.
    const char * str = "$\xF0\x9F\x8F\xB4\xF3\xA0\x81\xA7\xF3\xA0\x81\xA2\xF3\xA0\x81\xB3\xF3\xA0\x81\xA3\xF3\xA0\x81\xB4\xF3\xA0\x81\xBF.";
    auto s = gta_unicode_string_create(str, strlen(str), GTA_UNICODE_STRING_TYPE_TRUSTED);
    EXPECT_NE(nullptr, s);
    EXPECT_EQ(strlen(str), s->byte_length);
    EXPECT_EQ(3, s->grapheme_length);
    gta_unicode_string_destroy(s);
    ASSERT_EQ(gcu_get_alloc_count(), gcu_get_free_count());
  }
}

TEST(UnicodeString, Substring) {
  {
    // Testing an empty string.
    gcu_memory_reset_counts();
    size_t alloc_running_count = 0;
    size_t free_running_count = 0;
    auto s = gta_unicode_string_create("", 0, GTA_UNICODE_STRING_TYPE_TRUSTED);
    EXPECT_NE(nullptr, s);
    {
      // Requesting a substring of length 0.
      alloc_running_count += gcu_get_alloc_count();
      free_running_count += gcu_get_free_count();
      gcu_memory_reset_counts();
      auto s2 = gta_unicode_string_substring(s, 0, 0);
      EXPECT_NE(nullptr, s2);
      EXPECT_EQ(0, s2->byte_length);
      EXPECT_EQ(0, s2->grapheme_length);
      EXPECT_EQ(1, s2->string_type->count);
      gta_unicode_string_destroy(s2);
      ASSERT_EQ(gcu_get_alloc_count(), gcu_get_free_count());
    }
    {
      // Requesting a substring with length past the end of the string.
      alloc_running_count += gcu_get_alloc_count();
      free_running_count += gcu_get_free_count();
      gcu_memory_reset_counts();
      auto s2 = gta_unicode_string_substring(s, 0, 1);
      EXPECT_NE(nullptr, s2);
      EXPECT_EQ(0, s2->byte_length);
      EXPECT_EQ(0, s2->grapheme_length);
      EXPECT_EQ(1, s2->string_type->count);
      gta_unicode_string_destroy(s2);
      ASSERT_EQ(gcu_get_alloc_count(), gcu_get_free_count());
    }
    {
      // Requesting a substring whose start is past the end of the string.
      alloc_running_count += gcu_get_alloc_count();
      free_running_count += gcu_get_free_count();
      gcu_memory_reset_counts();
      auto s2 = gta_unicode_string_substring(s, 1, 1);
      EXPECT_NE(nullptr, s2);
      EXPECT_EQ(0, s2->byte_length);
      EXPECT_EQ(0, s2->grapheme_length);
      EXPECT_EQ(1, s2->string_type->count);
      gta_unicode_string_destroy(s2);
      ASSERT_EQ(gcu_get_alloc_count(), gcu_get_free_count());
    }
    gta_unicode_string_destroy(s);
    alloc_running_count += gcu_get_alloc_count();
    free_running_count += gcu_get_free_count();
    ASSERT_EQ(alloc_running_count, free_running_count);
  }
  {
    // Testing a string with ascii characters.
    gcu_memory_reset_counts();
    size_t alloc_running_count = 0;
    size_t free_running_count = 0;
    auto s = gta_unicode_string_create("abc", 3, GTA_UNICODE_STRING_TYPE_TRUSTED);
    EXPECT_NE(nullptr, s);
    {
      // Requesting a substring of length 0.
      alloc_running_count += gcu_get_alloc_count();
      free_running_count += gcu_get_free_count();
      gcu_memory_reset_counts();
      auto s2 = gta_unicode_string_substring(s, 0, 0);
      EXPECT_NE(nullptr, s2);
      EXPECT_EQ(0, s2->byte_length);
      EXPECT_EQ(0, s2->grapheme_length);
      EXPECT_EQ(1, s2->string_type->count);
      EXPECT_EQ(string{}, string{s2->buffer});
      gta_unicode_string_destroy(s2);
      ASSERT_EQ(gcu_get_alloc_count(), gcu_get_free_count());
    }
    {
      // Requesting a substring with length past the end of the string.
      alloc_running_count += gcu_get_alloc_count();
      free_running_count += gcu_get_free_count();
      gcu_memory_reset_counts();
      auto s2 = gta_unicode_string_substring(s, 0, 4);
      EXPECT_NE(nullptr, s2);
      EXPECT_EQ(3, s2->byte_length);
      EXPECT_EQ(3, s2->grapheme_length);
      EXPECT_EQ(1, s2->string_type->count);
      EXPECT_EQ(string{"abc"}, string{s2->buffer});
      gta_unicode_string_destroy(s2);
      ASSERT_EQ(gcu_get_alloc_count(), gcu_get_free_count());
    }
    {
      // Requesting a substring whose start is past the end of the string.
      alloc_running_count += gcu_get_alloc_count();
      free_running_count += gcu_get_free_count();
      gcu_memory_reset_counts();
      auto s2 = gta_unicode_string_substring(s, 4, 1);
      EXPECT_NE(nullptr, s2);
      EXPECT_EQ(0, s2->byte_length);
      EXPECT_EQ(0, s2->grapheme_length);
      EXPECT_EQ(1, s2->string_type->count);
      EXPECT_EQ(string{}, string{s2->buffer});
      gta_unicode_string_destroy(s2);
      ASSERT_EQ(gcu_get_alloc_count(), gcu_get_free_count());
    }
    {
      // Requesting a substring from the middle of the string.
      alloc_running_count += gcu_get_alloc_count();
      free_running_count += gcu_get_free_count();
      gcu_memory_reset_counts();
      auto s2 = gta_unicode_string_substring(s, 1, 1);
      EXPECT_NE(nullptr, s2);
      EXPECT_EQ(1, s2->byte_length);
      EXPECT_EQ(1, s2->grapheme_length);
      EXPECT_EQ(1, s2->string_type->count);
      EXPECT_EQ(string{"b"}, string{s2->buffer});
      gta_unicode_string_destroy(s2);
      ASSERT_EQ(gcu_get_alloc_count(), gcu_get_free_count());
    }
    gta_unicode_string_destroy(s);
    alloc_running_count += gcu_get_alloc_count();
    free_running_count += gcu_get_free_count();
    ASSERT_EQ(alloc_running_count, free_running_count);
  }
  {
    // Testing a string with multiple graphemes, one of which is is made of multiple
    // codepoints.
    // The long string of hex values is a UTF-8 encoding of the Scottish Flag.
    gcu_memory_reset_counts();
    size_t alloc_running_count = 0;
    size_t free_running_count = 0;
    const char * str = "$\xF0\x9F\x8F\xB4\xF3\xA0\x81\xA7\xF3\xA0\x81\xA2\xF3\xA0\x81\xB3\xF3\xA0\x81\xA3\xF3\xA0\x81\xB4\xF3\xA0\x81\xBF.";
    auto s = gta_unicode_string_create(str, strlen(str), GTA_UNICODE_STRING_TYPE_TRUSTED);
    EXPECT_NE(nullptr, s);
    {
      // Requesting a substring of the first two graphemes.
      alloc_running_count += gcu_get_alloc_count();
      free_running_count += gcu_get_free_count();
      gcu_memory_reset_counts();
      auto s2 = gta_unicode_string_substring(s, 0, 2);
      EXPECT_NE(nullptr, s2);
      EXPECT_EQ(29, s2->byte_length);
      EXPECT_EQ(2, s2->grapheme_length);
      EXPECT_EQ(1, s2->string_type->count);
      EXPECT_EQ(string{"$\xF0\x9F\x8F\xB4\xF3\xA0\x81\xA7\xF3\xA0\x81\xA2\xF3\xA0\x81\xB3\xF3\xA0\x81\xA3\xF3\xA0\x81\xB4\xF3\xA0\x81\xBF"}, string{s2->buffer});
      gta_unicode_string_destroy(s2);
      ASSERT_EQ(gcu_get_alloc_count(), gcu_get_free_count());
    }
    {
      // Requesting a substring of the last two graphemes.
      alloc_running_count += gcu_get_alloc_count();
      free_running_count += gcu_get_free_count();
      gcu_memory_reset_counts();
      auto s2 = gta_unicode_string_substring(s, 1, 2);
      EXPECT_NE(nullptr, s2);
      EXPECT_EQ(29, s2->byte_length);
      EXPECT_EQ(2, s2->grapheme_length);
      EXPECT_EQ(1, s2->string_type->count);
      EXPECT_EQ(string{"\xF0\x9F\x8F\xB4\xF3\xA0\x81\xA7\xF3\xA0\x81\xA2\xF3\xA0\x81\xB3\xF3\xA0\x81\xA3\xF3\xA0\x81\xB4\xF3\xA0\x81\xBF."}, string{s2->buffer});
      gta_unicode_string_destroy(s2);
      ASSERT_EQ(gcu_get_alloc_count(), gcu_get_free_count());
    }
    {
      // Requesting a substring of the middle grapheme.
      alloc_running_count += gcu_get_alloc_count();
      free_running_count += gcu_get_free_count();
      gcu_memory_reset_counts();
      auto s2 = gta_unicode_string_substring(s, 1, 1);
      EXPECT_NE(nullptr, s2);
      EXPECT_EQ(28, s2->byte_length);
      EXPECT_EQ(1, s2->grapheme_length);
      EXPECT_EQ(1, s2->string_type->count);
      EXPECT_EQ(string{"\xF0\x9F\x8F\xB4\xF3\xA0\x81\xA7\xF3\xA0\x81\xA2\xF3\xA0\x81\xB3\xF3\xA0\x81\xA3\xF3\xA0\x81\xB4\xF3\xA0\x81\xBF"}, string{s2->buffer});
      gta_unicode_string_destroy(s2);
      ASSERT_EQ(gcu_get_alloc_count(), gcu_get_free_count());
    }
    gta_unicode_string_destroy(s);
    alloc_running_count += gcu_get_alloc_count();
    free_running_count += gcu_get_free_count();
    ASSERT_EQ(alloc_running_count, free_running_count);
  }
}

TEST(UnicodeString, Concat) {
  gcu_memory_reset_counts();
  size_t alloc_running_count = 0;
  size_t free_running_count = 0;
  // An empty string.
  auto s1 = gta_unicode_string_create("", 0, GTA_UNICODE_STRING_TYPE_HTML);
  EXPECT_NE(nullptr, s1);
  // A string with ascii characters.
  auto s2 = gta_unicode_string_create("abc", 3, GTA_UNICODE_STRING_TYPE_TRUSTED);
  EXPECT_NE(nullptr, s2);
  // A string with a single unicode character.
  auto s3 = gta_unicode_string_create("\u00A3", 2, GTA_UNICODE_STRING_TYPE_TRUSTED);
  EXPECT_NE(nullptr, s3);
  // A string with multiple graphemes, one of which is is made of multiple
  // codepoints.
  // The long string of hex values is a UTF-8 encoding of the Scottish Flag.
  const char * str = "$\xF0\x9F\x8F\xB4\xF3\xA0\x81\xA7\xF3\xA0\x81\xA2\xF3\xA0\x81\xB3\xF3\xA0\x81\xA3\xF3\xA0\x81\xB4\xF3\xA0\x81\xBF.";
  auto s4 = gta_unicode_string_create(str, strlen(str), GTA_UNICODE_STRING_TYPE_HTML);
  EXPECT_NE(nullptr, s4);
  {
    // Concatenating an empty string with an empty string.
    alloc_running_count += gcu_get_alloc_count();
    free_running_count += gcu_get_free_count();
    gcu_memory_reset_counts();
    auto s5 = gta_unicode_string_concat(s1, s1);
    EXPECT_NE(nullptr, s5);
    EXPECT_EQ(0, s5->byte_length);
    EXPECT_EQ(0, s5->grapheme_length);
    EXPECT_EQ(1, s5->string_type->count);
    EXPECT_EQ(string{}, string{s5->buffer});
    EXPECT_EQ((uint32_t)GTA_UNICODE_STRING_TYPE_HTML, GTA_UC_GET_TYPE_FROM_TYPE_OFFSET_PAIR(s5->string_type->data[0]));
    gta_unicode_string_destroy(s5);
    ASSERT_EQ(gcu_get_alloc_count(), gcu_get_free_count());
  }
  {
    // Concatenating an empty string with a string with ascii characters.
    // The strings have different types.
    alloc_running_count += gcu_get_alloc_count();
    free_running_count += gcu_get_free_count();
    gcu_memory_reset_counts();
    auto s5 = gta_unicode_string_concat(s1, s2);
    char expected[] = "abc";
    EXPECT_NE(nullptr, s5);
    EXPECT_EQ(strlen(expected), s5->byte_length);
    EXPECT_EQ(3, s5->grapheme_length);
    EXPECT_EQ(1, s5->string_type->count);
    EXPECT_EQ(string{"abc"}, string{s5->buffer});
    EXPECT_EQ((uint32_t)GTA_UNICODE_STRING_TYPE_TRUSTED, GTA_UC_GET_TYPE_FROM_TYPE_OFFSET_PAIR(s5->string_type->data[0]));
    gta_unicode_string_destroy(s5);
    ASSERT_EQ(gcu_get_alloc_count(), gcu_get_free_count());
  }
  {
    // Concatenating a string with ascii characters with an empty string.
    // The strings have different types.
    alloc_running_count += gcu_get_alloc_count();
    free_running_count += gcu_get_free_count();
    gcu_memory_reset_counts();
    auto s5 = gta_unicode_string_concat(s2, s1);
    char expected[] = "abc";
    EXPECT_NE(nullptr, s5);
    EXPECT_EQ(strlen(expected), s5->byte_length);
    EXPECT_EQ(3, s5->grapheme_length);
    EXPECT_EQ(1, s5->string_type->count);
    EXPECT_EQ(string{expected}, string{s5->buffer});
    EXPECT_EQ((uint32_t)GTA_UNICODE_STRING_TYPE_TRUSTED, GTA_UC_GET_TYPE_FROM_TYPE_OFFSET_PAIR(s5->string_type->data[0]));
    gta_unicode_string_destroy(s5);
    ASSERT_EQ(gcu_get_alloc_count(), gcu_get_free_count());
  }
  {
    // Concatenating a string with ascii characters with a string with a single
    // unicode character.
    // The strings have the same type.
    alloc_running_count += gcu_get_alloc_count();
    free_running_count += gcu_get_free_count();
    gcu_memory_reset_counts();
    auto s5 = gta_unicode_string_concat(s2, s3);
    char expected[] = "abc\u00A3";
    EXPECT_NE(nullptr, s5);
    EXPECT_EQ(strlen(expected), s5->byte_length);
    EXPECT_EQ(4, s5->grapheme_length);
    EXPECT_EQ(1, s5->string_type->count);
    EXPECT_EQ(string{expected}, string{s5->buffer});
    EXPECT_EQ((uint32_t)GTA_UNICODE_STRING_TYPE_TRUSTED, GTA_UC_GET_TYPE_FROM_TYPE_OFFSET_PAIR(s5->string_type->data[0]));
    gta_unicode_string_destroy(s5);
    ASSERT_EQ(gcu_get_alloc_count(), gcu_get_free_count());
  }
  {
    // Concatenating two non-empty strings with multiple graphemes and
    // different types.
    alloc_running_count += gcu_get_alloc_count();
    free_running_count += gcu_get_free_count();
    gcu_memory_reset_counts();
    auto s5 = gta_unicode_string_concat(s2, s4);
    char expected[] = "abc$\xF0\x9F\x8F\xB4\xF3\xA0\x81\xA7\xF3\xA0\x81\xA2\xF3\xA0\x81\xB3\xF3\xA0\x81\xA3\xF3\xA0\x81\xB4\xF3\xA0\x81\xBF.";
    EXPECT_NE(nullptr, s5);
    EXPECT_EQ(strlen(expected), s5->byte_length);
    EXPECT_EQ(6, s5->grapheme_length);
    EXPECT_EQ(2, s5->string_type->count);
    EXPECT_EQ(string{expected}, string{s5->buffer});
    EXPECT_EQ((uint32_t)GTA_UNICODE_STRING_TYPE_TRUSTED, GTA_UC_GET_TYPE_FROM_TYPE_OFFSET_PAIR(s5->string_type->data[0]));
    EXPECT_EQ((uint32_t)GTA_UNICODE_STRING_TYPE_HTML, GTA_UC_GET_TYPE_FROM_TYPE_OFFSET_PAIR(s5->string_type->data[1]));
    gta_unicode_string_destroy(s5);
    ASSERT_EQ(gcu_get_alloc_count(), gcu_get_free_count());
  }
  {
    // Concatenate strings multiple times, sometimes with the same type, and
    // sometimes with different types.
    alloc_running_count += gcu_get_alloc_count();
    free_running_count += gcu_get_free_count();
    gcu_memory_reset_counts();
    char expected[] = "abc$\xF0\x9F\x8F\xB4\xF3\xA0\x81\xA7\xF3\xA0\x81\xA2\xF3\xA0\x81\xB3\xF3\xA0\x81\xA3\xF3\xA0\x81\xB4\xF3\xA0\x81\xBF.\u00A3abc$\xF0\x9F\x8F\xB4\xF3\xA0\x81\xA7\xF3\xA0\x81\xA2\xF3\xA0\x81\xB3\xF3\xA0\x81\xA3\xF3\xA0\x81\xB4\xF3\xA0\x81\xBF.";
    auto s5 = gta_unicode_string_concat(s2, s4);
    EXPECT_NE(nullptr, s5);
    auto s6 = gta_unicode_string_concat(s5, s3);
    EXPECT_NE(nullptr, s6);
    auto s7 = gta_unicode_string_concat(s6, s2);
    EXPECT_NE(nullptr, s7);
    auto s8 = gta_unicode_string_concat(s7, s4);
    EXPECT_NE(nullptr, s8);
    EXPECT_EQ(strlen(expected), s8->byte_length);
    EXPECT_EQ(13, s8->grapheme_length);
    EXPECT_EQ(4, s8->string_type->count);
    EXPECT_EQ(string{expected}, string{s8->buffer});
    EXPECT_EQ((uint32_t)GTA_UNICODE_STRING_TYPE_TRUSTED, GTA_UC_GET_TYPE_FROM_TYPE_OFFSET_PAIR(s8->string_type->data[0]));
    EXPECT_EQ((uint32_t)GTA_UNICODE_STRING_TYPE_HTML, GTA_UC_GET_TYPE_FROM_TYPE_OFFSET_PAIR(s8->string_type->data[1]));
    EXPECT_EQ((uint32_t)GTA_UNICODE_STRING_TYPE_TRUSTED, GTA_UC_GET_TYPE_FROM_TYPE_OFFSET_PAIR(s8->string_type->data[2]));
    EXPECT_EQ((uint32_t)GTA_UNICODE_STRING_TYPE_HTML, GTA_UC_GET_TYPE_FROM_TYPE_OFFSET_PAIR(s8->string_type->data[3]));
    EXPECT_EQ(0, GTA_UC_GET_OFFSET_FROM_TYPE_OFFSET_PAIR(s8->string_type->data[0]));
    EXPECT_EQ(3, GTA_UC_GET_OFFSET_FROM_TYPE_OFFSET_PAIR(s8->string_type->data[1]));
    EXPECT_EQ(6, GTA_UC_GET_OFFSET_FROM_TYPE_OFFSET_PAIR(s8->string_type->data[2]));
    EXPECT_EQ(10, GTA_UC_GET_OFFSET_FROM_TYPE_OFFSET_PAIR(s8->string_type->data[3]));
    gta_unicode_string_destroy(s8);
    gta_unicode_string_destroy(s7);
    gta_unicode_string_destroy(s6);
    gta_unicode_string_destroy(s5);
    ASSERT_EQ(gcu_get_alloc_count(), gcu_get_free_count());
  }
  gta_unicode_string_destroy(s1);
  gta_unicode_string_destroy(s2);
  gta_unicode_string_destroy(s3);
  gta_unicode_string_destroy(s4);
  alloc_running_count += gcu_get_alloc_count();
  free_running_count += gcu_get_free_count();
  ASSERT_EQ(alloc_running_count, free_running_count);
}

#define DO_ALL_TEST(SOURCE, EXPECTED, TYPE) \
  { \
    gcu_memory_reset_counts(); \
    auto s = gta_unicode_string_create(SOURCE, strlen(SOURCE), TYPE); \
    EXPECT_NE(nullptr, s); \
    GTA_Unicode_Rendered_String rendered = gta_unicode_string_render(s); \
    EXPECT_TRUE(rendered.buffer); \
    EXPECT_EQ(EXPECTED, string{rendered.buffer}); \
    EXPECT_EQ(strlen(EXPECTED), rendered.length); \
    gcu_free(rendered.buffer); \
    gta_unicode_string_destroy(s); \
    ASSERT_EQ(gcu_get_alloc_count(), gcu_get_free_count()); \
  }


TEST(Render, Trusted) {
  // Testing an empty string.
  DO_ALL_TEST("", "", GTA_UNICODE_STRING_TYPE_TRUSTED);
  // Testing a string with various characters.
  DO_ALL_TEST("<a>b&c$'\"\u00A3", "<a>b&c$'\"\u00A3", GTA_UNICODE_STRING_TYPE_TRUSTED);
}


TEST(Render, HTML) {
  // Testing an empty string.
  DO_ALL_TEST("", "", GTA_UNICODE_STRING_TYPE_HTML);
  // Testing a string with various characters.
  DO_ALL_TEST("<a>b&c$'\"\u00A3", "&lt;a&gt;b&amp;c$'\"\u00A3", GTA_UNICODE_STRING_TYPE_HTML);
}

TEST(Render, HTML_ATTRIBUTE) {
  // Testing an empty string.
  DO_ALL_TEST("", "", GTA_UNICODE_STRING_TYPE_HTML_ATTRIBUTE);
  // Testing a string with various characters.
  DO_ALL_TEST("<a>b&c$'\"\u00A3", "&lt;a&gt;b&amp;c$&#39;&quot;\u00A3", GTA_UNICODE_STRING_TYPE_HTML_ATTRIBUTE);
}

TEST(Render, JAVASCRIPT) {
  // Testing an empty string.
  DO_ALL_TEST("", "", GTA_UNICODE_STRING_TYPE_JAVASCRIPT);
  // Testing a string with various characters.
  DO_ALL_TEST("Test ' \" < > \\ & \n \r \t", R"(Test \' \" \u003C \u003E \\ \u0026 \n \r \t)", GTA_UNICODE_STRING_TYPE_JAVASCRIPT);
}


int main(int argc, char** argv) {
  testing::InitGoogleTest(&argc, argv);
  int result = RUN_ALL_TESTS();
  u_cleanup();
  return result;
}


