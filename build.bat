@echo off

set common_compiler_flags=-Od -fp:fast -nologo -MD -Oi -Gm- -GR- -EHa- -W4 -WX -wd4201 -wd4100 -wd4189 -wd4505 -wd4456 -wd4459 -wd4311 -wd4312 -wd4302 -wd4706 -wd4127 -wd4701 -wd4703 -FC -Z7

IF NOT EXIST bin mkdir bin
pushd bin

cl %common_compiler_flags% ..\src\videomation_encoder.cpp /I ..\include /link -incremental:no -opt:ref user32.lib gdi32.lib winmm.lib shell32.lib

popd
