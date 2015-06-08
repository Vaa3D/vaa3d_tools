//
//  ActionHook.cpp
//  BigNeuron Consensus Builder
//
//  Created by Todd Gillette on 5/28/15.
//  Copyright (c) 2015 Todd Gillette. All rights reserved.
//

#include "ActionHook.h"

ActionHook::ActionHook(HOOK_NAME name, HOOK_POSITION position){
    ah_name = name;
};

HOOK_NAME ActionHook::get_name(){
    return ah_name;
};
HOOK_POSITION ActionHook::get_position(){
    return ah_position;
};
