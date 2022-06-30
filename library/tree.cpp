#include "tree.h"
#include <algorithm>
#include <cassert>
#include <queue>
#include <stdexcept>
#include <string>
#include <utility>

namespace huffman {
tree::tree(std::array<size_t, CHARS_COUNT> const& counts) {
  // first - count, second - index of leaf, third - char
  std::vector<std::tuple<size_t, size_t, uint8_t>> first_array;
  size_t actual_size = 0;
  for (size_t i = 0; i < CHARS_COUNT; ++i) {
    if (counts[i] != 0) {
      first_array.emplace_back(counts[i], actual_size++, i);
    }
  }
  if (actual_size == 0) {
    throw std::runtime_error("Counts is zero, tree cannot be built");
  }
  if (actual_size == 1) {
    root = 2;
    children.emplace_back(0, 1);
    parents.resize(3, 2);
    leafs.push_back(std::get<2>(first_array[0]));
    leafs.push_back(leafs.back());
    return;
  }
  root = 2 * actual_size - 2;
  parents.resize(2 * actual_size - 1);
  parents[root] = root;
  leafs.resize(actual_size);
  children.resize(actual_size - 1);
  // pair is: first - count, second - index
  std::priority_queue<std::pair<size_t, size_t>,
      std::vector<std::pair<size_t, size_t>>,
      std::greater<>> pq;
  for (size_t i = 0; i < actual_size; ++i) {
    pq.emplace(std::get<0>(first_array[i]), std::get<1>(first_array[i]));
    leafs[std::get<1>(first_array[i])] = std::get<2>(first_array[i]);
  }
  for (size_t i = 0; i < leafs.size() - 1; ++i) {
    std::pair<size_t, size_t> left = pq.top();
    pq.pop();
    std::pair<size_t, size_t> right = pq.top();
    pq.pop();
    children[actual_size - leafs.size()] = {left.second, right.second};
    parents[left.second] = actual_size;
    parents[right.second] = actual_size;
    pq.emplace(left.first + right.first, actual_size++);
  }
  assert(actual_size == 2 * leafs.size() - 1);
  assert(actual_size = leafs.size() + children.size());
}

tree::tree(std::vector<uint16_t> const& traversal) {
  assert(traversal.size() % 2 == 1);
  leafs.resize(traversal.size() / 2 + 1);
  children.resize(leafs.size() - 1);
  parents.resize(traversal.size());
  root = leafs.size();
  size_t leaf_count = 0;
  size_t node_count = 0;
  std::vector<size_t> indexes;
  for (uint16_t code : traversal) {
    if (code < CHARS_COUNT) {
      leafs[leaf_count] = code;
      indexes.push_back(leaf_count++);
    } else {
      indexes.push_back(leafs.size() + node_count++);
    }
  }
  std::vector<size_t> need_right_queue;
  for (size_t i = 0; i < traversal.size() - 1; ++i) {
    if (traversal[i] == CHARS_COUNT) {
      children[indexes[i] - leafs.size()].first = indexes[i + 1];
      parents[indexes[i + 1]] = indexes[i];
      need_right_queue.push_back(indexes[i]);
    } else {
      children[need_right_queue.back() - leafs.size()].second = indexes[i + 1];
      parents[indexes[i + 1]] = need_right_queue.back();
      need_right_queue.pop_back();
    }
  }
  assert(need_right_queue.empty());
}
void tree::get_codes(std::array<bit_sequence, CHARS_COUNT>& result) const {
  bit_sequence current_code;
  size_t current_node = root;
  std::vector<bool> visited(parents.size(), false);
  size_t count = 0;
  while (++count != leafs.size() + 3 * children.size()) {
    visited[current_node] = true;
    if (current_node < leafs.size()) {
      result[leafs[current_node]] = current_code;
      current_code.pop_back();
      current_node = parents[current_node];
    } else {
      auto child = children[current_node - leafs.size()];
      if (!visited[child.first]) {
        current_node = child.first;
        current_code.append(false);
      } else if (!visited[child.second]) {
        current_node = child.second;
        current_code.append(true);
      } else {
        current_code.pop_back();
        current_node = parents[current_node];
      }
    }
  }
  assert(current_code.size() == 0);
}
bit_sequence tree::header() const {
  bit_sequence result;
  // encoding number of leafs - 1, which always is (size - 1) / 2, so we can
  // save one bit, so in decoder we will need to read only one byte, but not 9
  // bits, to determine length of header
  result.append(leafs.size() - 1, LOG_CHARS_COUNT).append(traversal());
  return result;
}
bit_sequence tree::traversal() const { // NOLINT(misc-no-recursion)
  std::vector<bool> visited(parents.size(), false);
  bit_sequence result;
  size_t current_node = root;
  size_t count = 0;
  while (++count != leafs.size() + 3 * children.size()) {
    if (!visited[current_node]) {
      uint16_t number =
          current_node < leafs.size() ? leafs[current_node] : CHARS_COUNT;
      result.append(number, LOG_MAX_NODE_NUMBER);
    }
    visited[current_node] = true;
    if (current_node < leafs.size()) {
      current_node = parents[current_node];
    } else {
      auto child = children[current_node - leafs.size()];
      if (!visited[child.first]) {
        current_node = child.first;
      } else if (!visited[child.second]) {
        current_node = child.second;
      } else {
        current_node = parents[current_node];
      }
    }
  }
  assert(result.size() == LOG_MAX_NODE_NUMBER * parents.size());
  return result;
}
bool tree::get_char(bit_sequence const& code,
                    size_t& idx, // NOLINT(bugprone-easily-swappable-parameters)
                    uint8_t& result) const {
  return get_char(code, idx, result, root);
}
std::vector<std::vector<std::pair<std::vector<uint8_t>, size_t>>>
tree::get_shortcuts() const {
  auto result =
      std::vector<std::vector<std::pair<std::vector<uint8_t>, size_t>>>();
  for (size_t i = 0; i < children.size(); ++i) {
    result.emplace_back();
    for (size_t j = 0; j < TREE_SHORTCUT_CHARS_COUNT; ++j) {
      std::vector<uint8_t> decoded;
      size_t current_node = i + leafs.size();
      size_t path = j;
      for (size_t k = 0; k < TREE_SHORTCUT_SIZE; ++k) {
        if ((path & 1u) == 0) {
          current_node = children[current_node - leafs.size()].first;
        } else {
          current_node = children[current_node - leafs.size()].second;
        }
        if (current_node < leafs.size()) {
          decoded.push_back(leafs[current_node]);
          current_node = root;
        }
        path >>= 1;
      }
      result.back().emplace_back(std::move(decoded), current_node);
    }
  }
  return result;
}
std::pair<size_t, size_t> tree::dump(bit_sequence const& buffer,
                                     size_t last_idx, std::ostream& output) {
  size_t current_node = root;
  size_t idx = 0;
  if (!calculated_shortcuts) {
    shortcuts = get_shortcuts();
    calculated_shortcuts = true;
  }
  std::string result;
  while (idx + TREE_SHORTCUT_SIZE <= last_idx) {
    uint8_t next_bits = buffer.get_number(TREE_SHORTCUT_SIZE, idx);
    auto tmp = shortcuts[current_node - leafs.size()][next_bits];
    for (uint8_t output_byte : tmp.first) {
      result.push_back(static_cast<char>(output_byte));
    }
    current_node = tmp.second;
    idx += TREE_SHORTCUT_SIZE;
  }
  size_t write_size = result.size();
  output.write(result.data(), static_cast<std::streamsize>(result.size()));
  size_t next_idx = idx;
  uint8_t next_byte; // NOLINT(cppcoreguidelines-init-variables)
  while (next_idx < last_idx &&
         get_char(buffer, next_idx, next_byte, current_node)) {
    current_node = root;
    output.put(static_cast<char>(next_byte));
    ++write_size;
    idx = next_idx;
  }
  while (current_node != root) {
    current_node = parents[current_node];
    --idx;
  }
  return {idx, write_size};
}
bool tree::get_char(const bit_sequence& code,
                    size_t& idx, // NOLINT(bugprone-easily-swappable-parameters)
                    uint8_t& result, size_t start_node) const {
  size_t current_node = start_node;
  while (true) {
    if (current_node < leafs.size()) {
      result = leafs[current_node];
      return true;
    }
    if (idx == code.size()) {
      return false;
    }
    if (code[idx++]) {
      current_node = children[current_node - leafs.size()].second;
    } else {
      current_node = children[current_node - leafs.size()].first;
    }
  }
}
} // namespace huffman
