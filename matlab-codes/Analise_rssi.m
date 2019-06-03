clear all
close all

x = [-88.2262	-88.8989	-89.5308	-90.1265	-90.6901	-91.2247	-91.7333	-92.2181	-92.6815]
tx  = [1       1       1       1       0.968   0.904   0.856   0.776   0.608];

plot(x,100*tx,'-bs','MarkerSize',6, 'MarkerFaceColor','b');
grid on

ylim([60 102])

xlabel('Valor RSSI (dBm)','FontSize',12,'FontName','Times');
ylabel('Taxa de sucesso (%)','FontSize',12,'FontName','Times');
