"""
max subvolumes=999999 (SV addresses are: %06d)
"""

import zmq
import sys
import os
import time
import sqlite3
from configparser import SafeConfigParser # For use with Python 2.7
import numpy as np
import pickle as pickle

from front import Front

from multiprocessing import Process

import inspect
def _me(bool) :
    if(bool) :
        print('%s \t Call -> ::%s' % (inspect.stack()[1][1], inspect.stack()[1][3]))
    else :
        pass

def print_with_rank(message) :
    print('%s \t \t%s' % (inspect.stack()[1][1], message))


class Admin_Agent(object) :
    """
    Admin agent: initializes the simulation, decomposes space and
    assigns space to Subvolume agents. Growth cones are also initialized
    here and distributed to the Subvolumes corresponding to the location
    of the growth cones.

    Modelled as conceptually close to biology but great in load balancing...
    """
    def __init__(self,total_processors,cfg_file="test_config.cfg") :
        self.cfg_file = cfg_file
        self.parser = SafeConfigParser()
        self.parser.read(cfg_file)
        np.random.seed(self.parser.getint("system","seed"))  # randomized locations of the initial parts of the structures to generate.
        if self.parser.has_option("system","recursion_limit"):  # Set the maximum depth of the Python interpreter stack to n
            sys.setrecursionlimit(self.parser.getint("system","recursion_limit"))
        self.substrate={}
        # a few variable should be global
        self.total_processors = total_processors # until I find a better way
        self.processor_ids = range(1,self.total_processors) # start from 1: skip the Admin
        self.summarized_constellations = {} # store summarized constellations based on their self.num / proc id
        # set up communication links for the Admin
        self._initialize_communication_links()
        self._setup_DBs()
        ret = self._initialize_SVs()
        time.sleep(1) # sleep a second: make sure al SVs are initialized (all messages processed) return 1
        if ret < 0 :
            self._destruction()
            return # and break out
        ret = self._initialize_GE_beta()  # finish return 1
        if ret < 0 :
            self._destruction()
            return
        # and continue with the real deal if we haven't been kicked out yet
        self.main_loop()
        
        self._destruction()

    def _initialize_communication_links(self) :
        """ Set up the communication links to and from the Admin
        """
        self.context = zmq.Context()

        """ Initialize socket to receive incoming messages. Incoming messages \
        are queued "fair" according to 0MQ...
        """
        self.socket_pull = self.context.socket(zmq.PULL)
        self.socket_pull.bind("tcp://*:%s" % self.parser.getint("system","pull_port"))

        # SETUP PROXY: to communicate through a proxy; all-to-all
        self.psub = self.context.socket(zmq.SUB)
        self.psub.connect("tcp://localhost:%s" % self.parser.getint("system","proxy_pub_port")  )
        self.psub.setsockopt(zmq.SUBSCRIBE, "Admin")        
        self.ppub = self.context.socket(zmq.PUB)
        self.ppub.connect("tcp://localhost:%s" % self.parser.getint("system","proxy_sub_port"))        

        self.poller = zmq.Poller()
        self.poller.register(self.psub, zmq.POLLIN)

        registered = 0
        while registered < len(self.processor_ids) :
            msg = self.socket_pull.recv()
            print_with_rank(str(msg))
            registered = registered + 1
        print("all Subvolumes registered. Proceed!")

    def _setup_DBs(self) :
        self.db_file_name = self.parser.get("system","out_db")
        try :
            os.remove(self.db_file_name)
        except Exception :
            pass
        self.conn = sqlite3.connect(self.db_file_name)
        self.conn.execute('''CREATE TABLE swc_data (id INTEGER PRIMARY KEY AUTOINCREMENT,\
                                            name text,\
                                            swc_type int,\
                                            from_x real, \
                                            from_y real, \
                                            from_z real,\
                                            to_x real,\
                                            to_y real,\
                                            to_z real,\
                                            radius real,\
                                            proc integer)''')

        # only if this parameter is set in the config file
        if self.parser.has_option("system","syn_db"):
            self.syn_file_name = self.parser.get("system","syn_db")
            try :
                os.remove(self.syn_file_name)
            except Exception :
                pass
            self.syn_conn = sqlite3.connect(self.syn_file_name)
            self.syn_conn.execute('''CREATE TABLE synapses (id INTEGER PRIMARY KEY AUTOINCREMENT,\
                                                pre_syn_entity text,\
                                                pre_x real,\
                                                pre_y real,\
                                                pre_z real,\
                                                post_syn_entity text,\
                                                post_x real,\
                                                post_y real,\
                                                post_z real)''')

                                            
    def _get_substrate_information(self) :
        for option in self.parser.options("substrate") :
            if not option.startswith("dim_") : # then this is an entity
                val = self.parser.get("substrate", option)
                if val.endswith("pkl") :
                    self.substrate[option] = pickle.load(open(val,"r"))
                else :
                    print_with_rank("substrate entity (%s) should be given as name of a pickle file" % option)
        time.sleep(0.0)

    def _get_sub_substrate(self,boundary) :  # get subvolume substrate
        sub_substrate = {}
        b0 = np.array(boundary[0])
        b1 = np.array(boundary[1])
        for entity in self.substrate :
            print("checking entity: ", entity, ", boundary: ", boundary)

            # 2014-08-08, make all internals sets of fronts.. bit redundant memory wise, but easy for administration
            sub_substrate[entity] = set()
            for ppoint in self.substrate[entity] :
                point = ppoint[0]
                t_front = Front(None,None,point,ppoint[1],0,0)
                sub_substrate[entity].add(t_front)
                #time.sleep(1)
            if len(sub_substrate[entity]) == 0 :
                sub_substrate.pop(entity,None)
        return sub_substrate
    

    def _get_virtual_substrate(self):
        virtual_substrate ={}
        for name,value in self.parser.items("substrate"):
            if name.startswith("virtual"):
                virtual_name = name.split("_")[1]
                print("Found virtual: ", virtual_name)
                virtual_substrate.update({virtual_name:eval(self.parser.get("substrate",name))})

        if virtual_substrate == {}:
            print("No virtual substrates found")
            #time.sleep(5)
        else:
            print("virtual_substrate:\n", virtual_substrate)
            #time.sleep(5)
        return virtual_substrate
                                            
    def _initialize_SVs(self) :  # each process assign  sv
        required = ["xa","ya","za"]  # slice  number x,y,z corrdination
        for item in required :
            if not self.parser.has_option("sub_volumes",item) :
                print_with_rank( "%s does not exists in the cfg file" % item)
        xa,ya,za = self.parser.getint("sub_volumes","xa"), \
          self.parser.getint("sub_volumes","ya"), \
          self.parser.getint("sub_volumes","za")

        # CHECK: do I still need this clause???
        if  xa*ya*za > (len(self.processor_ids)) :
            print_with_rank("Not enough processors (%i processors for %i SVs)" \
              % (len(self.processor_ids),xa*ya*za))
            return -1 # fetch in __init__

        substrate = self._get_substrate_information()  # volume substrate
            
        dim_xyz = eval(self.parser.get("substrate","dim_xyz"))
        x_space = dim_xyz[0] / xa
        y_space = dim_xyz[1] / ya
        z_space = dim_xyz[2] / za
        self.space_division = {} # per ZMQ processor what part of the Volume they control
        self.assigned_ids = {} # per chunk of Volume, which processor controls it
        count = 0
        for i in range(xa) :
            for j in range(ya) :
                for k in range(za) :
                    x0,y0,z0 = i*x_space,j*y_space,k*z_space
                    x1,y1,z1 = (i+1)*x_space,(j+1)*y_space,(k+1)*z_space
                    self.space_division[i,j,k] = [[x0,y0,z0],[x1,y1,z1]]
                    self.assigned_ids[i,j,k] = self.processor_ids[count]
                    print_with_rank("proc: %i [%i,%i,%i]= %1.f,%1.f,%1.f -> %1.f,%1.f,%1.f" \
                                    % (self.processor_ids[count],i,j,k,x0,y0,z0,x1,y1,z1))
                    count = count + 1

        # for now, pass the global virtual information to each SV (independent of SV boundaries)
        virtual_substrate = self._get_virtual_substrate()  # [substrate] virtual_
                    
        for key in sorted(self.space_division.keys()) :  # find neighbor sv(assigned_ids)
            dest = self.assigned_ids[key]
            print_with_rank("key: "+ str(key)+ "type: "+ str(type(key))+ "assigned to "+str(dest))
            boundary = self.space_division[key]
            neighbors = []
            for i in range(key[0]-1, key[0]+2) :
                for j in range(key[1]-1, key[1]+2) :
                    for k in range(key[2]-1, key[2]+2) :
                        new_key = (i,j,k)
                        if new_key in self.space_division and key != new_key :
                            neighbors.append(self.assigned_ids[i,j,k])
            print_with_rank("neighbors of "+str( dest)+  ": "+ str(neighbors))
            """ get part of the substrate contained in this SV, prepare to include in message.
            currently the substrate entities are represented by points
            """
            sub_substrate = self._get_sub_substrate(boundary)

            print_with_rank("sending Init_SV to %06d"%dest )
            message = ("Init_SV",boundary,neighbors,sub_substrate,virtual_substrate)
            self.ppub.send_multipart(["%06d"%dest,pickle.dumps(message)])
            # time.sleep(2)
        return 1 # positive: all is well

    def _initialize_GE_beta(self) :  # cell_type_
        cfg_sections = self.parser.sections()  # cfg file all sections like[system]
        ship_entity_to_proc = {}
        for i in self.processor_ids :
            ship_entity_to_proc[i]= []
        for name in cfg_sections :
            if name.startswith("cell_type") :
                entity_id = 0
                print_with_rank( "found cell type: %s" % name)
                # retrieve algo to use and how many to deploy
                no_seeds = self.parser.getint(name,"no_seeds")  # number of seeds
                algorithm_name = self.parser.get(name,"algorithm")  # growth rule
                """ Sample the soma position of the entity and assign \
                    to the correct processor
                """
                loc = eval(self.parser.get(name,"location"))
                rs =  eval(self.parser.get(name,"soma_radius"))

                for seed in range(no_seeds) :
                    details = {}
                    for option in self.parser.options(name) :  # section[cell_type_]  option
                        details.update({option:self.parser.get(name,option)})  # insert into details
                        
                    soma_xyz = np.array([])
                    for i in range(3): # 3D
                        soma_xyz = np.append(soma_xyz,(loc[1][i]-loc[0][i])*np.random.random()+loc[0][i])  # soma location
                        
                    # soma_pos = P3D2(soma_xyz,radius=rs)
                    details["radius"] = rs
                    details["soma_xyz"] = soma_xyz

                    dest = self._which_volume_contains_position(soma_xyz)  # contains soma subvolume
                    if dest == -1 :
                        print_with_rank("Could not find subvolume for soma: " + str(soma_xyz))
                        return -1
                    print_with_rank("sampled s%i:%s  (D:%i)" % (seed,str(soma_xyz),dest))
                    
                    entity_name = name + "__" +str(entity_id)  # cell_type__0 /cell_type__1
                    details.update({'entity_name':entity_name})
                    ship_entity_to_proc[dest].append(details)
                    entity_id = entity_id + 1

        # print_with_rank("ship_entity_to_proc: " + str(ship_entity_to_proc))

        for proc in ship_entity_to_proc.keys() :
            entries = ship_entity_to_proc[proc]
            message=("Initialize_GEs",entries)
            # comm.send(message,dest=proc,tag=2)
            self.ppub.send_multipart(["%06d"%proc,pickle.dumps(message)])
            print_with_rank("to %i: %s" % (proc,str(message)))
        return 1 # positive: all is well
        
    
    def main_loop(self) :
        """ Perform some update cycles. When the admin publishes an \
        "Update" note, the Subvolumes send their current "my_constellation" \
        and their summarized constellation to the Admin.
        1. wait/gather all the info/constellations from each SV
        2. wait for replies
        3. proceed to next Update cycle
        """
        for i in range(self.parser.getint("system","no_cycles")) :  # no_cycle:number of each front extend
            for dest in self.processor_ids :
                message = ("Update",i,self.summarized_constellations)
                self.ppub.send_multipart(["%06d"%dest,pickle.dumps(message)])
            responses = 0
            while responses < len(self.processor_ids) :
                # socks = dict(self.poller.poll())

                # if self.psub in socks and socks[self.psub] == zmq.POLLIN:
                [address,message] = self.psub.recv_multipart()
                message = pickle.loads(message)
                print_with_rank("received message on PULL")
                if message[0] == "Update_OK" :
                    responses = responses + 1
                    sender = int(message[1])
                    changes = message[2]
                    syn_locs = message[3]
                    t_constellation = message[4]
                    # write changes
                    self._temp_to_db(changes,sender)
                    # store putative synapse locations
                    if self.parser.has_option("system","syn_db"):
                        self._syn_to_db(syn_locs,sender)
                    # store internally
                    self.summarized_constellations[sender] = t_constellation

                    # ##### TEST
                    # print_with_rank("Received summ ("+str(sender)+"): " + str(t_constellation.keys())  )
                    # time.sleep(1)
                    # #### END OF TEST
                elif message[0] == "Migrate_Front" :
                    self._process_migrate_front(message)
                    time.sleep(0.)
                elif message[0] == "Extra_synapses" :
                    print_with_rank("EXTRA_SYNAPSES")
                    sender = int(message[1])
                    syn_locs = message[2]
                    # store putative synapse locations
                    if self.parser.has_option("system","syn_db"):
                        self._syn_to_db(syn_locs,sender)
                        
        self.conn.commit()
        if self.parser.has_option("system","syn_db"):
            self.syn_conn.commit()

    def _temp_to_db(self,changes,sender):#front,c_fronts) :
        for front,c_fronts in changes :
            pos = front.xyz
            conn = sqlite3.connect(self.db_file_name)
            cursor = conn.cursor()
            for c_front in c_fronts :
                cpos = c_front.xyz
                name = c_front.entity_name
                radius = c_front.radius
                swc_type = c_front.swc_type
                values = (None,name,swc_type,pos[0],pos[1],pos[2],cpos[0],cpos[1],cpos[2],radius,"%06d"%int(sender))
                self.conn.execute("INSERT into swc_data VALUES (?,?,?,?,?,?,?,?,?,?,?)",values )
        #self.conn.commit()                        

    def _syn_to_db(self,syn_locs,sender):
        for syn_loc in syn_locs:
            # pre_front = syn_loc[0]
            # post_front = syn_loc[1]
            # prp = pre_front.xyz
            # pop = post_front.xyz
            # values = (None,pre_front.entity_name,prp[0],prp[1],prp[2],post_front.entity_name,pop[0],pop[1],pop[2])

            pre_name = syn_loc[0]
            pre_x = syn_loc[1]
            pre_y = syn_loc[2]
            pre_z = syn_loc[3]
            post_name = syn_loc[4]
            post_x = syn_loc[5]
            post_y = syn_loc[6]
            post_z = syn_loc[7]
            
            # values = (None,pre_front.entity_name,prp[0],prp[1],prp[2],post_front.entity_name,pop[0],pop[1],pop[2])
            values = (None,pre_name,pre_x,pre_y,pre_z,post_name,post_x,post_y,post_z)

            print("values: ", values)
            self.syn_conn.execute("INSERT into synapses VALUES (?,?,?,?,?,?,?,?,?)",values)

        
    def _process_migrate_front(self,message) :
        new_front = message[1]
        parent_front = message[2]
        new_dest = self._which_volume_contains_position(new_front.xyz)
        print_with_rank("******: new_dest %i (for %s)" % (new_dest,str(new_front.entity_name)))
        if new_dest == -1 :
            # this front is dead. outside of space
            return
        message = ("Add_Front",new_front)
        #comm.send(message,dest=new_dest,tag=2)
        self.ppub.send_multipart(["%06d"%new_dest,pickle.dumps(message)]) 

    def _which_volume_contains_position(self,pos) :
        for key in self.space_division :
            sv = self.space_division[key]
            if (pos[0] >= sv[0][0]) and \
               (pos[1] >= sv[0][1]) and \
               (pos[2] >= sv[0][2]) and \
               (pos[0] < sv[1][0]) and \
               (pos[1] < sv[1][1]) and \
               (pos[2] < sv[1][2]) :
               return self.assigned_ids[key]
        return -1 # if not found, return -1. Fetch somewhere else
                            
    def _destruction(self) :
        """ Shut down the system. End all threads and all MPI instances
        """
        _me(True)
        print_with_rank("pickle_msg:"+ pickle.dumps("Done") )
        self.ppub.send_multipart(["All",pickle.dumps("Done")])
        self.context.destroy()

