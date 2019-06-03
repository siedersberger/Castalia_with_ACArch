clear all;
close all;

num_nodes = 101;



f1 = fopen('dataBase/deployment.txt');
r = textscan(f1,'0               SN.node[%f].MobilityManager               initial location(x:y:z) is %f:%f:0');
coor(:,1) = r{1,2};
coor(:,2) = r{1,3};
coor(:,3) = r{1,1}+1;

d = zeros(num_nodes);
cont = 1;
REL = zeros(1,4);   % nodo i | nodo j | distancia | peso (calculado depois)
dlimit = 20;        % limite usado para relacionar os nodos

%relaciona os nodos baseado em um limite da distancia euclidiana (dlimit)

for i=1:num_nodes
    for j=i+1:num_nodes
   
        d(i,j) = sqrt((coor(i,1) - coor(j,1))^2 + (coor(i,2) - coor(j,2))^2);
        
        if(d(i,j)<dlimit)
           
            REL(cont,1) = i;
            REL(cont,2) = j;
            REL(cont,3) = d(i,j);
            cont = cont + 1;
            
        end
    end
end

neigh = zeros(num_nodes,1);

%faz a contagem de vizinhos de cada nodo com base nas relacoes definidas

for i=1:num_nodes
    for j=1:length(REL(:,1))
           
        if(i == REL(j,1) || i == REL(j,2))
            
            neigh(i) = neigh(i) + 1;
               
        end
    end
end


%calcula o peso das relacoes baseado ma distancia e numero de vizinhos

for i=1:length(REL(:,1))
           
      numero_vizinhos = neigh(REL(i,1)) + neigh(REL(i,2));
      %REL(i,4) = uint8(0.25*REL(i,3) + 100*(1/numero_vizinhos));
      REL(i,4) = uint8(REL(i,3));
end


f1 = figure('Name','Spanning Tree');

p(1) = subplot(2,2,[1 2 3 4]);

%G = graph(REL(:,1),REL(:,2),REL(:,3));
G = graph(REL(:,1),REL(:,2));
%T = minspantree(G,'Type','tree','Method','dense','Root',1);
%T = shortestpathtree(G,'all',1,'Method','positive');
%MST = digraph(pred(pred~=0),find(pred~=0));
p1 = plot(G);
p1.XData = coor(:,1);
p1.YData = coor(:,2);
p1.MarkerSize = 6;
p1.EdgeAlpha = 0;
p1.LineWidth = 4;
labelnode(p1, 1:101, 0:100);
highlight(p1,1,'NodeColor','green');
axis(p(1),'tight');
axis equal;


% p(2) = subplot(2,2,[1 2 3 4]);
% p2 = plot(G);
% p2.XData = coor(:,1);
% p2.YData = coor(:,2);
% p2.MarkerSize = 6;
% p2.EdgeAlpha = 0.4;
% p2.LineWidth = 4;
% %labelnode(p2, 1:61, 0:60);
% highlight(p2,1,'NodeColor','green','MarkerSize',8);
% p2.XData = coor(:,1);
% p2.YData = coor(:,2);
% axis(p(2),'tight');

%PRINTA NA TELA AS POSICOES SETADAS NO ARQUIVO AROUND.INI

cch = zeros(1,2);
cont_cch = 1;
aux = 0;

%docNode = com.mathworks.xml.XMLUtils.createDocument('root');


%arvore = MST;       %spanning tree
%arvore = SP;       %shortest path

% gera uma lista de CHs com seus respectivos CCHs
% for i=1:height(arvore.Edges)
%         
%     for j=1:height(arvore.Edges)
% 
%         if(arvore.Edges(i,1).EndNodes(2) == arvore.Edges(j,1).EndNodes(1) && aux ~= arvore.Edges(i,1).EndNodes(2))
%             cch(cont_cch,1) = arvore.Edges(i,1).EndNodes(1);
%             cch(cont_cch,2) = arvore.Edges(i,1).EndNodes(2);
%             cont_cch = cont_cch + 1;
%             aux = arvore.Edges(i,1).EndNodes(2);
%         end
%             
%     end
%         
% end
% 
% for i=1:height(arvore.Edges)
%    fprintf('<node%d>%d</node%d>\n',arvore.Edges(i,1).EndNodes(1)-1,arvore.Edges(i,1).EndNodes(2)-1,arvore.Edges(i,1).EndNodes(1)-1);
% end
% 
% for i=1:length(cch)
%     fprintf('<cch%d>%d</cch%d>\n',cch(i,1)-1,cch(i,2)-1,cch(i,1)-1);
% end

