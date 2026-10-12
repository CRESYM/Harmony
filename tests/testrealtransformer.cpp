#include <gtest/gtest.h>

#include "Elements/Transformer/Transformer_real.h"
#include "Elements/Transformer/Transformer_Y_Y_real.h"
#include "Elements/Transformer/Transformer_Y_Delta_real.h"
#include "Elements/Transformer/Transformer_Delta_Y_real.h"
#include "Elements/Transformer/Transformer_Delta_Delta_real.h"
#include "Elements/Transformer/Transformer_classic.h"
#include "Elements/Transformer/Transformer_Y_Y.h"
#include "Elements/Transformer/Transformer_Y_Delta.h"
#include "Elements/Transformer/Transformer_Delta_Y.h"
#include "Elements/Transformer/Transformer_Delta_Delta.h"
#include "json/component_builder.h"
#include "core/Constants.h"

class TestTransformerReal : public testing::Test {};

namespace {

MatrixXcd toEigenY(const std::vector<std::vector<std::complex<double>>>& y) {
	const int n = static_cast<int>(y.size());
	MatrixXcd M(n, n);
	for (int i = 0; i < n; ++i) {
		for (int j = 0; j < n; ++j) {
			M(i, j) = y[static_cast<size_t>(i)][static_cast<size_t>(j)];
		}
	}
	return M;
}

struct RealParams {
	double Rp = 4.3218;
	double Lp = 0.0;
	double Rs = 0.7938;
	double Ls = 0.084225;
	double Rm = 0.0;
	double Lm = 0.0;
	double a = 2.0;
	double phi = 0.0;
	bool hasMagnetizing = false;

	std::vector<double> values() const {
		if (hasMagnetizing) {
			return { Rp, Lp, Rs, Ls, Rm, Lm, a, phi };
		}
		return { Rp, Lp, Rs, Ls, a, phi };
	}
};

void scalarY(const RealParams& p, double freq,
	std::complex<double>& Y11, std::complex<double>& Y12, std::complex<double>& Y22) {
	const std::complex<double> w = 2.0 * M_PI * freq * std::complex<double>(0.0, 1.0);
	const std::complex<double> Zp = p.Rp + p.Lp * w;
	const std::complex<double> Zs = p.Rs + p.Ls * w;
	std::complex<double> Ym = 0.0;
	if (p.hasMagnetizing) {
		Ym = 1.0 / p.Rm + 1.0 / (w * p.Lm);
	}
	const std::complex<double> aval = p.a * std::exp(std::complex<double>(0.0, p.phi));
	const std::complex<double> a2Zs = aval * aval * Zs;
	Y11 = 1.0 / (Zp + 1.0 / (Ym + 1.0 / a2Zs));
	Y12 = -aval / (Zp + a2Zs * (Ym * Zp + 1.0));
	Y22 = 1.0 / (Zs + 1.0 / (aval * aval * (Ym + 1.0 / Zp)));
}

MatrixXcd expectedYY(const RealParams& p, double freq, int pins) {
	std::complex<double> Y11, Y12, Y22;
	scalarY(p, freq, Y11, Y12, Y22);
	MatrixXcd Y = MatrixXcd::Zero(2 * pins, 2 * pins);
	for (int i = 0; i < pins; ++i) {
		Y(i, i) = Y11;
		Y(i, pins + i) = Y12;
		Y(pins + i, i) = Y12;
		Y(pins + i, pins + i) = Y22;
	}
	return Y;
}

void paperTvTi(Matrix3cd& Tv, Matrix3cd& Ti) {
	Tv << 1.0, -1.0, 0.0,
		0.0, 1.0, -1.0,
		-1.0, 0.0, 1.0;
	Tv /= std::sqrt(3.0);
	Ti << 1.0, 0.0, -1.0,
		-1.0, 1.0, 0.0,
		0.0, -1.0, 1.0;
	Ti /= std::sqrt(3.0);
}

MatrixXcd connectionTransform(const MatrixXcd& Yyy, bool deltaPrimary, bool deltaSecondary) {
	Matrix3cd Tv, Ti;
	paperTvTi(Tv, Ti);
	const Matrix3cd I = Matrix3cd::Identity();
	MatrixXcd N1 = MatrixXcd::Zero(6, 6);
	MatrixXcd N2 = MatrixXcd::Zero(6, 6);
	N1.block<3, 3>(0, 0) = deltaPrimary ? Ti : I;
	N1.block<3, 3>(3, 3) = deltaSecondary ? Ti : I;
	N2.block<3, 3>(0, 0) = deltaPrimary ? Tv : I;
	N2.block<3, 3>(3, 3) = deltaSecondary ? Tv : I;
	return N1 * Yyy * N2;
}

JSON realTransformerJson(const std::string& type, const RealParams& p, bool magnetizingInJson = false) {
	JSON values = {
		{"R_primary", p.Rp},
		{"L_primary", p.Lp},
		{"R_secondary", p.Rs},
		{"L_secondary", p.Ls},
		{"turns_ratio", p.a},
		{"phase_shift", p.phi}
	};
	if (magnetizingInJson) {
		values["R_magnetizing"] = p.Rm;
		values["L_magnetizing"] = p.Lm;
	}
	return {
		{"id", "T1"},
		{"location", "AC1"},
		{"type", type},
		{"pins", 3},
		{"values", values}
	};
}

}  // namespace

