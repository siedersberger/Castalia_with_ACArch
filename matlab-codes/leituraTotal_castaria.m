
clear all;
close all;

f1 = fopen('relacoes.txt');
r = textscan(f1,'300             SN.node[%f].Communication.MAC           ENTREI NO FORMATION TIME %d com AssociatedPAN: %d');

f2 = fopen('deployment.txt');
d = textscan(f2,'0               SN.node[%f].MobilityManager               initial location(x:y:z) is %f:%f:0');

REL(:,1) = r{1,2};
REL(:,2) = r{1,3};

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
%labelnode(p1, 1:1, 0:70);
highlight(p1,1,'NodeColor','green','MarkerSize',8);
axis(p(1),'tight');
axis equal;

% p(2) = subplot(2,2,[3 4]); 
% 
% [T,pred] = minspantree(G,'Type','forest','Root',findnode(G,1));
% rootedTree = digraph(pred(pred~=0),find(pred~=0));
% p2 = plot(rootedTree);
% p2.MarkerSize = 6;
% p2.EdgeAlpha = 0.4;
% p2.LineWidth = 4;
% %labelnode(p1, 1:71, 0:70);
% highlight(p2,1,'NodeColor','green');
% axis(p(2),'tight');


% p(2) = subplot(2,2,[2 4]);
% [T,pred] = minspantree(G,'Type','forest','Root',findnode(G,1));
% rootedTree = digraph(pred(pred~=0),find(pred~=0));
% p2 = plot(rootedTree);
% p2.MarkerSize = 6;
% highlight(p2,red,'NodeColor','red');
% highlight(p2,blue,'NodeColor','blue');
% labelnode(p2, CLUSTER(:,2), (CLUSTER(:,2)-1));
