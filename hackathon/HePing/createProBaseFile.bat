@echo off
set /p solutionName=Please input your solutionName:
python createProBaseFile.py %solutionName%
pause

