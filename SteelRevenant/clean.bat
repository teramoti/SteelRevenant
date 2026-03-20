::------------------------//------------------------
:: Contents(処理内容) clean.batの内容を書く
::------------------------//------------------------
:: user(作成者) Keishi Teramoto
:: Created date(作成日) 2026 / 03 / 16
:: last updated (最終更新日) 2026 / 03 / 17
::------------------------//------------------------
rmdir /s /q Debug
rmdir /s /q Release
rmdir /s /q ipch
del /s /q *.sdf
del /s /q *.db
