#!/usr/bin/env python3
"""Convert MATPOWER / MatACDC ``.m`` case files into Harmony's CSV case data.

Harmony's OPF reads headerless numeric CSVs from ``src/data`` (see
``src/Solver/OPF/Powerflow_data.cpp``). The layouts are MATPOWER's, with a
trailing ``grid`` column added to every AC matrix, and a 26-column converter
matrix that drops MatACDC's transformer/filter/reactor flags and inserts a
``gridac`` column.

Subcommands
-----------
convert       MATPOWER/MatACDC ``.m`` -> ``<prefix>_*_ac.csv`` (+ ``*_dc.csv``)
extract-grid  pull one ``grid`` out of an existing Harmony AC case

Examples
--------
    python tools/matpower_to_harmony.py convert case39.m --prefix ieee39 --ac-only
    python tools/matpower_to_harmony.py convert case39_10_he.m --prefix ieee39hvdc
    python tools/matpower_to_harmony.py extract-grid ac9ac14 --grid 1 --prefix ieee9
"""

from __future__ import annotations

import argparse
import re
import sys
from pathlib import Path

# Harmony column orders, from Powerflow_network_params.cpp (map2dense calls).
BUS_AC_COLS = 14      # MATPOWER bus (13) + grid
BRANCH_AC_COLS = 14   # MATPOWER branch (13) + grid
GEN_AC_COLS = 22      # MATPOWER gen (21) + grid
GENCOST_AC_COLS = 8   # model, startup, shutdown, n, c2, c1, c0, grid
BUS_DC_COLS = 13      # MATPOWER bus layout, no grid column
BRANCH_DC_COLS = 13   # MATPOWER branch layout, no grid column
CONV_DC_COLS = 26

CONV_DC_ORDER = [
    "busdc_i", "busac_i", "gridac", "type_dc", "type_ac",
    "P_g", "Q_g", "Vtar",
    "rtf", "xtf", "bf", "rc", "xc",
    "basekVac", "Vmmax", "Vmmin", "Imax", "status",
    "LossA", "LossB", "LossCrec", "LossCinv",
    "droop", "Pdcset", "Vdcset", "dVdcset",
]

# Canonical MatACDC orders, used when a file has no %column_names% header.
DEFAULT_DCBUS_COLS = [
    "busdc_i", "grid", "Pdc", "Vdc", "basekVdc", "Vdcmax", "Vdcmin", "Cdc",
]
DEFAULT_DCBRANCH_COLS = [
    "fbusdc", "tbusdc", "r", "l", "c", "rateA", "rateB", "rateC", "status",
]

FIELD_ALIASES = {
    "bus": ("bus",),
    "branch": ("branch",),
    "gen": ("gen",),
    "gencost": ("gencost",),
    "dcbus": ("dcbus", "busdc"),
    "dcconv": ("dcconv", "convdc"),
    "dcbranch": ("dcbranch", "branchdc"),
}


class CaseParseError(RuntimeError):
    pass


def _strip_comments(text: str) -> str:
    """Drop ``%`` comments but keep ``%column_names%`` markers intact."""
    out = []
    for line in text.splitlines():
        if line.lstrip().startswith("%column_names%"):
            out.append(line)
            continue
        out.append(line.split("%", 1)[0])
    return "\n".join(out)


def parse_case(path: Path) -> dict:
    """Return ``{"matrices": {field: rows}, "columns": {field: names}, "scalars": {...}}``."""
    raw = path.read_text(encoding="utf-8", errors="replace")

    # Column-name headers appear on the comment line above the matrix.
    columns: dict[str, list[str]] = {}
    lines = raw.splitlines()
    for idx, line in enumerate(lines):
        if "%column_names%" not in line:
            continue
        names = line.split("%column_names%", 1)[1].split()
        for follow in lines[idx + 1: idx + 4]:
            m = re.search(r"mpc\.([A-Za-z_]\w*)\s*=", follow)
            if m:
                columns[m.group(1)] = names
                break

    text = _strip_comments(raw)

    matrices: dict[str, list[list[float]]] = {}
    for m in re.finditer(r"mpc\.([A-Za-z_]\w*)\s*=\s*\[(.*?)\]\s*;", text, re.S):
        field, body = m.group(1), m.group(2)
        rows = []
        for chunk in body.split(";"):
            chunk = chunk.strip()
            if not chunk:
                continue
            cells = [c for c in re.split(r"[,\s]+", chunk) if c]
            try:
                rows.append([float(c) for c in cells])
            except ValueError:
                raise CaseParseError(
                    f"{path.name}: non-numeric entry in mpc.{field}: {chunk!r}"
                )
        if rows:
            matrices[field] = rows

    scalars: dict[str, float] = {}
    for m in re.finditer(r"mpc\.([A-Za-z_]\w*)\s*=\s*([-+0-9.eE]+)\s*;", text):
        scalars[m.group(1)] = float(m.group(2))

    return {"matrices": matrices, "columns": columns, "scalars": scalars}


