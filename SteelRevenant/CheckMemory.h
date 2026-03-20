//------------------------//------------------------
// Contents(処理内容) デバッグ時のメモリリーク検出マクロを定義する。
//------------------------//------------------------
// user(作成者) Keishi Teramoto
// Created date(作成日) 2026 / 03 / 16
// last updated (最終更新日) 2026 / 03 / 17
//------------------------//------------------------
#pragma once

//検出
#ifdef _DEBUG 
#ifndef DBG_NEW

#define DBG_NEW new ( _NORMAL_BLOCK , __FILE__ , __LINE__ )
#define new DBG_NEW

#endif	// _DEBUG 
#endif	// DBG_NEW

