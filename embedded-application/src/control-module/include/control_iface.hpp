#pragma once
#include <opencv2/opencv.hpp>
#include <vector>

// #define THREAD_SLEEP_MS 10
// #define FEEDBACK_THREAD_SLEEP_MS 50
// #define PAUSED_SLEEP_MS 500
// #define TARGET_FPS 5

#define THREAD_SLEEP_MS 3000
#define FEEDBACK_THREAD_SLEEP_MS 3000
#define PAUSED_SLEEP_MS 3000
#define TARGET_FPS 1/3

#define MAX_OBSTACLE_DISTANCE 4000

typedef struct Frame_ {
  cv::Mat depthMap;
  cv::Mat image;
} Frame;

typedef struct Obstacle_ {
  int label;          // etiqueta del componente 
  double area;        // área en píxeles
  double meanDepth;   // promedio de profundidad
  double score;       // criterio de selección
  double azimuth;     // ángulo horizontal
  double elevation;   // ángulo vertical
  cv::Point centroid; // centroide
  cv::Mat image;      // imagen del obstáculo
} Obstacle;

typedef struct Audio_ {
  std::vector<float> left_signal;
  std::vector<float> right_signal;
  uint64_t sample_rate;
} Audio;

class IControl{
private:
public:
  static Frame get_frame();
  static void set_frame(const Frame& frame);

  static Obstacle get_obstacle();
  static void set_obstacle(const Obstacle& obstacle);

  static Audio get_audio_data();
  static void set_audio_data(const Audio& data);
  
  static void start_feedback();
  static void stop_feedback();
  static void set_volume(uint64_t volume);
  static void set_feedback_mode();
  
  static void unlock_mutexes();
};
