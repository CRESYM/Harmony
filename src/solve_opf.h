#ifndef SOLVE_PF_H
#define SOLVE_PF_H

#include <string>
#include <map>

void solve_opf(const std::string& dc_name,
    const std::string& ac_name,
    bool  vscControl = true,
    bool  writeTxt = false,
    bool  plotResult = true);

#endif // SOLVE_OPF_H


