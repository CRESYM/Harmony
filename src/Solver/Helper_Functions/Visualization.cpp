#include "Visualization.h"


void bode_plot(const std::vector<double>& freq,
	const std::vector<std::vector<double>>& mag_dB,
	const std::vector<std::vector<double>>& phase_deg,
    const std::vector<std::string>& labels,
	const std::string& title) {


    // Before plotting
    std::ofstream null_stream;
    #ifdef _WIN32
        null_stream.open("nul");
    #else
        null_stream.open("/dev/null");
    #endif
        std::streambuf* old_cerr = std::cerr.rdbuf(null_stream.rdbuf());
		
    auto f = matplot::figure(true); // Use matplot namespace
    f->size(1600, 1000);

    matplot::tiledlayout(2, 1); // Use matplot namespace

    matplot::nexttile(); // Use matplot namespace
    matplot::hold(matplot::on); // Use matplot namespace

    for (size_t j = 0; j < labels.size(); ++j) {
        std::vector<double> mag;
        for (size_t i = 0; i < freq.size(); ++i) {
            mag.push_back(mag_dB[i][j]);
        }
        auto plt = matplot::semilogx(freq, mag);
        plt->display_name(labels[j]); // Use matplot namespace
		plt->line_width(2.0); // Set line width for better visibility
    }
    matplot::ylabel(u8"20 log_{10}|H(jω)|"); // Use matplot namespace
    matplot::xlabel("Frequency (Hz)"); // Use matplot namespace
    auto lgd = ::matplot::legend();
    lgd->location(legend::general_alignment::bottomleft);
    lgd->box(false);
    matplot::grid(matplot::on); // Use matplot namespace
    gca()->minor_grid(true);

    matplot::nexttile(); // Use matplot namespace
    matplot::hold(matplot::on); // Use matplot namespace
    for (size_t j = 0; j < labels.size(); ++j) {
        std::vector<double> phase;
        for (size_t i = 0; i < freq.size(); ++i) {
            phase.push_back(phase_deg[i][j]);
        }
        auto plt = matplot::semilogx(freq, phase);
        plt->display_name(labels[j]);; // Use matplot namespace
		plt->line_width(2.0); // Set line width for better visibility
    }
    matplot::xlabel("Frequency (Hz)"); // Use matplot namespace
    matplot::ylabel(u8"Angle H(jω)"); // Use matplot namespace
    auto lgd2 = ::matplot::legend();
    lgd2->location(legend::general_alignment::bottomleft);
    lgd2->box(false);
    matplot::grid(matplot::on); // Use matplot namespace
    gca()->minor_grid(true);

    matplot::show(); // Use matplot namespace

    // After plotting
    std::cerr.rdbuf(old_cerr);
    null_stream.close();

    //std::this_thread::sleep_for(std::chrono::seconds(60)); // Simulate condition
}

void nyquist_plot(const std::vector<std::vector<std::complex<double>>>& H_data,
    const std::vector<std::string>& labels,
    const std::string& title) {
	using namespace matplot;

    // Suppress warnings to console (same as in your bode_plot)
    std::ofstream null_stream;
	#ifdef _WIN32
		null_stream.open("nul");
	#else
		null_stream.open("/dev/null");
	#endif
		std::streambuf* old_cerr = std::cerr.rdbuf(null_stream.rdbuf());

    // Create figure
    auto f = figure(true);
    f->size(1000, 1000);
    f->name(title);
    hold(on);

    // Plot Nyquist curves
    for (size_t j = 0; j < H_data[0].size(); ++j) {
        std::vector<double> real_part, imag_part;
        for (size_t i = 0; i < H_data.size(); ++i) {
            real_part.push_back(std::real(H_data[i][j]));
            imag_part.push_back(std::imag(H_data[i][j]));
        }
        auto plt = plot(real_part, imag_part);
        plt->display_name(labels[j]);
        plt->line_width(2.0);
    }

    // Plot unit circle
    std::vector<double> theta = linspace(0, 2 * M_PI, 500);
    std::vector<double> xc(theta.size()), yc(theta.size());
    for (size_t i = 0; i < theta.size(); ++i) {
        xc[i] = cos(theta[i]);
        yc[i] = sin(theta[i]);
    }
    auto unit_circle = plot(xc, yc);
    unit_circle->line_width(1.5);
    unit_circle->color({0.3, 0.3, 0.3});
    unit_circle->display_name("Unit Circle");

    // Styling
    xlabel(u8"Re{H(jω)}");
    ylabel(u8"Im{H(jω)}");
    matplot::title(title);
    axis(matplot::equal);
    grid(on);
    gca()->minor_grid(true);
    auto lgd = matplot::legend();
    lgd->location(legend::general_alignment::bottomleft);
    lgd->box(false);

    show();

    // Restore cerr
    std::cerr.rdbuf(old_cerr);
    null_stream.close();
}