def pick(case: dict, kind: str):
    """Resolve a logical field to ``(rows, column_names_or_None)``."""
    for name in FIELD_ALIASES[kind]:
        if name in case["matrices"]:
            return case["matrices"][name], case["columns"].get(name)
    return None, None


def col_getter(names, defaults):
    """Build ``get(row, key, fallback)`` resolving by name, else by fixed position."""
    lookup = {}
    source = names if names else defaults
    for i, n in enumerate(source):
        lookup.setdefault(n.lower(), i)

    def get(row, key, fallback=0.0):
        i = lookup.get(key.lower())
        if i is None or i >= len(row):
            return fallback
        return row[i]

    return get


def write_csv(path: Path, rows, expected_cols: int) -> None:
    for r in rows:
        if len(r) != expected_cols:
            raise CaseParseError(
                f"{path.name}: built row with {len(r)} columns, expected {expected_cols}"
            )
    text = "".join(",".join(fmt(v) for v in r) + "\n" for r in rows)
    path.write_text(text, encoding="ascii")
    print(f"  wrote {path.name:<34} {len(rows):>4} rows x {expected_cols} cols")


def fmt(v: float) -> str:
    if v == int(v) and abs(v) < 1e15:
        return str(int(v))
    return repr(v)


def ac_islands(bus_rows, branch_rows) -> dict:
    """Map AC bus number -> island index (1-based, ordered by lowest bus number)."""
    ids = [int(r[0]) for r in bus_rows]
    adj = {i: set() for i in ids}
    for r in branch_rows:
        f_bus, t_bus = int(r[0]), int(r[1])
        adj[f_bus].add(t_bus)
        adj[t_bus].add(f_bus)

    island_of: dict[int, int] = {}
    index = 0
    for start in ids:
        if start in island_of:
            continue
        index += 1
        stack = [start]
        while stack:
            node = stack.pop()
            if node in island_of:
                continue
            island_of[node] = index
            stack.extend(adj[node] - island_of.keys())
    return island_of


def resolve_grid(bus_rows, branch_rows, mode: str) -> dict:
    """Map AC bus number -> Harmony grid index."""
    if mode == "area":
        areas = sorted({int(r[6]) for r in bus_rows})
        remap = {a: i + 1 for i, a in enumerate(areas)}
        return {int(r[0]): remap[int(r[6])] for r in bus_rows}
    if mode == "island":
        return ac_islands(bus_rows, branch_rows)
    return {int(r[0]): 1 for r in bus_rows}


def renumber_within_grids(bus_rows, grid_of: dict) -> dict:
    """Map AC bus number -> 1..N within its grid.

    ``load_params_ac`` sizes ``IDtoCountmap`` to the bus count of each grid and
    then indexes it with ``bus_id - 1``, so per-grid IDs must be contiguous from
    one (Powerflow_network_params.cpp, load_params_ac).
    """
    per_grid: dict[int, list[int]] = {}
    for r in bus_rows:
        per_grid.setdefault(grid_of[int(r[0])], []).append(int(r[0]))

    remap: dict[int, int] = {}
    for buses in per_grid.values():
        for new_id, old_id in enumerate(sorted(buses), start=1):
            remap[old_id] = new_id
    return remap


