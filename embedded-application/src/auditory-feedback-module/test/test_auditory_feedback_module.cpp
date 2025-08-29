#include <iostream>

#include "auditory_feedback_module.hpp"

int main() {
	auditory_feedback_module& feedback_module = auditory_feedback_module::get_instance();
	feedback_module.set_feedback_mode(VERBAL_MODE);
	feedback_module.start();

	feedback_module.set_feedback_mode(NON_VERBAL_MODE);
	feedback_module.start();

	// kd_tree<3> kd_tree;
	// kd_tree.insert(0, {0.0, 0.0, 0.0});
	// kd_tree.insert(1, {1.0, 1.0, 1.0});
	// kd_tree.insert(2, {2.0, 2.0, 2.0});
	// kd_tree.insert(3, {3.0, 3.0, 3.0});
	// kd_tree.insert(4, {4.0, 4.0, 4.0});

	// array<double, 3> target = {3.4, 0.21, 2.0};
	// uint64_t nearest = kd_tree.find_nearest(target);
	// cout << "Nearest neighbor index: " << nearest << endl;

	return 0;
}
