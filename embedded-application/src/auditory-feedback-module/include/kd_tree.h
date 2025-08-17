#pragma once

#include <array>
#include <cstdint>
#include <cmath>

#include <iostream>

using namespace std;

template <uint8_t K>
class kd_tree {
private:
  struct Node {
    uint64_t idx;
    array<double, K> point;
    Node* left;
    Node* right;
    Node(uint64_t idx, const array<double, K>& point)
      : idx(idx), point(point), left(nullptr), right(nullptr) {}
  };

  Node* root;

  double calculate_distance(Node* node, const array<double, K>& target);

  Node* insert_recursive(Node* node, uint64_t idx, const array<double, K>& point, uint64_t depth);
  Node* find_nearest_recursive(Node* node, Node*& best, const array<double, K>& target, uint64_t depth);
  void delete_recursive(Node* node);
public:
  kd_tree() : root(nullptr) {}
  ~kd_tree() { delete_recursive(root); }

  void insert(uint64_t idx, const array<double, K>& point);
  uint64_t find_nearest(const array<double, K>& target);
};
