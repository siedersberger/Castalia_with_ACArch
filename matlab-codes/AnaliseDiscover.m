clear all
close all

x = [200 ;400 ;600 ;800 ;1000 ;1200 ;1400 ;1600 ;1800 ;2000 ;2200 ;2400 ;2600]
tx  = [41.135     ;61.84      ;75.9       ;86.67      ;91.805      ;99.625      ;102.705   	;104.92      ;109.29      ;109.93      ;112.805     ;112.865     ;112.5];

plot(x,tx,'-bs','MarkerSize',6, 'MarkerFaceColor','b');
grid on

ylim([30 120])

xlabel('Número de slots','FontSize',12,'FontName','Times');
ylabel('Número de descobrimentos','FontSize',12,'FontName','Times');