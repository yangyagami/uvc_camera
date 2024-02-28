#include "camera.h"

#include <iostream>
#include <string>
#include <queue>
#include <mutex>
#include <thread>
#include <unistd.h>

#include <unistd.h>

#include "libuvc/libuvc.h"
#include "opencv2/opencv.hpp"

#define ERROR_HANDLE(x) { \
  if (res_ < 0) { \
    uvc_perror(res_, x); \
    return false; \
  } \
}

namespace uvc {

Camera::~Camera() {
  sleep(kThreadWaitSeconds);

  {
    std::lock_guard<std::mutex> lock(
        camera_connect_status_check_thread_running_lock_);
    camera_connect_status_check_thread_running_ = false;
  }

  Close();

  //if (camera_connect_status_check_thread_ != nullptr) {
  //  camera_connect_status_check_thread_->join();
  //}

  FreeResources();
}

bool Camera::Init(int width, int height, int fps) {
  res_ = uvc_init(&ctx_, nullptr);
  ERROR_HANDLE("uvc_init");

  std::cout << "UVC initialized" << std::endl;

  int vid_num = 0;
  int pid_num = 0;
  if (vid_ != "" && pid_ != "") {
    vid_num = std::stoi(vid_, 0, 16);
    pid_num = std::stoi(pid_, 0, 16);
  }
  res_ = uvc_find_device(ctx_, &dev_, vid_num, pid_num, nullptr);
  ERROR_HANDLE("uvc_find_device");

  std::cout << "Device found" << std::endl;

  res_ = uvc_open(dev_, &devh_);
  ERROR_HANDLE("uvc_open");

  std::cout << "Device opened" << std::endl;

  uvc_print_diag(devh_, nullptr);

  const uvc_format_desc_t *format_desc = uvc_get_format_descs(devh_);
  [[maybe_unused]]
  const uvc_frame_desc_t *frame_desc = format_desc->frame_descs;
  enum uvc_frame_format frame_format;

  switch (format_desc->bDescriptorSubtype) {
    case UVC_VS_FORMAT_MJPEG:
      frame_format = UVC_COLOR_FORMAT_MJPEG;
      break;
    case UVC_VS_FORMAT_FRAME_BASED:
      frame_format = UVC_FRAME_FORMAT_H264;
      break;
    default:
      frame_format = UVC_FRAME_FORMAT_YUYV;
      break;
  }
  res_ = uvc_get_stream_ctrl_format_size(
      devh_,
      &ctrl_,
      frame_format,
      width,
      height,
      fps);
  uvc_print_stream_ctrl(&ctrl_, nullptr);
  ERROR_HANDLE("get_mode");

  connected_ = true;
  //camera_connect_status_check_thread_running_ = true;
  //if (camera_connect_status_check_thread_ == nullptr) {
  //  camera_connect_status_check_thread_ = std::make_shared<std::thread>(
  //      &Camera::CameraConnectStatusCheck, this);
  //}
  return true;
}

bool Camera::Open() {
  res_ = uvc_start_streaming(devh_, &ctrl_, &Camera::FrameCallback,
                             (void *)this, 0);
  ERROR_HANDLE("start_streaming");

  std::cout << "Streaming" << std::endl;

  std::cout << "Enabling auto exposure" << std::endl;
  EnableAutoExposure();

  {
    std::lock_guard<std::mutex> opened_lock(opened_lock_);
    opened_ = true;
  }
  camera_connect_status_check_cv_.notify_all();
  return true;
}

void Camera::Close() {
  if (opened_) {
    uvc_stop_streaming(devh_);
    std::cout << "Stream Closed" << std::endl;
    {
      std::lock_guard<std::mutex> opened_lock(opened_lock_);
      opened_ = false;
    }
  }
}

bool Camera::Read(cv::Mat &frame) {
  std::lock_guard<std::mutex> lock(frame_lock_);
  if (frame_queue_.empty()) {
    return false;
  }
  frame = frame_queue_.front();
  frame_queue_.pop();
  return true;
}

bool Camera::ReInitWhenDisconnected(int width, int height, int fps) {
  Close();

  return Init(width, height, fps);
}

bool Camera::IsConnected() {
  std::lock_guard<std::mutex> connected_lock(connected_lock_);
  return connected_;
}

void Camera::EnableAutoExposure() {
  const uint8_t UVC_AUTO_EXPOSURE_MODE_AUTO = 2;
  res_ = uvc_set_ae_mode(devh_, UVC_AUTO_EXPOSURE_MODE_AUTO);
  if (res_ == UVC_SUCCESS) {
    std::cout << " ... enabled auto exposure" << std::endl;
  } else if (res_ == UVC_ERROR_PIPE) {
    /* this error indicates that the camera does not support the full AE mode;
     * try again, using aperture priority mode 
     * (fixed aperture, variable exposure time) */
    std::cout << " ... full AE not supported, trying aperture priority mode"
              << std::endl;
    const uint8_t UVC_AUTO_EXPOSURE_MODE_APERTURE_PRIORITY = 8;
    res_ = uvc_set_ae_mode(devh_, UVC_AUTO_EXPOSURE_MODE_APERTURE_PRIORITY);
    if (res_ < 0) {
      uvc_perror(
          res_,
          " ... uvc_set_ae_mode failed to enable aperture priority mode");
    } else {
      std::cout << " ... enabled aperture priority auto exposure mode"
                << std::endl;
    }
  } else {
    uvc_perror(res_, "... uvc_set_ae_mode failed to enable auto exposure mode");
  }
}

// **暂时只处理jpeg格式**。
void Camera::FrameCallback(uvc_frame_t *frame, void *ptr) {
  auto self = reinterpret_cast<Camera*>(ptr);

  cv::Mat f = cv::imdecode(cv::Mat(1, frame->data_bytes, CV_8UC1, frame->data),
                           cv::IMREAD_COLOR);

  std::lock_guard<std::mutex> lock(self->frame_lock_);
  self->frame_flag_ = true;
  if (self->frame_queue_.size() >= 4) {
    self->frame_queue_.pop();
  }
  self->frame_queue_.push(f);
}

void Camera::CameraConnectStatusCheck() {
  int no_frame_times = 0;
  while (true) {
    {
      std::lock_guard<std::mutex> running_lock(
          camera_connect_status_check_thread_running_lock_);
      if (camera_connect_status_check_thread_running_ == false) {
        return;
      }
    }

    std::unique_lock<std::mutex> opened_lock(opened_lock_);
    while (opened_ == false) {
      camera_connect_status_check_cv_.wait(opened_lock);
    }
    opened_lock.unlock();

    {
      std::lock_guard<std::mutex> frame_lock(frame_lock_);
      if (frame_flag_) {
        frame_flag_ = false;
        no_frame_times = 0;

        std::lock_guard<std::mutex> connected_lock(connected_lock_);
        connected_ = true;
      } else {
        no_frame_times++;
        if (no_frame_times >= kThreadWaitSeconds - 1) {
          no_frame_times = 0;

          std::lock_guard<std::mutex> connected_lock(connected_lock_);
          connected_ = false;
        }
      }
    }

    std::cout << this << " " << " no frame times: " << no_frame_times
              << std::endl;
    sleep(1);
  }
}

void Camera::FreeResources() {
  if (devh_ != nullptr) {
    uvc_close(devh_);
    devh_ = nullptr;
  }
  if (dev_ != nullptr) {
    uvc_unref_device(dev_);
    dev_ = nullptr;
  }
  if (ctx_ != nullptr) {
    uvc_exit(ctx_);
    ctx_ = nullptr;
  }
}

}  // namespace uvc
