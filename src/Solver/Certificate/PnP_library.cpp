/**
 * @file PnP_library.cpp
 */
#include "PnP_library.h"

#include <fstream>
#include <iostream>
#include <nlohmann/json.hpp>

using JSON = nlohmann::json;

void PnPCertificateLibrary::clear()
{
	records_.clear();
}

void PnPCertificateLibrary::add(const PnPDeviceRecord& rec)
{
	for (auto& r : records_) {
		if (r.device_id == rec.device_id) {
			r = rec;
			return;
		}
	}
	records_.push_back(rec);
}

void PnPCertificateLibrary::addFromGate(const DeviceGateResult& gate, const std::string& device_type)
{
	PnPDeviceRecord rec;
	rec.device_id = gate.device_id;
	rec.device_type = device_type;
	rec.allow = gate.allow;
	rec.worst_passivity = gate.sweep.worst_passivity;
	rec.worst_phase_deg = gate.sweep.worst_phase_deg;
	rec.worst_shifted = gate.sweep.worst_shifted;
	rec.phase_limit_deg = gate.sweep.phase_limit_deg;
	if (!gate.sweep.freq_Hz.empty()) {
		rec.f_min_Hz = gate.sweep.freq_Hz.front();
		rec.f_max_Hz = gate.sweep.freq_Hz.back();
	}
	rec.notes = gate.reason;
	add(rec);
}

const PnPDeviceRecord* PnPCertificateLibrary::find(const std::string& device_id) const
{
	for (const auto& r : records_) {
		if (r.device_id == device_id)
			return &r;
	}
	return nullptr;
}

bool PnPCertificateLibrary::isAllowed(const std::string& device_id) const
{
	const auto* r = find(device_id);
	return r && r->allow;
}

void PnPCertificateLibrary::saveJson(const std::string& path) const
{
	JSON arr = JSON::array();
	for (const auto& r : records_) {
		arr.push_back({
			{"device_id", r.device_id},
			{"device_type", r.device_type},
			{"allow", r.allow},
			{"worst_passivity", r.worst_passivity},
			{"worst_phase_deg", r.worst_phase_deg},
			{"worst_shifted", r.worst_shifted},
			{"phase_limit_deg", r.phase_limit_deg},
			{"f_min_Hz", r.f_min_Hz},
			{"f_max_Hz", r.f_max_Hz},
			{"notes", r.notes}
		});
	}
	JSON root;
	root["pnp_certificate_library"] = arr;
	std::ofstream f(path);
	f << root.dump(2);
}

void PnPCertificateLibrary::loadJson(const std::string& path)
{
	std::ifstream f(path);
	JSON root;
	f >> root;
	records_.clear();
	const JSON& arr = root.contains("pnp_certificate_library")
		? root.at("pnp_certificate_library")
		: root;
	for (const auto& j : arr) {
		PnPDeviceRecord r;
		r.device_id = j.value("device_id", "");
		r.device_type = j.value("device_type", "");
		r.allow = j.value("allow", false);
		r.worst_passivity = j.value("worst_passivity", 0.0);
		r.worst_phase_deg = j.value("worst_phase_deg", 0.0);
		r.worst_shifted = j.value("worst_shifted", 0.0);
		r.phase_limit_deg = j.value("phase_limit_deg", 90.0);
		r.f_min_Hz = j.value("f_min_Hz", 1.0);
		r.f_max_Hz = j.value("f_max_Hz", 1000.0);
		r.notes = j.value("notes", "");
		records_.push_back(r);
	}
}

void PnPCertificateLibrary::saveCsv(const std::string& path) const
{
	std::ofstream f(path);
	f << "device_id,device_type,allow,worst_passivity,worst_phase_deg,worst_shifted,notes\n";
	for (const auto& r : records_) {
		f << r.device_id << ',' << r.device_type << ',' << (r.allow ? 1 : 0) << ','
		  << r.worst_passivity << ',' << r.worst_phase_deg << ','
		  << r.worst_shifted << ',' << '"' << r.notes << '"' << '\n';
	}
}

void PnPCertificateLibrary::printSummary() const
{
	std::size_t n_ok = 0;
	for (const auto& r : records_)
		if (r.allow) ++n_ok;
	std::cout << "[PnPLibrary] " << n_ok << " allowed / " << records_.size() << " total\n";
	for (const auto& r : records_) {
		std::cout << "  " << (r.allow ? "ALLOW" : "BLOCK") << "  " << r.device_id
			<< "  (" << r.device_type << ")  nu=" << r.worst_passivity
			<< "  phi=" << r.worst_phase_deg << "deg\n";
	}
}
