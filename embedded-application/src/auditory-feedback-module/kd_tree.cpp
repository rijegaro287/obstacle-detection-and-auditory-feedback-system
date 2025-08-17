#include "kd_tree.h"

template <uint8_t K>
double kd_tree<K>::calculate_distance(Node* node, const array<double, K>& target) {
  double dist = 0.0;
  for (uint8_t i = 0; i < K; i++) {
    dist += pow((node->point[i] - target[i]), 2);
  }
  return sqrt(dist);
}

template <uint8_t K>
typename kd_tree<K>::Node* kd_tree<K>::insert_recursive(Node* node, uint64_t idx, const array<double, K>& point, uint64_t depth) {
  if (node == nullptr) return new Node(idx, point);

  uint8_t cd = depth % K;
  if (point[cd] < node->point[cd]) {
    node->left = insert_recursive(node->left, idx, point, depth + 1);
  }
  else {
    node->right = insert_recursive(node->right, idx, point, depth + 1);
  }

  return node;
}

template<uint8_t K>
typename kd_tree<K>::Node* kd_tree<K>::find_nearest_recursive(Node* node, Node*& best, const array<double, K>& target, uint64_t depth) {
  if (node == nullptr) return nullptr;

  double dist = calculate_distance(node, target);
  if (dist == 0) {
    best = node;
    return node;
  }

  double best_dist = (best == nullptr) ? INFINITY : calculate_distance(best, target);
  if (dist < best_dist) {
    best = node;
  }

  uint8_t cd = depth % K;
  Node* next_node = (node->point[cd] > target[cd]) ? node->left : node->right;
  Node* other_node = (next_node == node->left) ? node->right : node->left;

  find_nearest_recursive(next_node, best, target, depth + 1);

  if (fabs(node->point[cd] - target[cd]) < best_dist) {
    find_nearest_recursive(other_node, best, target, depth + 1);
  }

  printf("Best match found at index: %llu\n", best->idx);
  printf("Best match coordinates: (%f, %f, %f)\n", best->point[0], best->point[1], best->point[2]);

  return best;
}

template <uint8_t K>
void kd_tree<K>::insert(uint64_t idx, const array<double, K>& point) {
  root = insert_recursive(root, idx, point, 0);
}

template <uint8_t K>
uint64_t kd_tree<K>::find_nearest(const array<double, K>& target) {
  Node* best = nullptr;
  find_nearest_recursive(root, best, target, 0);
  return (best != nullptr) ? best->idx : UINT64_MAX;
}

template <uint8_t K>
void kd_tree<K>::delete_recursive(Node* node) {
  if (node != nullptr) {
    delete_recursive(node->left);
    delete_recursive(node->right);
    delete node;
  }
}


template class kd_tree<3>;
