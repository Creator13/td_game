#include "Application.h"

#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include <spdlog/spdlog.h>

#include <tracy/Tracy.hpp>
#include <tracy/TracyOpenGL.hpp>

#include "EcsDebug.h"
#include "core/EcsCore.h"
#include "core/Time.h"
#include "core/Window.h"
#include "rendering/EcsRendering.h"
#include "rendering/Renderer.h"

using namespace core;

namespace
{
    void glfw_errorCallback(int code, const char* description)
    {
        spdlog::error("GLFW Error {}: {}", code, description);
    }

    constexpr std::string_view glDebugEnumToString(GLenum e) noexcept
    {
        switch (e)
        {
            case GL_DEBUG_SOURCE_API: return "API";
            case GL_DEBUG_SOURCE_WINDOW_SYSTEM: return "WindowSystem";
            case GL_DEBUG_SOURCE_SHADER_COMPILER: return "ShaderCompiler";
            case GL_DEBUG_SOURCE_THIRD_PARTY: return "ThirdParty";
            case GL_DEBUG_SOURCE_APPLICATION: return "Application";
            case GL_DEBUG_SOURCE_OTHER: return "Other";

            case GL_DEBUG_TYPE_ERROR: return "Error";
            case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: return "Deprecated";
            case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR: return "Undefined";
            case GL_DEBUG_TYPE_PORTABILITY: return "Portability";
            case GL_DEBUG_TYPE_PERFORMANCE: return "Performance";
            case GL_DEBUG_TYPE_MARKER: return "Marker";
            case GL_DEBUG_TYPE_PUSH_GROUP: return "PushGroup";
            case GL_DEBUG_TYPE_POP_GROUP: return "PopGroup";
            case GL_DEBUG_TYPE_OTHER: return "Other";

            default: return "Unknown";
        }
    }

    void APIENTRY openglDebugOutputCallback(
        GLenum source, GLenum type,
        GLuint id, GLenum severity,
        GLsizei _length,
        const GLchar* msg,
        const void* _userParam)
    {
        if (id == 131169 || id == 13115 || id == 131218 || id == 131204) return;

        spdlog::level::level_enum level;
        switch (severity)
        {
            case GL_DEBUG_SEVERITY_HIGH:
                level = spdlog::level::critical;
                break;
            case GL_DEBUG_SEVERITY_MEDIUM:
                level = spdlog::level::err;
                break;
            case GL_DEBUG_SEVERITY_LOW:
                level = spdlog::level::warn;
                break;
            case GL_DEBUG_SEVERITY_NOTIFICATION:
                level = spdlog::level::debug;
                break;
            default:
                level = spdlog::level::info;
                break;
        }

        if (level == spdlog::level::critical)
        {
            ENGINE_ASSERT(false, "OpenGL debug - [{}] (t:{}|src:{}) : {}", id, glDebugEnumToString(type), glDebugEnumToString(source), msg);
        }
        else
        {
            spdlog::log(level, "OpenGL debug - [{}] (t:{}|src:{}) : {}",
                id, glDebugEnumToString(type), glDebugEnumToString(source), msg);
        }
    }
}

Application::Application(int argc, char* argv[], std::string_view resourceRoot, const WindowState& windowState)
    : _ecs(argc, argv), _windowState(windowState)
{
    spdlog::set_level(spdlog::level::debug);

    createWindow(windowState);

    // Asset database relies on an opengl context and cannot be created before opengl is initialized (createWindow initializes opengl)
    _assetDb = std::make_unique<assets::AssetDatabase>(resourceRoot);
    assets::bindAssetDatabase(*_assetDb);
    _renderer = std::make_unique<gfx::Renderer>();
    _debugRenderer = std::make_unique<debug::DebugRenderer>();
    debug::bindDebugRenderer(*_debugRenderer);

    initFlecs();
}

Application::~Application()
{
    cleanup();
}

