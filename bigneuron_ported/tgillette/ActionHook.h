//
//  ActionHook.h
//  BigNeuron Consensus Builder
//
//  Created by Todd Gillette on 5/28/15.
//  Copyright (c) 2015 Todd Gillette. All rights reserved.
//

#ifndef BigNeuron_Consensus_Builder_ActionHook_h
#define BigNeuron_Consensus_Builder_ActionHook_h

enum HOOK_NAME { LOAD_RECONSTRUCTIONS, LOAD_RECONSTRUCTION, DETECT_CROSSINGS, MATCH_BRANCHES, ADD_BRANCH_TO_COMPOSITE, ADD_DP_TO_COMPOSITE};
enum HOOK_POSITION { BEFORE, AFTER, REPLACE };

class ActionHook{
    HOOK_NAME ah_name;
    HOOK_POSITION ah_position;
    
public:
    ActionHook(HOOK_NAME name, HOOK_POSITION pos);
    HOOK_NAME get_name();
    HOOK_POSITION get_position();
};


#endif
