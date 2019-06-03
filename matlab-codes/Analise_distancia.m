clear all
close all

tx = [1 1 1 1 1 1 1 1 1 1 1	1	1	1	1	0.999736	0.999472	0.99868	0.9938411111	0.981268	0.965963	0.930871	0.697097	0.3462333333	0.154106	0.008978	0.0005866667	0.0002933333	0 0 0]
x  = 0:30;

plot(x,100*tx,'-bs','MarkerSize',6, 'MarkerFaceColor','b');
grid on

ylim([-10 110])

xlabel('Distância (metros)','FontSize',12,'FontName','Times');
ylabel('Taxa de sucesso (%)','FontSize',12,'FontName','Times');
