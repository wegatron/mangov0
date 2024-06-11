#include <engine/asset/asset_skeleton.h>
#include <engine/functional/global/engine_context.h>

namespace mango {
void Skeleton::inflate()
{
    auto driver = g_engine.getDriver();
    transform_buffer_ = std::make_shared<Buffer>(
        driver,
        bones_.size() * sizeof(Eigen::Matrix4f),
        VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
        0,
        VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT,
        VMA_MEMORY_USAGE_AUTO);
    std::vector<Eigen::Matrix4f> bone_transforms(bones_.size());
    for (size_t i = 0; i < bones_.size(); i++)
    {
        bone_transforms[i] = bones_[i].getTransform();
    }
    transform_buffer_->update(bone_transforms.data(), bones_.size() * sizeof(Eigen::Matrix4f), 0);
}
}