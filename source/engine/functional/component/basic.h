#pragma once

#include <Eigen/Dense>
#include <Eigen/Geometry>

namespace mango
{
    struct TransformRelationship
    {
        std::shared_ptr<TransformRelationship> parent;
        std::shared_ptr<TransformRelationship> child;
        std::shared_ptr<TransformRelationship> sibling;
        Eigen::Matrix4f ltransform{Eigen::Matrix4f::Identity()}; // local transformation
        Eigen::Matrix4f gtransform{Eigen::Matrix4f::Identity()}; // global transformation
        Eigen::AlignedBox3f aabb; // aabb of meshes in this node, not including children
    };
}