void plot_eigenvalues(const std::vector<std::complex<double>>& eigvals,
    const std::string& title) {
    using namespace matplot;

    // Silence console output
    std::ofstream null_stream;
    #ifdef _WIN32
        null_stream.open("nul");
    #else
        null_stream.open("/dev/null");
    #endif
        std::streambuf* old_cerr = std::cerr.rdbuf(null_stream.rdbuf());

    auto f = figure(true);
    f->size(1000, 1000);
    f->name(title);
    hold(on);

    // Real and imaginary parts
    std::vector<double> real_part, imag_part;
    for (const auto& λ : eigvals) {
        real_part.push_back(std::real(λ));
        imag_part.push_back(std::imag(λ));
    }

    // Scatter eigenvalues
    auto sc = scatter(real_part, imag_part);
    // Style
    sc->marker_style(matplot::line_spec::marker_style::cross); // ✚ cross marker
    sc->marker_size(10.0);                                     // smaller size
    sc->line_width(1.0);                                      // marker edge thickness
    // Colors
    sc->marker_face_color({ 0.1, 0.4, 0.8 });   // inner color (for filled markers)
    sc->color({ 0, 0, 0 });                     // edge color (black)
    
    // Styling
    xlabel(u8"Re(λ)");
    ylabel(u8"Im(λ)");
    matplot::title(title);
    axis(matplot::equal);
    grid(on);
    gca()->minor_grid(true);
    show();

    std::cerr.rdbuf(old_cerr);
    null_stream.close();
}

void plot_eigenvalue_sequence(const std::vector<std::vector<std::complex<double>>>& eigval_sequence,
    const std::string& title) {
    using namespace matplot;

    // Silence console output
    std::ofstream null_stream;
    #ifdef _WIN32
        null_stream.open("nul");
    #else
        null_stream.open("/dev/null");
    #endif
        std::streambuf* old_cerr = std::cerr.rdbuf(null_stream.rdbuf());

    auto f = figure(true);
    f->size(1000, 1000);
    f->name(title);
    hold(on);

    // Assume each column corresponds to the same eigenvalue over sequence steps
    size_t num_modes = eigval_sequence[0].size();
    size_t num_steps = eigval_sequence.size();

    for (size_t j = 0; j < num_modes; ++j) {
        std::vector<double> re, im;
        for (size_t i = 0; i < num_steps; ++i) {
            re.push_back(std::real(eigval_sequence[i][j]));
            im.push_back(std::imag(eigval_sequence[i][j]));
        }
        auto plt = plot(re, im);
        plt->line_width(2.0);
        plt->display_name(u8"λ_" + std::to_string(j + 1));
        scatter(re, im, 10)->marker_face_color(plt->color()); // Markers at each step
    }


    // Styling
    xlabel(u8"Re(λ)");
    ylabel(u8"Im(λ)");
    matplot::title(title);
    axis(matplot::equal);
    grid(on);
    gca()->minor_grid(true);
    auto lgd = matplot::legend();
    lgd->location(legend::general_alignment::bottomleft);
    lgd->box(false);
    show();

    std::cerr.rdbuf(old_cerr);
    null_stream.close();
}

void plot_participation_factors_normalized(const std::vector<std::vector<double>>& P,
    const std::vector<std::string>& state_labels,
    const std::vector<std::string>& mode_labels,
    const std::string& title) {
    using namespace matplot;

    // Suppress console output
    std::ofstream null_stream;
    #ifdef _WIN32
        null_stream.open("nul");
    #else
        null_stream.open("/dev/null");
    #endif
        std::streambuf* old_cerr = std::cerr.rdbuf(null_stream.rdbuf());

    auto f = figure(true);
    f->size(1400, 800);
    f->name(title);
    hold(on);

    // Dimensions
    size_t n_states = P.size();
    size_t n_modes = P[0].size();

    // Normalize participation factors so each mode sums to 1
    std::vector<std::vector<double>> P_norm = P;
    for (size_t j = 0; j < n_modes; ++j) {
        double col_sum = 0.0;
        for (size_t i = 0; i < n_states; ++i)
            col_sum += std::abs(P[i][j]);
        if (col_sum > 0) {
            for (size_t i = 0; i < n_states; ++i)
                P_norm[i][j] /= col_sum;
        }
    }

    // X positions (state variables)
    std::vector<double> x_positions(n_states);
    for (size_t i = 0; i < n_states; ++i) x_positions[i] = i + 1;

    // Plot bars for each mode
    for (size_t j = 0; j < n_modes; ++j) {
        std::vector<double> y;
        for (size_t i = 0; i < n_states; ++i)
            y.push_back(P_norm[i][j]);
        auto plt = bar(x_positions, y);
        plt->display_name(mode_labels[j]);
    }

    // Styling
    xticks(x_positions);
    xticklabels(state_labels);
    // Rotate labels vertically
    auto ax = gca();
    ax->xtickangle(90);  // <-- rotate labels 90 degrees

    xlabel("State Variables");
    ylabel("Normalized Participation Factor");
    matplot::title(title + " (Normalized)");
    grid(on);
    gca()->minor_grid(true);
    auto lgd = matplot::legend();
    lgd->font_size(6);
    lgd->location(legend::general_alignment::topright);
    lgd->box(false);
    show();

    // Restore cerr
    std::cerr.rdbuf(old_cerr);
    null_stream.close();



}

