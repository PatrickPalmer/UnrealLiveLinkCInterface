@echo off

RMDIR %~dp0Staging /S /Q
..\..\..\Build\BatchFiles\RunUAT.bat  BuildGraph -Script=Engine/Source/Programs/UnrealLiveLinkCInterface/BuildUnrealLiveLinkCInterface.xml -Target="Live Link CInterface Module"
