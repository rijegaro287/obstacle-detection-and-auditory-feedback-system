#pragma once

/**
 * @file kd_tree.hpp
 * @brief Lightweight k-d tree implementation tailored for spatial HRIR lookup.
 */

#include <array>
#include <cstdint>
#include <cmath>

#include <iostream>

using namespace std;

/**
 * @class kd_tree
 * @brief Minimal templated k-d tree that supports insertion and nearest
 * neighbour queries.
 *
 * @tparam K Dimensionality of the stored points.
 */
template <uint8_t K>
class kd_tree {
private:
	/**
	 * @brief Internal node structure containing point coordinates and links.
	 */
	struct Node {
		uint64_t idx;               /**< Identifier associated with the point. */
		array<double, K> point;     /**< Coordinates in K-dimensional space. */
		Node* left;                 /**< Left subtree pointer. */
		Node* right;                /**< Right subtree pointer. */
		Node(uint64_t idx, const array<double, K>& point)
			: idx(idx), point(point), left(nullptr), right(nullptr) {}
	};

	Node* root; /**< Root node of the tree. */

	/**
	 * @brief Compute Euclidean distance between the stored node and target.
	 * @param node Node holding the stored point.
	 * @param target Target coordinates.
	 * @return Euclidean distance between both points.
	 */
	double calculate_distance(Node* node, const array<double, K>& target);

	/**
	 * @brief Recursive helper that inserts a new node into the tree.
	 */
	Node* insert_recursive(Node* node, uint64_t idx, const array<double, K>& point, uint64_t depth);

	/**
	 * @brief Recursive helper that searches for the nearest neighbour.
	 */
	Node* find_nearest_recursive(Node* node, Node*& best, double& best_dist, const array<double, K>& target, uint64_t depth);

	/**
	 * @brief Recursively delete nodes during destruction.
	 */
	void delete_recursive(Node* node);
public:
	/** @brief Construct an empty k-d tree. */
	kd_tree() : root(nullptr) {}
	/** @brief Recursively release all allocated nodes. */
	~kd_tree() { delete_recursive(root); }

	/**
	 * @brief Insert a point associated with an index.
	 * @param idx Identifier to store together with the point.
	 * @param point Coordinates of the point to insert.
	 */
	void insert(uint64_t idx, const array<double, K>& point);

	/**
	 * @brief Find the index corresponding to the nearest stored point.
	 * @param target Coordinates used for the lookup.
	 * @return Index belonging to the closest point or `UINT64_MAX` when empty.
	 */
	uint64_t find_nearest(const array<double, K>& target);
};
