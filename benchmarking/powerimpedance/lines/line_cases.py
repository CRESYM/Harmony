"""Shared OHL/cable topology catalog for Harmony JSON, PowerImpedance Julia, and compare.py.

Electrical data follow the P2P / labanimal lines (50 km OHL, 100 km cable) so
organizations can be compared on equal material parameters. Geometry functions
mirror Harmony `overhead_line.cpp` (which matches PowerImpedance `overhead_line.jl`).
PSCAD expected coordinates are the Line Constants / tower-component conventions
(lowest-conductor height, isosceles delta, 2/3 sag, symmetrical bundle ring).
"""
from __future__ import annotations

import math
from typing import Any

FREQ = {"start": 10, "end": 1000, "points": 21}

# Shared OHL conductor / earth (P2P tl1, PI adm_OHL labanimal).
OHL_ELEC = dict(
    length_m=50e3,
    earth=(1.0, 1.0, 100.0),
    rc=0.015,
    Rdc=0.063,
    dsag=10.0,
    dx=10.0,
    ybc=30.0,
    dy=9.0,
    dxt=4.0,
    gw=dict(count=2, Rgdc=0.92, rg=0.0062, dgsag=10.0, dyg=7.5, dxg=6.5),
)

CABLE_LAYERS = dict(
    conductors=[
        {"id": "C1", "ri": 0.0, "ro": 0.02425, "resistivity": 1.72e-8},
        {"id": "C2", "ri": 0.04175, "ro": 0.04625, "resistivity": 2.2e-7},
        {"id": "C3", "ri": 0.04975, "ro": 0.06055, "resistivity": 1.8e-7, "permeability": 10.0},
    ],
    insulators=[
        {"id": "I1", "ri": 0.02425, "ro": 0.04175, "permittivity": 2.3},
        {"id": "I2", "ri": 0.04625, "ro": 0.04975, "permittivity": 2.3},
        {"id": "I3", "ri": 0.06055, "ro": 0.06575, "permittivity": 2.3},
    ],
    length_m=100e3,
    earth=(1.0, 1.0, 100.0),
)


def bundle_ring(n_sb: int, dsb: float) -> list[tuple[float, float]]:
    """Symmetrical bundle about the phase centre. PSCAD LCP and Harmony/PI."""
    if n_sb <= 1:
        return [(0.0, 0.0)]
    phi = 2.0 * math.pi / n_sb
    half = math.sin(phi / 2.0)
    r = 0.0 if abs(half) < 1e-18 else dsb / 2.0 / half
    phi_s = math.pi / 2.0
    if n_sb % 2 == 0:
        phi_s += phi / 2.0
    pts = []
    for _ in range(n_sb):
        pts.append((r * math.cos(phi_s), r * math.sin(phi_s)))
        phi_s += phi
    return pts


def apply_sag(y: float, dsag: float) -> float:
    """PSCAD: sag = tower height − midspan height; mean height ≈ H − (2/3) sag."""
    return y - (2.0 / 3.0) * dsag


def estimate_harmony(org: str, nb: int, dx: float, ybc: float, dy: float, dxt: float):
    """Phase-centre (x, y) as in Harmony/PowerImpedance (before sag and bundle)."""
    if org == "flat":
        if nb == 2:
            return [(-dx / 2, ybc), (dx / 2, ybc)]
        if nb == 3:
            return [(-dx, ybc), (0.0, ybc), (dx, ybc)]
        if nb == 6:
            ys = [ybc, ybc, ybc, ybc + dy, ybc + dy, ybc + dy]
            xs = [-dx, 0.0, dx, -dx, 0.0, dx]
            return list(zip(xs, ys))
    if org == "vertical":
        if nb == 3:
            return [(dx / 2, ybc + i * dy) for i in range(1, 4)]
        if nb == 6:
            xs = [dx / 2] * 3 + [-dx / 2] * 3
            ys = [ybc, ybc + dy, ybc + 2 * dy] * 2
            return list(zip(xs, ys))
    if org == "delta":
        if nb == 3:
            return [(-dx / 2, ybc), (dx / 2, ybc + dy), (0.0, ybc)]
        if nb == 6:
            xs = [
                -dx / 2 - dxt,
                -dx / 2 - dxt / 2,
                -dx / 2,
                dx / 2,
                dx / 2 + dxt / 2,
                dx / 2 + dxt,
            ]
            ys = [ybc, ybc + dy, ybc, ybc, ybc + dy, ybc]
            return list(zip(xs, ys))
    if org in ("concentric", "offset"):
        if nb == 3:
            return [(-dxt, ybc + dy), (0.0, ybc), (0.0, ybc + 2 * dy)]
        if nb == 6:
            xs = [
                -dx / 2 - dxt,
                -dx / 2,
                -dx / 2,
                dx / 2,
                dx / 2,
                dx / 2 + dxt,
            ]
            ys = [ybc + dy, ybc, ybc + 2 * dy, ybc, ybc + 2 * dy, ybc + dy]
            return list(zip(xs, ys))
    raise ValueError(f"unsupported {org} n_b={nb}")


