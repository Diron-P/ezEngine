#pragma once

#include <GameEngine/XR/XRSwapChain.h>
#include <OpenXRPlugin/Basics.h>
#include <OpenXRPlugin/OpenXRIncludes.h>

class ezOpenXR;

EZ_DEFINE_AS_POD_TYPE(XrSwapchainImageD3D11KHR);

class EZ_OPENXRPLUGIN_DLL ezGALOpenXRSwapChain : public ezGALXRSwapChain
{
public:
  ezSizeU32 GetRenderTargetSize() const { return m_CurrentSize; }
  XrSwapchain GetColorSwapchain() const { return m_ColorSwapchain.handle; }
  XrSwapchain GetDepthSwapchain() const { return m_DepthSwapchain.handle; }

  virtual void AcquireNextRenderTarget(ezGALDevice* pDevice) override;
  virtual void PresentRenderTarget(ezGALDevice* pDevice) override;
  void PresentRenderTarget() const;

protected:
  virtual ezResult InitPlatform(ezGALDevice* pDevice) override;
  virtual ezResult DeInitPlatform(ezGALDevice* pDevice) override;

private:
  friend class ezOpenXR;
  struct Swapchain
  {
    XrSwapchain handle = 0;
    int64_t format = 0;
    ezUInt32 imageCount = 0;
    XrSwapchainImageBaseHeader* images = nullptr;
    uint32_t imageIndex = 0;
  };
  enum class SwapchainType
  {
    Color,
    Depth,
  };

private:
  ezGALOpenXRSwapChain(ezOpenXR* pXrInterface, ezGALMSAASampleCount::Enum msaaCount);
  XrResult SelectSwapchainFormat(int64_t& colorFormat, int64_t& depthFormat);
  XrResult CreateSwapchainImages(Swapchain& swapchain, SwapchainType type);
  XrResult InitSwapChain(ezGALMSAASampleCount::Enum msaaCount);
  void DeinitSwapChain();

private:
  XrInstance m_pInstance = XR_NULL_HANDLE;
  uint64_t m_SystemId = XR_NULL_SYSTEM_ID;
  XrSession m_pSession = XR_NULL_HANDLE;
  ezEnum<ezGALMSAASampleCount> m_MsaaCount;

  // Swapchain
  XrViewConfigurationView m_PrimaryConfigView;
  Swapchain m_ColorSwapchain;
  Swapchain m_DepthSwapchain;

  ezHybridArray<XrSwapchainImageD3D11KHR, 3> m_ColorSwapChainImagesD3D11;
  ezHybridArray<XrSwapchainImageD3D11KHR, 3> m_DepthSwapChainImagesD3D11;
  ezHybridArray<ezGALTextureHandle, 3> m_ColorRTs;
  ezHybridArray<ezGALTextureHandle, 3> m_DepthRTs;

  bool m_bImageAcquired = false;
  ezGALTextureHandle m_hColorRT;
  ezGALTextureHandle m_hDepthRT;
};