//------------------------//------------------------
// Contents(処理内容) ビットフラグ補助クラスを宣言する。
//------------------------//------------------------
// user(作成者) Keishi Teramoto
// Created date(作成日) 2026 / 03 / 16
// last updated (最終更新日) 2026 / 03 / 17
//------------------------//------------------------
#pragma once
#ifndef  FLAG_DEFINED
#define  FLAG_DEFINED

//----------------------------------------------------------------------
//!
// フラグクラス。
namespace Utility
{
	class Flag
	{
	public:
		// コンストラクタ
		Flag()
			: m_Flag(0)
		{

		}

		// フラグを立てる処理
		void On(unsigned int flag)
		{
			m_Flag |= flag;
		}

		// フラグを伏せる処理
		void Off(unsigned int flag)
		{
			m_Flag &= ~flag;
		}
		
		// フラグが立ってるか確認処理
		bool Is(unsigned int flag)
		{
			return (m_Flag & flag) != 0;
		}

	private:
		// フラグ変数
		unsigned int m_Flag;
	};
}
#endif  //FLAG_DEFINED

