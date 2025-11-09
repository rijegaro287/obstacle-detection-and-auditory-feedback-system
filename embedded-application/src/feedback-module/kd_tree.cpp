/**
 * @file kd_tree.cpp
 * @brief Provides template definitions for the k-d tree implementation.
 */

#include "kd_tree.hpp"

/**
 * @brief Compute the Euclidean distance between a stored point and a target.
 */
template <uint8_t K>
double kd_tree<K>::calculate_distance(Node* node, const array<double, K>& target) {
  double dist = 0.0;
  for (uint8_t i = 0; i < K; i++) {
    dist += pow((node->point[i] - target[i]), 2);
  }
  return sqrt(dist);
}

/**
 * @brief Recursive helper used by @ref insert.
 */
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

/**
 * @brief Recursive helper used by @ref find_nearest.
 */
template<uint8_t K>
typename kd_tree<K>::Node* kd_tree<K>::find_nearest_recursive(Node* node, Node*& best, double& best_dist, const array<double, K>& target, uint64_t depth) {
  if (node == nullptr) return nullptr;

  double dist = calculate_distance(node, target);
  if (dist == 0) {
    best = node;
    best_dist = 0;
    return node;
  }

  best_dist = (best == nullptr) ? INFINITY : calculate_distance(best, target);
  if (dist < best_dist) {
    best = node;
    best_dist = dist;
  }

  uint8_t cd = depth % K;
  Node* next_node = (node->point[cd] > target[cd]) ? node->left : node->right;
  Node* other_node = (next_node == node->left) ? node->right : node->left;

  find_nearest_recursive(next_node, best, best_dist, target, depth + 1);

  if (fabs(node->point[cd] - target[cd]) < best_dist) {
    find_nearest_recursive(other_node, best, best_dist, target, depth + 1);
  }

  return best;
}

/**
 * @copydoc kd_tree<K>::insert
 */
template <uint8_t K>
void kd_tree<K>::insert(uint64_t idx, const array<double, K>& point) {
  root = insert_recursive(root, idx, point, 0);
}

/**
 * @copydoc kd_tree<K>::find_nearest
 */
template <uint8_t K>
uint64_t kd_tree<K>::find_nearest(const array<double, K>& target) {
  Node* best = nullptr;
  double best_dist;
  find_nearest_recursive(root, best, best_dist, target, 0);
  return (best != nullptr) ? best->idx : UINT64_MAX;
}

/**
 * @brief Recursively release nodes.
 */
template <uint8_t K>
void kd_tree<K>::delete_recursive(Node* node) {
  if (node != nullptr) {
    delete_recursive(node->left);
    delete_recursive(node->right);
    delete node;
  }
}


template class kd_tree<3>;
