### Installation of Ipopt in case that you did not use miniconda for package installation

If you haven't installed vcpkg yet:
- git clone https://github.com/microsoft/vcpkg.git
- cd vcpkg
- ./bootstrap-vcpkg.bat
- vcpkg integrate install

Install Ipopt
- vcpkg install ipopt
- vcpkg install ipopt:x64-windows
- add vcpkg/packages/coin-or-ipopt_x64-windows/bin to the path in the environment variable 
- Go to your project in Visual Studio 
- Under C/C++ -> General, add the path to the Ipopt headers (vcpkg/installed/x64-windows/include/coin-or) in the Additional Include Directories section.
- Under Linker -> General, add the path to the Ipopt library (vcpkg/installed/x64-windows/lib) in the Additional Library Directories section.
- Under Linker -> Input, add the Ipopt libraries you need (e.g., ipopt.lib) in the Additional Dependencies section.

