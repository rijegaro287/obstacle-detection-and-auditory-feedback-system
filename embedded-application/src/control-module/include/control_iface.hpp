#pragma once

#include <vector>

typedef struct obstacle_position_t_ {
  float azimuth;
  float elevation;
  float distance;
} obstacle_position_t;

typedef struct audio_data_t_ {
  std::vector<double> left_signal;
  std::vector<double> right_signal;
} audio_data_t;

class IControl{
private:
public:
  static void set_obstacle_position(obstacle_position_t position);
  static void set_audio_data(audio_data_t data);
  static obstacle_position_t get_obstacle_position();
  static audio_data_t get_audio_data();
};
