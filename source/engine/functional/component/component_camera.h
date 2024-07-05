#pragma once

#include <Eigen/Dense>
#include <engine/utils/base/trackball.h>
#define _USE_MATH_DEFINES
#include <math.h>
namespace mango {
class CameraComponent {
public:
  CameraComponent() = default;

  ~CameraComponent() = default;

  const std::string &getName() const noexcept { return name_; }

  void setName(const std::string &name) { name_ = name; }

  //// for view matrix stuff
  
  // default -z: front, x: right, y: up
  void setLookAt(const Eigen::Vector3f &eye, const Eigen::Vector3f &up,
                 const Eigen::Vector3f &center) {
    dis_ = (eye - center).norm();
    Eigen::Vector3f ny = up.normalized();
    Eigen::Vector3f nz = (eye - center).normalized();
    Eigen::Vector3f nx = ny.cross(nz).normalized();
    view_mat_.block<1, 3>(0, 0) = nx;
    view_mat_.block<1, 3>(1, 0) = ny;
    view_mat_.block<1, 3>(2, 0) = nz;
    view_mat_.block<3, 1>(0, 3) = view_mat_.block<3, 3>(0, 0) * -eye;
    view_mat_.block<1, 4>(3, 0) << 0, 0, 0, 1;
  }

  // default: z: front, x: right, y: up
  void setRotationEuraXYZ(const Eigen::Vector3f &eura_xyz) {
    Eigen::Matrix3f rotation_mat;
    rotation_mat = Eigen::AngleAxisf(eura_xyz.x(), Eigen::Vector3f::UnitX()) *
                   Eigen::AngleAxisf(eura_xyz.y(), Eigen::Vector3f::UnitY()) *
                   Eigen::AngleAxisf(eura_xyz.z(), Eigen::Vector3f::UnitZ());
    view_mat_.block<3, 3>(0, 0) = rotation_mat.transpose();
  }

  void setRotationQuat(const Eigen::Quaternionf &quat) {
    Eigen::Matrix3f rotation_mat = quat.toRotationMatrix();
    view_mat_.block<3, 3>(0, 0) = rotation_mat.transpose();
  }

  void setRotation(const Eigen::Matrix3f &r) {
    view_mat_.block<3, 3>(0, 0) = r;
  }

  const Eigen::Matrix4f &getViewMatrix() const noexcept { return view_mat_; }

  void setViewMatrix(const Eigen::Matrix4f &view_mat) {
    view_mat_ = view_mat;
  }

  const float getDis() const noexcept { return dis_; }

  const Eigen::Vector3f getCameraPos() const noexcept {
    return view_mat_.block<3, 3>(0, 0).transpose() *
           -view_mat_.block<3, 1>(0, 3); // [R T], 这里T是相机坐标系下(旋转之后)的偏移量. 因此在世界坐标系下是-R^T
  }

  const Eigen::Vector3f getCameraLookAt() const noexcept {
    return view_mat_.block<3,3>(0,0).transpose() * Eigen::Vector3f(0, 0, -1); // -z 向前, 再转换到世界坐标系
  }

  void rotate(const Eigen::Vector4f &normalized_mouse_pos)
  {
    doRotate(view_mat_, normalized_mouse_pos);     
  }

  /**
   * @brief 调整相机与目标的距离
   */
  void adjustDistance(const float delta)
  {
    doZoom(view_mat_, dis_, delta * 0.1f);
  }

  void pan(const Eigen::Vector4f &normalized_mouse_pos)
  {
    doPan(view_mat_, aspect_, dis_, normalized_mouse_pos);
  }

  //// for projection stuff  

  /**
   * \param near The near clipping plane distance from the camera < 0
   * \param far The far clipping plane distance from the camera < 0
   */
  void setClipPlanes(float n, float f) {
    assert(n < 0 && f < 0);
    near_ = n; // to camera coordinate
    far_ = f;
    dirty_proj_ = true;
  }

  /**
   * @brief set the fovy  in radians
   */
  void setFovy(float fovy) // height / focalLength
  {
    fovy_ = fovy;
    dirty_proj_ = true;
  }

  void setAspect(float aspect) // width / height
  {
    aspect_ = aspect;
    dirty_proj_ = true;
  }

  /**
   * \brief Get the projection matrix of the camera, right-handed coordinate
   * system depth: [0, 1] \return The projection matrix
   */
  const Eigen::Matrix4f &getProjMatrix() {
    if (!dirty_proj_)
      return proj_mat_;
    float f = 1.0f / tan(fovy_ * 0.5f);
    float r = 1.0f / (far_ - near_);
    proj_mat_ << f / aspect_, 0.0f, 0.0f, 0.0f, //
        0.0f, f, 0.0f, 0.0f, 0.0f,              //
        0.0f, -far_ * r, far_ * near_ * r,      //
        0.0f, 0.0f, -1.0f, 0.0f;
    dirty_proj_ = false;
    return proj_mat_;
  }  

  //// for exposure stuff
  void setExposureValue(const float ev100) {
    ev100_ = ev100;
  }
  float ev100() const noexcept { return ev100_; }

private:
  std::string name_;
  bool dirty_proj_{true};
  float near_{-0.1f};
  float far_{-1000.0f};
  float fovy_{M_PI * 0.333f};
  float aspect_{1.0f}; //!< width / height
  float dis_{1.0f};

  float ev100_{0.0f};

  Eigen::Matrix4f proj_mat_;
  Eigen::Matrix4f view_mat_{Eigen::Matrix4f::Identity()};
};
} // namespace mango