#include "bit_sequence.h"
#include "decoder.h"
#include "encoder.h"
#include "tree.h"
#include "gtest/gtest.h"
#include <array>
#include <set>
#include <sstream>
#include <stdexcept>
#include <vector>

using huffman::bit_sequence;
using huffman::decoder;
using huffman::encoder;
using huffman::tree;

static constexpr size_t N = 10000;
TEST(bit_sequence, default_constructor) {
  bit_sequence seq;
  ASSERT_EQ(0, seq.size());
}

TEST(bit_sequence, copy_constructor) {
  bit_sequence seq1;
  for (size_t i = 0; i < N; ++i) {
    seq1.append((i & 1234) == 0);
  }
  ASSERT_EQ(N, seq1.size());

  bit_sequence seq2(seq1);

  ASSERT_EQ(seq1.size(), seq2.size());
  for (size_t i = 0; i < N; ++i) {
    ASSERT_EQ(seq1[i], seq2[i]);
  }
}

TEST(bit_sequence, assignment) {
  bit_sequence seq1;
  for (size_t i = 0; i < N; ++i) {
    seq1.append((i & 1234) == 0);
  }

  bit_sequence seq2;
  for (size_t i = 0; i < 2 * N; ++i) {
    seq2.append((i & 4321) == 0);
  }
  ASSERT_NE(seq1.size(), seq2.size());

  seq2 = seq1;

  ASSERT_EQ(seq1.size(), seq2.size());
  for (size_t i = 0; i < N; ++i) {
    ASSERT_EQ(seq1[i], seq2[i]);
  }
}

TEST(bit_sequence, self_assignment) {
  bit_sequence seq;
  for (size_t i = 0; i < N; ++i) {
    seq.append((i & 1234) == 0);
  }

  bit_sequence prev(seq);
  seq = seq;

  ASSERT_EQ(prev.size(), seq.size());
  for (size_t i = 0; i < N; ++i) {
    ASSERT_EQ(prev[i], seq[i]);
  }
}

TEST(bit_sequence, subscription) {
  bit_sequence seq;
  for (size_t i = 0; i < N; ++i) {
    seq.append((i & 1234) == 0);
  }

  for (size_t i = 0; i < N; ++i) {
    ASSERT_EQ((i & 1234) == 0, seq[i]);
  }
}

TEST(bit_sequence, append_bitseq) {
  bit_sequence seq1;
  bit_sequence seq2;
  for (size_t i = 0; i < N; ++i) {
    seq1.append((i & 1234) == 0);
    seq2.append(true);
  }
  seq2.append(seq1);
  ASSERT_EQ(2 * N, seq2.size());
  for (size_t i = 0; i < N; ++i) {
    ASSERT_EQ(seq1[i], seq2[N + i]);
  }
}

TEST(bit_sequence, append_self) {
  bit_sequence seq;
  for (size_t i = 0; i < N; ++i) {
    seq.append((i & 1234) == 0);
  }

  seq.append(seq);

  ASSERT_EQ(2 * N, seq.size());
  for (size_t i = 0; i < N; ++i) {
    ASSERT_EQ(seq[i], seq[N + i]);
  }
}

TEST(bit_sequence, numbers) {
  std::vector<uint16_t> values;
  for (size_t i = 0; i < N; ++i) {
    values.push_back(i);
  }

  bit_sequence seq;

  for (size_t i = 0; i < N; ++i) {
    seq.append(values[i], 16);
  }

  ASSERT_EQ(16 * N, seq.size());

  for (size_t i = 0; i < N; ++i) {
    ASSERT_EQ(values[i], seq.get_number(16, 16 * i));
  }
}

TEST(bit_sequence, swap1) {
  bit_sequence seq1;
  bit_sequence seq2;
  for (size_t i = 0; i < N; ++i) {
    seq1.append((i & 1234) == 0);
    seq2.append((i & 4321) == 0);
  }

  seq1.swap(seq2);
  for (size_t i = 0; i < N; ++i) {
    ASSERT_EQ((i & 1234) == 0, seq2[i]);
    ASSERT_EQ((i & 4321) == 0, seq1[i]);
  }

  seq1.swap(seq2);
  for (size_t i = 0; i < N; ++i) {
    ASSERT_EQ((i & 1234) == 0, seq1[i]);
    ASSERT_EQ((i & 4321) == 0, seq2[i]);
  }
}

TEST(bit_sequence, swap2) {
  bit_sequence seq1;
  bit_sequence seq2;
  for (size_t i = 0; i < N; ++i) {
    seq1.append((i & 1234) == 0);
  }
  for (size_t i = 0; i < 2 * N; ++i) {
    seq2.append((i & 4321) == 0);
  }

  ASSERT_EQ(N, seq1.size());
  ASSERT_EQ(2 * N, seq2.size());

  seq1.swap(seq2);

  ASSERT_EQ(N, seq2.size());
  ASSERT_EQ(2 * N, seq1.size());

  seq1.swap(seq2);

  ASSERT_EQ(N, seq1.size());
  ASSERT_EQ(2 * N, seq2.size());
}

