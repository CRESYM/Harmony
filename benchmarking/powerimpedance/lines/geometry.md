# OHL and cable geometry vs PSCAD Line Constants

Harmony and PowerImpedance share the same named-organization formulas.
PSCAD stock towers (3H / 3V / 3D, Universal Tower) use the conventions
in the [overhead line tower](https://www.pscad.com/webhelp/Master_Library_Models/Transmission_Lines_Cables/Transmission_Line_Models/tower_comp.htm)
and [earth-return](https://www.pscad.com/webhelp-v502-ol/EMTDC/Transmission_Lines/Mutual_Impedance_with_Earth_Return.htm) pages.

Shared with PSCAD (not a discrepancy):

- DC resistance is **Ω/km** (PSCAD “entire conductor”). Harmony multiplies by `1e-3`.
- Sag: mean height `H − (2/3) sag` (PSCAD sag = tower height − mid-span).
- Symmetrical bundle: ring of radius `d/(2 sin(π/n_sb))`, even-n rotated by `π/n_sb`.
- Aerial earth return: Deri–Semlyen complex image (PSCAD default for overhead).
- Cable earth return: Wedepohl–Wilcox (PSCAD default for coaxial SC cables).
- Cable `ε0`: Harmony `8.854e-12` (SI). PowerImpedance uses `8.85e-12`.

## OHL phase centres (before sag), metres

| id | org | n | max |H−PSCAD| (m) | Harmony centres | PSCAD centres | Verdict |
|----|-----|---|----------------:|-----------------|---------------|---------|
| `ohl_flat2` | flat | 2 | 0.000 | `(-5.000,30.000); (5.000,30.000)` | `(-5.000,30.000); (5.000,30.000)` | match |
| `ohl_flat3` | flat | 3 | 0.000 | `(-10.000,30.000); (0.000,30.000); (10.000,30.000)` | `(-10.000,30.000); (0.000,30.000); (10.000,30.000)` | match |
| `ohl_flat3_nogw` | flat | 3 | 0.000 | `(-10.000,30.000); (0.000,30.000); (10.000,30.000)` | `(-10.000,30.000); (0.000,30.000); (10.000,30.000)` | match |
| `ohl_flat6` | flat | 6 | 0.000 | `(-10.000,30.000); (0.000,30.000); (10.000,30.000); (-10.000,39.000); (0.000,39.000); (10.000,39.000)` | `(-10.000,30.000); (0.000,30.000); (10.000,30.000); (-10.000,39.000); (0.000,39.000); (10.000,39.000)` | match |
| `ohl_vert3` | vertical | 3 | 9.000 | `(5.000,39.000); (5.000,48.000); (5.000,57.000)` | `(5.000,30.000); (5.000,39.000); (5.000,48.000)` | H=PI, not PSCAD |
| `ohl_vert6` | vertical | 6 | 0.000 | `(5.000,30.000); (5.000,39.000); (5.000,48.000); (-5.000,30.000); (-5.000,39.000); (-5.000,48.000)` | `(5.000,30.000); (5.000,39.000); (5.000,48.000); (-5.000,30.000); (-5.000,39.000); (-5.000,48.000)` | match |
| `ohl_delta3` | delta | 3 | 9.000 | `(-5.000,30.000); (5.000,39.000); (0.000,30.000)` | `(-5.000,30.000); (5.000,30.000); (0.000,39.000)` | H=PI, not PSCAD |
| `ohl_delta6` | delta | 6 | 0.000 | `(-9.000,30.000); (-7.000,39.000); (-5.000,30.000); (5.000,30.000); (7.000,39.000); (9.000,30.000)` | `(-9.000,30.000); (-7.000,39.000); (-5.000,30.000); (5.000,30.000); (7.000,39.000); (9.000,30.000)` | match |
| `ohl_conc3` | concentric | 3 | 0.000 | `(-4.000,39.000); (0.000,30.000); (0.000,48.000)` | `(-4.000,39.000); (0.000,30.000); (0.000,48.000)` | match |
| `ohl_off3` | offset | 3 | 0.000 | `(-4.000,39.000); (0.000,30.000); (0.000,48.000)` | `(-4.000,39.000); (0.000,30.000); (0.000,48.000)` | match |
| `ohl_conc6` | concentric | 6 | 0.000 | `(-9.000,39.000); (-5.000,30.000); (-5.000,48.000); (5.000,30.000); (5.000,48.000); (9.000,39.000)` | `(-9.000,39.000); (-5.000,30.000); (-5.000,48.000); (5.000,30.000); (5.000,48.000); (9.000,39.000)` | match |
| `ohl_vert3_twin` | vertical | 3 | 9.000 | `(5.000,39.000); (5.000,48.000); (5.000,57.000)` | `(5.000,30.000); (5.000,39.000); (5.000,48.000)` | H=PI, not PSCAD |
| `ohl_2gw` | vertical | 3 | 9.000 | `(2.000,21.000); (2.000,30.000); (2.000,39.000)` | `(2.000,12.000); (2.000,21.000); (2.000,30.000)` | H=PI, not PSCAD |

## Notes per layout

- **ohl_flat2**: PSCAD 2-conductor horizontal: same height, ±Δx/2. Matches.
- **ohl_flat3**: PSCAD 3H: x={−Δx,0,Δx} at ybc. Matches.
- **ohl_flat3_nogw**: Same 3H without GW elimination. Matches.
- **ohl_flat6**: Two 3H circuits stacked by Δy. Matches a double-circuit horizontal tower.
- **ohl_vert3**: Harmony/PI put the lowest phase at ybc+Δy (i=1,2,3). PSCAD 3V uses ybc as the lowest (i=0,1,2). Shared H=PI, differs from PSCAD.
- **ohl_vert6**: Two vertical circuits at ±Δx/2. Lowest at ybc (not ybc+Δy). Matches PSCAD double-circuit vertical.
- **ohl_delta3**: Harmony/PI raise the right phase, not the centre. PSCAD 3D is isosceles (two at ybc, apex at centre). Shared H=PI, not PSCAD delta.
- **ohl_delta6**: Two leaning triangles. No single PSCAD stock tower; Universal Tower equivalent.
- **ohl_conc3**: Offset vertical: one phase shifted by Δ̃x. Universal Tower, not a named PSCAD type.
- **ohl_off3**: Same coordinates as concentric. Harmony and PI alias the two organizations.
- **ohl_conc6**: Double-circuit offset vertical. Universal Tower.
- **ohl_vert3_twin**: Same vertical-centre offset as ohl_vert3, plus PSCAD-symmetric 2-bundle ring.
- **ohl_2gw**: Same 3V i=1,2,3 offset as ohl_vert3. Used by the unbalanced_yeff 2 GW OHL network (Harmony-only H vs Yeff).

## Cable

- Layers are the P2P coaxial stack (core / sheath / armour).
- Kron reduction of sheath/armour is the default in both codes.
- Equal-depth bipolar: self earth-return terms agree.
- Staggered depths: Harmony uses per-cable $H=2 y_i$; PowerImpedance
  copies the first cable’s self earth return onto every core.
- Semiconductor screens (`a`,`b` ≠ 0) are not in this sweep.

