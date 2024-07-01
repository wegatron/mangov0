#pragma once

#include <memory>

namespace mango {

class StaticMesh;
class Material;
class TransformRelationship;

using StaticMeshComponent = std::shared_ptr<StaticMesh>;
using MaterialComponent = std::shared_ptr<Material>;
using TransformComponent = std::shared_ptr<TransformRelationship>;

} // namespace mango