#include <fastgltf/core.hpp>
#include <fastgltf/tools.hpp>

#include "Logging.h"
#include "assets/AssetInfo.h"
#include "assets/AssetRegistery.h"
#include "assets/File.h"
#include "assets/GltfElementTraits.h"
#include "core/Mesh.h"
#include "rendering/MeshGpuAllocator.h"
#include "util/PagedStorage.h"

using namespace core;
using namespace assets;
using namespace file;
using namespace math;

namespace
{
    fastgltf::Parser gltfParser = fastgltf::Parser();
    gpu::MeshGpuAllocator allocator;
    util::PagedStorage<Mesh, 64> meshStorage;

    constexpr vec3 transformGltfToEngineCoordinateSpace(const vec3& in)
    {
        return vec3(in.x, -in.z, in.y);
    }
}

Mesh::~Mesh()
{
    allocator.destroyMesh(gpuHandle);
}

void Mesh::recalculateBounds()
{
    bounds = boundsFromVertices(vertices);
};

AssetRef<Mesh> Mesh::loadFromFile(std::string_view path)
{
    AssetId id = AssetId::idFromPath(path);

    fs::path absPath = fs::path(resolveResourcePath(path));

    auto data = fastgltf::GltfDataBuffer::FromPath(absPath);
    if (data.error() != fastgltf::Error::None)
    {
        spdlog::error("Error loading gltf/glb file at {}: {}::{}", path, getErrorName(data.error()), getErrorMessage(data.error()));
        return AssetRef<Mesh>::null();
    }

    constexpr fastgltf::Options parserOptions = fastgltf::Options::LoadExternalBuffers;

    auto load = gltfParser.loadGltfBinary(data.get(), absPath.parent_path(), parserOptions);
    if (auto error = load.error(); error != fastgltf::Error::None)
    {
        spdlog::error("Error parsing gltf/glb file at {}: {}::{}", path, getErrorName(data.error()), getErrorMessage(data.error()));
    }

    fastgltf::Asset asset = std::move(load.get());

    const fastgltf::Mesh& mesh = asset.meshes[0];

    Mesh out;

    size_t vertex_base = 0;

    for (const auto& primitive : mesh.primitives)
    {
        // position
        auto posAttribute = primitive.findAttribute("POSITION");
        if (posAttribute == nullptr)
        {
            spdlog::error("No POSITION attribute (TODO add more info)"); // TODO add more info
            continue;
        }
        const fastgltf::Accessor& posAccessor = asset.accessors[posAttribute->accessorIndex];

        size_t count = posAccessor.count;
        out.vertices.resize(vertex_base + count);

        fastgltf::iterateAccessorWithIndex<vec3>(asset, posAccessor,
            [&](vec3 pos, size_t index)
            {
                Vertex v;
                v.position = transformGltfToEngineCoordinateSpace(pos);
                v.normal = vec3::zero;
                v.uv0 = vec2::one;
                out.vertices[vertex_base + index] = v;
            });


        // normals
        auto normalAttribute = primitive.findAttribute("NORMAL");
        if (normalAttribute)
        {
            const fastgltf::Accessor& normalAccessor = asset.accessors[normalAttribute->accessorIndex];
            fastgltf::iterateAccessorWithIndex<vec3>(asset, normalAccessor,
                [&](vec3 normal, size_t index)
                {
                    out.vertices[vertex_base + index].normal = transformGltfToEngineCoordinateSpace(normal);
                });
        }
        else
        {
            // TODO calc normals?
        }

        if (primitive.indicesAccessor.has_value())
        {
            const fastgltf::Accessor& indexAccessor = asset.accessors[primitive.indicesAccessor.value()];
            out.indices.resize(indexAccessor.count);
            fastgltf::copyFromAccessor<uint32_t>(asset, indexAccessor, out.indices.data());
        }

        auto texcoordAttribute = primitive.findAttribute("TEXCOORD_0");
        if (texcoordAttribute)
        {
            const fastgltf::Accessor& texAccessor = asset.accessors[texcoordAttribute->accessorIndex];
            fastgltf::iterateAccessorWithIndex<vec2>(asset, texAccessor,
                [&](vec2 texcoord, size_t index)
                {
                    out.vertices[vertex_base+index].uv0 = texcoord;
                });
        }

        vertex_base += count;
    }

    Mesh* outMesh = meshStorage.allocate_uninitialized();
    ::new (outMesh) Mesh();

    outMesh->vertices = std::move(out.vertices);
    outMesh->indices = std::move(out.indices);
    outMesh->recalculateBounds();

    outMesh->gpuHandle = allocator.uploadMesh(*outMesh);

    registerAsset(path, AssetType::Mesh, outMesh);
    return AssetRef(outMesh, id);
}

AssetRef<Mesh> Mesh::create()
{
    ENGINE_ASSERT(false, "TODO not implemented");
}
