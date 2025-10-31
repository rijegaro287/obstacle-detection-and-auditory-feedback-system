#include "control_module.hpp"
#include "configuration_iface.hpp"
#include "control_iface.hpp"
#include "image_capture_iface.hpp"
#include "obstacle_detection_iface.hpp"
#include "transmission_iface.hpp"
#include "feedback_iface.hpp"

#include <iostream>
#include <thread>
#include <chrono>

static bool received_audio_commands = false;

ControlModule& ControlModule::get_instance() {
	static ControlModule instance;
	return instance;
}

ControlModule::ControlModule()
  : performance_monitor(PerformanceMonitor::get_instance(true)) {

  this->frame_mtx.lock();
  this->obstacle_mtx.lock();
  this->audio_mtx.lock();

  this->frame = Frame();
  this->obstacle = Obstacle();
  this->audio_data = Audio();
}

ControlModule::~ControlModule() {

}

Frame ControlModule::get_frame() {
  lock_guard<mutex> guard(this->frame_mtx);
  Frame frame = this->frame;
  this->frame = Frame();
  return frame;
}

void ControlModule::set_frame(Frame frame) {
  lock_guard<mutex> guard(this->frame_mtx);
  this->frame = frame;
}

Obstacle ControlModule::get_obstacle() {
  lock_guard<mutex> guard(this->obstacle_mtx);
  Obstacle obstacle = this->obstacle;
  this->obstacle = Obstacle();
  return obstacle;
}

void ControlModule::set_obstacle(const Obstacle obstacle) {
  lock_guard<mutex> guard(this->obstacle_mtx);
  this->obstacle.azimuth = obstacle.azimuth;
  this->obstacle.elevation = obstacle.elevation;
  this->obstacle.meanDepth = obstacle.meanDepth;
  this->obstacle.image = obstacle.image;
}

Audio ControlModule::get_audio_data() {
  lock_guard<mutex> guard(this->audio_mtx);
  Audio data = this->audio_data;
  this->audio_data = Audio();
  return data;
}

void ControlModule::set_audio_data(const Audio& data) {
  lock_guard<mutex> guard(this->audio_mtx);
  this->audio_data.left_signal = data.left_signal;
  this->audio_data.right_signal = data.right_signal;
  this->audio_data.sample_rate = data.sample_rate;
  this->audio_data.gain = data.gain;
}

void ControlModule::start_feedback() {
  IImageCapture::start_capture();
  IObstacleDetection::start_detection();
  IFeedback::start_feedback();
  ITransmission::start_transmission();
}

void ControlModule::stop_feedback() {
  IImageCapture::stop_capture();
  IObstacleDetection::stop_detection();
  IFeedback::stop_feedback();
  ITransmission::stop_transmission();

  this->frame = Frame();
  this->obstacle = Obstacle();
  this->audio_data = Audio();
}

void ControlModule::set_volume(uint64_t volume) {
  IFeedback::set_volume(volume);
}

void ControlModule::set_feedback_mode(FEEDBACK_MODES mode) {
  IFeedback::set_feedback_mode(mode);
}

void ControlModule::set_received_audio_commands(bool status) {
  received_audio_commands = status;
}

void ControlModule::unlock_mutexes() {
  this->frame_mtx.unlock();
  this->obstacle_mtx.unlock();
  this->audio_mtx.unlock();
}

// Método viewDetection: muestra el resultado de la deteccion de obstáculos
void ControlModule::viewDetection(Obstacle& obstacle){
    lock_guard<mutex> guard(this->obstacle_mtx);
    cv::Mat display;

    if(obstacle.image.empty()){
      return;
    }

    cv::cvtColor(obstacle.image, display, cv::COLOR_GRAY2BGR);

    std::ostringstream oss;
    /**oss << std::fixed << std::setprecision(2)
        << obstacle.meanDepth << " m | "
        << "Az: " << mapAzimuth(obstacle.azimuth) << " | "
        << "El: " << obstacle.elevation;**/

    std::string infoText = oss.str();

    // Dibujar la profundidad promedio 
    cv::putText(
        display,
        infoText,
        cv::Point(10, 30),
        cv::FONT_HERSHEY_SIMPLEX,
        0.4,
        cv::Scalar(255, 255, 0), // celeste
        2
    );

    // Centro de la imagen
    cv::Point center(display.cols/2, display.rows/2);

    // FOV de la cámara
    const double FOV_X_DEG = 62.8;
    const double FOV_Y_DEG = 37.9;

    // Calcular posición del punto que indica la dirección del obstáculo
    cv::Point tip(
        center.x + static_cast<int>(obstacle.azimuth   / (FOV_X_DEG/2.0) * center.x),
        center.y - static_cast<int>(obstacle.elevation / (FOV_Y_DEG/2.0) * center.y)
    );

    // Dibujar un punto morado en la dirección del obstáculo
    cv::circle(display, tip, 5, cv::Scalar(255,0,255), cv::FILLED); // morado

    cv::imshow("Distancia y Angulo del obstaculo seleccionado", display);
}

void ControlModule::start() {
  this->unlock_mutexes();
  this->start_feedback();

  while (true) {
	  // printf("==========> CONTROL =======================================================\n");
    /**if (received_audio_commands) {
      received_audio_commands = false;
    }
    else {
      printf("No commands received in the last %.1f seconds. Stopping feedback...\n", CONTROL_THREAD_SLEEP_MS / 1000.0);
      this->stop_feedback();
    }
    this_thread::sleep_for(chrono::milliseconds(CONTROL_THREAD_SLEEP_MS));
  }**/
  
  //viewDetection(this->obstacle);
  this_thread::sleep_for(chrono::milliseconds(1000));
 }
}
