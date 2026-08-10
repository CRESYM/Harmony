#ifndef PNP_LIBRARY_H
#define PNP_LIBRARY_H

/**
 * @file PnP_library.h
 * @brief Workflow 2 — catalog of device certificate results for plug-and-play.
 */

#include "Device_gate.h"

#include <string>
#include <vector>

/** @brief One catalog entry for a certified (or rejected) device. */
struct PnPDeviceRecord {
	std::string device_id;
	std::string device_type; ///< e.g. "MMC_GFM", "MMC_GFL"
	bool allow = false;
	double worst_passivity = 0.0;
	double worst_phase_deg = 0.0;
	double worst_shifted = 0.0;
	double phase_limit_deg = 90.0;
	double f_min_Hz = 1.0;
	double f_max_Hz = 1000.0;
	std::string notes;
};

/**
 * @brief In-memory / JSON-backed library of device gate results.
 *
 * Use after running @ref evaluateDeviceGate on candidate converters; persist
 * with @ref saveJson / reload with @ref loadJson for PnP screening.
 */
class PnPCertificateLibrary {
public:
	void clear();
	void add(const PnPDeviceRecord& rec);
	void addFromGate(const DeviceGateResult& gate, const std::string& device_type = "");

	const std::vector<PnPDeviceRecord>& records() const { return records_; }
	std::size_t size() const { return records_.size(); }

	/** @brief Lookup by id; returns nullptr if missing. */
	const PnPDeviceRecord* find(const std::string& device_id) const;

	/** @brief True if id exists and allow==true. */
	bool isAllowed(const std::string& device_id) const;

	void saveJson(const std::string& path) const;
	void loadJson(const std::string& path);
	void saveCsv(const std::string& path) const;

	/** @brief Count of allow / block entries. */
	void printSummary() const;

private:
	std::vector<PnPDeviceRecord> records_;
};

#endif // PNP_LIBRARY_H
