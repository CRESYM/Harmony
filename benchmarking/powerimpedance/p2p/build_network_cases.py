"""Write Harmony JSON cases for the PowerImpedance P2P HVDC example."""
from __future__ import annotations

import copy
import json
import math
from pathlib import Path

ROOT = Path(__file__).resolve().parent
PI_ROOT = ROOT.parent
HARMONY = ROOT / "harmony"
STANDALONE_HARMONY = PI_ROOT / "standalone" / "harmony"

# geometry = [Δxbc, ybc]; rc/Rdc/dsag/dsb match C++ Conductors(...).
P2P_OHL = {
    "organization": "flat",
    "number_bundles": [3, 1],
    "geometry": [10.0, 30.0],
    "rc": 0.015,
    "Rdc": 0.063,
    "dsag": 10.0,
    "dsb": 0.0,
}
P2P_GW = {
    "count": 2,
    "geometry": [0.92, 0.0062, 10.0, 7.5, 6.5],
    "mu_g": 1.0,
}


def gfl_si(vll_rms: float, s_va: float, vdc: float):
    """PI GFL pu defaults converted to Harmony SI at the given plant bases."""
    vm = vll_rms * math.sqrt(2.0 / 3.0)
    zac = vll_rms ** 2 / s_va
    zdc = vdc ** 2 / s_va
    iac = 2.0 * s_va / (3.0 * vm)
    omega = 2.0 * math.pi * 50.0
    pq = [1, 0, 0.1 / (1.5 * vm), 31.4159 / (1.5 * vm), 1]
    return {
        "vm": vm,
        "zac": zac,
        "zdc": zdc,
        "iac": iac,
        "omega": omega,
        "pll": [1, 0, 0.28 * omega / vm, 12.5664 * omega / vm, 1, 0],
        "occ": [1, 0, 0.7691 * zac, 522.7654 * zac, 2, iac, 0.0],
        "ccc": [1, 0, 0.1048 * zdc, 48.1914 * zdc, 2, 0.0, 0.0],
        "pq": pq,
        "kp_vdc": 5.0 * iac / vdc,
        "ki_vdc": 15.0 * iac / vdc,
    }


def buses(*ids_pins_loc):
    out = []
    for item in ids_pins_loc:
        if len(item) == 2:
            bid, pins = item
            loc = "AC1"
        else:
            bid, pins, loc = item
        out.append({"id": bid, "location": loc, "pins": pins, "enabled": True})
    return out


def connect(a, t1, b, t2):
    return [
        {"bus_id": a, "terminal": t1},
        {"bus_id": b, "terminal": t2},
    ]


def ohl(cid, length_km, bus_a, bus_b, location="AC1"):
    return {
        "id": cid,
        "location": location,
        "type": "overhead_line",
        "length_km": length_km,
        "earth": [1.0, 1.0, 100.0],
        "conductor": dict(P2P_OHL),
        "groundwire": dict(P2P_GW),
        "connected_buses": connect(bus_a, 1, bus_b, 2),
        "enabled": True,
    }


def cable_dc():
    return {
        "id": "dc_line",
        "location": "DC1",
        "type": "cable",
        "cable_type": "underground",
        "length": 100000.0,
        "earth": [1.0, 1.0, 1.0],
        "conductors": [
            {"id": "C1", "ri": 0.0, "ro": 0.02425, "resistivity": 1.72e-8},
            {"id": "C2", "ri": 0.04175, "ro": 0.04625, "resistivity": 2.2e-7},
            {"id": "C3", "ri": 0.04975, "ro": 0.06055, "resistivity": 1.8e-7, "permeability": 10.0},
        ],
        "insulators": [
            {"id": "I1", "ri": 0.02425, "ro": 0.04175, "permittivity": 2.3},
            {"id": "I2", "ri": 0.04625, "ro": 0.04975, "permittivity": 2.3},
            {"id": "I3", "ri": 0.06055, "ro": 0.06575, "permittivity": 2.3},
        ],
        "positions": [[-0.5, 1.0], [0.5, 1.0]],
        "pins": 2,
        "connected_buses": connect("B4", 1, "B5", 2),
        "enabled": True,
    }


