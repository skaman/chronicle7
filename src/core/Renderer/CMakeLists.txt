add_subdirectory(Common)
add_subdirectory(Data)
add_subdirectory(Vulkan)

target_sources(chronicle-core
PRIVATE
    "BaseCommandBuffer.h"
    "BaseDescriptorSetOld.h"
    "BaseFrameBuffer.h"
    "BaseIndexBuffer.h"
    "BasePipeline.h"
    "BaseRenderContext.h"
    "BaseRenderPass.h"
    
    "BaseShader.h"
    "BaseTexture.h"
    "BaseVertexBuffer.h"
    "Renderer.h"
)