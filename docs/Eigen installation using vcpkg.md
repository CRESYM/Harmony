If you haven't already installed vcpkg, follow these instructions:

- git clone https://github.com/microsoft/vcpkg.git
- cd vcpkg
3- ./bootstrap-vcpkg.bat
4- vcpkg integrate install
Install eigen with vcpkg
1- vcpkg install eigen3
2- vcpkg install eigen3:x64-windows
3- Open your project in Visual Studio.
4- Right-click your project in Solution Explorer, and select Properties.
5- Under C/C++ -> General, add the path to the Eigen headers (vcpkg/installed/x64-windows/include/Eigen) in the Additional Include Directories section.

#include <Eigen/Dense>


Sample to test: 
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