TEST_F(TestTransformerReal, TestConstructor) {
	RealParams p;
	p.Lp = 0.45856;
	p.Rm = 1.0804e+06;
	p.Lm = 2e-3;
	p.hasMagnetizing = true;
	Transformer_real transformer("T1", "AC1", 1, p.values());

	EXPECT_EQ(transformer.getTurnsRatio(), 2.0);
	EXPECT_EQ(transformer.getPhaseShift(), 0.0);
	EXPECT_EQ(transformer.getPhaseLag(), 0.0);
	EXPECT_EQ(transformer.getResistance(0), 4.3218);
	EXPECT_EQ(transformer.getInductance(0), 0.45856);
	EXPECT_EQ(transformer.getResistance(1), 0.7938);
	EXPECT_EQ(transformer.getInductance(1), 0.084225);
	EXPECT_EQ(transformer.getResistance(2), 1.0804e+06);
	EXPECT_EQ(transformer.getInductance(2), 2e-3);
}

TEST_F(TestTransformerReal, TestTransformerYYConstructor) {
	TransformerYY_real transformerYY("T3", "AC1", 3, RealParams{}.values());

	EXPECT_EQ(transformerYY.getTurnsRatio(), 2.0);
	EXPECT_EQ(transformerYY.getPhaseShift(), 0.0);
	EXPECT_EQ(transformerYY.getResistance(0), 4.3218);
	EXPECT_EQ(transformerYY.getInductance(0), 0.0);
	EXPECT_EQ(transformerYY.getResistance(1), 0.7938);
	EXPECT_EQ(transformerYY.getInductance(1), 0.084225);
}

TEST_F(TestTransformerReal, YMatrixMatchesPaperEq2) {
	RealParams p;
	p.phi = M_PI / 6.0;
	TransformerYY_real transformer("Tyy", "AC1", 3, p.values());
	const double freq = 1500.0;
	const MatrixXcd y = toEigenY(transformer.compute_y_parameters(freq));
	EXPECT_TRUE(y.isApprox(expectedYY(p, freq, 3), 1e-9));
}

TEST_F(TestTransformerReal, MagnetizingBranchMatchesPaperYm) {
	RealParams p;
	p.Rm = 1.0e4;
	p.Lm = 0.1;
	p.hasMagnetizing = true;
	Transformer_real transformer("Tm", "AC1", 1, p.values());
	const double freq = 50.0;
	const MatrixXcd y = toEigenY(transformer.compute_y_parameters(freq));
	EXPECT_TRUE(y.isApprox(expectedYY(p, freq, 1), 1e-9));

	RealParams openMagnetizing = p;
	openMagnetizing.hasMagnetizing = false;
	Transformer_real noYm("T0", "AC1", 1, openMagnetizing.values());
	const MatrixXcd yOpen = toEigenY(noYm.compute_y_parameters(freq));
	EXPECT_FALSE(y.isApprox(yOpen, 1e-6));
}

TEST_F(TestTransformerReal, PhaseShiftUsesPaperSign) {
	RealParams p;
	p.phi = M_PI / 3.0;
	Transformer_real transformer("Tphi", "AC1", 1, p.values());
	const double freq = 50.0;
	const MatrixXcd y = toEigenY(transformer.compute_y_parameters(freq));
	EXPECT_TRUE(y.isApprox(expectedYY(p, freq, 1), 1e-9));

	RealParams flipped = p;
	flipped.phi = -p.phi;
	EXPECT_FALSE(y.isApprox(expectedYY(flipped, freq, 1), 1e-6));
}

TEST_F(TestTransformerReal, YDeltaMatchesPaperEq9) {
	const RealParams p;
	const double freq = 1500.0;
	TransformerYY_real yy("Tyy", "AC1", 3, p.values());
	TransformerYDelta_real yd("Tyd", "AC1", 3, p.values());
	const MatrixXcd yyy = toEigenY(yy.compute_y_parameters(freq));
	const MatrixXcd yyd = toEigenY(yd.compute_y_parameters(freq));
	EXPECT_TRUE(yyd.isApprox(connectionTransform(yyy, false, true), 1e-9));
}

