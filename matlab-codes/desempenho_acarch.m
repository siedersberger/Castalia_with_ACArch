clear all
close all

txA = [94.10	99.38	99.61	99.47];

txS = [93.46	98.54	98.44	98.65];

txL = [70.97	71.48	73.42	72.28];

errorA = [4.93	0.88	0.13	0.16];

errorS = [4.91	1.25	0.87	0.74];

errorL = [5.80	1.91	4.82	2.90];

nNodes = [50 100 150 200];

p3 = errorbar(nNodes,txA,errorA,'-o','Color',[0.8 0 0],'MarkerSize',18,'LineWidth',1.5);
hold on
p4 = errorbar(nNodes,txS,errorS,'-.k^','MarkerSize',18,'MarkerFaceColor','k','LineWidth',1.5);
hold on
p5 = errorbar(nNodes,txL,errorL,'-x','Color',[0.3 0.3 0.8],'MarkerSize',20,'LineWidth',1.5);
hold off


ylim([64 101]);
xlim([45 205]);

grid on

title('Cenário de 100x100 m²');
xlabel('Número de nodos','FontSize',16);
ylabel('Taxa de sucesso (%)','FontSize',16);
%legend([p3 p4 p5],{'ACArch','SPH','BI'},'FontSize',16)
legend([p3 p4 p5],{'ACArch','SPH','LEACH'},'FontSize',16)