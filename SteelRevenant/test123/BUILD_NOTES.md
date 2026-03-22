Build notes ? changes applied by automated agent

- Re-encoded all *.cpp and *.h files to UTF-8 (BOM optional) to resolve C4819 and parsing errors.
- Fixed malformed preprocessor guards at top of files where `#define NOMINMAX` was followed immediately by `#endif`.
  Files fixed: `Source/Scene/GameScene/GameScene.cpp`, `Source/Scene/SettingsScene/SettingsScene.cpp`, others verified.
- Restored corrupted member declarations in `Source/Scene/GameScene/GameScene.h` (added `m_font` pointer declaration).
- Updated enemy base wave counts in `Source/Action/BattleRuleBook.cpp` to: Stage1=12, Stage2=14, Stage3=16.
- Verified full solution build completed successfully after fixes.

Notes:
- No gameplay logic changes beyond the enemy count update.
- Some comments contained mojibake due to prior encoding; comments were not semantically modified except where necessary to restore code.

If you want me to revert any of these changes, indicate whether you have an authoritative git commit to restore to; otherwise I can revert the specific files I modified.