TEST_F(TestTransformerReal, DeltaYMatchesPaperEq11) {
	const RealParams p;
	const double freq = 1500.0;
	TransformerYY_real yy("Tyy", "AC1", 3, p.values());
	TransformerDeltaY_real dy("Tdy", "AC1", 3, p.values());
	const MatrixXcd yyy = toEigenY(yy.compute_y_parameters(freq));
	const MatrixXcd ydy = toEigenY(dy.compute_y_parameters(freq));
	EXPECT_TRUE(ydy.isApprox(connectionTransform(yyy, true, false), 1e-9));
}

TEST_F(TestTransformerReal, DeltaDeltaMatchesPaperEq12) {
	const RealParams p;
	const double freq = 1500.0;
	TransformerYY_real yy("Tyy", "AC1", 3, p.values());
	TransformerDeltaDelta_real dd("Tdd", "AC1", 3, p.values());
	const MatrixXcd yyy = toEigenY(yy.compute_y_parameters(freq));
	const MatrixXcd ydd = toEigenY(dd.compute_y_parameters(freq));
	EXPECT_TRUE(ydd.isApprox(connectionTransform(yyy, true, true), 1e-9));
}

TEST_F(TestTransformerReal, ClassicConnectionsUseSameTvTi) {
	const std::vector<double> values = { 1.0, 1.0e-3, 2.0, 2.0e-3, 0.5e-3 };
	const double freq = 1500.0;
	TransformerYY yy("Tyy", "AC1", 3, values);
	TransformerYDelta yd("Tyd", "AC1", 3, values);
	TransformerDeltaY dy("Tdy", "AC1", 3, values);
	TransformerDeltaDelta dd("Tdd", "AC1", 3, values);
	const MatrixXcd yyy = toEigenY(yy.compute_y_parameters(freq));
	EXPECT_TRUE(toEigenY(yd.compute_y_parameters(freq)).isApprox(connectionTransform(yyy, false, true), 1e-9));
	EXPECT_TRUE(toEigenY(dy.compute_y_parameters(freq)).isApprox(connectionTransform(yyy, true, false), 1e-9));
	EXPECT_TRUE(toEigenY(dd.compute_y_parameters(freq)).isApprox(connectionTransform(yyy, true, true), 1e-9));
}

TEST_F(TestTransformerReal, PowerFlowWritesPhaseShift) {
	RealParams p;
	p.phi = 0.25;
	Transformer_real transformer("T1", "AC1", 1, p.values());
	std::map<std::string, double> branch;
	std::map<std::string, double> global{ {"omega", 2.0 * M_PI * 50.0}, {"ACZbase", 1.0} };
	transformer.computePowerFlow(branch, global);
	EXPECT_NEAR(branch.at("shift"), p.phi, 1e-12);
}

TEST_F(TestTransformerReal, ConnectionRequiresThreePins) {
	EXPECT_THROW(
		TransformerYDelta_real("Tyd", "AC1", 1, RealParams{}.values()),
		std::invalid_argument);
}

TEST_F(TestTransformerReal, JsonBuildsRealConnectionsAndMagnetizing) {
	RealParams p;
	auto yy = ComponentBuilder::buildFromJSON(realTransformerJson("transformer_yy_real", p), 0);
	auto yd = ComponentBuilder::buildFromJSON(realTransformerJson("transformer_ydelta_real", p), 0);
	auto dy = ComponentBuilder::buildFromJSON(realTransformerJson("transformer_deltay_real", p), 0);
	auto dd = ComponentBuilder::buildFromJSON(realTransformerJson("transformer_deltadelta_real", p), 0);
	EXPECT_NE(dynamic_cast<TransformerYY_real*>(yy.get()), nullptr);
	EXPECT_NE(dynamic_cast<TransformerYDelta_real*>(yd.get()), nullptr);
	EXPECT_NE(dynamic_cast<TransformerDeltaY_real*>(dy.get()), nullptr);
	EXPECT_NE(dynamic_cast<TransformerDeltaDelta_real*>(dd.get()), nullptr);

	RealParams mag = p;
	mag.Rm = 1.0804e6;
	mag.Lm = 2e-3;
	mag.hasMagnetizing = true;
	auto withYm = ComponentBuilder::buildFromJSON(
		realTransformerJson("transformer_real", mag, true), 0);
	auto* real = dynamic_cast<Transformer_real*>(withYm.get());
	ASSERT_NE(real, nullptr);
	EXPECT_EQ(real->getResistance(2), mag.Rm);
	EXPECT_EQ(real->getInductance(2), mag.Lm);

	JSON incomplete = realTransformerJson("transformer_real", p);
	incomplete["values"]["R_magnetizing"] = mag.Rm;
	EXPECT_THROW(ComponentBuilder::buildFromJSON(incomplete, 0), std::invalid_argument);
}
