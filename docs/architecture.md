# Harmony Architecture



## Elements
The diagrams below illustrate the class structure and relationships of all Elements in Harmony. All Elements inherit from the `Element` base class.

For readability and ease of navigation, the diagrams are divided into several smaller, interconnected components rather than presenting all classes in a single diagram.

These diagrams are intended to provide an overview of the class hierarchy and the relationships between classes, rather than document their individual implementations or functionality. For detailed information about each class, including its properties and methods, please refer to the [API documentation](https://cresym.github.io/Harmony-Doc/api/index.html).


### Network and Passive Elements

<!-- ***************************************************************** -->
<!-- SWITCH -->
<!-- TRANSMISSION LINE -->
<!-- ***************************************************************** -->

<details>
  <summary>Switch and Transmission Line</summary>
  
```mermaid
---
config:
  theme: redux-color
  themeVariables:
    lineColor: "#64748B"
  class:
    hideEmptyMembersBox: true
---
classDiagram
  namespace Harmony-Elements {
    class Element {}
  }
  namespace Harmony-Elements.Switch {
    class Switch {}
  }
  Element <|-- Switch

  namespace Harmony-Elements.Transmission_Line {
    class Cable {}
    class Overhead_Line {}
    class Transmission_Line {}
  }
  Element <|-- Cable
  Element <|-- Overhead_Line
  Element <|-- Transmission_Line

```

</details>

<!-- ***************************************************************** -->
<!-- IMPEDANCE -->
<!-- ***************************************************************** -->

<details>
  <summary>Impedance</summary>
  
```mermaid
---
config:
  theme: redux-color
  themeVariables:
    lineColor: "#64748B"
  class:
    hideEmptyMembersBox: true
---
classDiagram
  namespace Harmony-Elements {
    class Element {}
  }
  namespace Harmony-Elements {
    class Impedance_base {}
    class Admittance {}
    class Impedance {}
    class Capacitor {}
    class Inductor {}
    class Resistor {}
  }
  Element <|-- Impedance_base
  Element <|-- Capacitor
  Element <|-- Inductor
  Element <|-- Resistor
  Impedance_base <|-- Admittance
  Impedance_base <|-- Impedance
```

</details>

<!-- ***************************************************************** -->
<!-- TRANSFORMERS -->
<!-- ***************************************************************** -->

<details>
  <summary>Transformers</summary>
  
```mermaid
---
config:
  theme: redux-color
  themeVariables:
    lineColor: "#64748B"
  class:
    hideEmptyMembersBox: true
---
classDiagram
  direction LR

  namespace Harmony-Elements {
    class Element {}
  }

namespace Harmony-Elements {
    class Transformer_base
    class Transformer_classic
    class Transformer_real
    class Transformer_Delta_Delta["Transformer_Delta_Delta"]
    class Transformer_Delta_Delta_real["Transformer_Delta_Delta<br/>_real"]
    class Transformer_Delta_Y["Transformer_Delta_Y"]
    class Transformer_Delta_Y_real["Transformer_Delta_Y<br/>_real"]
    class Transformer_Y_Delta["Transformer_Y_Delta"]
    class Transformer_Y_Delta_real["Transformer_Y_Delta<br/>_real"]
    class Transformer_Y_Y["Transformer_Y_Y"]
    class Transformer_Y_Y_real["Transformer_Y_Y_real"]
  }
  Element <|-- Transformer_base
  Transformer_base <|-- Transformer_classic
  Transformer_base <|-- Transformer_real
  Transformer_classic <|-- Transformer_Delta_Delta
  Transformer_classic <|-- Transformer_Delta_Y
  Transformer_classic <|-- Transformer_Y_Delta
  Transformer_classic <|-- Transformer_Y_Y
  Transformer_real <|-- Transformer_Delta_Delta_real
  Transformer_real <|-- Transformer_Delta_Y_real
  Transformer_real <|-- Transformer_Y_Delta_real
  Transformer_real <|-- Transformer_Y_Y_real
```

</details>


### Generators, Loads and Converters

<!-- ***************************************************************** -->
<!-- CONVERTER -->
<!-- ***************************************************************** -->

<details>
  <summary>Converter</summary>
  
```mermaid
---
config:
  theme: redux-color
  themeVariables:
    lineColor: "#64748B"
  class:
    hideEmptyMembersBox: true
---
classDiagram
  namespace Harmony-Elements {
    class Element {}
  }
  namespace Harmony-Elements {
    class Converter {}
    class MMC {}
  }
  Element <|-- Converter
  Converter <|-- MMC 
```

</details>

<!-- ***************************************************************** -->
<!-- LOAD -->
<!-- ***************************************************************** -->

<details>
  <summary>Load</summary>
  
```mermaid
---
config:
  theme: redux-color
  themeVariables:
    lineColor: "#64748B"
  class:
    hideEmptyMembersBox: true
---
classDiagram
  direction LR
  namespace Harmony-Elements {
    class Element {}
  }
  namespace Harmony-Elements {
    class Load_base {}
    class LoadPQ {}
    class Load {}
  }
  Element <|-- Load_base
  Load_base <|-- LoadPQ
  Load_base <|-- Load
```

</details>

<!-- ***************************************************************** -->
<!-- RES -->
<!-- ***************************************************************** -->

<details>
  <summary>RES</summary>
  
```mermaid
---
config:
  theme: redux-color
  themeVariables:
    lineColor: "#64748B"
  class:
    hideEmptyMembersBox: true
---
classDiagram
  namespace Harmony-Elements {
    class Element {}
  }
  namespace Harmony-Elements {
    class PV_plant {}
    class RES_base {}
    class WP_plant {}
    class WT_type_3 {}
    class WT_type_4 {}
  }
  Element <|-- RES_base
  RES_base <|-- PV_plant
  RES_base <|-- WP_plant
  RES_base <|-- WT_type_3
  RES_base <|-- WT_type_4
```

</details>

<!-- ***************************************************************** -->
<!-- SOURCE -->
<!-- ***************************************************************** -->

<details>
  <summary>Source</summary>
  
```mermaid
---
config:
  theme: redux-color
  themeVariables:
    lineColor: "#64748B"
  class:
    hideEmptyMembersBox: true
---
classDiagram
  namespace Harmony-Elements {
    class Element {}
  }
  namespace Harmony-Elements {
    class Source_base {}
    class AC_source {}
    class DC_source {}
    class Generator {}
  }
  Element <|-- Source_base
  Source_base <|-- AC_source
  Source_base <|-- DC_source
  Source_base <|-- Generator
```

</details>

## Solvers

The diagrams below illustrate the class structure of the Solvers in Harmony. Unlike Elements, which share a common Element base class, each Solver is an independent class with its own structure and implementation. Consequently, each Solver is represented separately, without any shared class hierarchy or common structure.

For detailed information about each class, including its properties and methods, please refer to the [API documentation](https://cresym.github.io/Harmony-Doc/api/index.html).

<!-- ***************************************************************** -->
<!-- Certificate -->
<!-- ***************************************************************** -->

<details>
  <summary>Certificate</summary>
  
```mermaid
---
config:
  theme: redux-color
  class:
    hideEmptyMembersBox: true
---
classDiagram
    %% ===== Configuration Specs (Certificate_spec.h) =====
    class CertificateSpec {
        <<struct>>
        +double f_min_Hz
        +double f_max_Hz
        +int n_points
        +bool ac_dq_block
        +double phase_limit_deg
        +double passivity_eps
        +double shift_delta
        +double gain_limit
        +double sector_alpha
        +double sector_beta
        +bool require_passivity
        +bool require_phase
        +bool require_shifted
        +bool require_nr_phase
        +bool require_nr_zero_outside
        +bool require_small_gain
        +bool require_dw
        +bool require_sector
    }

    class OperatingRegionGrid {
        <<struct>>
        +double P_min
        +double P_max
        +double Q_min
        +double Q_max
        +int n_P
        +int n_Q
        +double Vm
        +double Vdc
    }

    class GfmTuningBounds {
        <<struct>>
        +double Kdroop_P_min
        +double Kdroop_P_max
        +double Kdroop_Q_min
        +double Kdroop_Q_max
        +int n_P
        +int n_Q
        +double scale_P_lo
        +double scale_P_hi
        +double scale_Q_lo
        +double scale_Q_hi
    }

    %% ===== Core Sweep & Geometric Engine =====
    class StabilityCertificate {
        <<module>>
        +elementAdmittance(...) MatrixXcd
        +sweepDeviceCertificate(...) CertificateSweep
        +sweepReturnRatioCertificate(...) CertificateSweep
        +finalizeCertificateSweep(...) void
        +sweepPassesSpec(...) bool
        +passivityIndex(...) double
        +shiftedPassivityIndex(...) double
        +maxEigenPhaseDeg(...) double
        +writeCertificateSweepCsv(...) void
        +writeOperatingRegionCsv(...) void
        +plot_certificate_figures(...) void
    }

    class GeometricCertificates {
        <<module>>
        +excessPassivity(...) double
        +shortagePassivity(...) double
        +spectralNorm(...) double
        +numericalRangeBoundary(...) vector
        +certifyNumericalRange(...) NumericalRangeCertificate
        +certifyDwShell(...) DwShellCertificate
        +loopShiftedSector(...) MatrixXcd
        +certifySector(...) bool
    }

    class CertificateSweep {
        <<struct>>
        +vector~double~ freq_Hz
        +vector~double~ passivity_index
        +vector~double~ shifted_passivity
        +vector~double~ max_phase_deg
        +vector~double~ nr_phase_deg
        +vector~double~ nr_zero_margin
        +vector~double~ small_gain
        +vector~double~ dw_margin
        +bool pass_passivity
        +bool pass_phase
        +bool pass_nr_phase
        +bool pass_small_gain
        +bool pass_dw
        +bool pass_sector
        +double worst_passivity
        +double worst_phase_deg
    }

    %% ===== Workflow 1: Device Gate =====
    class DeviceGate {
        <<module>>
        +evaluateDeviceGate(...) DeviceGateResult
        +reportDeviceGate(...) void
    }

    class DeviceGateResult {
        <<struct>>
        +string device_id
        +CertificateSweep sweep
        +bool allow
        +bool pass_passivity
        +bool pass_phase
        +bool pass_dw
        +bool pass_sector
        +string reason
    }

    %% ===== Workflow 2: PnP Library =====
    class PnPCertificateLibrary {
        -vector~PnPDeviceRecord~ records_
        +clear() void
        +add(...) void
        +addFromGate(...) void
        +find(...) PnPDeviceRecord*
        +isAllowed(...) bool
        +saveJson(...) void
        +loadJson(...) void
        +saveCsv(...) void
        +printSummary() void
    }

    class PnPDeviceRecord {
        <<struct>>
        +string device_id
        +string device_type
        +bool allow
        +double worst_passivity
        +double worst_phase_deg
        +double worst_shifted
        +string notes
    }

    %% ===== Workflow 3: Operating Region =====
    class OperatingRegion {
        <<module>>
        +certifyOperatingRegion(...) OperatingRegionResult
        +certifyOperatingPoints(...) OperatingRegionResult
        +reportOperatingRegion(...) void
    }

    class OperatingRegionResult {
        <<struct>>
        +vector~OperatingRegionSample~ samples
        +size_t n_certified
        +double P_opf
        +double Q_opf
        +bool opf_point_certified
    }

    %% ===== Workflow 4: GFM Tuning Assistant =====
    class TuningAssistant {
        <<module>>
        +tuneGfmDroops(...) GfmTuningResult
        +reportGfmTuning(...) void
    }

    class GfmTuningResult {
        <<struct>>
        +bool found
        +double Kdroop_P_best
        +double Kdroop_Q_best
        +CertificateSweep before
        +CertificateSweep after
        +vector~GfmTuningSample~ samples
        +string message
    }

    %% ===== Workflow 5: Local vs System =====
    class LocalVsSystem {
        <<module>>
        +compareLocalVsSystem(...) LocalVsSystemResult
        +reportLocalVsSystem(...) void
    }

    class LocalVsSystemResult {
        <<struct>>
        +CertificateSweep local
        +CertificateSweep system_H
        +bool local_pass
        +bool system_pass
        +bool consistent
        +string message
    }

    %% ===== External Solver / Element Links =====
    class Element
    class MMC
    class StabilityEstimate

    Element <|-- MMC

    %% Core relationships
    StabilityCertificate ..> CertificateSpec : configured by
    StabilityCertificate ..> GeometricCertificates : evaluates metrics
    StabilityCertificate ..> CertificateSweep : produces
    StabilityCertificate ..> Element : queries Y(jw)
    StabilityCertificate ..> StabilityEstimate : queries H = Y Z_eq

    %% Workflow 1
    DeviceGate ..> StabilityCertificate : sweeps device
    DeviceGate ..> DeviceGateResult : returns
    DeviceGateResult *-- CertificateSweep

    %% Workflow 2
    PnPCertificateLibrary ..> DeviceGateResult : ingests
    PnPCertificateLibrary *-- "0..*" PnPDeviceRecord : catalogs

    %% Workflow 3
    OperatingRegion ..> MMC : re-equilibrates (P,Q)
    OperatingRegion ..> OperatingRegionGrid : samples
    OperatingRegion ..> StabilityCertificate : sweeps each point
    OperatingRegion ..> OperatingRegionResult : returns

    %% Workflow 4
    TuningAssistant ..> MMC : tunes droops
    TuningAssistant ..> GfmTuningBounds : searches
    TuningAssistant ..> StabilityCertificate : evaluates before/after
    TuningAssistant ..> GfmTuningResult : returns
    GfmTuningResult *-- CertificateSweep

    %% Workflow 5
    LocalVsSystem ..> StabilityCertificate : compares sweeps
    LocalVsSystem ..> StabilityEstimate : system H benchmark
    LocalVsSystem ..> LocalVsSystemResult : returns
    LocalVsSystemResult *-- CertificateSweep
  
  
```

</details>



<!-- ***************************************************************** -->
<!-- DQSYM -->
<!-- ***************************************************************** -->

<details>
  <summary>DQsym</summary>
  
```mermaid
---
config:
  theme: redux-color
  class:
    hideEmptyMembersBox: true
---
classDiagram
  
  class DQsym {
        +DQsym()
        +initialize(...) void
        +run(...) DQsymResult
        +reset() void
        +exportCSV(...) void
        +plot() void
        +analyzeAtTime(...) void
        +setResult(...) void
        +getResult() DQsymResult&
        +hasRun() bool
        +DSSS(...) MatrixXcd
        +buildMatricesForState(...) void
    }

    class Config {
        <<struct>>
        +double dt
        +double t_start
        +double t_end
        +double f
        +double omega
        +int nKeep
        +VectorXd swOnRes
        +VectorXd swOffRes
        +VectorXi swType
        +std::function breakerFunction
        +std::vector~Bus*~ outputBuses
        +MatrixXcd resumeX
    }

    class DQsymResult {
        <<struct>>
        +std::vector~double~ time
        +MatrixXi brkHistory
        +std::vector~MatrixXd~ DSSabcHist
        +std::map~std::string, MatrixXd~ stateHist
        +std::map~std::string, MatrixXd~ inputHist
        +std::vector~MatrixXcd~ xHist
        +Config cfg
    }

    class DSSState {
        <<struct>>
        +MatrixXcd Ads, Bds, Cds, Dds
        +MatrixXcd A0, B0, C0, D0
        +MatrixXcd x_old
        +VectorXi swVec, swVecOld
        +bool initialized
    }

    DQsym ..> Config : configures
    DQsym *-- DQsymResult : stores
    DQsym *-- DSSState : manages
    DQsymResult *-- Config : retains
```

</details>

<!-- ***************************************************************** -->
<!-- OPF -->
<!-- ***************************************************************** -->

<details>
  <summary>Operational Power Flow (OPF)</summary>
  
```mermaid
---
config:
  theme: redux-color
  class:
    hideEmptyMembersBox: true
---
classDiagram
  
  class PowerFlow {
    +PowerFlow()
    +make_OPF(...) void
    +make_AC_OPF(...) void
    +solve_opf(...) void
  }
```

</details>

<!-- ***************************************************************** -->
<!-- STABILITY ESTIMATE -->
<!-- ***************************************************************** -->

<details>
  <summary>Stability Estimate</summary>
  
```mermaid
---
config:
  theme: redux-color
  class:
    hideEmptyMembersBox: true
---
classDiagram
  
  class StabilityEstimate {
    +StabilityEstimate()
    +add_areas(...) void
    +compute_equivalent_admittance_parameters_num(...) MatrixXcd
    +setParkPerComponent(...) void
    +getParkPerComponent() bool
    +setYeff(...) void
    +getYeff() bool
    +compute_transfer_function(...) MatrixXcd
    +compute_driving_point_impedance(...) MatrixXcd
    +writeFileTF(...) void
    +writeFileZin(...) void
    +bodeplotTF(...) void
    +nyquistplotTF(...) void
    +print_summary() void
    +get_ac_grids() std::unordered_map~std::string, SubNetwork*~&
    +get_dc_grids() std::unordered_map~std::string, SubNetwork*~&
  }
```

</details>

<!-- ***************************************************************** -->
<!-- STATE SPACE MODEL -->
<!-- ***************************************************************** -->

<details>
  <summary>State Space Model</summary>
  
```mermaid
---
config:
  theme: redux-color
  class:
    hideEmptyMembersBox: true
---
classDiagram
 direction LR
  
  class StateSpaceModel {
        +StateSpaceModel()
        +form_cutset_nodes(...) std::vector$
        +form_cutsets(...) std::vector$
        +generateBusToElementsMap(...) std::map$
        +findLoops(...) std::vector$
        +formState(...) void
        +getA() Eigen::MatrixXd&
        +getB() Eigen::MatrixXd&
        +getC() Eigen::MatrixXd&
        +getD() Eigen::MatrixXd&
        +getBRaw() Eigen::MatrixXd&
        +getMode() SSMMode
        +getInputMap() std::vector~InputMapping~&
        +getStateMap() std::vector~StateMapping~&
        +getOutputMap() std::vector~OutputMapping~&
        +getInputGroups() std::vector~InputGroup~&
        +getInputIndex(...) int
        +getStateIndex(...) int
        +buildInputVector(...) MatrixXcd
        +printMapping() void
    }

    class Tree {
        +Tree(...)
        +addPathElement(...) void
        +getRoot() Bus*
        +getElement() Element*
        +getCurrentNode() Bus*
        +getSubelements() std::vector~Element*~&
        +getParent() std::shared_ptr~Tree~
        +containsNode(...) bool
        +getPath() std::vector~Element*~
        +toString() std::string
    }
    StateSpaceModel ..> Tree : uses
```

</details>




