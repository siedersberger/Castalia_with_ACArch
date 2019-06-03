//****************************************************************************
//*  Copyright (c) Faculty of Engineering, University of Porto, Portugal     *
//*  All rights reserved                                                     *
//*                                                                          *
//*  Module:   ARounD Approach for Castalia Simulator                        *
//*  File: AroundAppControlMessage.msg                                       *
//*  Date: 2015-03-05                                                        *
//*  Version:  0.1                                                           *
//*  Author(s): Erico Leão <ericoleao@gmail.com>                             *
//****************************************************************************/

// The commands will be received by App from other layers
enum AroundControlCommand_type {
	SET_AROUND_FLOW = 0;
} 

message AroundControlCommand {
	int AroundControlCommandKind enum (AroundControlCommand_type);
	int Source;
    int Destination;
    double StartTime;
    int duration;
}
