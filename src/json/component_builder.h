#pragma once
#include <iostream>
#include <memory>

#include <nlohmann/json.hpp>
using JSON = nlohmann::json;

#include "../network.h"
#include "../Include_components.h"

/**
 * @file component_builder.h
 * @brief Factory for constructing @ref Element objects from JSON component entries.
 * @ingroup input
 */

class ComponentBuilder {

public:

	static std::unique_ptr<Element> buildFromJSON(const JSON& comp, unsigned int index);

	static void findNumber(const std::string& key, const JSON& j);
	static void findNonEmptyString(const std::string& key, const JSON& j);
	static void findNonEmptyNumericArray(const std::string& key, const JSON& j, int arraysize = -1);

private:
	static Capacitor* buildCapacitor(const JSON& comp);
	static Inductor* buildInductor(const JSON& comp);
	static Resistor* buildResistor(const JSON& comp);
	static Impedance* buildImpedance(const JSON& comp);
	static Admittance* buildAdmittance(const JSON& comp);
	static Load* buildLoad(const JSON& comp);
	static LoadPQ* buildLoadPQ(const JSON& comp);
	static AC_source* buildACSource(const JSON& comp);
	static Generator* buildGenerator(const JSON& comp);
	static DC_source* buildDCSource(const JSON& comp);
	static Switch* buildSwitch(const JSON& comp);
	static TransmissionLine* buildTransmissionLine(const JSON& comp);
	static Cable* buildCable(const JSON& comp);
	static Overhead_Line* buildOverheadLine(const JSON& comp);
	static MMC* buildMMC(const JSON& comp);
	static WTtype3* buildWTtype3(const JSON& comp);
	static WTtype4* buildWTtype4(const JSON& comp);
	static WPplant* buildWPplant(const JSON& comp);
	static PVplant* buildPVplant(const JSON& comp);
	static Transformer_real* buildTransformerReal(const JSON& comp);
	static Transformer_classic* buildTransformerClassic(const JSON& comp);
	static TransformerDeltaDelta* buildTransformerDeltaDelta(const JSON& comp);
	static TransformerDeltaY_real* buildTransformerDeltaYReal(const JSON& comp);
	static TransformerDeltaY* buildTransformerDeltaY(const JSON& comp);
	static TransformerYDelta* buildTransformerYDelta(const JSON& comp);
	static TransformerYY_real* buildTransformerYYReal(const JSON& comp);
	static TransformerYY* buildTransformerYY(const JSON& comp);

	static std::vector<double> readTransformerClassicValues(const JSON& comp);
	static std::vector<double> readTransformerRealValues(const JSON& comp);
	static std::vector<double> readNumericArray(const JSON& comp, const std::string& key);
	static DenseMatrix readDenseMatrixValues(const JSON& comp, const std::string& key);
	static std::vector<double> readVoltageVector(const JSON& comp);
};