TEST(bit_sequence, swap3) {
  bit_sequence seq;
  for (size_t i = 0; i < N; ++i) {
    seq.append((i & 1234) == 0);
  }

  seq.swap(seq);

  ASSERT_EQ(N, seq.size());
  for (size_t i = 0; i < N; ++i) {
    ASSERT_EQ((i & 1234) == 0, seq[i]);
  }
}

TEST(tree, uniform_distr) {
  std::array<size_t, huffman::CHARS_COUNT> counts{};
  counts.fill(1);

  tree tree_(counts);

  std::array<bit_sequence, huffman::CHARS_COUNT> codes{};
  tree_.get_codes(codes);

  std::set<uint8_t> codes_set;
  std::set<uint8_t> all_values;
  for (size_t i = 0; i < huffman::CHARS_COUNT; ++i) {
    ASSERT_EQ(8, codes[i].size());

    codes_set.insert(codes[i].get_number(8, 0));
    all_values.insert(i);
  }

  ASSERT_EQ(codes_set, all_values);
}

TEST(tree, header_leaf_count1) {
  std::array<size_t, huffman::CHARS_COUNT> counts{};
  counts.fill(1);

  tree tree_(counts);
  bit_sequence header(tree_.header());

  ASSERT_EQ(255, header.get_number(8, 0));
}

TEST(tree, header_leaf_count2) {
  std::array<size_t, huffman::CHARS_COUNT> counts{};
  counts.fill(0);
  for (size_t i = 0; i < 80; ++i) {
    counts[i] = 1;
  }
  for (size_t i = 80; i < 120; ++i) {
    counts[i] = 2;
  }

  tree tree_(counts);
  bit_sequence header(tree_.header());

  ASSERT_EQ(119, header.get_number(8, 0));
}

TEST(tree, code_lengths1) {
  std::array<size_t, huffman::CHARS_COUNT> counts{};
  counts.fill(0);
  counts[0] = 1;
  counts[1] = 1;
  counts[2] = 2;
  counts[3] = 2;
  counts[4] = 2;
  counts[5] = 4;
  counts[6] = 4;

  tree tree_(counts);
  std::array<bit_sequence, huffman::CHARS_COUNT> codes{};
  tree_.get_codes(codes);

  ASSERT_EQ(2, codes[5].size());
  ASSERT_EQ(2, codes[6].size());
  ASSERT_EQ(3, codes[2].size());
  ASSERT_EQ(3, codes[3].size());
  ASSERT_EQ(3, codes[4].size());
  ASSERT_EQ(4, codes[0].size());
  ASSERT_EQ(4, codes[1].size());
}

TEST(tree, code_lengths2) {
  std::array<size_t, huffman::CHARS_COUNT> counts{};
  counts.fill(0);
  counts[0] = 4;
  counts[1] = 3;
  counts[2] = 7;
  counts[3] = 92;
  counts[4] = 42;
  counts[5] = 47;
  counts[6] = 55;

  tree tree_(counts);
  std::array<bit_sequence, huffman::CHARS_COUNT> codes{};
  tree_.get_codes(codes);

  ASSERT_EQ(5, codes[0].size());
  ASSERT_EQ(5, codes[1].size());
  ASSERT_EQ(4, codes[2].size());
  ASSERT_EQ(2, codes[3].size());
  ASSERT_EQ(3, codes[4].size());
  ASSERT_EQ(2, codes[5].size());
  ASSERT_EQ(2, codes[6].size());
}

TEST(tree, traversal_header_constructor) {
  std::array<size_t, huffman::CHARS_COUNT> counts{};
  // "random" tree
  for (size_t i = 0; i < huffman::CHARS_COUNT; ++i) {
    if ((i & 12) != 0) {
      counts[i] = i;
    } else {
      counts[i] = 0;
    }
  }

  tree tree_(counts);

  bit_sequence header(tree_.header());

  size_t nodes_number = 2 * header.get_number(8, 0) + 1;

  std::vector<uint16_t> traversal;
  for (size_t i = 0; i < nodes_number; ++i) {
    traversal.push_back(header.get_number(9, 8 + 9 * i));
  }

  tree other(traversal);

  std::array<bit_sequence, huffman::CHARS_COUNT> codes1{};
  std::array<bit_sequence, huffman::CHARS_COUNT> codes2{};
  tree_.get_codes(codes1);
  other.get_codes(codes2);

  for (size_t i = 0; i < huffman::CHARS_COUNT; ++i) {
    ASSERT_EQ(codes1[i].size(), codes2[i].size());

    for (size_t j = 0; j < codes1[i].size(); ++j) {
      ASSERT_EQ(codes1[i][j], codes2[i][j]);
    }
  }
}

