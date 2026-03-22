#pragma once
#ifndef NOMINMAX
#define NOMINMAX
#endif

#include "../Base/ActionSceneBase.h"
#include "../SceneManager/SceneManager.h"
#include "../../Utility/Sound/AudioTypes.h"

#include <d3d11.h>
#include <SimpleMath.h>
#include <SpriteBatch.h>
#include <wrl/client.h>

class SettingsScene : public ActionSceneBase
{
public:
    explicit SettingsScene(SceneManager* sceneManager);
    ~SettingsScene() override;

    void Initialize() override;
    void Update(const DX::StepTimer& stepTimer) override;
    void Render() override;
    void Finalize() override;

private:
    void DrawSolidRect(
        DirectX::SpriteBatch* batch,
        const DirectX::SimpleMath::Vector2& position,
        const DirectX::SimpleMath::Vector2& size,
        const DirectX::SimpleMath::Color& color) const;

    void ApplyMouseSensitivity(float value);
    float GetSettingValue(int index) const;
    void SetSettingValue(int index, float value);
    void ApplyAudioBusVolume(GameAudio::AudioBus bus, float value);
    void ReturnToBackScene(SceneID backScene);

    SceneID m_backScene = TITLE_SCENE; // 險ｭ螳夂判髱｢繧帝幕縺丞燕縺ｮ繧ｷ繝ｼ繝ｳ

private:
    int m_selectedItem = 0;
    float m_sceneTime = 0.0f;
    float m_mouseSensitivity = 0.08f;
    GameAudio::AudioVolumeSettings m_audioVolume;
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_uiSolidTexture;
    float m_clickFxTimer = 0.0f;
    DirectX::SimpleMath::Vector2 m_clickFxPos = DirectX::SimpleMath::Vector2::Zero;
};

