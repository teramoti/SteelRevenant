#include <filesystem>
#include <fstream>
#include <cstdlib>
#include <iostream>
int wmain(){
  wchar_t* localAppData=nullptr; size_t len=0;
  if(_wdupenv_s(&localAppData,&len,L"LOCALAPPDATA")!=0||!localAppData) return 2;
  std::filesystem::path p=std::filesystem::path(localAppData)/L"SteelRevenant"/L"app_state_probe.txt";
  free(localAppData);
  std::error_code ec; std::filesystem::create_directories(p.parent_path(),ec);
  std::wofstream s(p, std::ios::trunc); if(!s.is_open()){ std::wcout<<L"open fail\n"; return 3; }
  s<<L"ok=1\n"; s.close(); std::wcout<<p.wstring()<<L"\n"; return 0;
}
