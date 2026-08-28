/**
 * @file example_OPF_SLO.cpp
 * @brief SLO9 AC OPF assembled explicitly, one component at a time.
 */
#include "Examples.h"

#include "../network.h"
#include "../Bus.h"
#include "../Include_components.h"
#include "../Solver/OPF/Powerflow.h"

#include <complex>
#include <map>
#include <string>
#include <vector>

void example_OPF_SLO(bool plotting_enabled /*=true*/)
{
    /* ---------- 0. Create network and define base values ---------- */
    Network net;

    const double base_mva = 100.0;
    const double base_kv = 400.0;
    const double frequency = 50.0;
    const double omega = 2.0 * M_PI * frequency;
    const double z_base = base_kv * base_kv / base_mva;

    /* ---------- 1. Create AC buses ---------- */
    Bus* bus1 = new Bus("ACBUS01", "AC1", 3);
    Bus* bus2 = new Bus("ACBUS02", "AC1", 3);
    Bus* bus3 = new Bus("ACBUS03", "AC1", 3);
    Bus* bus4 = new Bus("ACBUS04", "AC1", 3);
    Bus* bus5 = new Bus("ACBUS05", "AC1", 3);
    Bus* bus6 = new Bus("ACBUS06", "AC1", 3);
    Bus* bus7 = new Bus("ACBUS07", "AC1", 3);
    Bus* bus8 = new Bus("ACBUS08", "AC1", 3);
    Bus* bus9 = new Bus("ACBUS09", "AC1", 3);

    /* ---------- 2. Create and connect series R-L-C loads ---------- */
    std::vector<double> load1_parameters = {
        1254.90196078431, 0.998619250772677, 0.0
    };
    Load* load1 = new Load("SLO_LOAD01", "AC1", 3, load1_parameters);
    net.connectElementToBus(load1, 1, bus1);

    std::vector<double> load2_parameters = {
        1158.37104072398, 0.921802385328625, 0.0
    };
    Load* load2 = new Load("SLO_LOAD02", "AC1", 3, load2_parameters);
    net.connectElementToBus(load2, 1, bus2);

    std::vector<double> load3_parameters = {
        1882.35294117647, 1.49792887615902, 0.0
    };
    Load* load3 = new Load("SLO_LOAD03", "AC1", 3, load3_parameters);
    net.connectElementToBus(load3, 1, bus3);

    std::vector<double> load4_parameters = {
        2151.26050420168, 1.71191871561030, 0.0
    };
    Load* load4 = new Load("SLO_LOAD04", "AC1", 3, load4_parameters);
    net.connectElementToBus(load4, 1, bus4);

    std::vector<double> load5_parameters = {
        1075.63025210084, 0.855959357805151, 0.0
    };
    Load* load5 = new Load("SLO_LOAD05", "AC1", 3, load5_parameters);
    net.connectElementToBus(load5, 1, bus5);

    std::vector<double> load6_parameters = {
        1673.20261437908, 1.33149233436357, 0.0
    };
    Load* load6 = new Load("SLO_LOAD06", "AC1", 3, load6_parameters);
    net.connectElementToBus(load6, 1, bus6);

    std::vector<double> load7_parameters = {
        1882.35294117647, 1.49792887615902, 0.0
    };
    Load* load7 = new Load("SLO_LOAD07", "AC1", 3, load7_parameters);
    net.connectElementToBus(load7, 1, bus7);

    std::vector<double> load8_parameters = {
        1003.92156862745, 0.798895400618141, 0.0
    };
    Load* load8 = new Load("SLO_LOAD08", "AC1", 3, load8_parameters);
    net.connectElementToBus(load8, 1, bus8);

    std::vector<double> load9_parameters = {
        2509.80392156863, 1.99723850154535, 0.0
    };
    Load* load9 = new Load("SLO_LOAD09", "AC1", 3, load9_parameters);
    net.connectElementToBus(load9, 1, bus9);

    /* ---------- 3. Create and connect AC generators ---------- */
    const std::vector<double> generator_parameters = {0.02, 0.3, 0.05};

    Generator* gen1 = new Generator("SLO_GEN01", "AC1", 3,
        base_kv * 1e3, generator_parameters);
    std::map<std::string, double> gen1_info = {
        {"Pg", -0.00875302319548246}, {"Qg", 94.5323291594691},
        {"Qmax", 300.0}, {"Qmin", -300.0}, {"mBase", base_mva},
        {"status", 1.0}, {"Pmax", 400.0}, {"Pmin", 0.0},
        {"c2", 0.0080}, {"c1", 14.0}, {"c0", 0.0}, {"Ref", 1.0},
        {"Vg", base_kv * 1.0}
    };
    gen1->setOPFInfo(gen1_info);
    net.connectElementToBus(gen1, 1, bus1);

    Generator* gen2 = new Generator("SLO_GEN02", "AC1", 3,
        base_kv * 1e3, generator_parameters);
    std::map<std::string, double> gen2_info = {
        {"Pg", 9.17708761386777e-06}, {"Qg", 41.9843205074752},
        {"Qmax", 300.0}, {"Qmin", -300.0}, {"mBase", base_mva},
        {"status", 1.0}, {"Pmax", 250.0}, {"Pmin", 0.0},
        {"c2", 0.0100}, {"c1", 16.0}, {"c0", 0.0}
    };
    gen2->setOPFInfo(gen2_info);
    net.connectElementToBus(gen2, 1, bus2);

    Generator* gen3 = new Generator("SLO_GEN03", "AC1", 3,
        base_kv * 1e3, generator_parameters);
    std::map<std::string, double> gen3_info = {
        {"Pg", 3.85238528180846e-06}, {"Qg", 21.7573983887442},
        {"Qmax", 300.0}, {"Qmin", -300.0}, {"mBase", base_mva},
        {"status", 1.0}, {"Pmax", 180.0}, {"Pmin", 0.0},
        {"c2", 0.0120}, {"c1", 20.0}, {"c0", 0.0}
    };
    gen3->setOPFInfo(gen3_info);
    net.connectElementToBus(gen3, 1, bus3);

    Generator* gen4 = new Generator("SLO_GEN04", "AC1", 3,
        base_kv * 1e3, generator_parameters);
    std::map<std::string, double> gen4_info = {
        {"Pg", 5.01819267835460e-10}, {"Qg", 0.00283416140947088},
        {"Qmax", 300.0}, {"Qmin", -300.0}, {"mBase", base_mva},
        {"status", 1.0}, {"Pmax", 160.0}, {"Pmin", 0.0},
        {"c2", 0.0130}, {"c1", 21.0}, {"c0", 0.0}
    };
    gen4->setOPFInfo(gen4_info);
    net.connectElementToBus(gen4, 1, bus4);

    Generator* gen5 = new Generator("SLO_GEN05", "AC1", 3,
        base_kv * 1e3, generator_parameters);
    std::map<std::string, double> gen5_info = {
        {"Pg", 1.06440386429786e-05}, {"Qg", 56.8362119618761},
        {"Qmax", 300.0}, {"Qmin", -300.0}, {"mBase", base_mva},
        {"status", 1.0}, {"Pmax", 300.0}, {"Pmin", 0.0},
        {"c2", 0.0090}, {"c1", 15.0}, {"c0", 0.0}
    };
    gen5->setOPFInfo(gen5_info);
    net.connectElementToBus(gen5, 1, bus5);

    Generator* gen6 = new Generator("SLO_GEN06", "AC1", 3,
        base_kv * 1e3, generator_parameters);
    std::map<std::string, double> gen6_info = {
        {"Pg", 2.25271030007021e-06}, {"Qg", 10.2799604727832},
        {"Qmax", 300.0}, {"Qmin", -300.0}, {"mBase", base_mva},
        {"status", 1.0}, {"Pmax", 180.0}, {"Pmin", 0.0},
        {"c2", 0.0140}, {"c1", 23.0}, {"c0", 0.0}
    };
    gen6->setOPFInfo(gen6_info);
    net.connectElementToBus(gen6, 1, bus6);

    Generator* gen7 = new Generator("SLO_GEN07", "AC1", 3,
        base_kv * 1e3, generator_parameters);
    std::map<std::string, double> gen7_info = {
        {"Pg", 9.26114025128187e-08}, {"Qg", 18.2977780258715},
        {"Qmax", 300.0}, {"Qmin", -300.0}, {"mBase", base_mva},
        {"status", 1.0}, {"Pmax", 180.0}, {"Pmin", 0.0},
        {"c2", 0.0150}, {"c1", 25.0}, {"c0", 0.0}
    };
    gen7->setOPFInfo(gen7_info);
    net.connectElementToBus(gen7, 1, bus7);

    Generator* gen8 = new Generator("SLO_GEN08", "AC1", 3,
        base_kv * 1e3, generator_parameters);
    std::map<std::string, double> gen8_info = {
        {"Pg", 4.84613128247982e-06}, {"Qg", 33.4677491706715},
        {"Qmax", 300.0}, {"Qmin", -300.0}, {"mBase", base_mva},
        {"status", 1.0}, {"Pmax", 350.0}, {"Pmin", 0.0},
        {"c2", 0.0085}, {"c1", 14.5}, {"c0", 0.0}
    };
    gen8->setOPFInfo(gen8_info);
    net.connectElementToBus(gen8, 1, bus8);

    Generator* gen9 = new Generator("SLO_GEN09", "AC1", 3,
        base_kv * 1e3, generator_parameters);
    std::map<std::string, double> gen9_info = {
        {"Pg", 5.97411264127487e-07}, {"Qg", 4.12043624015661},
        {"Qmax", 300.0}, {"Qmin", -300.0}, {"mBase", base_mva},
        {"status", 1.0}, {"Pmax", 120.0}, {"Pmin", 0.0},
        {"c2", 0.0160}, {"c1", 27.0}, {"c0", 0.0}
    };
    gen9->setOPFInfo(gen9_info);
    net.connectElementToBus(gen9, 1, bus9);

    /* ---------- 4. Create and connect AC branches ---------- */
    Impedance* branch1 = new Impedance("SLO_BRANCH01", "AC1", 3,
        std::complex<double>(0.00144730612635612 * z_base,
                             0.0153871476650238 * z_base));
    std::map<std::string, double> branch1_info = {
        {"b", 0.43485078125}, {"rateAC", 692.820323027551},
        {"rateB", 692.820323027551}, {"rateC", 692.820323027551},
        {"ratio", 0.0}, {"angle", 0.0}, {"status", 1.0},
        {"angmin", -30.0}, {"angmax", 30.0}
    };
    branch1->setOPFInfo(branch1_info);
    net.connectElementToBus(branch1, 1, bus1);
    net.connectElementToBus(branch1, 2, bus3);

    Impedance* branch2 = new Impedance("SLO_BRANCH02", "AC1", 3,
        std::complex<double>(0.00134900003671646 * z_base,
                             0.0143331253528595 * z_base));
    std::map<std::string, double> branch2_info = {
        {"b", 0.4025662109375}, {"rateAC", 692.820323027551},
        {"rateB", 692.820323027551}, {"rateC", 692.820323027551},
        {"ratio", 0.0}, {"angle", 0.0}, {"status", 1.0},
        {"angmin", -30.0}, {"angmax", 30.0}
    };
    branch2->setOPFInfo(branch2_info);
    net.connectElementToBus(branch2, 1, bus1);
    net.connectElementToBus(branch2, 2, bus5);

    Impedance* branch3 = new Impedance("SLO_BRANCH03", "AC1", 3,
        std::complex<double>(0.00134900003671646 * z_base,
                             0.0143331253528595 * z_base));
    std::map<std::string, double> branch3_info = {
        {"b", 0.4025662109375}, {"rateAC", 692.820323027551},
        {"rateB", 692.820323027551}, {"rateC", 692.820323027551},
        {"ratio", 0.0}, {"angle", 0.0}, {"status", 1.0},
        {"angmin", -30.0}, {"angmax", 30.0}
    };
    branch3->setOPFInfo(branch3_info);
    net.connectElementToBus(branch3, 1, bus1);
    net.connectElementToBus(branch3, 2, bus5);

    Impedance* branch4 = new Impedance("SLO_BRANCH04", "AC1", 3,
        std::complex<double>(0.000582862496376038 * z_base,
                             0.00617009997367859 * z_base));
    std::map<std::string, double> branch4_info = {
        {"b", 0.182960717773438}, {"rateAC", 692.820323027551},
        {"rateB", 692.820323027551}, {"rateC", 692.820323027551},
        {"ratio", 0.0}, {"angle", 0.0}, {"status", 1.0},
        {"angmin", -30.0}, {"angmax", 30.0}
    };
    branch4->setOPFInfo(branch4_info);
    net.connectElementToBus(branch4, 1, bus7);
    net.connectElementToBus(branch4, 2, bus1);

    Impedance* branch5 = new Impedance("SLO_BRANCH05", "AC1", 3,
        std::complex<double>(0.000582862496376038 * z_base,
                             0.00617009997367859 * z_base));
    std::map<std::string, double> branch5_info = {
        {"b", 0.182960717773438}, {"rateAC", 692.820323027551},
        {"rateB", 692.820323027551}, {"rateC", 692.820323027551},
        {"ratio", 0.0}, {"angle", 0.0}, {"status", 1.0},
        {"angmin", -30.0}, {"angmax", 30.0}
    };
    branch5->setOPFInfo(branch5_info);
    net.connectElementToBus(branch5, 1, bus7);
    net.connectElementToBus(branch5, 2, bus1);

    Impedance* branch6 = new Impedance("SLO_BRANCH06", "AC1", 3,
        std::complex<double>(0.000958162471652031 * z_base,
                             0.010220400094986 * z_base));
    std::map<std::string, double> branch6_info = {
        {"b", 0.283998828125}, {"rateAC", 692.820323027551},
        {"rateB", 692.820323027551}, {"rateC", 692.820323027551},
        {"ratio", 0.0}, {"angle", 0.0}, {"status", 1.0},
        {"angmin", -30.0}, {"angmax", 30.0}
    };
    branch6->setOPFInfo(branch6_info);
    net.connectElementToBus(branch6, 1, bus1);
    net.connectElementToBus(branch6, 2, bus8);

    Impedance* branch7 = new Impedance("SLO_BRANCH07", "AC1", 3,
        std::complex<double>(0.00110200002789497 * z_base,
                             0.0117159998416901 * z_base));
    std::map<std::string, double> branch7_info = {
        {"b", 0.3311017578125}, {"rateAC", 692.820323027551},
        {"rateB", 692.820323027551}, {"rateC", 692.820323027551},
        {"ratio", 0.0}, {"angle", 0.0}, {"status", 1.0},
        {"angmin", -30.0}, {"angmax", 30.0}
    };
    branch7->setOPFInfo(branch7_info);
    net.connectElementToBus(branch7, 1, bus5);
    net.connectElementToBus(branch7, 2, bus2);

    Impedance* branch8 = new Impedance("SLO_BRANCH08", "AC1", 3,
        std::complex<double>(0.000342000015079975 * z_base,
                             0.00363375008106232 * z_base));
    std::map<std::string, double> branch8_info = {
        {"b", 0.102837145996094}, {"rateAC", 692.820323027551},
        {"rateB", 692.820323027551}, {"rateC", 692.820323027551},
        {"ratio", 0.0}, {"angle", 0.0}, {"status", 1.0},
        {"angmin", -30.0}, {"angmax", 30.0}
    };
    branch8->setOPFInfo(branch8_info);
    net.connectElementToBus(branch8, 1, bus6);
    net.connectElementToBus(branch8, 2, bus2);

    Impedance* branch9 = new Impedance("SLO_BRANCH09", "AC1", 3,
        std::complex<double>(0.000342000015079975 * z_base,
                             0.00363599985837936 * z_base));
    std::map<std::string, double> branch9_info = {
        {"b", 0.102755712890625}, {"rateAC", 692.820323027551},
        {"rateB", 692.820323027551}, {"rateC", 692.820323027551},
        {"ratio", 0.0}, {"angle", 0.0}, {"status", 1.0},
        {"angmin", -30.0}, {"angmax", 30.0}
    };
    branch9->setOPFInfo(branch9_info);
    net.connectElementToBus(branch9, 1, bus6);
    net.connectElementToBus(branch9, 2, bus2);

    Impedance* branch10 = new Impedance("SLO_BRANCH10", "AC1", 3,
        std::complex<double>(0.00100699998438358 * z_base,
                             0.0106993758678436 * z_base));
    std::map<std::string, double> branch10_info = {
        {"b", 0.302798266601563}, {"rateAC", 692.820323027551},
        {"rateB", 692.820323027551}, {"rateC", 692.820323027551},
        {"ratio", 0.0}, {"angle", 0.0}, {"status", 1.0},
        {"angmin", -30.0}, {"angmax", 30.0}
    };
    branch10->setOPFInfo(branch10_info);
    net.connectElementToBus(branch10, 1, bus8);
    net.connectElementToBus(branch10, 2, bus2);

    Impedance* branch11 = new Impedance("SLO_BRANCH11", "AC1", 3,
        std::complex<double>(0.000247860010713339 * z_base,
                             0.00264384001493454 * z_base));
    std::map<std::string, double> branch11_info = {
        {"b", 0.0824072387695313}, {"rateAC", 692.820323027551},
        {"rateB", 692.820323027551}, {"rateC", 692.820323027551},
        {"ratio", 0.0}, {"angle", 0.0}, {"status", 1.0},
        {"angmin", -30.0}, {"angmax", 30.0}
    };
    branch11->setOPFInfo(branch11_info);
    net.connectElementToBus(branch11, 1, bus9);
    net.connectElementToBus(branch11, 2, bus8);

    Impedance* branch12 = new Impedance("SLO_BRANCH12", "AC1", 3,
        std::complex<double>(0.0005 * z_base, 0.02 * z_base));
    std::map<std::string, double> branch12_info = {
        {"b", 0.0}, {"rateAC", 600.0}, {"rateB", 600.0}, {"rateC", 600.0},
        {"ratio", 1.0}, {"angle", 0.0}, {"status", 1.0},
        {"angmin", -30.0}, {"angmax", 30.0}
    };
    branch12->setOPFInfo(branch12_info);
    net.connectElementToBus(branch12, 1, bus3);
    net.connectElementToBus(branch12, 2, bus4);

    /* ---------- 5. Configure and run AC OPF ---------- */
    std::map<std::string, double> global_parameters = {
        {"omega", omega},
        {"baseMVA", base_mva},
        {"ACbaseKV", base_kv},
        {"DCbaseKV", 0.0},
        {"ACZbase", z_base},
        {"DCZbase", 0.0}
    };

    PowerFlow pf;
    pf.make_AC_OPF(&net, global_parameters,
        /*writeTxt=*/false,
        /*plotResult=*/plotting_enabled,
        /*print_info=*/false);
}