def convert(args) -> int:
    src = Path(args.infile)
    if not src.is_file():
        print(f"error: no such file: {src}", file=sys.stderr)
        return 2

    case = parse_case(src)
    out = Path(args.outdir)
    out.mkdir(parents=True, exist_ok=True)
    prefix = args.prefix

    bus, _ = pick(case, "bus")
    branch, _ = pick(case, "branch")
    gen, _ = pick(case, "gen")
    gencost, _ = pick(case, "gencost")
    if bus is None or branch is None or gen is None:
        print(f"error: {src.name} lacks mpc.bus / mpc.branch / mpc.gen", file=sys.stderr)
        return 2

    base_mva = case["scalars"].get("baseMVA")
    if base_mva is None:
        print(f"error: {src.name} has no mpc.baseMVA", file=sys.stderr)
        return 2

    mode = "area" if args.grid_from_area else ("island" if args.grid_from_island else "single")
    grid_of = resolve_grid(bus, branch, mode)
    bus_id = renumber_within_grids(bus, grid_of)

    ngrids = len(set(grid_of.values()))
    renumbered = sum(1 for old, new in bus_id.items() if old != new)
    print(f"{src.name} -> {prefix}_*  (baseMVA={fmt(base_mva)}, AC grids={ngrids})")
    if renumbered:
        print(f"  note: renumbered {renumbered} AC buses to 1..N within each grid")

    for g in sorted(set(grid_of.values())):
        slacks = [bus_id[int(r[0])] for r in bus
                  if grid_of[int(r[0])] == g and int(r[1]) == 3]
        if not slacks:
            raise CaseParseError(f"AC grid {g} has no slack (type 3) bus")

    # --- AC ---------------------------------------------------------------
    write_csv(out / f"{prefix}_baseMVA_ac.csv", [[base_mva]], 1)

    bus_out = []
    for r in bus:
        row = list(r[:13])
        row[0] = bus_id[int(r[0])]
        bus_out.append(row + [grid_of[int(r[0])]])
    bus_out.sort(key=lambda row: (row[-1], row[0]))
    write_csv(out / f"{prefix}_bus_ac.csv", bus_out, BUS_AC_COLS)

    branch_out = []
    for r in branch:
        f_bus, t_bus = int(r[0]), int(r[1])
        gf, gt = grid_of[f_bus], grid_of[t_bus]
        if gf != gt:
            raise CaseParseError(
                f"AC branch {f_bus}-{t_bus} crosses grids {gf}/{gt}; "
                "separate AC grids cannot be linked by an AC branch"
            )
        row = list(r[:13])
        row[0], row[1] = bus_id[f_bus], bus_id[t_bus]
        branch_out.append(row + [gf])
    write_csv(out / f"{prefix}_branch_ac.csv", branch_out, BRANCH_AC_COLS)

    gen_out = []
    for r in gen:
        row = list(r[:21])
        row += [0.0] * (21 - len(row))
        row[0] = bus_id[int(r[0])]
        gen_out.append(row + [grid_of[int(r[0])]])
    write_csv(out / f"{prefix}_gen_ac.csv", gen_out, GEN_AC_COLS)

    if gencost is None:
        gencost = [[2, 0, 0, 3, 0, 1, 0] for _ in gen]
        print("  note: no mpc.gencost; using unit linear cost")
    cost_out = []
    for r, g in zip(gencost, gen_out):
        model, startup, shutdown, ncost = r[0], r[1], r[2], int(r[3])
        coeffs = r[4:4 + ncost]
        if ncost == 3:
            c2, c1, c0 = coeffs
        elif ncost == 2:
            c2, (c1, c0) = 0.0, coeffs
        elif ncost == 1:
            c2, c1, c0 = 0.0, 0.0, coeffs[0]
        else:
            raise CaseParseError(
                f"gencost with n={ncost} is not a polynomial Harmony can store "
                "(needs n<=3)"
            )
        cost_out.append([model, startup, shutdown, 3, c2, c1, c0, g[-1]])
    write_csv(out / f"{prefix}_gencost_ac.csv", cost_out, GENCOST_AC_COLS)

    # Harmony always opens this file; an empty one means "no RES units".
    (out / f"{prefix}_res_ac.csv").write_text("", encoding="ascii")
    print(f"  wrote {prefix + '_res_ac.csv':<34}    0 rows (no RES units)")

    if args.ac_only:
        return 0

    # --- DC ---------------------------------------------------------------
    dcbus, dcbus_names = pick(case, "dcbus")
    dcconv, dcconv_names = pick(case, "dcconv")
    dcbranch, dcbranch_names = pick(case, "dcbranch")
    if dcbus is None or dcconv is None or dcbranch is None:
        print("  note: no DC data in this file; wrote AC case only")
        return 0

    pol = case["scalars"].get("dcpol", 1.0)
    base_mw = case["scalars"].get("baseMVAdc", base_mva)

    gb = col_getter(dcbus_names, DEFAULT_DCBUS_COLS)
    gc = col_getter(dcconv_names, CONV_DC_ORDER)
    gr = col_getter(dcbranch_names, DEFAULT_DCBRANCH_COLS)

    # The OPF couples converter i to DC bus i directly (Powerflow_solver.cpp:
    # "pc_dc(i) + pn_dc(i) + convPloss_dc(i) == 0"), so there must be exactly one
    # converter per DC bus and the rows must be in DC bus order.
    if len(dcconv) != len(dcbus):
        raise CaseParseError(
            f"{len(dcbus)} DC buses but {len(dcconv)} converters; Harmony's OPF "
            "requires exactly one converter per DC bus"
        )
    dcconv = sorted(dcconv, key=lambda r: int(gc(r, "busdc_i")))
    dcbus = sorted(dcbus, key=lambda r: int(gb(r, "busdc_i")))
    for b, c in zip(dcbus, dcconv):
        if int(gb(b, "busdc_i")) != int(gc(c, "busdc_i")):
            raise CaseParseError(
                "converter DC bus numbers do not match the DC bus list one-to-one"
            )

    # DC buses whose converter controls DC voltage are flagged as slack.
    slack_dc = {int(gc(r, "busdc_i")) for r in dcconv if int(gc(r, "type_dc")) == 2}

    bus_dc_out = []
    for r in dcbus:
        i = int(gb(r, "busdc_i"))
        bus_dc_out.append([
            i,
            3 if i in slack_dc else 1,
            0.0, 0.0, 0.0, 0.0,                  # Pd, Qd, Gs, Bs
            gb(r, "grid", 1.0),                  # area
            gb(r, "Vdc", 1.0), 0.0,              # Vm, Va
            gb(r, "basekVdc", 345.0),            # baseKV
            1.0,                                 # zone
            gb(r, "Vdcmax", 1.1), gb(r, "Vdcmin", 0.9),
        ])
    write_csv(out / f"{prefix}_bus_dc.csv", bus_dc_out, BUS_DC_COLS)

    branch_dc_out = []
    for r in dcbranch:
        rate = gr(r, "rateA", 100.0)
        branch_dc_out.append([
            int(gr(r, "fbusdc")), int(gr(r, "tbusdc")),
            gr(r, "r"), 0.0, 0.0,                # DC: no reactance/susceptance
            rate, gr(r, "rateB", rate), gr(r, "rateC", rate),
            0.0, 0.0,                            # ratio, angle
            gr(r, "status", 1.0),
            0.0, 0.0,                            # angmin, angmax
        ])
    write_csv(out / f"{prefix}_branch_dc.csv", branch_dc_out, BRANCH_DC_COLS)

    conv_out = []
    for r in dcconv:
        ac_bus_orig = int(gc(r, "busac_i"))
        if ac_bus_orig not in bus_id:
            raise CaseParseError(
                f"converter references AC bus {ac_bus_orig}, which is not in mpc.bus"
            )
        ac_bus = bus_id[ac_bus_orig]
        # MatACDC gates the transformer / filter / reactor with 0-1 flags that
        # Harmony's 26-column layout does not carry, so fold them in here.
        has_tf = gc(r, "transformer", 1.0)
        has_filter = gc(r, "filter", 1.0)
        has_reactor = gc(r, "reactor", 1.0)
        conv_out.append([
            int(gc(r, "busdc_i")), ac_bus,
            grid_of[ac_bus_orig],
            gc(r, "type_dc", 1.0), gc(r, "type_ac", 1.0),
            gc(r, "P_g"), gc(r, "Q_g"), gc(r, "Vtar", 1.0),
            gc(r, "rtf") if has_tf else 0.0,
            gc(r, "xtf") if has_tf else 0.0,
            gc(r, "bf") if has_filter else 0.0,
            gc(r, "rc") if has_reactor else 0.0,
            gc(r, "xc") if has_reactor else 0.0,
            gc(r, "basekVac", 345.0),
            gc(r, "Vmmax", 1.1), gc(r, "Vmmin", 0.9),
            gc(r, "Imax", 1.1), gc(r, "status", 1.0),
            gc(r, "LossA"), gc(r, "LossB"),
            gc(r, "LossCrec"), gc(r, "LossCinv"),
            gc(r, "droop"), gc(r, "Pdcset"),
            gc(r, "Vdcset"), gc(r, "dVdcset"),
        ])
    write_csv(out / f"{prefix}_conv_dc.csv", conv_out, CONV_DC_COLS)

    write_csv(out / f"{prefix}_baseMW_dc.csv", [[base_mw]], 1)
    write_csv(out / f"{prefix}_pol_dc.csv", [[pol]], 1)

    if not slack_dc:
        print("  note: no converter is in DC voltage control (type_dc=2); "
              "run this case with vsc_control disabled so the OPF picks setpoints")
    return 0