def mmc(cid, location, ac_bus, dc_bus, p, q, vdc_ctrl, lr, rr):
    # P2P ElectricalMMC: S=1000 MW, Vac LL=380 kV, Vdc=800 kV.
    si = gfl_si(380e3, 1000e6, 800e3)
    conv = [
        si["omega"],
        p,
        q,
        0.0,
        si["vm"],
        p,
        800e3,
        0.05,
        1.07,
        10e-3,
        400.0,
        lr,
        rr,
        0.0,
    ]
    ctrl = list(si["pll"])
    if vdc_ctrl:
        ctrl += [1, 0, si["kp_vdc"], si["ki_vdc"], 1, 800e3]
        ctrl += [0]  # active_power off
    else:
        ctrl += [0]  # dc_voltage off
        ctrl += si["pq"] + [p]
    ctrl += [0]
    ctrl += si["pq"] + [q]
    ctrl += [0, 0]
    ctrl += si["occ"] + si["ccc"] + [0]  # droop
    ctrl += [0]  # gfm
    return {
        "id": cid,
        "location": location,
        "type": "mmc",
        "converter_params": conv,
        "controller_params": ctrl,
        "connected_buses": connect(ac_bus, 1, dc_bus, 2),
        "enabled": True,
        # PowerImpedance MMC PF uses LossA=B=C=0 and no AC filter (bf=0).
        "opf_info": {
            "LossA": 0.0,
            "LossB": 0.0,
            "LossCrec": 0.0,
            "LossCinv": 0.0,
            "bf": 0.0,
        },
    }


def y_matrix(cid, start, end, points):
    return {
        "type": "y_matrix",
        "component_id": cid,
        "frequency_range": {"start": start, "end": end, "points": points},
        "plot": False,
    }


def dump(name, payload, dest=None):
    folder = dest or HARMONY
    folder.mkdir(parents=True, exist_ok=True)
    path = folder / name
    path.write_text(json.dumps(payload, indent=4) + "\n", encoding="utf-8")
    print("wrote", path)


# Power-flow linearisation point used by the overlay JSON (skip Harmony OPF).
# Indices in converter_params: Pac, Qac, theta, Vm, Pdc, Vdc.
P2P_PI_OP = {
    "c1": [-100316619.29, 100000000.0, -0.006596211679321095, 307649.6295688735, -100165413.04, 800000.0],
    "c2": [100000000.0, 100000000.0, 0.031205439828107417, 304179.4385933678, 100136222.4, 799766.85551],
}


def p2p_payload():
    return {
        "simulation": {
            "title": "pi_p2p",
            "description": "PowerImpedance P2P HVDC topology in Harmony (dq cut at c2 AC).",
            "output_directory": "./files",
            "frequency_range": {"start": 10, "end": 1000, "points": 41},
            "nominal_power": 1000.0,
            "nominal_voltage": 380.0,
            "dc_nominal_voltage": 800.0,
        },
        "buses": buses(
            ("B2", 3, "AC1"),
            ("B3", 3, "AC1"),
            ("B6", 3, "AC2"),
            ("B7", 3, "AC2"),
            ("B4", 2, "DC1"),
            ("B5", 2, "DC1"),
        ),
        "components": [
            {
                "id": "g4",
                "location": "AC1",
                "type": "ac_source",
                "pins": 3,
                "voltage": 380000.0,
                "values": [1e-6],
                "connected_bus": {"bus_id": "B2", "terminal": 1},
                "enabled": True,
            },
            {
                "id": "g1",
                "location": "AC2",
                "type": "ac_source",
                "pins": 3,
                "voltage": 380000.0,
                "values": [1e-6],
                "connected_bus": {"bus_id": "B7", "terminal": 1},
                "enabled": True,
            },
            ohl("tl1", 25.0, "B2", "B3", "AC1"),
            ohl("tl78", 90.0, "B6", "B7", "AC2"),
            cable_dc(),
            mmc("c1", "AC1_DC1", "B3", "B4", -100e6, 100e6, True, 0.06, 0.535),
            mmc("c2", "AC2_DC1", "B6", "B5", 100e6, 100e6, False, 0.0461, 0.4103),
        ],
        "computations": [
            {"type": "network_summary"},
            {
                "type": "power_flow",
                "vsc_control": True,
                "write_txt": False,
                "plot_result": False,
                "print_info": True,
            },
            y_matrix("dc_line", 10, 1000, 41),
            {
                "type": "stability_assessment",
                "converter_id": "c2",
                "location": "AC",
                "frequency_range": {"start": 10, "end": 1000, "points": 41},
                "plot": False,
                "park_per_component": False,
            },
        ],
    }


