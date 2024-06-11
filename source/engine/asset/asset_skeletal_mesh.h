#pragma once

#include <engine/asset/asset_mesh.h>

namespace mango {

struct SkeletalVertex : public StaticVertex
{
	Eigen::Vector4i bone_indices;
	Eigen::Vector4f bone_weights;
};

/**
 * @brief SkeletalMesh is a mesh with bones and weights
 * 
 */
class SkeletalMesh : public Mesh, public Asset {
public:
    void inflate() override;
    void setVertices(std::vector<SkeletalVertex> &&vertices) { vertices_ = std::move(vertices); }
protected:
    void calcBoundingBox() override;
private:
    std::vector<SkeletalVertex> vertices_;
};
} // namespace mango