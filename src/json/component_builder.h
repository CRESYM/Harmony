#pragma once
#include <iostream>
#include <memory>

#include <nlohmann/json.hpp>
using JSON = nlohmann::json;

#include "../network.h"
#include "../Include_components.h"
#include "json_parameters.h"
#include "json_expression.h"

/**
 * @file component_builder.h
 * @brief Factory for constructing @ref Element objects from JSON component entries.
 * @ingroup input
 */

class ComponentBuilder {

public:

	static std::unique_ptr<Element> buildFromJSON(
		const JSON& comp,
		unsigned int index,
		const JsonParameterTable& params = {});

	static void findNumber(const std::string& key, const JSON& j);
	static void findScalar(const std::string& key, const JSON& j, const JsonParameterTable& params);
	static void findNonEmptyString(const std::string& key, const JSON& j);
	static void findNonEmptyNumericArray(const std::string& key, const JSON& j, int arraysize = -1);
	static void findNumericOrReferenceArray(
		const std::string& key,
		const JSON& j,
		const JsonParameterTable& params,
		int arraysize = -1);

private:
	static Capacitor* buildCapacitor(const JSON& comp, const JsonParameterTable& params);
	static Inductor* buildInductor(const JSON& comp, const JsonParameterTable& params);
	static Resistor* buildResistor(const JSON& comp, const JsonParameterTable& params);
	static Impedance* buildImpedance(const JSON& comp, const JsonParameterTable& params);
	static Admittance* buildAdmittance(const JSON& comp, const JsonParameterTable& params);
	static Load* buildLoad(const JSON& comp, const JsonParameterTable& params);
	static LoadPQ* buildLoadPQ(const JSON& comp, const JsonParameterTable& params);
	static AC_source* buildACSource(const JSON& comp, const JsonParameterTable& params);
	static Generator* buildGenerator(const JSON& comp, const JsonParameterTable& params);
	static DC_source* buildDCSource(const JSON& comp, const JsonParameterTable& params);
	static Switch* buildSwitch(const JSON& comp);
	static TransmissionLine* buildTransmissionLine(const JSON& comp, const JsonParameterTable& params);
	static Cable* buildCable(const JSON& comp, const JsonParameterTable& params);
	static Overhead_Line* buildOverheadLine(const JSON& comp, const JsonParameterTable& params);
	static MMC* buildMMC(const JSON& comp, const JsonParameterTable& params);
	static WTtype3* buildWTtype3(const JSON& comp, const JsonParameterTable& params);
	static WTtype4* buildWTtype4(const JSON& comp, const JsonParameterTable& params);
	static WPplant* buildWPplant(const JSON& comp, const JsonParameterTable& params);
	static PVplant* buildPVplant(const JSON& comp, const JsonParameterTable& params);
	static Transformer_real* buildTransformerReal(const JSON& comp, const JsonParameterTable& params);
	static Transformer_classic* buildTransformerClassic(const JSON& comp, const JsonParameterTable& params);
	static TransformerDeltaDelta* buildTransformerDeltaDelta(const JSON& comp, const JsonParameterTable& params);
	static TransformerDeltaY_real* buildTransformerDeltaYReal(const JSON& comp, const JsonParameterTable& params);
	static TransformerDeltaY* buildTransformerDeltaY(const JSON& comp, const JsonParameterTable& params);
	static TransformerYDelta* buildTransformerYDelta(const JSON& comp, const JsonParameterTable& params);
	static TransformerYY_real* buildTransformerYYReal(const JSON& comp, const JsonParameterTable& params);
	static TransformerYY* buildTransformerYY(const JSON& comp, const JsonParameterTable& params);

	static std::vector<double> readTransformerClassicValues(const JSON& comp, const JsonParameterTable& params);
	static std::vector<double> readTransformerRealValues(const JSON& comp, const JsonParameterTable& params);
	static std::vector<double> readNumericArray(
		const JSON& comp,
		const std::string& key,
		const JsonParameterTable& params);
	static DenseMatrix readDenseMatrixValues(
		const JSON& comp,
		const std::string& key,
		const JsonParameterTable& params);
	static std::vector<double> readVoltageVector(const JSON& comp, const JsonParameterTable& params);
	static double readScalarField(
		const JSON& comp,
		const std::string& key,
		const JsonParameterTable& params);

	static Admittance* buildSymbolicAdmittance(
		const JSON& comp,
		const JsonParameterTable& params,
		const char* exprField);
};