int Application::run()
{
    time::init();

    while (!glfwWindowShouldClose(_windowPtr))
    {
        FrameMark;

        glfwPollEvents();

        time::markFrame();

        if (!_ecs.progress())
        {
            glfwSetWindowShouldClose(_windowPtr, GLFW_TRUE);
        }

        _renderer->renderFrame();
        _debugRenderer->render();

        glfwSwapBuffers(_windowPtr);
        TracyGpuCollect;

        _inputState.endFrame();
    }

    return 0;
}

bool Application::createWindow(const WindowState& windowState)
{
    if (!glfwInit())
    {
        spdlog::error("Failed to initialize GLFW");
        return false;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);


#ifdef DEBUG_BUILD
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);
#endif

    _windowPtr = glfwCreateWindow(windowState.width, windowState.height, windowState.title.c_str(), nullptr, nullptr);
    if (!_windowPtr)
    {
        spdlog::error("Failed to create GLFW window.");
        glfwTerminate();
    }

    glfwMakeContextCurrent(_windowPtr);

    glfwSetFramebufferSizeCallback(_windowPtr, core::glfw_framebufferSizeCallback);

    if (int version = gladLoadGL(glfwGetProcAddress); version == 0)
    {
        spdlog::error("Failed to initialize GLAD");
        return false;
    }
    TracyGpuContext;

    glfwSetErrorCallback(glfw_errorCallback);

#ifdef DEBUG_BUILD
    int flags;
    glGetIntegerv(GL_CONTEXT_FLAGS, &flags);
    if (flags & GL_CONTEXT_FLAG_DEBUG_BIT)
    {
        glEnable(GL_DEBUG_OUTPUT);
        glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
        glDebugMessageCallback(openglDebugOutputCallback, nullptr);
        glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE);
    }
#endif

    glfwSwapInterval(0);

    _glfwOwnerContext = {
        &_inputState, &_windowState
    };
    glfwSetWindowUserPointer(_windowPtr, &_glfwOwnerContext);

    glfwSetCursorPosCallback(_windowPtr, glfw_cursorPosCallback);
    glfwSetKeyCallback(_windowPtr, glfw_keyCallback);
    glfwSetMouseButtonCallback(_windowPtr, glfw_mouseButtonCallback);

    return true;
}

void Application::initFlecs()
{
#ifdef FLECS_LOG
    flecs::log::enable_colors(false);
#endif

    const u32 numOsThreads = std::thread::hardware_concurrency();
    if (numOsThreads == 0)
    {
        // if value is 0, something went wrong in obtaining the value and we cannot rely on it, so assume 2 is a decent value
        spdlog::debug("Failed to detect cpu threads; spawning 2 flecs worker threads.");
        _ecs.set_threads(2);
    }
    else
    {
        const i32 numFlecsThreads = numOsThreads - 2;
        spdlog::debug("Detected {} threads, spawning {} flecs worker threads.", numOsThreads, numFlecsThreads);
        _ecs.set_threads(math::max(0, numFlecsThreads));
    }

    _ecs.import<ecs::engine_core>();
    _ecs.import<ecs::rendering>();

    // Add debug modules if in debug build TODO change this to dev build, they are not always compiled in debug mode
#ifdef DEBUG_BUILD
    _ecs.import<flecs::stats>();
    _ecs.set<flecs::Rest>({ });
    spdlog::debug("Open flecs explorer at https://flecs.dev/explorer");

    _ecs.import<::debug::ecs::engine_debug>();
    _ecs.set<::debug::ecs::DebugRendererSingleton>({_debugRenderer.get()});
#endif

    _ecs.component<ecs::WindowSingleton>().add(flecs::Singleton);

    // Attach application state to flecs singletons (relies on engine_core module)
    _ecs.set<ecs::WindowSingleton>({&_windowState});
    _ecs.set<ecs::RendererSingleton>({_renderer.get()});
    _ecs.set<ecs::GlobalInput>({&_inputState});
}

void Application::cleanup()
{
    cleanWindow();
}

void Application::cleanWindow()
{
    glfwDestroyWindow(_windowPtr);
    glfwTerminate();
}
