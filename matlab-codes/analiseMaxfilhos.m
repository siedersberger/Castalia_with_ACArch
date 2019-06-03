clear all
close all

x = [30 ;40 ;50 ;60 ;70 ;80 ;90]
tx  = [0.99935   ;0.9987    ;0.99785   ;0.9966    ;0.9935    ;0.98325   ;0.97355];

plot(x,100*tx,'-bs','MarkerSize',6, 'MarkerFaceColor','b');
grid on

ylim([95 100])

xlabel('Número máximo de filhos por cluster','FontSize',12,'FontName','Times');
ylabel('Taxa de sucesso (%)','FontSize',12,'FontName','Times');