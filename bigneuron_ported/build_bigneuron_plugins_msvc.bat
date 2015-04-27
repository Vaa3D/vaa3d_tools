:: batch build all BigNeuron plugin projects using a VC Studio complier's nmake function
:: (window version) by Zhi Zhou
:: 2015-04-27
:: revised from the original plugin bat file

set PATH=%PATH%;

for /D %%i in ( * ) do (
  cd %%i
  	if exist *.pro (
  		qmake
  		nmake -f Makefile.Release
  	)else (
		for /D %%j in ( * ) do (
  			cd %%j
  			if exist *.pro (
  				qmake
  				nmake -f Makefile.Release
			)else	(
				for /D %%p in ( * ) do (
  					cd %%p
  					if exist *.pro (
  						qmake
  						nmake -f Makefile.Release
					)
					cd ..
				)
			)
			cd .. 
 	       )
  	)
 	 
  cd ..
)
goto :eof
