#pragma once

#include <engine/utils/vk/buffer.h>
#include <engine/asset/asset.h>
#include <Eigen/Dense>

namespace mango
{
class Bone {
public:
  /**
   * @brief Construct a new Bone object
   * local_transform = translation * rotation * scale
   */
  Bone() = default;
  ~Bone() = default;
  
  void setScale(const Eigen::Vector3f &scale)
  {
    scale_ = scale;
  }

  void setRotation(const Eigen::Quaternionf &rotation)
  {
    rotation_ = rotation;
  }

  void setTranslation(const Eigen::Vector3f &translation)
  {
    translation_ = translation;
  }

  void update(const Eigen::Matrix4f &parent_global_bind_pose)
  {
    Eigen::Affine3f tr = Eigen::Translation3f(translation_) * rotation_ * Eigen::Scaling(scale_);
    global_bind_pose_ = parent_global_bind_pose * tr.matrix();
  }

  Eigen::Matrix4f getTransform() const
  {
    return global_bind_pose_ * global_inverse_bind_pose_;
  }

private:
  std::string name_;
  uint32_t parent_index_;
  
  // local transform
  Eigen::Vector3f scale_;
  Eigen::Vector3f translation_;
  Eigen::Quaternionf rotation_;

  Eigen::Matrix4f global_bind_pose_; //!< transform from local bind space into global space  
  Eigen::Matrix4f global_inverse_bind_pose_; //!< transform from global space into local bind space
};
class Skeleton : public Asset {
public:
  Skeleton() = default;
  ~Skeleton() = default;

  std::shared_ptr<Buffer> getTransformBuffer() const { return transform_buffer_; }

  void inflate() override;

  void setBones(uint32_t root_index, std::vector<Bone> &&bones)
  {
    root_index_ = root_index;
    bones_ = std::move(bones);
  }

private:
  uint32_t root_index_{0};
  std::vector<Bone> bones_;
  std::shared_ptr<Buffer> transform_buffer_;
};
}