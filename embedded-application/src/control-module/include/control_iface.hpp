#pragma once
#include <opencv2/opencv.hpp>
#include <vector>

typedef struct Audio_ {
  std::vector<double> left_signal;
  std::vector<double> right_signal;
  uint64_t sample_rate;
} Audio;

struct Frame {
  cv::Mat depthMap;
  cv::Mat image;
};

struct Obstacle {
    int label;          // etiqueta del componente 
    double area;        // área en píxeles
    double meanDepth;   // promedio de profundidad
    double score;       // criterio de selección
    double azimuth;     // ángulo horizontal
    double elevation;   // ángulo vertical
    cv::Point centroid; // centroide
    cv::Mat image;      // imagen del obstáculo
};

class IControl{
private:
public:
  static Frame get_frame();
  static Obstacle get_obstacle();
  static Audio get_audio_data();
  static void set_frame(const Frame& frame);
  static void set_obstacle(const Obstacle& obstacle);
  static void set_audio_data(const Audio& data);
};