def extract_grid(args) -> int:
    """Split one grid out of an existing multi-grid Harmony AC case."""
    src = Path(args.outdir)
    prefix, grid = args.source_prefix, args.grid

    files = {
        "bus": (BUS_AC_COLS, 0),
        "branch": (BRANCH_AC_COLS, None),
        "gen": (GEN_AC_COLS, 0),
        "gencost": (GENCOST_AC_COLS, None),
    }

    print(f"{prefix}_* grid {grid} -> {args.prefix}_*")

    kept: dict[str, list[list[float]]] = {}
    for kind, (ncols, _) in files.items():
        path = src / f"{prefix}_{kind}_ac.csv"
        if not path.is_file():
            print(f"error: missing {path}", file=sys.stderr)
            return 2
        rows = []
        for line in path.read_text(encoding="ascii").splitlines():
            if not line.strip():
                continue
            row = [float(c) for c in line.split(",")]
            if len(row) != ncols:
                print(f"error: {path.name} row has {len(row)} cols, expected {ncols}",
                      file=sys.stderr)
                return 2
            if int(row[-1]) == grid:
                rows.append(row)
        if not rows:
            print(f"error: no rows with grid={grid} in {path.name}", file=sys.stderr)
            return 2
        kept[kind] = rows

    # gencost rows pair with gen rows positionally, so filter them together.
    all_gen = [
        [float(c) for c in line.split(",")]
        for line in (src / f"{prefix}_gen_ac.csv").read_text(encoding="ascii").splitlines()
        if line.strip()
    ]
    all_cost = [
        [float(c) for c in line.split(",")]
        for line in (src / f"{prefix}_gencost_ac.csv").read_text(encoding="ascii").splitlines()
        if line.strip()
    ]
    if len(all_gen) == len(all_cost):
        kept["gencost"] = [c for g, c in zip(all_gen, all_cost) if int(g[-1]) == grid]

    # Renumber buses to 1..N and normalise the grid column to 1.
    bus_ids = [int(r[0]) for r in kept["bus"]]
    remap = {b: i + 1 for i, b in enumerate(bus_ids)}

    for r in kept["bus"]:
        r[0] = remap[int(r[0])]
        r[-1] = 1
    for r in kept["branch"]:
        r[0], r[1] = remap[int(r[0])], remap[int(r[1])]
        r[-1] = 1
    for r in kept["gen"]:
        r[0] = remap[int(r[0])]
        r[-1] = 1
    for r in kept["gencost"]:
        r[-1] = 1

    for kind, (ncols, _) in files.items():
        write_csv(src / f"{args.prefix}_{kind}_ac.csv", kept[kind], ncols)

    base = (src / f"{prefix}_baseMVA_ac.csv").read_text(encoding="ascii").strip()
    (src / f"{args.prefix}_baseMVA_ac.csv").write_text(base + "\n", encoding="ascii")
    print(f"  wrote {args.prefix + '_baseMVA_ac.csv':<34}    1 row (baseMVA={base})")
    (src / f"{args.prefix}_res_ac.csv").write_text("", encoding="ascii")
    print(f"  wrote {args.prefix + '_res_ac.csv':<34}    0 rows (no RES units)")
    return 0


