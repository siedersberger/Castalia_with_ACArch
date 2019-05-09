//****************************************************************************
//*  Copyright (c) Faculty of Engineering, University of Porto, Portugal     *
//*  All rights reserved                                                     *
//*                                                                          *
//*  Module: Mac ARounD Approach for Castalia Simulator                      *
//*  File: StaticGTS802154Around.ned                                         *
//*  Date: 2015-03-05                                                        *
//*  Version:  0.1                                                           *
//*  Author(s): Erico Leão <ericoleao@gmail.com>                             *
//****************************************************************************/

package node.communication.mac.mac802154Around.staticGTS802154Around;

simple StaticGTS802154Around extends node.communication.mac.mac802154Around.Basic802154Around {
 parameters:
	bool gtsOnly = default (false);
	int requestGTS = default (0);
	@class(StaticGTS802154Around);
}

