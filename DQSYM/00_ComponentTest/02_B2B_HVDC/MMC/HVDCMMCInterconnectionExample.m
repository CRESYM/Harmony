%% HVDC-MMC Interconnection (1000-MW, +/- 320 kV)
%
% This example shows a model of a High Voltage Direct Current (HVDC) interconnection using Voltage-Sourced Converters (VSC) based on the Modular Multi- level Converter (MMC) technology. The simulation is optimized by the use of an aggregate MMC model.  
%

% Copyright 2022 Hydro-Quebec, and The MathWorks, Inc.

%%

open_system('HVDCMMCInterconnection')

%% Description 
%
% DC links are becoming the preferred means for power exchange between countries and transmission of power from renewables (hydroelectric plants, off-shore wind power and solar farms) to power grids. Several of these systems are presently in operation, for example the INELFE project (France-Spain interconnection), the Dolwin1 project (offshore wind HVDC link), or planned for future projects such as the Northern Pass (Canada-USA 1090-MW DC link).
%
% The MMC converter is implemented using an aggregate model to simulate the
% power modules of one arm. With this aggregate model, control system dynamics, converter harmonics and circulating currents phenomena are all well-represented. However, having only one virtual capacitor to represent all capacitors of one arm, the model assumes that capacitor voltages of all power modules are well-balanced, and therefore capacitor voltage-balancing scheme cannot be simulated. The aggregate model runs much faster than a detailed model that would use two switching devices and one capacitor for each individual power module of one arm. This aggregate model is also well-suited for real-time simulation.
%

%% Main Components
%
%%
% 
% # The electrical grid is modeled using a 400-kV, 50-Hz equivalent and two breakers to energize Converter 1.
% # Converter 1 is implemented using 6 half-bridge MMC blocks, each representing 36 power modules. This custom SPS block uses a switching-function model where only one equivalent module is used to represent all power modules. The control signal is a two-element vector [ Nin, Nbl ], where Nin represents the number of inserted modules and Nbl represents the number of modules in the blocked-state. The output Vc (capacitor voltage) has only one element and gives the average value of the capacitor modules.
% # Inside the DC Circuit subsystem, you will find a simplified model of the cable, as well as the second converter modeled using DC sources and ideal switches. You will also find a switch to apply a fault on the cable.
% # The Controllers subsystem contains the various control systems required
% to operate the interconnection. It includes the Active & Reactive Power
% Regulators, the DC Voltage Regulator, a dq current regulator with
% feed-forward, PLL & measurement subsystem, PWM Generators to control
% Converter1 half-bridge MMC's. You will also find the Sequencer Area where the various modes of operation of Converter1 are programmed.
% # Scopes & Measurements contains all the scopes used to observe several signals during the simulation. Power and fundamental voltage calculations are also performed in the subsystem
% 

%% Simulation
%
% Simulating our SPS model for 10 seconds allows observation of the operation of the interconnection during start-up (capacitor charging), voltage regulation, and power regulation. All parameters required to run the model can be found in the following file: HVDC_MMC_param.m. This file is automatically executed in the MATLAB workspace when the example is open. Run the model and observe the following events:
%
%%
% 
% # At 0.1s, Breaker 1 is closed and Converter 1 is energized through a resistor to reduce the charging current. Capacitors are being charged and at 1s, the start-up resistor is short- circuited by closing Breaker 2.
% # At 1.5s, Converter 1 is deblocked and the Voltage Regulator is enabled.
% # At 2s, the Voltage Regulator setpoint is ramped to the nominal DC operating voltage of the interconnection: 640 kV (+/-320 kV).
% # At 4s, PQ regulators are enabled and Converter 2 switches are closed.
% # At 4.2s, the Active Power Regulator setpoint is ramped to 1pu (1000 MW).
% # At 7.5s, the Reactive Power Regulator setpoint is ramped to 0.25pu (250 Mvar).
%
% If you set the value of parameter Tfault to 7 (actual default value=9999) in the workspace and re-start the simulation, a DC fault will be applied at the middle of the cable at 7s. The half-bridge MMC will be blocked and the interconnection will be shut down after two cycles (Brk1 will be opened).
%

%% References
%%
% 
% # VSC-HVDC Transmission with Cascaded Two-Level Converters, Bjorn Jacobson, Patrik Karlsson, Gunnar Asplund, Lennart Harnefors, Tomas Jonsson ABB,Sweden, CIGRE 2010 B4-110
% # Setup and Performance of the Real-Time Simulator used for Hardware-in-Loop-Tests of a VSC-Based HVDC scheme for Offshore Applications., O. Venjakob, S. Kubera, R. Hibberts-Caswell, P.A. Forsyth, T.L. Maguire Siemens, Germany & RTDS Technologies, Canada. Paper submitted to the International Conference on Power Systems Transients (IPST2013) in Vancouver, Canada July 18-20, 2013
% 

%%