def write_p2p():
    dump("p2p.json", p2p_payload())


def write_p2p_pi_op():
    payload = copy.deepcopy(p2p_payload())
    payload["simulation"]["title"] = "pi_p2p_pi_op"
    payload["simulation"]["description"] = (
        "P2P HVDC linearised at the PowerImpedance power-flow operating point "
        "(Harmony OPF skipped)."
    )
    for comp in payload["components"]:
        cid = comp.get("id")
        if cid not in P2P_PI_OP:
            continue
        pac, qac, theta, vm, pdc, vdc = P2P_PI_OP[cid]
        conv = list(comp["converter_params"])
        conv[1:7] = [pac, qac, theta, vm, pdc, vdc]
        comp["converter_params"] = conv
    payload["computations"] = [
        {"type": "network_summary"},
        y_matrix("dc_line", 10, 1000, 41),
        {
            "type": "stability_assessment",
            "converter_id": "c2",
            "location": "AC",
            "frequency_range": {"start": 10, "end": 1000, "points": 41},
            "plot": False,
            "park_per_component": False,
            "skip_opf": True,
        },
    ]
    dump("p2p_pi_op.json", payload)


def write_mmc_c1():
    dump(
        "mmc_c1.json",
        {
            "simulation": {
                "title": "pi_p2p_mmc_c1",
                "description": "Standalone P2P c1: Vdc+Q, S=1000 MW, Vac=380 kV, Vdc=800 kV.",
                "output_directory": "./files",
                "frequency_range": {"start": 1, "end": 1000, "points": 81},
                "nominal_power": 1000.0,
                "nominal_voltage": 380.0,
                "dc_nominal_voltage": 800.0,
            },
            "buses": [
                {"id": "ac_bus", "location": "AC1", "pins": 3, "enabled": True},
                {"id": "dc_bus", "location": "DC1", "pins": 2, "enabled": True},
            ],
            "components": [
                mmc("c1", "AC1_DC1", "ac_bus", "dc_bus", -100e6, 100e6, True, 0.06, 0.535),
            ],
            "computations": [y_matrix("c1", 1, 1000, 81)],
        },
        STANDALONE_HARMONY,
    )


def write_mmc_c2(q_ac=100e6, name="mmc_c2.json", cid="c2"):
    dump(
        name,
        {
            "simulation": {
                "title": "pi_p2p_mmc_c2",
                "description": "Standalone P2P c2: P+Q, Lr=0.0461 H, Rr=0.4103 Ohm.",
                "output_directory": "./files",
                "frequency_range": {"start": 1, "end": 1000, "points": 81},
                "nominal_power": 1000.0,
                "nominal_voltage": 380.0,
                "dc_nominal_voltage": 800.0,
            },
            "buses": [
                {"id": "ac_bus", "location": "AC1", "pins": 3, "enabled": True},
                {"id": "dc_bus", "location": "DC1", "pins": 2, "enabled": True},
            ],
            "components": [
                mmc(cid, "AC1_DC1", "ac_bus", "dc_bus", 100e6, q_ac, False, 0.0461, 0.4103),
            ],
            "computations": [y_matrix(cid, 1, 1000, 81)],
        },
        STANDALONE_HARMONY,
    )


if __name__ == "__main__":
    HARMONY.mkdir(parents=True, exist_ok=True)
    write_p2p()
    write_p2p_pi_op()
    write_mmc_c1()
    write_mmc_c2()

