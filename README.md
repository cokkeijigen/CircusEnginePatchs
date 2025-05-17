# nightshade
```cmd
git clone -b nightshade https://github.com/cokkeijigen/circus_engine_patchs
cd .\circus_engine_patchs\CircusEnginePatchs\
mkdir output
cd output
cmake -A Win32 -DCMAKE_BUILD_TYPE=Release ..
cmake --build . --target Nightshade_Patch --config Release
```