#pragma once
#ifndef NOMINMAX
#define NOMINMAX
#endif
//------------------------
// Contents(蜃ｦ逅・・螳ｹ) 繝薙ャ繝医ヵ繝ｩ繧ｰ陬懷勧繧ｯ繝ｩ繧ｹ繧貞ｮ｣險縺吶ｋ縲・//------------------------
// user(菴懈・閠・ Keishi Teramoto
// Created date(菴懈・譌･) 2026 / 03 / 16
// last updated (譛邨よ峩譁ｰ譌･) 2026 / 03 / 17
//------------------------
#ifndef  FLAG_DEFINED
#define  FLAG_DEFINED

//----------------------------------------------------------------------
//!
// 繝輔Λ繧ｰ繧ｯ繝ｩ繧ｹ縲・namespace Utility
{
	class Flag
	{
	public:
		// 繧ｳ繝ｳ繧ｹ繝医Λ繧ｯ繧ｿ
		Flag()
			: m_Flag(0)
		{

		}

		// 繝輔Λ繧ｰ繧堤ｫ九※繧句・逅・		void On(unsigned int flag)
		{
			m_Flag |= flag;
		}

		// 繝輔Λ繧ｰ繧剃ｼ上○繧句・逅・		void Off(unsigned int flag)
		{
			m_Flag &= ~flag;
		}
		
		// 繝輔Λ繧ｰ縺檎ｫ九▲縺ｦ繧九°遒ｺ隱榊・逅・		bool Is(unsigned int flag)
		{
			return (m_Flag & flag) != 0;
		}

	private:
		// 繝輔Λ繧ｰ螟画焚
		unsigned int m_Flag;
	};
}
#endif  //FLAG_DEFINED


