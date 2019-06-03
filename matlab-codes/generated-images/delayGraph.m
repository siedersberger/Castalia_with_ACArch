clear all;
close all;

f1 = fopen('Topology.txt');
d = textscan(f1,'0               SN.node[%f].MobilityManager             initial location(x:y:z) is %f:%f:0');

tam = 50;
offset = 20;

%%%%%%%%%____LEACH_METOD____%%%%%%%%%

f2 = fopen('Delay.txt');
cL = textscan(f2,'%f');

ID(:,1) = d{1,1}; %ID
xL = d{1,2}; %Xcoord
yL = d{1,3}; %Ycoord
rateL = cL{1,1}; %successRate
rateL = rateL;


xyL = (-offset:tam+offset)';
xyL(:,2) = -offset;
xL = cat(1,xL,xyL(:,1));
yL = cat(1,yL,xyL(:,2));
length(xL)

xyL = (-offset:tam+offset)';
xyL(:,2) = tam+offset;
xL = cat(1,xL,xyL(:,1));
yL = cat(1,yL,xyL(:,2));
length(xL)

xyL = (-offset:tam+offset)';
xyL(:,2) = -offset;
xL = cat(1,xL,xyL(:,2));
yL = cat(1,yL,xyL(:,1));
length(xL)

xyL = (-offset:tam+offset)';
xyL(:,2) = tam+offset;
xL = cat(1,xL,xyL(:,2));
yL = cat(1,yL,xyL(:,1));
length(xL)

aux = zeros(4*length(xyL),1);
aux(:) = min(rateL);
rateL = cat(1,rateL,aux);

xlinL = linspace(min(xL),max(xL),50);
ylinL = linspace(min(yL),max(yL),50);

[XL,YL] = meshgrid(xlinL,ylinL);

f = scatteredInterpolant(xL,yL,rateL);
f.Method = 'natural';
ZL = f(XL,YL);

%%%%%%%%%____ACArch_METOD____%%%%%%%%%

figure

mesh(XL,YL,ZL,'EdgeColor','b','FaceAlpha',0.6) %interpolated
hold on
p1 = plot3(xL,yL,rateL,'o','Color','b','MarkerSize',7,'MarkerFaceColor','b'); %nonuniform
hold on;

xlim([0 50]);
ylim([0 50]);
zlim([9 13]);

grid on

xlabel('Coordenada X','FontSize',12);
ylabel('Coordenada Y','FontSize',12);
zlabel('Atraso fim-a-fim (s)','FontSize',12);
legend(p1,'LEACH','FontSize',12)