def estimate_pscad(org: str, nb: int, dx: float, ybc: float, dy: float, dxt: float):
    """Coordinates implied by PSCAD named towers (Universal Tower / 3H, 3V, 3D).

    ybc is the height of the lowest phase conductor at the tower (before sag).
    Delta is an isosceles triangle: two phases at ybc, apex on the centre line.
    Vertical stacks from ybc upward (i = 0,1,2), not from ybc+Δy.
    Flat, double-circuit, concentric/offset: same as Harmony.
    """
    if org == "vertical" and nb == 3:
        return [(dx / 2, ybc + i * dy) for i in range(3)]
    if org == "delta" and nb == 3:
        return [(-dx / 2, ybc), (dx / 2, ybc), (0.0, ybc + dy)]
    return estimate_harmony(org, nb, dx, ybc, dy, dxt)


def groundwire_xy(ng: int, dxg: float, dyg: float, ybc: float, dgsag: float):
    if ng <= 0:
        return []
    pts = []
    for i in range(ng):
        x = dxg * (-(ng - 1.0) / 2.0 + i)
        y = apply_sag(dyg + ybc, dgsag)
        pts.append((x, y))
    return pts


def expand_conductors(centers, n_sb: int, dsb: float, dsag: float):
    ring = bundle_ring(n_sb, dsb)
    out = []
    for cx, cy in centers:
        for sx, sy in ring:
            out.append((cx + sx, apply_sag(cy + sy, dsag)))
    return out


def ohl_geometry_row(case: dict) -> dict:
    e = ohl_params(case)
    org, nb, nsb = case["org"], case["nb"], case["nsb"]
    dsb = case.get("dsb", 0.0)
    h = estimate_harmony(org, nb, e["dx"], e["ybc"], e["dy"], e["dxt"])
    p = estimate_pscad(org, nb, e["dx"], e["ybc"], e["dy"], e["dxt"])
    h_exp = expand_conductors(h, nsb, dsb, e["dsag"])
    p_exp = expand_conductors(p, nsb, dsb, e["dsag"])
    max_c = max(math.hypot(a[0] - b[0], a[1] - b[1]) for a, b in zip(h, p)) if h and p else 0.0
    gw = []
    if case.get("ng", 2) > 0:
        gw = groundwire_xy(e["gw"]["count"], e["gw"]["dxg"], e["gw"]["dyg"], e["ybc"], e["gw"]["dgsag"])
    return {
        "id": case["id"],
        "org": org,
        "nb": nb,
        "nsb": nsb,
        "harmony_centres": h,
        "pscad_centres": p,
        "centre_max_m": max_c,
        "harmony_conductors": h_exp,
        "pscad_conductors": p_exp,
        "groundwires": gw,
        "pscad_match": max_c < 1e-9,
        "note": case.get("pscad_note", ""),
    }


OHL_CASES: list[dict[str, Any]] = [
    dict(id="ohl_flat2", title="Flat 2-bundle twin (n_sb=2)", org="flat", nb=2, nsb=2, dsb=0.4572, ng=2,
         pscad_note="PSCAD 2-conductor horizontal: same height, ±Δx/2. Matches."),
    dict(id="ohl_flat3", title="Flat 3-phase (P2P tl1)", org="flat", nb=3, nsb=1, dsb=0.0, ng=2,
         pscad_note="PSCAD 3H: x={−Δx,0,Δx} at ybc. Matches."),
    dict(id="ohl_flat3_nogw", title="Flat 3-phase, no ground wire", org="flat", nb=3, nsb=1, dsb=0.0, ng=0,
         pscad_note="Same 3H without GW elimination. Matches."),
    dict(id="ohl_flat6", title="Flat double circuit (6)", org="flat", nb=6, nsb=1, dsb=0.0, ng=2,
         pscad_note="Two 3H circuits stacked by Δy. Matches a double-circuit horizontal tower."),
    dict(id="ohl_vert3", title="Vertical 3-phase", org="vertical", nb=3, nsb=1, dsb=0.0, ng=2,
         pscad_note="Harmony/PI put the lowest phase at ybc+Δy (i=1,2,3). PSCAD 3V uses ybc as the lowest (i=0,1,2). Shared H=PI, differs from PSCAD."),
    dict(id="ohl_vert6", title="Vertical double circuit (6)", org="vertical", nb=6, nsb=1, dsb=0.0, ng=2,
         pscad_note="Two vertical circuits at ±Δx/2. Lowest at ybc (not ybc+Δy). Matches PSCAD double-circuit vertical."),
    dict(id="ohl_delta3", title="Delta 3-phase", org="delta", nb=3, nsb=1, dsb=0.0, ng=2,
         pscad_note="Harmony/PI raise the right phase, not the centre. PSCAD 3D is isosceles (two at ybc, apex at centre). Shared H=PI, not PSCAD delta."),
    dict(id="ohl_delta6", title="Delta double circuit (6)", org="delta", nb=6, nsb=1, dsb=0.0, ng=2,
         pscad_note="Two leaning triangles. No single PSCAD stock tower; Universal Tower equivalent."),
    dict(id="ohl_conc3", title="Concentric 3-phase", org="concentric", nb=3, nsb=1, dsb=0.0, ng=2,
         pscad_note="Offset vertical: one phase shifted by Δ̃x. Universal Tower, not a named PSCAD type."),
    dict(id="ohl_off3", title="Offset 3-phase (alias of concentric)", org="offset", nb=3, nsb=1, dsb=0.0, ng=2,
         pscad_note="Same coordinates as concentric. Harmony and PI alias the two organizations."),
    dict(id="ohl_conc6", title="Concentric double circuit (6)", org="concentric", nb=6, nsb=1, dsb=0.0, ng=2,
         pscad_note="Double-circuit offset vertical. Universal Tower."),
    dict(id="ohl_vert3_twin", title="Vertical 3-phase twin bundle", org="vertical", nb=3, nsb=2, dsb=0.4572, ng=2,
         pscad_note="Same vertical-centre offset as ohl_vert3, plus PSCAD-symmetric 2-bundle ring."),
    dict(
        id="ohl_2gw",
        title="150 km vertical twin (2 GW corridor)",
        org="vertical",
        nb=3,
        nsb=2,
        dsb=0.4572,
        ng=2,
        length_m=150e3,
        dx=4.0,
        ybc=12.0,
        dy=9.0,
        gw=dict(count=2, Rgdc=0.92, rg=0.0062, dgsag=10.0, dyg=35.0, dxg=8.0),
        pscad_note="Same 3V i=1,2,3 offset as ohl_vert3. Used by the unbalanced_yeff 2 GW OHL network (Harmony-only H vs Yeff).",
    ),
]

