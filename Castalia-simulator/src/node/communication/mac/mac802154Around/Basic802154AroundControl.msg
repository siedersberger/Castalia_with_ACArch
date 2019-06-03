//****************************************************************************
//*  Copyright (c) Faculty of Engineering, University of Porto, Portugal     *
//*  All rights reserved                                                     *
//*                                                                          *
//*  Module: Mac ARounD Approach for Castalia Simulator                      *
//*  File: Basic802154AroundControl.msg                                      *
//*  Date: 2015-03-05                                                        *
//*  Version:  0.1                                                           *
//*  Author(s): Erico Leão <ericoleao@gmail.com>                             *
//****************************************************************************/


enum AroundControlMessage_type {
    SET_RATE = 0;
    BUFFER_FULL = 1;
    SET_AROUND = 2;
    AUT_AROUND = 3;
    CANCEL_AROUND = 4;
}

struct around{
    int src;
    int dst;
    double pktrate;
    int duration;
}

message AroundControlMessage {
	int AroundControlMessageKind enum (AroundControlMessage_type);
    double rate = 0.0;  //to be used with SET_RATE
    around config;  //configuração de fluxo around, usado com SET_AROUND
    int num;
}
