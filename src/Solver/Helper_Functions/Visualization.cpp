#include "Visualization.h"


void bode_plot(const std::vector<double>& freq,
	const std::vector<std::vector<double>>& mag_dB,
	const std::vector<std::vector<double>>& phase_deg,
    const std::vector<std::string>& labels,
	const std::string& title) {

    // Before plotting
    std::ofstream null_stream;
    null_stream.open("nul"); // On Windows, use "nul"; on Linux, use "/dev/null"
	std::streambuf* old_cerr = std::cerr.rdbuf(null_stream.rdbuf());
		
    auto f = matplot::figure(true); // Use matplot namespace
    f->size(1200, 800);

    matplot::tiledlayout(2, 1); // Use matplot namespace

    matplot::nexttile(); // Use matplot namespace
    matplot::hold(matplot::on); // Use matplot namespace
    for (size_t j = 0; j < labels.size(); ++j) {
        std::vector<double> mag;
        for (size_t i = 0; i < freq.size(); ++i) {
            mag.push_back(mag_dB[i][j]);
        }
        matplot::semilogx(freq, mag)->display_name(labels[j]); // Use matplot namespace
    }
    matplot::ylabel("20 log_{10}|H(jw)|"); // Use matplot namespace
    matplot::xlabel("Frequency (Hz)"); // Use matplot namespace
    matplot::legend(); // Use matplot namespace
    matplot::grid(matplot::on); // Use matplot namespace

    matplot::nexttile(); // Use matplot namespace
    matplot::hold(matplot::on); // Use matplot namespace
    for (size_t j = 0; j < labels.size(); ++j) {
        std::vector<double> phase;
        for (size_t i = 0; i < freq.size(); ++i) {
            phase.push_back(phase_deg[i][j]);
        }
        matplot::semilogx(freq, phase)->display_name(labels[j]); // Use matplot namespace
    }
    matplot::xlabel("Frequency (Hz)"); // Use matplot namespace
    matplot::ylabel("Angle H(jw) "); // Use matplot namespace
    matplot::legend(); // Use matplot namespace
    matplot::grid(matplot::on); // Use matplot namespace

    matplot::show(); // Use matplot namespace

    // After plotting
    std::cerr.rdbuf(old_cerr);
    null_stream.close();
}

void nyquist_plot(const std::vector<double>& real_part,
    const std::vector<double>& imag_part,
    const std::string& title) {
    using namespace matplot;
    auto f = figure(true);
    f->size(800, 600);

    plot(real_part, imag_part);
    xlabel("Re{H(jw)}");
    ylabel("Im{H(jw)}");
    //title(title);
    grid(on);

    show();
}