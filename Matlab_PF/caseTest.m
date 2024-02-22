clc; clear all; close all; 

%% single AC grid and a MTDC and case is "case5_stagg" from
% MatACDC_UserManual https://www.esat.kuleuven.be/electa/teaching/matacdc/MatACDCManual
fprintf('5-Bus-AC connected with 3-terimnal-DC')
acmtdcpf ('case5_stagg.m','case5_stagg_MTDCdroop.m')
 
%% two AC grids and a MTDC, case is set by self
fprintf('\n 5-Bus-AC and 9-Bus-AC interconnected with 3-terimnal-DC')
acmtdcpf ('ac5ac9.m','mtdc3slack_a.m')
  
%% two AC grids and a MTDC, case is set by self
fprintf('\n Test 5-Bus-AC and 9-Bus-AC connected with 3-terimnal-DC')
acmtdcpf ('ac5ac9.m','mtdc3droop.m')

%% three AC grids and a MTDC, case is set by self
fprintf('\n 5-Bus-AC, 9-Bus-AC, 14-Bus-AC interconnected with 3-terimnal-DC')
acmtdcpf ('ac5ac9ac14.m','mtdc3slack_b.m')

%% three AC grids and a MTDC, case is set by self
fprintf('\n 5-Bus-AC, 9-Bus-AC, 14-Bus-AC interconnected with 4-terimnal-DC')
acmtdcpf ('ac5ac9ac14.m','mtdc4slack.m')