TEST(tree, get_char) {
  std::array<size_t, huffman::CHARS_COUNT> counts{};
  // "random" tree
  for (size_t i = 0; i < huffman::CHARS_COUNT; ++i) {
    if ((i & 12) != 0) {
      counts[i] = i;
    } else {
      counts[i] = 0;
    }
  }

  tree tree_(counts);

  std::array<bit_sequence, huffman::CHARS_COUNT> codes{};
  tree_.get_codes(codes);

  for (size_t i = 0; i < huffman::CHARS_COUNT; ++i) {
    if ((i & 12) != 0) {
      uint8_t ch = ~i;
      bit_sequence code(codes[i]);
      size_t idx = 0;
      ASSERT_TRUE(tree_.get_char(code, idx, ch));

      ASSERT_EQ(code.size(), idx);
      ASSERT_EQ(i, ch);
    }
  }
}

TEST(tree, empty) {
  std::array<size_t, huffman::CHARS_COUNT> counts{};
  counts.fill(0);

  EXPECT_THROW(new tree(counts), std::runtime_error);
}

TEST(tree, one_char) {
  std::array<size_t, huffman::CHARS_COUNT> counts{};
  counts.fill(0);
  counts[0] = 1;

  tree tree_(counts);

  bit_sequence one;
  one.append(true);
  bit_sequence zero;
  zero.append(false);

  size_t idx = 0;
  uint8_t ch = 255;

  ASSERT_TRUE(tree_.get_char(one, idx, ch));
  ASSERT_EQ(1, idx);
  ASSERT_EQ(0, ch);

  idx = 0;
  ch = 255;

  ASSERT_TRUE(tree_.get_char(zero, idx, ch));
  ASSERT_EQ(1, idx);
  ASSERT_EQ(0, ch);
}

TEST(encoder, default_constructor) {
  encoder encoder_;

  bit_sequence header(encoder_.header());

  ASSERT_EQ(8, header.size());
  ASSERT_EQ(0, header.get_number(8, 0));

  for (size_t i = 0; i < N / 100; ++i) {
    std::vector<uint8_t> chars;
    for (size_t j = 0; j < N / 10; ++j) {
      chars.push_back(((i * j & 1234u) * 5 + i) % 256);
    }
    EXPECT_THROW(encoder_.encode(chars), std::runtime_error);
  }
}

TEST(encoder, codes) {
  encoder encoder_;

  for (size_t i = 0; i < N; ++i) {
    encoder_.add_char(((i & 1234) * 5) % 256);
  }
  bit_sequence header(encoder_.header());

  size_t nodes_number = 2 * header.get_number(8, 0) + 1;

  std::vector<uint16_t> traversal;
  for (size_t i = 0; i < nodes_number; ++i) {
    traversal.push_back(header.get_number(9, 8 + 9 * i));
  }

  tree tree_(traversal);

  std::array<bit_sequence, huffman::CHARS_COUNT> codes{};
  tree_.get_codes(codes);

  for (size_t i = 0; i < huffman::CHARS_COUNT; ++i) {
    std::vector<uint8_t> vec;
    vec.push_back(i);
    if (codes[i].size() != 0) {

      bit_sequence encoded(encoder_.encode(vec));

      ASSERT_EQ(encoded.size(), codes[i].size());
      for (size_t j = 0; j < encoded.size(); ++j) {
        ASSERT_EQ(encoded[j], codes[i][j]);
      }
    } else {
      EXPECT_THROW(encoder_.encode(vec), std::runtime_error);
    }
  }
}
TEST(correctness, streams) {
  encoder encoder_;
  std::string test_string =
      "Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod "
      "tempor incididunt ut labore et dolore magna aliqua. Ut enim ad minim "
      "veniam, quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea "
      "commodo consequat. Duis aute irure dolor in reprehenderit in voluptate "
      "velit esse cillum dolore eu fugiat nulla pariatur. Excepteur sint "
      "occaecat cupidatat non proident, sunt in culpa qui officia deserunt "
      "mollit anim id est laborum.";
  std::stringstream count_stream(test_string);
  encoder_.add_chars(count_stream);

  std::stringstream encoder_input(test_string);
  std::stringstream encoder_output;

  encoder_.encode(encoder_input, encoder_output);

  std::stringstream decoder_output;

  decoder decoder_;
  decoder_.decode(encoder_output, decoder_output);

  ASSERT_EQ(test_string, decoder_output.str());
}
