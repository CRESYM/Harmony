### Installation of Eigen in case that you did not use miniconda for installation

If you haven't already installed vcpkg, follow these instructions:

- git clone https://github.com/microsoft/vcpkg.git
- cd vcpkg
- ./bootstrap-vcpkg.bat
- vcpkg integrate install

Install eigen with vcpkg:
- vcpkg install eigen3
- vcpkg install eigen3:x64-windows
- Open your project in Visual Studio.
- Right-click your project in Solution Explorer, and select Properties.
- Under C/C++ -> General, add the path to the Eigen headers (vcpkg/installed/x64-windows/include/Eigen) in the Additional Include Directories section.

To test the successful installation, use the following code.
#include <Eigen/Dense>


Sample to test: 
```
#include <iostream>
#include <Eigen/dense>
#include <coin-or/IpIpoptApplication.hpp>
#include <string>


int main()
{
    // Test Eigen
    Eigen::MatrixXd mat(2, 2);
    mat(0, 0) = 3;
    mat(1, 0) = 2.5;
    mat(0, 1) = -1;
    mat(1, 1) = mat(1, 0) + mat(0, 1);
    std::cout << "Eigen matrix:\n" << mat << std::endl;

    // Test Ipopt
    Ipopt::SmartPtr<Ipopt::IpoptApplication> app = new Ipopt::IpoptApplication();

    app->Initialize();

    std::cout << "Ipopt successfully linked and initialized." << std::endl;
 
    std::cout << "Hello World!\n";
    return 0;
}
```