def start_proxy(cfg_file) :
    print("starting proxy")
    parser = SafeConfigParser()
    parser.read(cfg_file)
    try:
        print_with_rank("PROXY really starting <-----")
        context = zmq.Context(1)
        # Socket facing clients
        frontend = context.socket(zmq.SUB)
        frontend.bind("tcp://*:%i" % parser.getint("system","proxy_sub_port"))
        frontend.setsockopt(zmq.SUBSCRIBE, "")
        print_with_rank("PROXY subscribed <-----")
        # Socket facing services
        backend = context.socket(zmq.PUB)
        backend.bind("tcp://*:%i" % parser.getint("system","proxy_pub_port"))
        zmq.device(zmq.FORWARDER, frontend, backend)
        print_with_rank("PROXY forwarded <-----")
        print_with_rank("PROXY up and running <-----")
    except Exception as e:
        print(e)
        print_with_rank("PROXY encountered an error. PROXY down")
    finally:
        print_with_rank("PROXY finally down")
        frontend.close()
        backend.close()
        context.term()
                    
if __name__=="__main__" :
    """
    Start a simulation. Initialize the proxy for message routing,\
    the admin doing the house hold tasks and thethe Subvolumes \
     holding the developing Fronts
    """
    no = int(sys.argv[1])
    cfg_file = sys.argv[2]
    
    proxy = Process(target=start_proxy,args=(cfg_file,))  # initialize the proxy for message routing
    proxy.start()
    time.sleep(1) # can I do this in a smarter way? What for a signal that the proxy is ready?
    
    import Subvolume
    svs = []
    for i in range(1,no+1) :
        p = Process(target=Subvolume.start,args=(i,cfg_file))  # process: multiprocessing send message, i subvolume
        svs.append(p)
        svs[-1].start()
    print("Admin going to start with cfg_file: ", cfg_file)
    admin = Admin_Agent(no+1,cfg_file)  # total processors n+1
    print("Admin is ready")
    time.sleep(0.5)
    print("Switching off proxy")
    proxy.terminate()
    print("Proxy switched off")
    for sv in svs :
        sv.terminate()
    print("Admin done. Terminated.")
    sys.exit(0) # don't understand why it doesn't clean properly...
