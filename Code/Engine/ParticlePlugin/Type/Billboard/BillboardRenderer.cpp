#include <ParticlePlugin/PCH.h>
#include <ParticlePlugin/Type/Billboard/BillboardRenderer.h>
#include <RendererFoundation/Device/Device.h>
#include <RendererCore/RenderContext/RenderContext.h>
#include <RendererFoundation/Context/Context.h>
#include <RendererCore/Pipeline/RenderDataBatch.h>
#include <RendererCore/Shader/ShaderResource.h>

EZ_BEGIN_DYNAMIC_REFLECTED_TYPE(ezParticleBillboardRenderData, 1, ezRTTINoAllocator)
EZ_END_DYNAMIC_REFLECTED_TYPE

EZ_BEGIN_DYNAMIC_REFLECTED_TYPE(ezParticleBillboardRenderer, 1, ezRTTIDefaultAllocator<ezParticleBillboardRenderer>);
EZ_END_DYNAMIC_REFLECTED_TYPE

ezParticleBillboardRenderData::ezParticleBillboardRenderData()
{
  m_uiNumParticles = 0;
}


ezParticleBillboardRenderer::ezParticleBillboardRenderer()
{
  int i = 0;
}

void ezParticleBillboardRenderer::GetSupportedRenderDataTypes(ezHybridArray<const ezRTTI*, 8>& types)
{
  types.PushBack(ezGetStaticRTTI<ezParticleBillboardRenderData>());
}

void ezParticleBillboardRenderer::CreateDataBuffer()
{
  if (m_hDataBuffer.IsInvalidated())
  {
    ezGALBufferCreationDescription desc;
    desc.m_uiStructSize = sizeof(ezBillboardParticleData);
    desc.m_uiTotalSize = s_uiMaxBufferSize;
    desc.m_BufferType = ezGALBufferType::Generic;
    desc.m_bUseAsStructuredBuffer = true;
    desc.m_bAllowShaderResourceView = true;
    desc.m_ResourceAccess.m_bImmutable = false;

    m_hDataBuffer = ezGALDevice::GetDefaultDevice()->CreateBuffer(desc);
  }
}

void ezParticleBillboardRenderer::RenderBatch(const ezRenderViewContext& renderViewContext, ezRenderPipelinePass* pPass, const ezRenderDataBatch& batch)
{
  ezGALDevice* pDevice = ezGALDevice::GetDefaultDevice();
  ezRenderContext* pContext = renderViewContext.m_pRenderContext;
  ezGALContext* pGALContext = pContext->GetGALContext();

  // Bind the billboard particle shader
  {
    if (!m_hShader.IsValid())
    {
      m_hShader = ezResourceManager::LoadResource<ezShaderResource>("Shaders/Particles/Billboards.ezShader");
    }

    renderViewContext.m_pRenderContext->BindShader(m_hShader);
  }

  const ezUInt32 uiParticlesPerBatch(s_uiMaxBufferSize / sizeof(ezBillboardParticleData));

  // make sure our structured buffer is allocated and bound
  {
    CreateDataBuffer();
    renderViewContext.m_pRenderContext->BindMeshBuffer(ezGALBufferHandle(), ezGALBufferHandle(), nullptr, ezGALPrimitiveTopology::Triangles, uiParticlesPerBatch * 2);
    renderViewContext.m_pRenderContext->BindBuffer(ezGALShaderStage::VertexShader, "particleData", pDevice->GetDefaultResourceView(m_hDataBuffer));
  }

  // now render all particle effects of type billboard
  for (auto it = batch.GetIterator<ezParticleBillboardRenderData>(0, batch.GetCount()); it.IsValid(); ++it)
  {
    const ezParticleBillboardRenderData* pRenderData = it;
    ezUInt32 uiNumParticles = pRenderData->m_uiNumParticles;

    const ezBillboardParticleData* pParticleData = pRenderData->m_GpuData->m_Content.GetData();

    renderViewContext.m_pRenderContext->BindTexture(ezGALShaderStage::PixelShader, "ParticleTexture", pRenderData->m_hTexture);

    while (uiNumParticles > 0)
    {
      // upload this batch of particle data
      const ezUInt32 uiNumParticlesInBatch = ezMath::Min<ezUInt32>(uiNumParticles, uiParticlesPerBatch);
      pGALContext->UpdateBuffer(m_hDataBuffer, 0, ezMakeArrayPtr(pParticleData, uiNumParticlesInBatch).ToByteArray());

      // do one drawcall
      renderViewContext.m_pRenderContext->DrawMeshBuffer(uiNumParticlesInBatch * 2);

      uiNumParticles -= uiNumParticlesInBatch;
      pParticleData += uiNumParticlesInBatch;
    }
  }
}

