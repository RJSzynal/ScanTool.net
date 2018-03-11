# ScanTool.net OBD-II Software
ScanTool.net OBD-II Software for ElmScan is free software that allows you to use your computer and an inexpensive hardware interface to read information from your car's computer. Current version allows you to read trouble codes and see their descriptions, clear the codes and turn off the "Check Engine" light, and display real-time sensor data such as RPM, Engine Load, Vehicle Speed, Coolant Temperature, and Timing Advance.
This project is a fork of the original open source project.

## Compilation guide
The tools required can be found in the build_tools directory
1. Install compiler MinGW - mingw-get-setup.exe
2. Select and install min32-base
4. UnZip Allegro graphics C library into the C:\MinGW Directory - allegro-mingw-4.2.3.zip
5. UnZip Allegro/DirectX 7 library into the C:\MinGW Directory - dx70_mgw.zip
6. Update compile_scantool.bat with your source code location
7. Run compile_scantool.bat
8. The compiled application with it's library dependencies are now in the 'compiled' directory