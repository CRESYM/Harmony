#include "Writer.h"

void write_file(const std::vector<double>& time, const std::vector<Eigen::MatrixXd>& values, const std::vector<std::string>& headers, const std::string& filename)
{
    if (time.empty())
        throw std::runtime_error("write_file: time vector is empty");

    if (values.empty())
        throw std::runtime_error("write_file: values are empty");

    const int N = static_cast<int>(time.size());

    // check all matrices
    for (size_t i = 0; i < values.size(); ++i)
    {
        if (values[i].rows() != N || values[i].cols() != 3)
        {
            throw std::runtime_error(
                "write_file: values[" + std::to_string(i) + "] must be Nx3");
        }
    }

    std::ofstream f(filename);

    if (!f)
    {
        std::cerr << "ERROR: cannot open " << filename << "\n";
        return;
    }

    f << std::setprecision(17);

    // -----------------------
    // HEADER
    // -----------------------
    f << "t";

    for (size_t i = 0; i < values.size(); ++i)
    {
        f << ","
            << headers[i] << "_a"
            << "," << headers[i] << "_b"
            << "," << headers[i] << "_c";
    }

    f << "\n";

    // -------------------------
    // Data
    // -------------------------
    for (int k = 0; k < N; ++k)
    {
        f << time[k];

        for (size_t i = 0; i < values.size(); ++i)
        {
            f << ","
                << values[i](k, 0) << ","
                << values[i](k, 1) << ","
                << values[i](k, 2);
        }

        f << "\n";
    }

    f.close();
}