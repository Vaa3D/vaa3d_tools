:: batch build all released plugin projects
:: (window version) by Hanchuan Peng
:: 2010-04-30
:: Best add release argument to make release\v3d.exe can recongnize plugin
:: revised from the original bat file

set PATH=%PATH%;c:/mingw/bin;c:/Qt/4.4.3/bin

cd v3d_plugins
call :makepro %*
cd ..
goto :eof

:makepro
for /D %%i in ( * ) do (
  cd %%i
  if exist *.pro (
  	qmake
  	make release %1 %2 %3 %4 %5 %6 %7 %8 %9
  ) 
  cd ..
)
goto :eof
