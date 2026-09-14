#pragma once

#include <vector>

#include "datatype.h"
#include "assets/Mesh.h"
#include "rendering/DataLayout.h"
#include "rendering/Framebuffer.h"
#include "rendering/GraphicsBuffer.h"
#include "rendering/Material.h"
#include "rendering/MeshGpuHandle.h"
#include "rendering/RenderingDataStructures.h"

namespace core::gfx
{
    class Pipeline;

    struct DrawCommand
    {
        enum class RenderQueue : u8 { OPAQUE, UI, SHADOW, INVALID };

        u64 sortKey;
        gpu::MeshGpuHandle mesh;
        assets::AssetRef<Material> material = assets::AssetRef<Material>::null();
        RenderQueue queue = RenderQueue::INVALID;
        InstanceData instanceData;

        u16 getPipelineId() const { return (sortKey >> 32) & 0xFFFF; }
        u16 getMaterialId() const { return (sortKey >> 16) & 0xFFFF; }
        u16 getMeshId() const { return sortKey & 0xFFFF; }
    };

    struct PointLight
    {
        math::vec3 position = math::vec3::zero;
        Color color = Color::black;
        float intensity = 1;
        float range;
    };

    struct DirectionalLight
    {
        math::vec3 direction = math::vec3::right;
        Color color = Color::black;
        float intensity = 1;
    };

    struct Spotlight
    {
        math::vec3 position = math::vec3::zero;
        math::vec3 direction = math::vec3::right;
        float innerCutoff;
        float outerCutoff;
        float range;

        Color color = Color::black;
        float intensity = 1;
    };

    struct ShadowData
    {
        math::mat4 viewMatrix = math::mat4::identity;
        math::mat4 projectionMatrix = math::mat4::identity;
        math::mat4 viewProjectionMatrix = math::mat4::identity;
    };

    struct EnvironmentSettings
    {
        float ambientIntensity = .01f;
    };

    static_assert(std::is_trivially_destructible_v<DrawCommand>, "DrawCommand should be a trivially destructible type for good performance.");

    class Renderer
    {
        using CommandQueue = std::vector<DrawCommand>;

        struct FrameStats
        {
            i32 numDrawCalls = 0;
            i32 numPipelineBinds = 0;
            i32 numCommands = 0;
            i32 numShadowCasters = 0;
            u64 triCount = 0;
        } _frameStats;

        ViewportData _viewportData = ViewportData();
        EnvironmentSettings _environmentSettings = EnvironmentSettings();
        std::vector<PointLight> _pointLights = std::vector<PointLight>();
        std::vector<DirectionalLight> _dirLights = std::vector<DirectionalLight>();
        std::vector<Spotlight> _spotlights = std::vector<Spotlight>();
        Color _clearColor;
        int _shadowMapResolution = 512;

        CommandQueue _shadowCommandQueue;
        CommandQueue _opaqueCommandQueue;
        CommandQueue _uiCommandQueue;

        gl::buffer_t _viewportDataUboHandle;
        gl::buffer_t _frameDataUboHandle;
        gl::buffer_t _lightingDataUboHandle;

        GraphicsBuffer _instanceDataBuffer;

        Framebuffer _mainFramebuffer;
        Framebuffer _shadowFramebuffer;
        std::array<Framebuffer, 2> _pingPongFramebuffers;

        gl::Uint _defaultSampler = 0; // TODO define samplers as a type
        gl::Uint _shadowSampler = 0;
        gl::vert_arr_t _emptyVao = 0;

        assets::AssetRef<Material> _depthMaterial;
        assets::AssetRef<Material> _fullscreenBlitEffect;
        std::vector<assets::AssetRef<Material>> _postEffects;

    public:
        Renderer();

        void init(int fbWidth, int fbHeight, int shadowMapResolution);

        [[nodiscard]] const FrameStats& getFrameStats() const noexcept { return _frameStats; }

        void setViewportData(const ViewportData& params);
        void setEnvironmentSettings(const EnvironmentSettings& env);
        void setPostEffectStack(const std::vector<assets::AssetRef<Material>>& stack);
        void setClearColor(const Color& clearColor);

        void submitPointLight(const PointLight& light);
        void submitDirectionalLight(const DirectionalLight& light);
        void submitSpotlight(const Spotlight& spotlight);

        void submitDrawCommand(const DrawCommand& command, bool castShadow);

        void renderFrame();

        static u64 buildSortKey(assets::AssetRef<Material> material, u16 meshKey);

    private:
        void bindPipeline(const Pipeline& pipeline);
        void bindMaterial(assets::AssetRef<Material> material);
        void bindPassData(const ViewportDataBlock& passData) const;
        void bindFrameData(const FrameDataBlock& passData) const;
        void bindLightingData(const LightingDataBlock& lightingData) const;

        void resizeFrameBuffers(int newWidth, int newHeight);

        void appendInstanceData(CommandQueue& queue);
        LightingDataBlock collectLightingData() const;

        void executePass(const ViewportDataBlock& passData, CommandQueue& queue, usize instanceIndex);
        void executePostEffectStack();
        void executePostEffect(assets::AssetRef<Material> material, const Framebuffer& src, gl::framebuffer_t dst);

        static void sortCommandList(std::vector<DrawCommand>& queue);
    };
}
