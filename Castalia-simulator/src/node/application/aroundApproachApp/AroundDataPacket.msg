//***************************************************************************************
//*  Copyright: National ICT Australia, 2007 - 2011                                     *
//*  Developed at the ATP lab, Networked Systems research theme                         *
//*  Author(s): Dimosthenis Pediaditakis, Yuriy Tselishchev                             *
//*  This file is distributed under the terms in the attached LICENSE file.             *
//*  If you do not find this file, copies can be found by writing to:                   *
//*                                                                                     *
//*      NICTA, Locked Bag 9013, Alexandria, NSW 1435, Australia                        *
//*      Attention:  License Inquiry.                                                   *
//*                                                                                     *
//***************************************************************************************

cplusplus {{
#include "ApplicationPacket_m.h"
}}

class ApplicationPacket;

enum AroundDataPacket_type {
    AROUND_DATA_BACKGROUND = 1001;
    AROUND_DATA_SRC2SNK = 1002;
    AROUND_DATA_FLOW = 1003;
}

packet AroundDataPacket extends ApplicationPacket {
    int AroundDataPacketType enum (AroundDataPacket_type);
    int SourceNode;       // Endereço de Origem do pacote
    int DestinationNode;  // Endereço de Destino do Pacote
    double Data;        // Dado do pacote
    int SeqNumber;      // Número de sequencia do pacote
}