void plot_abc_waveforms(const std::vector<double>& t,
    const Eigen::MatrixXd& Xabc,
    const std::string& title) {
    using namespace matplot;

    if (Xabc.cols() != 3 || Xabc.rows() != static_cast<int>(t.size())) {
        throw std::runtime_error("plot_abc_waveforms: size mismatch or Xabc must have 3 columns.");
    }

    // Silence backend warnings, same pattern as your other plotting functions
    std::ofstream null_stream;
#ifdef _WIN32
    null_stream.open("nul");
#else
    null_stream.open("/dev/null");
#endif
    std::streambuf* old_cerr = std::cerr.rdbuf(null_stream.rdbuf());

    auto f = figure(true);
    f->size(1400, 900);
    f->name(title);
    hold(on);

    std::vector<double> xa(t.size()), xb(t.size()), xc(t.size());
    for (size_t i = 0; i < t.size(); ++i) {
        xa[i] = Xabc(static_cast<int>(i), 0);
        xb[i] = Xabc(static_cast<int>(i), 1);
        xc[i] = Xabc(static_cast<int>(i), 2);
    }

    auto p1 = plot(t, xa);
    p1->display_name("xa");
    p1->line_width(2.0);

    auto p2 = plot(t, xb);
    p2->display_name("xb");
    p2->line_width(2.0);

    auto p3 = plot(t, xc);
    p3->display_name("xc");
    p3->line_width(2.0);

    xlabel("Time (s)");
    ylabel("Amplitude");
    matplot::title(title);
    grid(on);
    gca()->minor_grid(true);

    /*auto lgd = legend();*/
    auto lgd = matplot::legend();
    lgd->location(legend::general_alignment::bottomleft);
    lgd->box(false);

    show();
    
    std::cerr.rdbuf(old_cerr);
    null_stream.close();
}

void plot_abc_groups(
    const std::vector<double>& t,
    const std::vector<Eigen::MatrixXd>& Xabc_groups,
    const std::string& title)
{
    using namespace matplot;

    if (t.empty())
        throw std::runtime_error("plot_abc_groups: time vector is empty");

    if (Xabc_groups.empty())
        throw std::runtime_error("plot_abc_groups: no groups provided");

    // check all matrices
    for (size_t g = 0; g < Xabc_groups.size(); ++g)
    {
        if (Xabc_groups[g].cols() != 3)
            throw std::runtime_error("plot_abc_groups: each group must have 3 columns");

        if (Xabc_groups[g].rows() != static_cast<int>(t.size()))
            throw std::runtime_error("plot_abc_groups: row mismatch in group " + std::to_string(g));
    }

    const int groupsPerFigure = 3;
    const int nGroups = static_cast<int>(Xabc_groups.size());
    const int nFigures = (nGroups + groupsPerFigure - 1) / groupsPerFigure;

    // silence backend warnings
    std::ofstream null_stream;
#ifdef _WIN32
    null_stream.open("nul");
#else
    null_stream.open("/dev/null");
#endif
    std::streambuf* old_cerr = std::cerr.rdbuf(null_stream.rdbuf());

    for (int fig = 0; fig < nFigures; ++fig)
    {
        int gStart = fig * groupsPerFigure;
        int gEnd = std::min(gStart + groupsPerFigure, nGroups);
        int rowsThisFigure = gEnd - gStart;

        auto f = figure(true);
        f->size(1400, 900);

        std::string figTitle =
            title + " (Figure " + std::to_string(fig + 1) + ")";

        f->name(figTitle);

        tiledlayout(rowsThisFigure, 1);

        for (int g = gStart; g < gEnd; ++g)
        {
            nexttile();
            hold(on);

            const auto& X = Xabc_groups[g];

            std::vector<double> xa(t.size());
            std::vector<double> xb(t.size());
            std::vector<double> xc(t.size());

            for (size_t i = 0; i < t.size(); ++i)
            {
                xa[i] = X(i, 0);
                xb[i] = X(i, 1);
                xc[i] = X(i, 2);
            }

            auto p1 = plot(t, xa);
            p1->line_width(2);
            p1->display_name("xa" + std::to_string(g + 1));

            auto p2 = plot(t, xb);
            p2->line_width(2);
            p2->display_name("xb" + std::to_string(g + 1));

            auto p3 = plot(t, xc);
            p3->line_width(2);
            p3->display_name("xc" + std::to_string(g + 1));

            ylabel("Group " + std::to_string(g + 1));
            grid(on);
            gca()->minor_grid(true);

            auto lgd = legend();
            lgd->location(legend::general_alignment::bottomleft);
            lgd->box(false);
        }

        xlabel("Time (s)");
        matplot::title(figTitle);

        show();
    }

    std::cerr.rdbuf(old_cerr);
    null_stream.close();
}