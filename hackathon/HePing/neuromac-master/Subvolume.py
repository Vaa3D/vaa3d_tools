import zmq
import sys
import time
import copy
import sqlite3
import pickle as pickle
from configparser import SafeConfigParser # For use with Python 2.7
import numpy as np

from front import Front
from segment_distance import dist3D_segment_to_segment

#from memory_profiler import profile  --------
# -*- coding: UTF-8 -*-
import inspect
def _me(bool) :
    if(bool) :
        print('%s \t Call -> ::%s' % (inspect.stack()[1][1], inspect.stack()[1][3]))
    else :
        pass

def print_with_rank(num,message) :
    print('%s \t%i \t%s' % (inspect.stack()[1][1], num, message))


"""
auxilliary functions for the set operations needed for the \
dictionary merging using sets
"""    
def code_tuple(p):
    return (tuple(p[0]),p[1])

def decode_tuple(p):
    return (np.array(p[0]),p[1]) 
        
class Subvolume_Agent(object) :  #
    def __init__(self,num,cfg_file="test_config.cfg") :  #
        self.num=num
        self.active_fronts = []
        print_with_rank(num, "Online!")

        # static: boundaries
        self.static_constellation = {}
        
        self.virtual_constellation ={}
        self.neighbor_constellation ={}
        self.distal_constellation = {}
        self.dynamic_constellation ={}
        
        # substance definitions: both static and dynamic (for now, only dynamic supported)
        self.substances_constellation={}
        
        self.parser = SafeConfigParser()
        self.parser.read(cfg_file)
        if self.parser.has_option("system","recursion_limit"):
            sys.setrecursionlimit(self.parser.getint("system","recursion_limit"))

        """ check the minimum distance between fronts of each entity_type.
            Used to test the validity of front. If not set, the default
            of current_diameter / 2 is used. Otherwise as specified.
            Setting this value in the cfg-file comes handy when using
            a very small increment to achieve "detailed" avoidance/attraction.

            Check once when starting the Subvolume agent.
        """
        allowed_self_dist = {}
        for section in self.parser.sections():
            if section.startswith("cell_type_"):
                if self.parser.has_option(section, "minimum_self_distance"):
                    allowed_self_dist[section] = self.parser.getfloat(\
                                                      section,\
                                                      "minimum_self_distance")
                    print("allowed_s_d[%s]=%f" % (section, allowed_self_dist[section]))
        self.allowed_self_dist = allowed_self_dist
                
        if self.parser.has_option("system","recursion_limit"):
            sys.setrecursionlimit(self.parser.getint("system","recursion_limit"))                    

        # ready. set. go!
        self._initialize_communication_links()
        self.main_loop()

    def _initialize_communication_links(self) :
        self.context = zmq.Context()

        # to communicate through the proxy
        self.psub = self.context.socket(zmq.SUB)
        self.psub.connect("tcp://localhost:%s" % self.parser.getint("system","proxy_pub_port") )
        self.psub.setsockopt(zmq.SUBSCRIBE, "All")
        self.psub.setsockopt(zmq.SUBSCRIBE, "%06d"%self.num)
        self.ppub = self.context.socket(zmq.PUB)
        self.ppub.connect("tcp://localhost:%s" % self.parser.getint("system","proxy_sub_port") )

        # register/sync with Admin
        self.socket_push = self.context.socket(zmq.PUSH)
        self.socket_push.connect("tcp://127.0.0.1:%s" % self.parser.getint("system","pull_port"))
        self.socket_push.send("SV_%06d online"%self.num)

    def main_loop(self) :
        running = True
        while running :
            #[address,message] = self.socket_sub.recv_multipart()
            #print_with_rank(self.num, "Waiting for a message to arrive")
            [address,message] = self.psub.recv_multipart()
            message = pickle.loads(message)
            #print_with_rank(self.num,"addr: "+str(address)+", message >"+str(message)+"<")
            if message == "Done" :
                running = False
                print_with_rank(self.num,"going home")
            elif message == "Update" :
                print_with_rank(self.num," update")
            elif message[0] == "Init_SV" :
                #print_with_rank(self.num,"Init_SV received")
                self._process_init_sv(message)
            elif message[0] == "Initialize_GEs" :
                self._process_initialize_ges(message)
            elif message[0] == "Update" :
                self._process_update(message)
            elif message[0] == "Reply_constellation" :
                self._process_reply_constellation(message)
            elif message[0] == "Request_constellation" :
                self._process_request_constellation(message)                
            elif message[0] == "Add_Front" :
                self._add_front(message)

    def _process_update(self,message) :
        print_with_rank(self.num," received Update from admin (cycle: %s)" % message[1])
        
        self.update_cycle = int(message[1])
        all_summarized_constellations = message[2]

        # clear the distal_constellation & neighbor_constellation from the previous update cycle
        self.distal_constellation = {}
        self.neighbor_constellation = {}

        try:
            total_l = sum([len(v) for k,v in all_summarized_constellations[1].iteritems()])
            no_keys = len(all_summarized_constellations[1].keys())
            
            print_with_rank(self.num, "L(all_summarized_constellations): %i, no_keys: %i" % \
                           (total_l,no_keys))
            # print_with_rank(self.num, "all_sum_con[1].keys: " + str(all_summarized_constellations[1].keys()))
            # print_with_rank(self.num, "all_sum_con[2].keys: " + str(all_summarized_constellations[2].keys()))
        except Exception:
            pass
        
        # 2014-08-06
        """
        Special MERGE needed for distal:
        Onlt use those keys that are NOT in DYNAMIC. (because this information is close)
        Or, make the summary so to only include dynamic, and not distal
        """
        for num in all_summarized_constellations :
            if not num == self.num :
                if not num in self.neighbors :
                    self.distal_constellation = \
                        self._merge_constellations(self.distal_constellation,\
                                                   all_summarized_constellations[num])
                    #print "all_s_constel: ", all_summariz_constellations[num]

        # print "distal_constel: ", self.distal_constellation
                                                   
        # deal with the special case of having only one SV
        if len(self.neighbors) == 0 :
            self._perform_update()
        else:
            self._ask_neighboring_constellations()
            self._constellation_responses = 0
                
    def _ask_neighboring_constellations(self):
        for dest in self.neighbors :
            message = ("Request_constellation",self.num)
            self.ppub.send_multipart(["%06d"%dest,pickle.dumps(message)])

    def _process_request_constellation(self,message):
        ret_dest = message[1]
        # total_l = sum([len(v) for k,v in self.expanded_constellation.iteritems()])
        # no_keys = len(self.expanded_constellation.keys())
        # print_with_rank(self.num, "L(expanded constellation): %i, no_keys: %i" % \
        #                (total_l,no_keys))

        """
        # 2014-08-06
        maybe I should also send the static constellation???
        NO!!! The static environment is known to all and needs not to be send anymore.
        every SV knows the complete substrate; this might cause issues in the future when the substrate can become
        very large. in principle: if there is nothing in the neighborhood, nothing should be know.
        but because of the "in the eye of the beholder" interaction principle, it is up to the
        developing front to decide what is "near" and what is "distal"
        """
        ret_message = ("Reply_constellation",self.dynamic_constellation)
        self.ppub.send_multipart(["%06d"%ret_dest,pickle.dumps(ret_message)])

    def _process_reply_constellation(self,message):
        # merge received constellation in an expanded one
        temp_con = message[1]

        # try:
        #     total_l = sum([len(v) for k,v in temp_con.iteritems()])
        #     no_keys = len(temp_con.keys())
        #     print_with_rank(self.num, "L(temp_con): %i, no_keys: %i" % \
        #                    (total_l,no_keys))
        #     print_with_rank(self.num, "temp_con.keys: " + str(temp_con.keys()))
        #     print_with_rank(self.num, "temp_con.keys: " + str(temp_con.keys()))
        # except Exception:
        #     pass

        # 2014-08-06       
        self.neighbor_constellation = self._merge_constellations(self.neighbor_constellation,temp_con)
        #print "received temp_con: ", temp_con

        self._constellation_responses = self._constellation_responses +1

        # if this is the last answer, self._perform_update()
        if self._constellation_responses == len(self.neighbors) :
            self._perform_update()

    def _merge_constellations(self,d1,d2):
        for key in d2:
           if key in d1:
               d1[key] = d1[key].union(d2[key]) # sets only allow unique entries. Front-objects are now hashable
           else:
               d1[key] = d2[key]
        return d1
                
    def _process_init_sv(self,message) :
        # initialize the seed, make the seed unqiue by adding the SV ID
        np.random.seed(self.parser.getint("system","seed")+self.num)
        
        #print_with_rank(self.num,"processing init message: " + str(message) )
        print_with_rank(self.num, "got virtual w/ keys: " + str(message[4].keys()))
        #time.sleep(2)
        boundary = message[1]
        neighbors = message[2]
        static_constellation = message[3]
        virtual_constellation = message[4]
        self.boundary = boundary
        self.neighbors = neighbors

        # 2014-08-06
        self.static_constellation =static_constellation
        self.virtual_constellation=virtual_constellation

        print_with_rank(self.num,"INIT_SV, static: "+str(static_constellation.keys()))
        print_with_rank(self.num,"INIT_SV, static: "+str(self.static_constellation.keys()))
        
    def _process_initialize_ges(self,message):
        print_with_rank(self.num,"message: " + str(message))
        entries = message[1]
        for details in entries :
            algo_name = details["algorithm"]
            entity_name = details["entity_name"]
            radius = details["radius"]
            soma_xyz = details["soma_xyz"]
            new_front = Front(entity_name,algo_name,soma_xyz,radius,\
                              0,0) # 0: pathlength, 0: order
            new_front.soma_pos = soma_xyz

            # self.fronts.append(new_front)

            # # 2014-02-19: in principle the "all_contained" can be deleted once
            # # I include the diameters in all constellations
            # self.all_contained_entities[entity_name] = []
            # self.all_contained_entities[entity_name].append(new_front)

            # 2014-08-06
            if entity_name in self.dynamic_constellation :
                print("Help! I am overwriting something")
                time.sleep(10)
            self.dynamic_constellation[entity_name] = set()
            self.dynamic_constellation[entity_name].add(new_front)#append((soma_xyz,radius))
            self.active_fronts.append(new_front)

    def _get_pos_only_constellation(self,c):
        new_c = {}
        for key in c:
            pos_only = [f.xyz for f in c[key]]
            new_c[key] = pos_only
        return new_c    

    def _perform_update(self,debug_mem=False) :
        # real deal
        #merged_constellation = copy.deepcopy(self.static_constellation)
        merged_constellation = copy.copy(self.static_constellation)
        merged_constellation = self._merge_constellations(merged_constellation,self.dynamic_constellation)
        merged_constellation = self._merge_constellations(merged_constellation,self.neighbor_constellation)
        merged_constellation = self._merge_constellations(merged_constellation,self.distal_constellation)
        merged_constellation = self._merge_constellations(merged_constellation,self.substances_constellation)
        
        pos_only_constellation = self._get_pos_only_constellation(merged_constellation)

        new_fronts = []
        # randomize order
        np.random.shuffle(self.active_fronts)
        changes = []
        all_synapse_locs = []
        for i,front in zip(range(len(self.active_fronts)),self.active_fronts) :
            print_with_rank(self.num,"i= "+ str(i)+ ": "+str(front))

            c_seed = 0#np.random.randint(0,1000) #+self.num*(self.update_cycle + i)
            
            front.update_cycle = self.update_cycle
            ret = front.extend_front(c_seed,pos_only_constellation,self.virtual_constellation)
            if isinstance(ret,tuple) :
                # front is trying to update the environment
                # store the update information, likely in my_constellation?
                update_info = ret[1]
                # print "update_info: ", update_info
                entity_name = update_info.keys()[0]
                entity_front = update_info[entity_name] # 2014-08-11
                
                # # 2014-02-19
                # 2014-08-11
                # store and...
                # ... update the current pos_only_constellation
                if entity_name in self.substances_constellation :
                    self.substances_constellation[entity_name].append(entity_front)
                    pos_only_constellation[entity_name].add(entity_front.xyz)
                else :
                    self.substances_constellation[entity_name] = set()
                    self.substances_constellation[entity_name].add(entity_front)
                    pos_only_constellation[entity_name] = []
                    pos_only_constellation[entity_name].append(entity_front.xyz)

                # set the ret for subsequent processing (without updating my code)
                ret = ret[0]
                #print_with_rank(self.num, "SECRETION NOT YET IMPLEMENTED")
                #time.sleep(100)
                                
            elif isinstance(ret,list) or ret == None:
                pass # front is only extending
            else :
                print("ret: ", ret)
                print("type: ", type(ret))
                print("extend_front must return either list \
                (for continuation, branch or termination or tuple (list and dict)")
                sys.exit(0)
            if ret == None :
                # that's the end of this front
                pass
            else :
                for f in ret :
                    if self._within_this_volume(f.xyz) :
                        """TODO: PERFORM CHECK: can this front be added at this location
                        If yes: ok
                        If not: wiggle a few times and check each time, if too difficut: discard front
                        """
                        valid,syn_locs = self._valid_and_wiggle(f)
                        
                        if valid:
                            new_fronts.append(f)
                            all_synapse_locs.extend(syn_locs)
                            # self.my_constellation[f.entity_name].append(f.xyz)

                            self.dynamic_constellation[f.entity_name].add(f)
                            pos_only_constellation[f.entity_name].append(f.xyz)
                        else:
                            print("NOT VALID TO ADD THIS POINT")
                            pass
                    else :
                        # print_with_rank(self.num,"front(%s) not in this SV (%s)" % (str(f.xyz),str(self.boundary)))
                        # make message and send to admin, admin then distributes to correct subvolume
                        message=("Migrate_Front",f,front) # send new front and parent
                        self.ppub.send_multipart(["Admin",pickle.dumps(message)])
                # self._temp_to_db(front,ret)
                changes.append((front,ret))
        self.active_fronts = new_fronts
        
        #self.socket_push.send(pickle.dumps(("Update_OK",self.my_constellation)))

        """should I only send a summary of dynamic and substances?
        distal and neighbors are not interesting to send and static is already known from the start to all others
        so yes,
        """
        core_constellation = {}
        core_constellation = self._merge_constellations(core_constellation,self.dynamic_constellation)
        core_constellation = self._merge_constellations(core_constellation,self.substances_constellation)
        core_pos_only_constellation = self._get_pos_only_constellation(core_constellation)

        # 2014-08-12
        # my_summarized_constellation = self._summarize_constellation(pos_only_constellation)
        my_summarized_constellation = self._summarize_constellation(core_pos_only_constellation)
        
        msg = ("Update_OK","%06d"%self.num,changes,all_synapse_locs,my_summarized_constellation)
        self.ppub.send_multipart(["Admin",pickle.dumps(msg)])

        if debug_mem:
            self._gather_constellation_size(merged_constellation)

    def _valid_and_wiggle(self,f):
        valid,syn_locs = self._is_front_valid(f,check_synapses=self.parser.has_option("system","syn_db"))
        attempts = 0
        avoidance_attempts = 0
        if self.parser.has_option("system","avoidance_attempts"):
            avoidance_attempts = self.parser.getint("system","avoidance_attempts")

        while valid == False and attempts < avoidance_attempts:
            # wiggle front.xyz a bit...
            noise = (2*f.radius)*np.random.random(len(f.xyz))-f.radius
            f.xyz = f.xyz+ noise
            valid,syn_locs = self._is_front_valid(f,check_synapses=self.parser.has_option("system","syn_db"))
            attempts = attempts + 1
        return valid,syn_locs

    def _gather_constellation_size(self,merged_constellation):
        """
        # real deal
        merged_constellation = self.static_constellation
        merged_constellation = self._merge_constellations(merged_constellation,self.dynamic_constellation)
        merged_constellation = self._merge_constellations(merged_constellation,self.neighbor_constellation)
        merged_constellation = self._merge_constellations(merged_constellation,self.distal_constellation)
        merged_constellation = self._merge_constellations(merged_constellation,self.substances_constellation)
        
        pos_only_constellation = self._get_pos_only_constellation(merged_constellation)        
        """
        try:
            mc_total_l = sum([len(v) for k,v in merged_constellation.iteritems()])
            mc_no_keys = len(merged_constellation.keys())

            stc_total_l = sum([len(v) for k,v in self.static_constellation.iteritems()])
            stc_no_keys = len(self.static_constellation.keys())

            dyc_total_l = sum([len(v) for k,v in self.dynamic_constellation.iteritems()])
            dyc_no_keys = len(self.dynamic_constellation.keys())

            nc_total_l = sum([len(v) for k,v in self.neighbor_constellation.iteritems()])
            nc_no_keys = len(self.neighbor_constellation.keys())
                                                                  
            dc_total_l = sum([len(v) for k,v in self.distal_constellation.iteritems()])
            dc_no_keys = len(self.distal_constellation.keys())

            sc_total_l = sum([len(v) for k,v in self.substances_constellation.iteritems()])
            sc_no_keys = len(self.substances_constellation.keys())

            ss = "\nDEBUG: STC K=: %i, L: %i \n" % (stc_no_keys,stc_total_l)
            ss += "DEBUG: DYC K=: %i, L: %i \n" % (dyc_no_keys,dyc_total_l)
            ss += "DEBUG: NC K=: %i, L: %i \n" %  (nc_no_keys,nc_total_l)
            ss += "DEBUG: DC K=: %i, L: %i \n" % (dc_no_keys,dc_total_l)
            ss += "DEBUG: SUC K=: %i, L: %i \n" % (sc_no_keys,sc_total_l)
            ss += "DEBUG: TOTAL K=: %i, L: %i \n" % (mc_no_keys,mc_total_l)
            print_with_rank(self.num,ss)
            # print_with_rank(self.num, "DEBUG: STC K=: %i, L: %i" % \
            #                (stc_no_keys,stc_total_l))
            # print_with_rank(self.num, "DEBUG: DYC K=: %i, L: %i" % \
            #                (dyc_no_keys,dyc_total_l))
            # print_with_rank(self.num, "DEBUG: NC K=: %i, L: %i" % \
            #                (nc_no_keys,nc_total_l))
            # print_with_rank(self.num, "DEBUG: DC K=: %i, L: %i" % \
            #                (dc_no_keys,dc_total_l))
            # print_with_rank(self.num, "DEBUG: SC K=: %i, L: %i" % \
            #                (sc_no_keys,sc_total_l))

            # print_with_rank(self.num, "DEBUG: TOTAL MC K=: %i, L: %i" % \
            #                (mc_no_keys,mc_total_l))
            print_with_rank(self.num,"static keys: "+ str(self.static_constellation.keys()))
        except Exception as e:
            print("CANNOT DETERMINE CONSTELLATION SIZE")
            print(e)
            time.sleep(20)
         
    def _is_front_valid(self,front,check_synapses=False):
        ret = True
        syn_locs = []
        # check against all developing processes
        for entity_name in self.dynamic_constellation:
            #print "checking validity of entity_name:", entity_name
            if not entity_name == front.entity_name:
                for o_front in self.dynamic_constellation[entity_name]:
                    if front.parent == None or o_front.parent == None:
                        D = np.sqrt(np.sum((front.xyz-o_front.xyz)**2))
                    else:
                        D = dist3D_segment_to_segment (front.xyz,front.parent.xyz,o_front.parent.xyz,o_front.xyz)

                    if check_synapses:
                        if D < (front.radius + o_front.radius) :
                            print("radii too close")
                            ret = False
                        elif D < (front.radius + o_front.radius + self.parser.getfloat("system","synapse_distance")):
                            #print "synapse!!!"
                            ##### TODO TODO TODO TODO ####
                            # of course, additional check required to have "axon -> dendrite" instead of "all -> all"
                            # or even more fancy pancy if you can specify which pre synapse entity_name can synapse to which post entity_name
                            
                            #pre_post = (front,o_front)
                            pre_post=(front.entity_name,front.xyz[0],front.xyz[1],front.xyz[2],\
                                      o_front.entity_name,o_front.xyz[0],o_front.xyz[1],o_front.xyz[2])
                            syn_locs.append(pre_post)
                    else:
                        if D < (front.radius + o_front.radius) :
                            print("radii too close [w/o syns]: D=%f (fr=%f, or=%f)" % (D, front.radius, o_front.radius))
                            return False,[]

            else:
                # check against itself!!! HOW-O-HOW
                # for now synapses between itself are out of question
                """
                Check validity of a new front against other fronts of
                the same structure (front.entity_name).
                """
                # OK to have: pathL < soma.diam
                for o_front in self.dynamic_constellation[entity_name]:
                    min_distance =  o_front.radius / 2.0
                    EN = entity_name.split("__")[0] 
                    if EN in self.allowed_self_dist:
                        min_distance = self.allowed_self_dist[EN]                
                    
                    D = np.sqrt(np.sum((front.xyz-o_front.xyz)**2))
                    if np.all(o_front.xyz == front.soma_pos):
                        # comparing to the soma
                        #print "checking against soma"
                        if front.path_length < o_front.radius*2:
                            pass
                        else:
                            if D < o_front.radius:
                                print("self colliding with soma")
                                return False,[]
                            
                    # elif D < o_front.radius:
                    elif D <= min_distance: 
                        print("self refused on radius (D=%f)" % D)
                        return False,[]
        return ret, syn_locs        

    def _summarize_constellation_OLD(self,c) :
        summarized_constellation = {}

        # 2014-01-19
        for key in c :
            temp = [np.mean(c[key],axis=0)]
            temp2 = []
            for t in temp:
                temp2.append((t,0))
            summarized_constellation[key] = temp2
            
        #print_with_rank(self.num,"summarized: "+str(summarized_constellation))
        return summarized_constellation

    def _summarize_constellation(self,c) :
        summarized_constellation = {}
        # 2014-01-19, 2014-08-06
        for key in c :
            temp = [np.mean(c[key],axis=0)]
            temp2 = []
            for t in temp:
                temp2.append((t,0))
            #print "summary for key=",key,'=>',temp2
            sum_front = Front(key,"",temp2[0][0],0,0,0)
            summarized_constellation[key] = set([sum_front])
        #print_with_rank(self.num,"summarized: "+str(summarized_constellation))
        return summarized_constellation    
        
    def _add_front(self,message) :
        """
        TODO 2014-08-05: check if front is valid, if not: wiggle until valid or refute
        """
        new_front = message[1]
        self.active_fronts.append(new_front)

        """ Migrate front (2014-10-06)
        """
        # check if position is occupied already
        valid,syn_locs = self._valid_and_wiggle(new_front)
        if valid:
            if new_front.entity_name in self.dynamic_constellation:
                self.dynamic_constellation[new_front.entity_name].add(new_front)
            else:
                self.dynamic_constellation[new_front.entity_name] = set()
                self.dynamic_constellation[new_front.entity_name].add(new_front)

        # in case of new synapses, notify the Admin agent
        msg = ("Extra_synapses","%06d"%self.num,syn_locs)
        self.ppub.send_multipart(["Admin",pickle.dumps(msg)])        
        
    def _temp_to_db(self,front,c_fronts) :
        pos = front.xyz
        conn = sqlite3.connect(self.db_file_name)
        cursor = conn.cursor()
        for c_front in c_fronts :
            cpos = c_front.xyz
            name = c_front.entity_name
            values = (None,name,pos[0],pos[1],pos[2],cpos[0],cpos[1],cpos[2],1,self.num)
            cursor.execute("INSERT into swc_data VALUES (?,?,?,?,?,?,?,?,?,?)",values )
            conn.commit()

    def _within_this_volume(self,pos) :
        ret = True
        for i in range(3) :
            if (pos[i] < self.boundary[0][i]) or (pos[i] > self.boundary[1][i])  :
                return False
        return ret

def start(num,cfg_file) :
    sv = Subvolume_Agent(num,cfg_file) 
                                
if __name__=="__main__":
    sv = Subvolume_Agent(int(sys.argv[1]))        
