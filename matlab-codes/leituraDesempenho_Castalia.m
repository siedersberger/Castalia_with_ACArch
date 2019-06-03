clear all;
close all;


% Executa operacoes com o deployment (mesmo para todos)

f1 = fopen('dataBase/db_deployment1.txt');
d = textscan(f1,'0               SN.node[%f].MobilityManager               initial location(x:y:z) is %f:%f:0');

tam = 100;
offset = 20;

ID(:,1) = d{1,1}; %ID
x = d{1,2}; %Xcoord
y = d{1,3}; %Ycoord

% Adiciona pontos ao redor do cenário para melhorar a polarizacao
xy = (-offset:tam+offset)';
xy(:,2) = -offset;
x = cat(1,x,xy(:,1));
y = cat(1,y,xy(:,2));
length(x)

xy = (-offset:tam+offset)';
xy(:,2) = tam+offset;
x = cat(1,x,xy(:,1));
y = cat(1,y,xy(:,2));
length(x)

xy = (-offset:tam+offset)';
xy(:,2) = -offset;
x = cat(1,x,xy(:,2));
y = cat(1,y,xy(:,1));
length(x)

xy = (-offset:tam+offset)';
xy(:,2) = tam+offset;
x = cat(1,x,xy(:,2));
y = cat(1,y,xy(:,1));
length(x)
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%


% CONSUMED ENERGY

f2 = fopen('dataBase/dbMST_consumed1.txt');
c = textscan(f2,'%f');
consumed = c{1,1}; %consumedEnergy

% Define os valores dos pontos como o valor min do arquivo
aux = zeros(4*length(xy),1);
aux(:) = min(consumed);
consumed = cat(1,consumed,aux); 

% Executa a polarizacao dos pontos
xlinL = linspace(min(x),max(x),50);
ylinL = linspace(min(y),max(y),50);

[X_CONSUMED,Y_CONSUMED] = meshgrid(xlinL,ylinL);

f = scatteredInterpolant(x,y,consumed);
f.Method = 'natural';
Z_CONSUMED = f(X_CONSUMED,Y_CONSUMED);
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

% DELAY END-TO-END

f2 = fopen('dataBase/dbMST_delay1.txt');
c = textscan(f2,'%f');
delay = c{1,1}; %consumedEnergy

% Define os valores dos pontos como o valor min do arquivo
aux = zeros(4*length(xy),1);
aux(:) = max(delay);
delay = cat(1,delay,aux); 


%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% Executa a polarizacao dos pontos
xlinL = linspace(min(x),max(x),50);
ylinL = linspace(min(y),max(y),50);

[X_DELAY,Y_DELAY] = meshgrid(xlinL,ylinL);

f = scatteredInterpolant(x,y,delay);
f.Method = 'natural';
Z_DELAY = f(X_DELAY,Y_DELAY);
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%





figure

subplot(2,2,[1 3]);
mesh(X_CONSUMED,Y_CONSUMED,Z_CONSUMED,'EdgeColor','k','FaceAlpha',0.8) %interpolated
hold on
p1 = plot3(x,y,consumed,'o','Color','k','MarkerSize',7,'MarkerFaceColor','k'); %nonuniform
hold on;

xlim([0 100]);
ylim([0 100]);
%zlim([70 110]);

xlabel('Eixo X','FontSize',12,'FontName','Times');
ylabel('Eixo Y','FontSize',12,'FontName','Times');
zlabel('Energia consumida (J)','FontSize',12,'FontName','Times');

subplot(2,2,[2 4]);
mesh(X_DELAY,Y_DELAY,Z_DELAY,'EdgeColor','k','FaceAlpha',0.8) %interpolated
hold on
p2 = plot3(x,y,delay,'o','Color','k','MarkerSize',7,'MarkerFaceColor','k'); %nonuniform
hold on;

xlabel('Eixo X','FontSize',12,'FontName','Times');
ylabel('Eixo Y','FontSize',12,'FontName','Times');
zlabel('Atraso fim-a-fim (s)','FontSize',12,'FontName','Times');

xlim([0 100]);
ylim([0 100]);
%zlim([10 35]);
