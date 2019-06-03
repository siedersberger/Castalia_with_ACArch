clear all;
close all;

f1 = fopen('leach_rel.txt');
r = textscan(f1,'%f  SN.node[%f].Communication.Routing        Node %f Received TDMA pkt from %f, I am: %fth');

f2 = fopen('db_deployment1PAN.txt');
d = textscan(f2,'0               SN.node[%f].MobilityManager               initial location(x:y:z) is %f:%f:0');

REL(:,1) = r{1,3};
REL(:,2) = r{1,4};

ch = zeros(1);
for i=1:length(REL)
    ch(i,1) = REL(i,2);
    ch(i,2) = 0;
end

REL = cat(1,REL,ch);

    
REL = unique(REL,'rows');

XY(:,1) = d{1,1};
XY(:,2) = d{1,2};
XY(:,3) = d{1,3};


for i=1:length(REL)
    
 
    REL(i,1) = REL(i,1)+1; 
    REL(i,2) = REL(i,2)+1; 

   
end

f1 = figure('Name','Spanning Tree');

p(1) = subplot(2,2,[1 2 3 4]); 

G = graph(REL(:,1),REL(:,2));
p1 = plot(G);
p1.XData = XY(:,2);
p1.YData = XY(:,3);
p1.MarkerSize = 6;
p1.EdgeAlpha = 0.4;
p1.LineWidth = 4;
labelnode(p1, 1:101, 0:100);
highlight(p1,1,'NodeColor','green','MarkerSize',8);
axis(p(1),'tight');
axis equal;