CABLE_CASES: list[dict[str, Any]] = [
    dict(id="cable_aerial", title="Aerial coaxial, one core", cable_type="aerial",
         positions=[(0.0, 1.0)],
         pscad_note="Wedepohl/Wilcox earth return (PSCAD underground/aerial default). Single core."),
    dict(id="cable_ug_single", title="Underground coaxial, one core", cable_type="underground",
         positions=[(0.0, 1.0)],
         pscad_note="Adds log(D2/D1) potential coefficients for burial. PSCAD LCP coaxial SC cable."),
    dict(id="cable_ug_bipolar", title="Underground bipolar, equal depth", cable_type="underground",
         positions=[(-0.5, 1.0), (0.5, 1.0)],
         pscad_note="P2P DC cable. Equal y ⇒ Harmony per-cable Zg equals PI copy-from-first."),
    dict(id="cable_ug_staggered", title="Underground bipolar, different depths", cable_type="underground",
         positions=[(-0.5, 0.8), (0.5, 1.2)],
         pscad_note="PSCAD uses each cable's own depth in self Zg. Harmony does too. PI stamps Zg(y1) on every core."),
    dict(id="cable_aerial_bipolar", title="Aerial bipolar, equal height", cable_type="aerial",
         positions=[(-0.5, 1.0), (0.5, 1.0)],
         pscad_note="No underground P_ij. Mutual earth return only."),
]


def ohl_params(case: dict) -> dict:
    e = dict(OHL_ELEC)
    e["gw"] = dict(e["gw"])
    for k in ("length_m", "dx", "ybc", "dy", "dxt", "rc", "Rdc", "dsag"):
        if k in case:
            e[k] = case[k]
    if "gw" in case:
        e["gw"].update(case["gw"])
    return e


def _geo_array(org: str, nb: int, e: dict) -> list[float]:
    if org == "flat" and nb in (2, 3):
        return [e["dx"], e["ybc"]]
    if org == "flat" and nb == 6:
        return [e["dx"], e["ybc"], e["dy"]]
    if org == "vertical":
        return [e["dx"], e["ybc"], e["dy"]]
    return [e["dx"], e["ybc"], e["dy"], e["dxt"]]


def all_line_ids() -> list[str]:
    return [c["id"] for c in OHL_CASES] + [c["id"] for c in CABLE_CASES]


def compare_cases() -> list[dict]:
    out = []
    for c in OHL_CASES:
        n = c["nb"]
        entries = ["Y11", "Y12"] if n >= 2 else ["Y11"]
        if n >= 3:
            entries.append("Y13")
        out.append(
            {
                "suite": "lines",
                "id": c["id"],
                "title": c["title"],
                "harmony_csv": f"{c['id']}.csv",
                "pi_csv": f"pi_{c['id']}.csv",
                "spot_hz": [10, 50, 100, 500, 1000],
                "plot_entries": entries,
            }
        )
    for c in CABLE_CASES:
        n = len(c["positions"])
        # After Kron, Y is n×n per end → 2n×2n two-port.
        entries = ["Y11", "Y12"]
        if n >= 2:
            entries.append("Y13")
        out.append(
            {
                "suite": "lines",
                "id": c["id"],
                "title": c["title"],
                "harmony_csv": f"{c['id']}.csv",
                "pi_csv": f"pi_{c['id']}.csv",
                "spot_hz": [10, 50, 100, 500, 1000],
                "plot_entries": entries,
            }
        )
    return out
