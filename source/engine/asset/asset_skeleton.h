#pragma once

#include <engine/asset/asset.h>
#include <Eigen/Dense>

namespace mango
{
class Bone {
public:
  Bone() = default;
  ~Bone() = default;
  
  void setRotation(const Eigen::Quaternionf &rotation);

  void setScale(const Eigen::Vector3f &scale);

  void setTranslation(const Eigen::Vector3f &translation);

  void update(const Eigen::Matrix4f &parent_global_bind_pose)
  {
    global_bind_pose_ = parent_global_bind_pose * local_bind_pose_;    
  }

  Eigen::Matrix4f getTransform() const
  {
    return global_bind_pose_ * global_inverse_bind_pose_;
  }

private:
  std::string name_;
  uint32_t parent_index_;

  Eigen::Matrix4f local_bind_pose_; //!< transform from local bind space into parent space
  Eigen::Matrix4f global_bind_pose_; //!< transform from local bind space into global space  
  Eigen::Matrix4f global_inverse_bind_pose_; //!< transform from global space into local bind space
};
class Skeleton : public Asset {
public:
  Skeleton() = default;
  ~Skeleton() = default;
};
}