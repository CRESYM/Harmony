function [CS] = CaseStudyData()

CS = struct();

CS.Fnom     = 50;
CS.wn       = 2*pi*CS.Fnom;

%%%%%%%%%%%%%% PER UNIT %%%%%%%%%%%%%%%%%
CS.Sb       = 1000*1e6;
CS.Vnom     = 380e3;
CS.Vb       = CS.Vnom*sqrt(2/3);
CS.Ib       = (2/3)*CS.Sb/CS.Vb;
CS.Zb       = CS.Vb/CS.Ib;
CS.Cb       = 1/(CS.Zb*CS.wn);
CS.Lb       = CS.Zb/CS.wn;

CS.Vb_dc    = CS.Vb*2*sqrt(3/2);
CS.Ib_dc    = CS.Sb/CS.Vb_dc;
CS.Zb_dc    = CS.Vb_dc/CS.Ib_dc;
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

end

