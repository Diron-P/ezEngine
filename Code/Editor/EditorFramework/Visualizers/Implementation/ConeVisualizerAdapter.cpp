#include <EditorFramework/EditorFrameworkPCH.h>

#include <EditorFramework/Assets/AssetDocument.h>
#include <EditorFramework/Visualizers/ConeVisualizerAdapter.h>
#include <ToolsFoundation/Object/ObjectAccessorBase.h>

ezConeVisualizerAdapter::ezConeVisualizerAdapter() = default;

ezConeVisualizerAdapter::~ezConeVisualizerAdapter() = default;

void ezConeVisualizerAdapter::Finalize()
{
  auto* pDoc = m_pObject->GetDocumentObjectManager()->GetDocument()->GetMainDocument();
  const ezAssetDocument* pAssetDocument = ezDynamicCast<const ezAssetDocument*>(pDoc);
  EZ_ASSERT_DEV(pAssetDocument != nullptr, "Visualizers are only supported in ezAssetDocument.");

  const ezConeVisualizerAttribute* pAttr = static_cast<const ezConeVisualizerAttribute*>(m_pVisualizerAttr);

  m_hGizmo.ConfigureHandle(nullptr, ezEngineGizmoHandleType::Cone, pAttr->m_Color, ezGizmoFlags::ShowInOrtho | ezGizmoFlags::Visualizer);

  pAssetDocument->AddSyncObject(&m_hGizmo);
  m_hGizmo.SetVisible(m_bVisualizerIsVisible);
}

void ezConeVisualizerAdapter::Update()
{
  const ezConeVisualizerAttribute* pAttr = static_cast<const ezConeVisualizerAttribute*>(m_pVisualizerAttr);
  ezObjectAccessorBase* pObjectAccessor = GetObjectAccessor();

  m_fAngleScale = 1.0f;
  if (!pAttr->GetAngleProperty().IsEmpty())
  {
    ezVariant value;
    pObjectAccessor->GetValue(m_pObject, GetProperty(pAttr->GetAngleProperty()), value).AssertSuccess();

    EZ_ASSERT_DEBUG(value.IsValid() && value.CanConvertTo<ezAngle>(), "Invalid property bound to ezConeVisualizerAttribute 'angle'");
    m_fAngleScale = ezMath::Tan(value.ConvertTo<ezAngle>() * 0.5f);
  }

  if (!pAttr->GetColorProperty().IsEmpty())
  {
    ezVariant value;
    pObjectAccessor->GetValue(m_pObject, GetProperty(pAttr->GetColorProperty()), value).AssertSuccess();

    EZ_ASSERT_DEBUG(value.IsValid() && value.CanConvertTo<ezColor>(), "Invalid property bound to ezConeVisualizerAttribute 'color'");
    m_hGizmo.SetColor(value.ConvertTo<ezColor>());
  }

  m_fFinalScale = pAttr->m_fScale;
  if (!pAttr->GetRadiusProperty().IsEmpty())
  {
    ezVariant value;
    pObjectAccessor->GetValue(m_pObject, GetProperty(pAttr->GetRadiusProperty()), value).AssertSuccess();

    EZ_ASSERT_DEBUG(value.IsValid() && value.CanConvertTo<float>(), "Invalid property bound to ezConeVisualizerAttribute 'radius'");
    m_fFinalScale *= value.ConvertTo<float>();
  }

  m_hGizmo.SetVisible(m_bVisualizerIsVisible && m_fAngleScale != 0.0f && m_fFinalScale != 0.0f);
}

void ezConeVisualizerAdapter::UpdateGizmoTransform()
{
  const ezConeVisualizerAttribute* pAttr = static_cast<const ezConeVisualizerAttribute*>(m_pVisualizerAttr);

  const ezQuat axisRotation = ezBasisAxis::GetBasisRotation_PosX(pAttr->m_Axis);

  ezTransform t = GetObjectTransform();
  t.m_vScale = t.m_vScale.CompMul(ezVec3(1.0f, m_fAngleScale, m_fAngleScale) * m_fFinalScale);
  t.m_qRotation = axisRotation * t.m_qRotation;
  m_hGizmo.SetTransformation(t);
}