def main(argv=None) -> int:
    ap = argparse.ArgumentParser(description=__doc__,
                                formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--outdir", default=str(Path(__file__).resolve().parent.parent / "src" / "data"),
                    help="destination directory (default: src/data)")
    sub = ap.add_subparsers(dest="cmd", required=True)

    c = sub.add_parser("convert", help="convert a MATPOWER/MatACDC .m file")
    c.add_argument("infile")
    c.add_argument("--prefix", required=True, help="Harmony case prefix, e.g. ieee39")
    c.add_argument("--ac-only", action="store_true", help="skip DC matrices")
    c.add_argument("--grid-from-area", action="store_true",
                   help="derive the grid column from the bus AREA column")
    c.add_argument("--grid-from-island", action="store_true",
                   help="derive the grid column from AC connectivity (separate islands "
                        "become separate Harmony AC grids)")
    c.set_defaults(func=convert)

    e = sub.add_parser("extract-grid", help="split one grid out of a Harmony AC case")
    e.add_argument("source_prefix")
    e.add_argument("--grid", type=int, required=True)
    e.add_argument("--prefix", required=True)
    e.set_defaults(func=extract_grid)

    args = ap.parse_args(argv)
    try:
        return args.func(args)
    except CaseParseError as exc:
        print(f"error: {exc}", file=sys.stderr)
        return 2


if __name__ == "__main__":
    raise SystemExit(main())
