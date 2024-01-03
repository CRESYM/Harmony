clear all;
close all;


Ts = 1e-6;

sps = power_analyze('power_circ2ss_sps','structure')
% [A,B,C,D,x0,states,inputs,outputs,uss,xss,yss,frequencies,Hlin] =...
% power_analyze('sys')
% sps = power_analyze('sys','sort')
% sps = power_analyze('sys','ss')
% power_analyze('sys','net')
% SW = power_analyze('sys','getSwitchStatus')
% sps = power_analyze('sys','setSwitchStatus',SW)