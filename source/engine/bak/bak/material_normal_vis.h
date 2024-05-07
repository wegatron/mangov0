#pragma once

#include <engine/functional/component/material.h>

namespace mango {
class NormalVisMaterial : public Material {
public:
  /**
   * \brief Constructor of PBR material defines all possible input
   * parameters/textures during construction. During compilation, the
   * corresponding descriptor layout is set based on the configured inputs.
   */
  NormalVisMaterial();

  ~NormalVisMaterial() override = default;

  void setPipelineState(GPipelineState &pipeline_state) override;

  void compile() override;

protected:
  /**
   * \brief Create paramset including material's uniform buffer and material's
   * descriptor.
   *
   * This will invoke when requestDescriptor for rendering.
   * For different variations, we keep the uniform buffer of the material
   * unchanged, even though some parameters may not be used. An image is
   * considered an external input parameter for textures, unlike the uniform
   * buffer, and is not created by the Material.
   */
  std::shared_ptr<MatParamsSet>
  createMatParamsSet(const std::shared_ptr<VkDriver> &driver,
                     DescriptorPool &desc_pool) override;
};

} // namespace mango