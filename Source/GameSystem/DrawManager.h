//------------------------//------------------------
// Contents(処理内容) 2Dスプライト描画管理を宣言する。
//------------------------//------------------------
// user(作成者) Keishi Teramoto
// Created date(作成日) 2026 / 03 / 16
// last updated (最終更新日) 2026 / 03 / 17
//------------------------//------------------------
#pragma once

//----------------------------------------------------------------------------- 
// DrawManager
//----------------------------------------------------------------------------- 
// 2D描画の共通窓口。
// DrawData で描画パラメータを受け取り、SpriteBatchで一括描画する。
//----------------------------------------------------------------------------- 

#include <memory>

#include "WICTextureLoader.h"
#include <d3d11.h>
#include <SimpleMath.h>
#include <SpriteBatch.h>

#include "../Utility/Flag.h"
#include "../Utility/SingletonBase.h"

namespace System
{
	typedef ID3D11ShaderResourceView* DxTexture;

	// 1スプライト分の描画情報を保持する。
	struct DrawData
	{
	private:
		enum eDrawManagerFlag
		{
			NOT_TEXTURE_DELETE = (1 << 0)
		};

		// 破棄制御フラグ。
		Utility::Flag m_Flag;
		// 使用テクスチャ。
		DxTexture m_Texture;
		// 描画座標(スクリーン座標)。
		DirectX::SimpleMath::Vector2 m_Pos;
		// 回転角(ラジアン)。
		float m_Rot;
		// テクスチャ切り抜き領域。
		RECT m_Rect;
		// 回転・拡縮の基準点。
		DirectX::XMFLOAT2 m_Origin;
		// 拡縮率。
		DirectX::XMFLOAT2 m_scale;

	public:
		// 既定値で初期化する。
		DrawData()
			: m_Texture(nullptr)
			, m_Pos()
			, m_Rot(0.0f)
			, m_Rect()
			, m_Origin()
			, m_scale(1.0f, 1.0f)
		{
		}

		// 所有しているテクスチャ参照を必要に応じて解放する。
		~DrawData()
		{
			if (m_Texture == nullptr)
			{
				return;
			}

			if (m_Flag.Is(eDrawManagerFlag::NOT_TEXTURE_DELETE))
			{
				return;
			}

			m_Texture->Release();
		}

	public: // getter
		// 保持テクスチャへのポインタを返す。
		DxTexture* GetTexture() { return &m_Texture; }
		// 描画座標を返す。
		DirectX::SimpleMath::Vector2 GetPos() const { return m_Pos; }
		// 回転角を返す。
		float GetRot() const { return m_Rot; }
		// 切り抜き矩形を値コピーで返す。
		RECT GetRect() const { return m_Rect; }
		// 切り抜き矩形の参照を返す。
		const RECT* GetRectPtn() const { return &m_Rect; }
		// 基準点を返す。
		DirectX::XMFLOAT2 GetOrigin() const { return m_Origin; }
		// 拡縮率を返す。
		DirectX::XMFLOAT2 GetScale() const { return m_scale; }

	public: // setter
		// 外部所有テクスチャを設定し、本構造体では解放しない。
		void SetTexture(DxTexture pTexture)
		{
			m_Texture = pTexture;
			m_Flag.On(eDrawManagerFlag::NOT_TEXTURE_DELETE);
		}

		// 描画座標を設定する。
		void SetPos(float x, float y) { m_Pos.x = x; m_Pos.y = y; }
		// 描画座標を設定する。
		void SetPos(DirectX::SimpleMath::Vector2 pos) { m_Pos = pos; }
		// 回転角を設定する。
		void SetRotation(float r) { m_Rot = r; }

		// 切り抜き矩形を設定する。
		void SetRect(LONG left, LONG top, LONG right, LONG bottom)
		{
			m_Rect.left = left;
			m_Rect.top = top;
			m_Rect.right = right;
			m_Rect.bottom = bottom;
		}
		// 左上(0,0)基準で切り抜き矩形を設定する。
		void SetRect(LONG right, LONG bottom) { SetRect(0, 0, right, bottom); }

		// 基準点を設定する。
		void SetOrigin(float x, float y) { m_Origin.x = x; m_Origin.y = y; }
		// 基準点を設定する。
		void SetOrigin(DirectX::SimpleMath::Vector2 origin) { m_Origin = origin; }

		// 拡縮率を設定する。
		void SetScale(float x, float y) { m_scale.x = x; m_scale.y = y; }
		// 拡縮率を設定する。
		void SetScale(DirectX::SimpleMath::Vector2 scale) { m_scale = scale; }
	};

	class DrawManager : public Utility::SingletonBase<DrawManager>
	{
	public:
		friend class Utility::SingletonBase<DrawManager>;

	protected:
		// シングルトン生成専用コンストラクタ。
		DrawManager() {}
		// シングルトン破棄時に呼ばれるデストラクタ。
		~DrawManager() {}

	public:
		// 描画に必要なデバイス/コンテキストを設定する。
		void Initialize(ID3D11Device* pDevise, ID3D11DeviceContext* pContext);

		// DrawDataに従って1スプライト描画する。
		void Draw(DrawData& data);

		// SpriteBatchの描画開始処理。
		void Begin();
		// SpriteBatchの描画終了処理。
		void End();

		// 画像ファイルを読み込み、DrawDataへ設定する。
		bool LoadTexture(DrawData& data, wchar_t* pFileName);
		// 画像ファイルを読み込み、SRVへ設定する。
		bool LoadTexture(DxTexture& pTexture, wchar_t* pFileName);

		// 内部SpriteBatchを取得する。
		DirectX::SpriteBatch* GetSprite()
		{
			return mSpriteBatch.get();
		}

		// 使用デバイスを設定する。
		void SetDevise(ID3D11Device* pDevise)
		{
			mpDevice = pDevise;
		}

		// 使用デバイスを取得する。
		ID3D11Device* GetDevise()
		{
			return mpDevice;
		}

	private:
		// 描画デバイス。
		ID3D11Device* mpDevice;
		// 2Dスプライト描画バッチ。
		std::unique_ptr<DirectX::SpriteBatch> mSpriteBatch;
	};
}

