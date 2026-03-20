//------------------------//------------------------
// Contents(処理内容) シングルトン基底テンプレートを宣言する。
//------------------------//------------------------
// user(作成者) Keishi Teramoto
// Created date(作成日) 2026 / 03 / 16
// last updated (最終更新日) 2026 / 03 / 17
//------------------------//------------------------
#pragma once
#ifndef  SINGLETON_DEFINED
#define  SINGLETON_DEFINED

namespace Utility
{
	//シングルトン化するため
	template<class T> class SingletonBase
	{
	protected:
		// 派生シングルトンだけが生成できるようにする。
		SingletonBase() {}
		// 基底経由で安全に破棄できるようにする。
		virtual ~SingletonBase() {}

	public:

		// インスタンス呼び出し
		static inline T& GetInstance()
		{
			static T ins;
			return ins;
		}

	private:
		// 代入を禁止する。
		void operator=(const SingletonBase& obj) {}
		// コピー構築を禁止する。
		SingletonBase(const SingletonBase &obj) {}
	};
}
#endif  //SINGLETON_DEFINED

