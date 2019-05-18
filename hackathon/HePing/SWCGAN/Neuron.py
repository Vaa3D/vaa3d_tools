from __future__ import print_function

"""Neuron class for making neuron object and extracting the features."""

import numpy as np
from numpy import mean, dot, transpose
from numpy import linalg as LA
import math
from scipy.sparse import csr_matrix, lil_matrix
from builtins import str
from copy import deepcopy
import matplotlib.pyplot as plt
from numpy.linalg import inv
import sys
#np.random.seed(0)
# -*- coding: utf-8 -*-
class Neuron(object):
    """Neuron Class
    This class represents the neuron by a list of `Node`s. Borrowed from swc format, each node indicates a point on the neuron. each node has parent and children (at most two children) and set of all node with their parents make a tree structure; a connected graph without loop. The Soma represents by a few nodes and one of them is called root node and it's decendent of all the nodes in the neuron (including other soma nodes). Notice that all nodes from index 0 to index of `n_soma` in the `nodes_list` are soma.
    This class contains the attributes to calculate different features of the neuron. The iput file can be a swc file or the list of nodes.



    all indexing for not-soma nodes (or the nodes that are very close to the soma) coming from self.nodes_list

    Attributes
    ----------
    n_soma : int
        The number of the nodes that represents the soma.

    n_node : int
        The number of all the nodes in the neuron.

    nodes_list : list of Node
        The list of all the nodes in the neuron.

    root : Node
        The represented node of root.

    location : array of shape = [3, n_node]
        Three dimentional location of the nodes.

    parent_index : array of shape = [n_node]
        The index of the parent of each node in the nodes_list.

    child_index : array of shape = [2, n_node]
        first row: The index of first child of the node ( the ordering of the nodes are arrbitraty).
        second row: nan if neuron is order oner and index of second child of the node if it's a branching node.

    branch_order : array of shape = [n_node]
        The number of children of the nodes. It can be and integer number for the root (first element) and only 0, 1 or 2 for other nodes.

    ext_red_list : array of shape = [3, n_node]
        first row: end points and order one nodes (for extension)
        second row: end points (for removing)
        third row: end point wich thier parents are order one nodes (for extension)

    connection :  array of shape = [n_node, n_node]
        The matrix of connectivity of the nodes. The element (i,j) of the matrix is not np.nan if node i is a decendent of node j. The value at this array is the distance of j to its parent. It's useful for the calculation of the neural distance over Euclidain distance.

    frustum : array of shape = [n_node] !!!NOT IMPLEMENTED!!!
        The value of th fustum from the node toward its parent.

    branch_order : array of shape = [n_node]
        The number of children for each of nodes. Notice that for the nodes rther than root it should be 0, 1 or 2. For root it can be any integer number.

    rall_ratio :  array of shape = [n_node] !!!NOT IMPLEMENTED!!!
        It's not nan only in branching nodes which its value is the rall ratio.

    distance_from_root : array of shape = [n_node]
        Euclidain distance toward the root.

    distance_from_parent : array of shape = [n_node]
        Euclidain distance toward the parent of the node.

    slope : array of shape = [n_node]
        ratio of euclidain distance toward the parent of the node over their diameter difference.

    branch_angle : array of shape [3, n_nodes]
        it shows the angles at the branching nodes: First row is the angle of two outward segments at the branching point Second and third rows are the angle betwen two outward segments and previous segment at the branching in arbitrary order (nan at other nodes).

    angle_global : array of shape = [n_node]
        The angle between the line linking the node to the root and the line likning it to its parent.

    local_angle : array of shape = [n_node]
        The angle between the line linking the node to its parent and its child and nan otherwise.

    References
    ----------

    .. [1] R.Farhoodi, K.P.Kording, "Generating Neuron Morphologies using naive Bayes MCMC"

    """

    def __init__(self, file_format = None, input_file = None):
        """
        Default constructor.
        Parameters
        -----------
        file_format : string, optional (default=None)
            - In 'swc' the swc file in given and the output is a Neuron calss
            with all of its attributes.
            - In 'swc without attributes' the swc file in given and the output
            is a Neuron calss without its attributes. It's useful for the case
            that only nodes are important, e.g. visualization of the neurpn,
            in a fast way.
            - In 'only list of nodes' the list of all the nodes of the neuron is
            given.
            - In 'Matrix of swc' a numpy array of the shape n*7 is presented,
            and the Neuron will be made accordingly.

        input_file : string or list
            - In 'swc' it contains a .swc file
            - In 'swc without attributes' it contains a .swc file
            - In 'only list of nodes' it contains the list of all the nodes of
            the neuron is given.
            if it is string, it will read the swc file from that address,
            if it is list, the elements of the list should be object from Node's class
            and corrsponding Tree is created.


        """
        if(file_format == 'swc'): # swc is given

            self.read_swc(input_file)
            self.ratio_red_to_ext = 1.
            self.n_node = len(self.nodes_list)

            #self.set_n_soma_n_node()
            self.set_parent()
            self.parent_index = self.parent_index.astype(int)
            #self.set_loc_diam()
            #self.fit()

        if(file_format == 'swc without attributes'):
            self.read_swc(input_file)
            self.set_parent()
            self.parent_index = self.parent_index.astype(int)

        if(file_format == 'only list of nodes'):
            self.nodes_list = input_file
            self.root = self.find_root(self.nodes_list[0])
            self.set_n_soma_n_node()
            self.set_parent()
            self.parent_index = self.parent_index.astype(int)
            self.set_loc_diam()
            self.set_location()
            self.set_branch_order()

        if(file_format == 'Matrix of swc'):
            # the n*7 array is given.
            self.read_swc_matrix(input_file)
            self.set_parent()
            self.parent_index = self.parent_index.astype(int)  # transform data type -->int
            self.set_branch_order()
            self.set_distance_from_parent()  # accumlate child from parent Elucid distance


        #self.set_sholl()

    def __str__(self):
        """
        describtion.
        """
        return "Neuron found with " + str(self.n_node) + " number of nodes and"+ str(self.n_soma) + "number of node representing soma."


    def fit(self):
        """
        dependency:
            self.nodes_list
            self.n_soma
            self.location
            self.diameter
            self.parent_index
            self.child_index
            self.root
            self.n_nodes
        output attributes are:
            self.branch_order
            self.connection
            self.ext_red_list
            self.rall_ratio
            self.distance_from_root
            self.distance_from_parent
            self.branch_angle
            self.global_angle
            self.local_angle
            self.frustum
        """
        self.set_branch_order()
        self.set_distance_from_root()
        self.set_distance_from_parent()
        # self.set_connection2()  ----not used-------
        #self.set_rall_ratio()
        self.set_branch_angle()
        self.set_global_angle()
        self.set_local_angle()
        #self.sholl_r = np.array([0]) # the position of the jumps for sholl analysis
        #self.sholl_n = np.array([0]) # the value at the jumping (the same size as self.sholl_x)
        #self.set_sholl()
        self.ratio_red_to_ext = 1.
        self.set_ext_red_list()
        # self.set_features()  -----not used---------


    def set_n_soma_n_node(self):
        self.n_soma = 0
        for n in self.nodes_list:
            if n.type is 'soma':
                self.n_soma += 1
        if(self.n_soma == 0): # for no soma representation
            self.n_soma = 1
        self.n_node = len(self.nodes_list)

    def set_features(self):
        self.features = {}
        self.features['Nnodes'] = np.array([self.n_node - self.n_soma])
        #self.features['asymetric']
        (num_branches,) = np.where(self.branch_order[self.n_soma:] == 2)
        self.features['Nbranch'] = np.array([len(num_branches)])
        self.features['initial_segments'] = np.array([len(self.root.children)])
        self.features['global_angle'] = np.pi - self.global_angle[self.n_soma:]
        #self.features['diameter'] = self.diameter[self.n_soma:]
        self.features['distance_from_parent'] = self.distance_from_parent[self.n_soma:]
        self.features['distance_from_root'] = self.distance_from_root[self.n_soma:]
        self.features['ratio_euclidian_neuronal'] = np.nansum(self.connection[self.n_soma:,self.n_soma:],axis = 1)/self.distance_from_root[self.n_soma:]
        x = np.abs(self.branch_angle[0,self.n_soma:])
        self.features['branch_angle'] = x[~np.isnan(x)]
        x = self.local_angle[self.n_soma:]
        self.features['local_angle'] = x[~np.isnan(x)]
        if(len(self.features['local_angle'])==0):
            self.features['local_angle'] = np.array([0])
        if(len(self.features['branch_angle']) == 0):
            self.features['branch_angle'] = np.array([0])
        self.features['discrepancy_space'] = np.array([self.discrepancy(10.,10.,10.)])
        #self.features['repellent'] = np.array([self.repellent(10.,10.,10.)])
        self.features['curvature'] = self.set_curvature()
        important_node = self.get_index_main_nodes()
        parent_important = self.parent_index_for_node_subset(important_node)
        (neural, euclidan) = self.get_neural_and_euclid_lenght(important_node, parent_important)
        self.features['neural_important'] = neural
        self.features['euclidian_important'] = euclidan
        self.features['ratio_neural_euclidian_important'] = neural/euclidan
        self.features['branch_angle_segment'] = self.set_branch_angle_segment(important_node, parent_important)


    def get_index_main_nodes(self):
        """
        Returing the index of end points and branching points.

        Returns
        -------
        important_node: numpy array
            the index of main points.
        """
        (branch_index, ) = np.where(self.branch_order[self.n_soma:] == 2)
        (end_nodes, ) = np.where(self.branch_order[self.n_soma:] == 0)
        important_node = np.append(branch_index, end_nodes)
        if(len(important_node) != 0):
            important_node = self.n_soma + important_node
        return important_node


    def get_neural_and_euclid_lenght(self, initial_index_of_node,thier_parents):
        """
        Returning the neural and Euclidain length for the given points.
        """
        neural = np.array([])
        euclidan = np.array([])
        for i in range(initial_index_of_node.shape[0]):
            neural_length = self.distance(initial_index_of_node[i],
                                          thier_parents[i])
            euclidan_length = \
                LA.norm(self.location[:, initial_index_of_node[i]] -
                        self.location[:, thier_parents[i]], 2)
            neural = np.append(neural, neural_length)
            euclidan = np.append(euclidan, euclidan_length)
        return neural, euclidan


    def discrepancy(self, x_mesh, y_mesh, z_mesh):
        X = self.normlize(self.location[0,:],x_mesh)
        Y = self.normlize(self.location[1,:],y_mesh)
        Z = self.normlize(self.location[2,:],z_mesh)
        L = X + x_mesh*Y + x_mesh*y_mesh*Z
        a, N = np.unique(L, return_counts=True)
        return len(a) # np.flipud(np.sort(N))

    def repellent(self, x_mesh, y_mesh, z_mesh):
        X = self.normlize(self.location[0,:],x_mesh)
        Y = self.normlize(self.location[1,:],y_mesh)
        Z = self.normlize(self.location[2,:],z_mesh)
        L = X + x_mesh*Y + x_mesh*y_mesh*Z
        a, N = np.unique(L, return_counts=True)
        return len(a) # np.flipud(np.sort(N))

    def normlize(self, vector, mesh):
        m = min(vector)
        M = max(vector)
        if(M==m):
            a = 0
        else:
            a = np.floor(mesh*((vector - m)/(M-m)))
        return a

    def set_branch_order(self):
         # terminal = 0, passig (non of them) = 1, branch = 2
        """
        dependency:
            nodes_list
        """
        self.branch_order = np.array([])
        for n in self.nodes_list:
            self.branch_order = np.append(self.branch_order, len(n.children))

    def set_ratio_red_to_ext(self,c):
        self.ratio_red_to_ext = c

    def set_ext_red_list(self):
        """
        In the extension-reduction perturbation, one of the node will be removed or one node will be added. In the first case, the node can only be
        an end point, but in the second case the new node might be added to any node that has one or zero child.

        dependency:
            self.nodes_list
            self.branch_order
            self.n_soma
            self.ratio_red_to_ext

        ext_red_list:
            first row: end points and order one nodes (for extension)
            second row: end points (for removing)
            third row: end point wich thier parents are order one nodes (for extension)

        Remarks:
            1) The list is zero for the soma nodes.
            2) The value for first and second is binary but the third row is self.ratio_red_to_ext
        """
        (I,) = np.where(self.branch_order[self.n_soma:] == 0)
        I = I + self.n_soma
        self.ext_red_list = np.zeros((3, self.n_node))
        self.ext_red_list[0,I] = 1
        self.ext_red_list[0,np.where(self.branch_order == 1)] = 1
        self.ext_red_list[1,I] = self.ratio_red_to_ext
        J = np.array([])
        for i in I:
            if(len((self.nodes_list[i].parent).children) == 1):
                J = np.append(J,i)
        J = np.array(J, dtype = int)
        self.ext_red_list[2,J] = 1
        self.ext_red_list.astype(int)
        self.ext_red_list[:,0:self.n_soma] = 0

    def set_distance_from_root(self):
        """
        dependency:
            self.location
        """
        self.distance_from_root = np.sqrt(sum(self.location ** 2))

    def set_distance_from_parent(self):
        """
        given:
            self.location
            self.parent_index
        """
        a = (self.location - self.location[:,self.parent_index.astype(int)]) ** 2
        self.distance_from_parent = np.sqrt(sum(a))  # Eulic distance

    def set_branch_angle_segment(self, important_node, parent_important):
        I = np.array([])
        for i in important_node:
            (J,) = np.where(parent_important == i)
            if(len(J) == 2):
                vec0 = np.expand_dims(self.location[:,important_node[J[0]]] - self.location[:,i], axis = 1)
                vec1 = np.expand_dims(self.location[:,important_node[J[1]]] - self.location[:,i], axis = 1)
                I = np.append(I,self.angle_vec_matrix(vec0,vec1))
        return I

    def set_branch_angle(self):
        """
        An array with size [3, n_nodes] and shows the angles at the branching nodes:
        First row is the angle of two outward segments at the branching point
        Second and third rows are the angle betwen two outward segments and previous segment at the branching in arbitrary order (nan at other nodes).

        dependency:
            self.nodes_list
            self.branch_order
            self.location
            self.parent_index
            self.child_index
            self.n_soma
        """

        self.branch_angle = np.nan*np.zeros([3,self.n_node])
        (I,) = np.where(self.branch_order == 2)
        I = I[I>self.n_soma]
        vec0 = self.location[:,self.child_index[0,I].astype(int)] - self.location[:,I]
        vec1 = self.location[:,self.child_index[1,I].astype(int)] - self.location[:,I]
        vec2 = self.location[:,self.parent_index[I].astype(int)] - self.location[:,I]
        self.branch_angle[0,I] = self.angle_vec_matrix(vec0,vec1)
        self.branch_angle[1,I] = self.angle_vec_matrix(vec0,vec2)
        self.branch_angle[2,I] = self.angle_vec_matrix(vec2,vec1)

    def set_global_angle(self):
        """
        dependency:
            sefl.location
            self.parent_index
            self.n_soma
        """
        dir = self.location - self.location[:,self.parent_index.astype(int)]
        self.global_angle = self.angle_vec_matrix(self.location, dir)

    def set_local_angle(self):
        """
        dependency:
            self.location
            self.n_soma
            self.branch_order
            self.parent_index
        """
        self.local_angle = np.nan*np.ones(self.n_node)
        (I,) = np.where(self.branch_order[self.n_soma:] == 1)
        I = I + self.n_soma
        dir1 = self.location[:,I] - self.location[:,self.parent_index[I].astype(int)]
        dir2 = self.location[:,I] - self.location[:,self.child_index[0,I].astype(int)]
        self.local_angle[I] = self.angle_vec_matrix(dir1, dir2)

    def set_frustum(self):
        """
        dependency:
            self.distance_from_parent
            self.n_soma
            self.diameter
            self.parent_index
        """
        self.frustum = np.array([0])
        l = self.distance_from_parent[self.n_soma:]
        r = self.diameter[self.n_soma:]
        R = self.diameter[self.parent_index][self.n_soma:]
        f = (np.pi/3.0)*l*(r ** 2 + R ** 2 + r * R)
        self.frustum = np.append(np.zeros(self.n_soma), f)

    def set_curvature(self):
        par = self.parent_index
        papar = par[par]
        papapar = par[par[par]]
        dir1 = self.location[:,par] - self.location
        dir2 = self.location[:,papar] - self.location[:,par]
        dir3 = self.location[:,papapar] - self.location[:,papar]
        cros1 = np.cross(np.transpose(dir1), np.transpose(dir2))
        cros2 = np.cross(np.transpose(dir2), np.transpose(dir3))
        I = self.angle_vec_matrix(np.transpose(cros1), np.transpose(cros2))
        return I[self.n_soma:]

    def set_rall_ratio(self):
        """
        dependency:
            self.diameter
            self.child_index
            self.n_soma
            self.n_node
        """
        self.rall_ratio = np.nan*np.ones(self.n_node)
        (I,) = np.where(self.branch_order[self.n_soma:] == 2)
        ch1 = np.power(self.diameter[self.child_index[0,I]],2./3.)
        ch2 = np.power(self.diameter[self.child_index[1,I]],2./3.)
        n = np.power(self.diameter[I],2./3.)
        self.rall_ratio[I] = (ch1+ch2)/n

    def set_values_ite(self):
        """
        set iteratively the following attributes:
            parent_index
            child_index
            location
            diameter
            rall_ratio
            distance_from_root
            distance_from_parent
            slope
            branch_angle
            branch_order
        """
        self.parent_index = np.zeros(self.n_soma)
        self.child_index = np.nan * np.ones([2,self.n_soma])
        for n in self.nodes_list[1:]:
            self.location = np.append(self.location, n.xyz.reshape([3,1]), axis = 1)
            self.diameter = np.append(self.diameter, n.r)
        for n in self.nodes_list[1:]:
            #self.frustum = np.append(self.frustum,  self.calculate_frustum(n))
            #self.rall_ratio = np.append(self.rall_ratio, self.calculate_rall(n))
            self.distance_from_root = np.append(self.distance_from_root, self.calculate_distance_from_root(n))
            self.distance_from_parent = np.append(self.distance_from_parent, self.calculate_distance_from_parent(n))
            #self.slope = np.append(self.slope, self.calculate_slope(n))
            ang, ang1, ang2 = self.calculate_branch_angle(n)
            an = np.zeros([3,1])
            an[0,0] = ang
            an[1,0] = ang1
            an[2,0] = ang2

            if(self.branch_angle.shape[1] == 0):
                self.branch_angle = an
            else:
                self.branch_angle = np.append(self.branch_angle, an, axis = 1)
            glob_ang, local_ang = self.calculate_node_angles(n)
            self.global_angle = np.append(self.global_angle, glob_ang)
            self.local_angle = np.append(self.local_angle, local_ang)
            #self.neural_distance_from_soma = np.append(self.neural_distance_from_soma, self.calculate_neural_distance_from_soma(n))
        for n in self.nodes_list[self.n_soma:]:
            self.parent_index = np.append(self.parent_index, self.get_index_for_no_soma_node(n.parent))
            if(self.branch_order[self.get_index_for_no_soma_node(n)]==2):
                a = np.array([self.get_index_for_no_soma_node(n.children[0]),self.get_index_for_no_soma_node(n.children[1])]).reshape(2,1)
                self.child_index = np.append(self.child_index, a, axis = 1)
            if(self.branch_order[self.get_index_for_no_soma_node(n)]==1):
                a = np.array([self.get_index_for_no_soma_node(n.children[0]),np.nan]).reshape(2,1)
                self.child_index = np.append(self.child_index, a, axis = 1)
            if(self.branch_order[self.get_index_for_no_soma_node(n)]==0):
                a = np.array([np.nan,np.nan]).reshape(2,1)
                self.child_index = np.append(self.child_index, a, axis = 1)

    def set_parent(self):
        self.parent_index = np.zeros(self.n_soma)
        self.child_index = np.zeros([2,self.n_node])
        for n in self.nodes_list[self.n_soma:]:
            # print(n)
            par = self.get_index_for_no_soma_node(n.parent)
            node = self.get_index_for_no_soma_node(n)
            self.parent_index = np.append(self.parent_index, par)
            if self.child_index[0,par] != 0:
                self.child_index[1,par] = node
            else:
                self.child_index[0,par] = node
        self.child_index[self.child_index == 0] = np.nan
        self.child_index[:,0:self.n_soma] = np.nan
        #self.parent_index.astype(int)

    def set_loc_diam(self):
        self.location = np.zeros([3,self.n_node])
        self.diameter = np.zeros(self.n_node)
        for n in range(self.n_node):
            self.location[:,n] = self.nodes_list[n].xyz
            self.diameter[n] = self.nodes_list[n].r

    def set_connection2(self):
        """
        dependency:
            self.nodes_list
            self.n_soma
            self.n_node
            self.parent_index
            self.distance_from_parent
        """
        connection = np.zeros([self.n_node,self.n_node]) # the connectivity matrix
        connection[np.arange(self.n_node), self.parent_index.astype(int)] = 1
        connection[0,0] = 0
        connection = inv(np.eye(self.n_node) - connection)
        connection[connection != 1] = np.nan
        for i in range(self.n_node):
            (J,) = np.where(~np.isnan(connection[:,i]))
            connection[J,i] = self.distance_from_parent[i]
        connection[:,0] = 1
        connection[np.arange(self.n_soma),np.arange(self.n_soma)] = 1
        self.connection = connection
        #return connection

    def set_connection(self):
        """
        connection is an array with size [n_node, n_node]. The element (i,j) is not np.nan if
        node i is a decendent of node j. The value at this array is the distance of  to its parent.

        dependency:
            self.nodes_list
            self.n_somai
            self.parent_indexi
            self.distance_from_parent
        """
        self.parent_index = np.array(self.parent_index, dtype = int)
        L = self.n_node - self.n_soma
        C = csr_matrix((np.ones(L),(range(self.n_soma,self.n_node), self.parent_index[self.n_soma:])), shape = (self.n_node,self.n_node))
        self.connection = np.zeros([self.n_node,self.n_node]) # the connectivity matrix
        new = 0
        i = 0
        old = C.sum()
        while(new != old):
            self.connection = C.dot(csr_matrix(self.connection)) + C
            old = new
            new = self.connection.sum()
        self.connection = self.connection.toarray()
        self.connection[range(1,self.n_node),range(1,self.n_node)] = 1
        self.connection[:,:self.n_soma] = 0

        # fill the matrix with the distance
        for i in range(self.n_node):
            self.connection[self.connection[:,i] != 0,i] = self.distance_from_parent[i]
        self.connection[self.connection == 0] = np.nan

    def set_sholl(self):
        self.sholl_r = np.array([])
        for n in self.nodes_list:
            dis = LA.norm(self.xyz(n) - self.root.xyz,2)
            self.sholl_r = np.append(self.sholl_r, dis)

        self.sholl_r = np.sort(np.array(self.sholl_r))
        self.sholl_n = np.zeros(self.sholl_r.shape)
        for n in self.nodes_list:
            if(n.parent != None):
                par = n.parent
                dis_par = LA.norm(self.xyz(par) - self.root.xyz,2)
                dis_n = LA.norm(self.xyz(par) - self.root.xyz,2)
                M = max(dis_par, dis_n)
                m = min(dis_par, dis_n)
                I = np.logical_and(self.sholl_r>=m, self.sholl_r<=M)
                self.sholl_n[I] = self.sholl_n[I] + 1

    def set_location(self):
        self.location = np.zeros([3, len(self.nodes_list)])
        for i in range(len(self.nodes_list)):
            self.location[:, i] = self.nodes_list[i].xyz

    def xyz(self, node):
        return self.location[:,self.get_index_for_no_soma_node(node)]

    def _r(self, node):
        return self.diameter[self.get_index_for_no_soma_node(node)]


    def parent_index_for_node_subset(self, subset):
        """
        inputs
        ------
            index of subset of the nodes without root node
        output
        ------
            Index of grand parent inside of the subset for each member of subset
        """
        if((subset==0).sum() == 0):
            subset = np.append(0,subset)
        n = subset.shape[0]
        self.connection[:,0] = 1.
        self.connection[np.arange(self.n_soma),np.arange(self.n_soma)] = 1.
        A = self.connection[np.ix_(subset,subset)]
        A[np.isnan(A)] = 0
        A[A!=0] = 1.
        print(inv(A))
        B = np.eye(subset.shape[0]) - inv(A)
        return subset[np.where(B==1)[1]]


    def distance(self, index1, index2):
        """
        Neural distance between two nodes in the neuron.

        inputs
        ------
            index1, index2 : the indecies of the nodes.
        output
        ------
            the neural distance between the node.
        """
        return min(self.distance_two_node_up_down(index1,index2),self.distance_two_node_up_down(index2,index1))

    def distance_two_node_up_down(self, Upindex, Downindex):
        (up,) = np.where(~np.isnan(self.connection[Downindex,:]))
        (down,) = np.where(~np.isnan(self.connection[:,Upindex]))
        I = np.intersect1d(up,down)
        if(I.shape[0] != 0):
            return sum(self.distance_from_parent[I]) - self.distance_from_parent[Upindex]
        else:
            return np.inf

    def calculate_overall_matrix(self, node):
        j = self.get_index_for_no_soma_node(node)
        k = self.get_index_for_no_soma_node(node.parent)
        (J,)  = np.where(~ np.isnan(self.connection[:,j]))
        dis = LA.norm(self.location[:,k] - self.location[:,j],2)
        self.connection[J,j] = dis

    def calculate_branch_order(self,node):
        """
        terminal = 0, passig (non of them) = 1, branch = 2
        """
        return len(node.children)

    def calculate_frustum(self,node):
        """
        the Volume of the frustum ( the node with its parent) at each location. (nan for the nodes of soma)
        """
        r = self._r(node)
        r_par = self._r(node.parent)
        dis = LA.norm(self.xyz(node) - self.xyz(node.parent) ,2)
        f = dis*(np.pi/3.0)*(r*r + r*r_par + r_par*r_par)
        return f

    def calculate_rall(self,node):
        if(len(node.children) == 2):
            n1, n2 = node.children
            r1 = self._r(n1)
            r2 = self._r(n2)
            r = self._r(node)
            rall = (np.power(r1,2.0/3.0)+(np.power(r2,2.0/3.0)))/np.power(r,2.0/3.0)
        else:
            rall = np.nan
        return rall

    def calculate_distance_from_root(self,node):
        return LA.norm(self.xyz(node) - self.root.xyz,2)

    def calculate_distance_from_parent(self,node):
        return LA.norm(self.xyz(node) - self.xyz(node.parent),2)

    def calculate_slope(self,node):
        # the ratio of: delta(pos)/delta(radius)
        dis = LA.norm(self.xyz(node) - self.xyz(node.parent),2)
        rad = node.r - node.parent.r
        if(dis == 0):
            val = rad
        else:
            val = rad/dis
        return val

    def calculate_branch_angle(self,node):
        # the mean of the angle betwen two outward segments and previous segment at the branching (nan at other nodes)
        if(len(node.children) == 2):
            n1, n2 = node.children
            nodexyz = self.xyz(node)
            node_parxyz = self.xyz(node.parent)
            node_chixyz1 = self.xyz(n1)
            node_chixyz2 = self.xyz(n2)
            vec  = node_parxyz - nodexyz
            vec1 = node_chixyz1 - nodexyz
            vec2 = node_chixyz2 - nodexyz
            ang = self.angle_vec(vec1,vec2) # the angle of two outward segments at the branching point (nan for non-branchings)
            ang1 = self.angle_vec(vec1,vec)
            ang2 = self.angle_vec(vec2,vec)
        else:
            ang = np.nan
            ang1 = np.nan
            ang2 = np.nan
        return ang, ang1, ang2

    def calculate_node_angles(self,node):
        par = node.parent
        nodexyz = self.xyz(node)
        node_parxyz = self.xyz(node.parent)
        vec1 = node_parxyz - nodexyz
        vec2 = self.root.xyz - nodexyz
        glob_ang = self.angle_vec(vec1,vec2)
        if(node.children != None):
            if(len(node.children) ==1):
                [child] = node.children
                vec3 = self.xyz(child) - nodexyz
                local_ang = self.angle_vec(vec1,vec3)
            else:
                local_ang = np.nan
        else:
            local_ang = np.nan
        return glob_ang, local_ang

    # Axulary functions
    def angle_vec_matrix(self,matrix1,matrix2):
        """
        Takes two matrix 3*n of matrix1 and matrix2 and gives back
        the angles for each corresponding n vectors.
        Note: if the norm of one of the vectors is zeros the angle is np.pi
        """
        ang = np.zeros(matrix1.shape[1])
        norm1 = LA.norm(matrix1, axis = 0)
        norm2 = LA.norm(matrix2, axis = 0)
        domin = norm1*norm2
        (J,) = np.where(domin != 0)
        ang[J] = np.arccos(np.maximum(np.minimum(sum(matrix1[:,J]*matrix2[:,J])/domin[J],1),-1))
        return ang

    def angle_vec(self,vec1,vec2):
        val = sum(vec1*vec2)/(LA.norm(vec1,2)*LA.norm(vec2,2))
        if(LA.norm(vec1,2)==0 or LA.norm(vec2,2) == 0):
            val = -1
        return math.acos(max(min(val,1),-1))

    def choose_random_node_index(self):
            n = np.floor((self.n_node-self.n_soma)*np.random.random_sample()).astype(int)
            return n + self.n_soma

    def p_ext_red_whole(self):
        """
        Thos function gives back the probabiliy of the chossing one of the node add_node
        extend it.
        """
        return self.ext_red_list[0:2,:].sum()+1 # 1 added because the root may extend

    def p_ext_red_end_point(self):
        """
        Those function gives back the probabiliy of the chossing one of the node add_node
        extend it.
        """
        return self.ext_red_list[1:3,:].sum()

    def get_index_for_no_soma_node(self,node):
        return self.nodes_list.index(node)

    def _list_for_local_update(self,node):
        """
        Return the index of node, its parent and any children it may have.
        The node should be a no_soma node
        """
        update_list = np.array([]) # index of all nodes for update
        update_list = np.append(update_list, self.get_index_for_no_soma_node(node))
        if(node.parent.type != 'soma'):
            update_list = np.append(update_list, self.get_index_for_no_soma_node(node.parent)) # if the node doesnt have a parent in no_soma list, i.e. its parent is a soma, get_index would return nothing
        if(node.children != None):
            for n in node.children:
                update_list = np.append(update_list, self.get_index_for_no_soma_node(n))
        return update_list.astype(int)

    def _update_attribute(self,update_list):
        for ind in update_list:
            #self.frustum[ind] = self.calculate_frustum(self.nodes_list[ind])
            #self.rall_ratio[ind] = self.calculate_rall(self.nodes_list[ind])
            self.distance_from_root[ind] =  self.calculate_distance_from_root(self.nodes_list[ind])
            self.distance_from_parent[ind] = self.calculate_distance_from_parent(self.nodes_list[ind])
            #self.slope[ind] = self.calculate_slope(self.nodes_list[ind])
            self.branch_order[ind] = self.calculate_branch_order(self.nodes_list[ind])
            ang, ang1, ang2 = self.calculate_branch_angle(self.nodes_list[ind])
            self.branch_angle[0, ind] = ang
            self.branch_angle[1, ind] = ang1
            self.branch_angle[2, ind] = ang2
            ang1, ang2 = self.calculate_node_angles(self.nodes_list[ind])
            self.global_angle[ind] = ang1
            self.local_angle[ind] = ang2
            self.calculate_overall_matrix(self.nodes_list[ind])
        #self.sholl_r = np.array([]) # the position of the jumps for sholl analysis
        #self.sholl_n = np.array([]) # the value at the jumping (the same size as self.sholl_x)

    def change_location(self,index,displace):
        """
        Change the location of one of the node in the neuron updates the attribute accordingly.
        Parameters:
        ___________
        index: the index of node in no_soma_list to change its diameter

        displace: the location of new node is the xyz of the current locatin + displace
        """
        # First change the location of the node by displace
        node = self.nodes_list[index]
        self.location[:,index] += displace
        self._update_attribute(self._list_for_local_update(node))
        self.set_features()

    def change_location_toward_end_nodes(self,index,displace):
        (I,) = np.where(~np.isnan(self.connection[:,index]))
        self.location[0,I] += displace[0]
        self.location[1,I] += displace[1]
        self.location[2,I] += displace[2]
        self.set_distance_from_root()
        self.set_distance_from_parent()
        self.connection[np.ix_(I,[index])] = self.distance_from_parent[index]
        self.set_branch_angle()
        self.set_global_angle()
        self.set_local_angle()
        self.set_features()

    def change_location_important(self, index, displace):
        (branch_index,)  = np.where(self.branch_order[self.n_soma:]==2)
        (end_nodes,)  = np.where(self.branch_order[self.n_soma:]==0)
        branch_index += self.n_soma
        end_nodes += self.n_soma
        I = np.append(branch_index, end_nodes)
        parents = self.parent_index_for_node_subset(I)
        (ind,) = np.where(I == index)
        origin = deepcopy(self.location[:,index])
        # correct the segment to the parent
        par = parents[ind][0]
        (up,) = np.where(~np.isnan(self.connection[index,:]))
        (down,) = np.where(~np.isnan(self.connection[:,par]))
        J = np.intersect1d(up,down)
        A = self.location[:,J]
        loc = self.location[:,par]
        A[0,:] = A[0,:] - loc[0]
        A[1,:] = A[1,:] - loc[1]
        A[2,:] = A[2,:] - loc[2]
        r1 = origin - loc
        r2 = r1 + displace
        M = self.scalar_rotation_matrix_to_map_two_vector(r1, r2)
        A = np.dot(M,A)
        A[0,:] = A[0,:] + loc[0]
        A[1,:] = A[1,:] + loc[1]
        A[2,:] = A[2,:] + loc[2]
        self.location[:,J] = A
        changed_ind = J
        # correct the children
        (ch,) = np.where(parents == index)
        for i in I[ch]:
            (up,) = np.where(~np.isnan(self.connection[i,:]))
            (down,) = np.where(~np.isnan(self.connection[:,index]))
            J = np.intersect1d(up,down)
            A = self.location[:,J]
            loc = self.location[:,i]
            A[0,:] = A[0,:] - loc[0]
            A[1,:] = A[1,:] - loc[1]
            A[2,:] = A[2,:] - loc[2]
            r1 = origin - loc
            r2 = r1 + displace
            M = self.scalar_rotation_matrix_to_map_two_vector( r1, r2)
            A = np.dot(M,A)
            A[0,:] = A[0,:] + loc[0]
            A[1,:] = A[1,:] + loc[1]
            A[2,:] = A[2,:] + loc[2]
            self.location[:,J] = A
            changed_ind = np.append(changed_ind, J)
        self.location[:,index] = origin + displace
        self.set_distance_from_root()
        self.set_distance_from_parent()
        for i in changed_ind:
            (J,) = np.where(~np.isnan(self.connection[:,i]))
            self.connection[J,i] = self.distance_from_parent[i]
        self.set_branch_angle()
        self.set_global_angle()
        self.set_local_angle()
        self.set_features()

    def scalar_rotation_matrix_to_map_two_vector(self, v1, v2):
        r1 = LA.norm(v1,2)
        norm1 = v1/r1
        r2 = LA.norm(v2,2)
        normal2 = v2/r2
        a = sum(normal2*norm1)
        theta = -np.arccos(a)
        normal2 = normal2 - a*norm1
        norm2 = normal2/LA.norm(normal2,2)
        cross = np.cross(norm1, norm2)
        B = np.zeros([3,3])
        B[:,0] = norm1
        B[:,1] = norm2
        B[:,2] = cross
        A = np.eye(3)
        A[0,0] = np.cos(theta)
        A[1,0] = - np.sin(theta)
        A[0,1] = np.sin(theta)
        A[1,1] = np.cos(theta)
        return (r2/r1) * np.dot(np.dot(B,A),inv(B))

    def change_diameter(self,index,ratio):
        """
        Change the diameter of one node in the neuron updates the attribute accordingly.
        Parameters:
        ___________
        index: the index of node in no_soma_list to change its diameter

        ratio: the radius of new node is the radius of current node times ratio
        """
        node = self.nodes_list[index]
        node.r = ratio*node.r
        r = node.r
        self.diameter[index] = r
        self._update_attribute(self._list_for_local_update(node))
        self.set_features()

    def rescale_toward_end(self,node, rescale):
        """
        Rescale the part of neuron form the node toward the end nodes.
        input
        -----
        node : `Node` class
            the node of the neuron which the location of other nodes in the neuron have it as thier decendent would be changed.
            rescale : positive float
            The value to rescale the part of the neuron.
        """
        index = self.get_index_for_no_soma_node(node)
        (I,) = np.where(~np.isnan(self.connection[:,index]))
        A = self.location[:,I]
        loc = self.xyz(node)
        A[0,:] = A[0,:] - loc[0]
        A[1,:] = A[1,:] - loc[1]
        A[2,:] = A[2,:] - loc[2]
        A = rescale*A
        A[0,:] = A[0,:] + loc[0]
        A[1,:] = A[1,:] + loc[1]
        A[2,:] = A[2,:] + loc[2]
        self.location[:,I] = A
        self.set_distance_from_root()
        self.set_distance_from_parent()
        I = I.tolist()
        I.remove(index)
        I = np.array(I,dtype = int)
        self.connection[:,I] *= rescale
        self.set_branch_angle()
        self.set_global_angle()
        self.set_local_angle()
        self.set_features()

    def rotate(self, node, matrix):
        """
        Rotate the neuron around the parent of the node with the given matrix.
        The attribute to update:
            location
            distance_from_root
            branch_angle
            angle_global
            local_angle
        """
        # set of nodes under parent of node
        par = node.parent
        (I,) = np.where(~np.isnan(self.connection[:,self.get_index_for_no_soma_node(par)]))
        A = self.location[:,I]
        loc = self.xyz(par)
        A[0,:] = A[0,:] - loc[0]
        A[1,:] = A[1,:] - loc[1]
        A[2,:] = A[2,:] - loc[2]
        A = np.dot(matrix, A)
        A[0,:] = A[0,:] + loc[0]
        A[1,:] = A[1,:] + loc[1]
        A[2,:] = A[2,:] + loc[2]
        self.location[:,I] = A
        self.set_distance_from_root()
        self.set_branch_angle()
        self.set_global_angle()
        self.set_local_angle()
        self.set_features()

    def rotate_from_branch(self, node, matrix):
        branch_index = self.get_index_for_no_soma_node(node.parent)
        (I,) = np.where(~np.isnan(self.connection[:,self.get_index_for_no_soma_node(node)]))
        #I = np.append(I, branch_index)
        A = self.location[:,I]
        loc = self.xyz(node.parent)
        A[0,:] = A[0,:] - loc[0]
        A[1,:] = A[1,:] - loc[1]
        A[2,:] = A[2,:] - loc[2]
        A = np.dot(matrix, A)
        A[0,:] = A[0,:] + loc[0]
        A[1,:] = A[1,:] + loc[1]
        A[2,:] = A[2,:] + loc[2]
        self.location[:,I] = A
        self.set_distance_from_root()
        self.set_branch_angle()
        self.set_global_angle()
        self.set_local_angle()
        self.set_features()

    def remove_node(self, index):
        """
        Removes a non-soma node from the neuron and updates the features

        Parameters
        ----------
        Node : the index of the node in the no_soma_list
            the node should be one of the end-points, otherwise gives an error
        """
        self.n_node -= 1
        node = self.nodes_list[index]
        parent_index = self.get_index_for_no_soma_node(node.parent)
        # details of the removed node for return
        p = node.parent
        node.parent.remove_child(node)
        l = self.location[:,index] - self.location[:,parent_index]
        r = self.diameter[index]/self.diameter[parent_index]
        self.location = np.delete(self.location,index, axis = 1)
        self.nodes_list.remove(node)
        self.branch_order = np.delete(self.branch_order,index)
        new_parent_index = self.get_index_for_no_soma_node(p)
        self.branch_order[new_parent_index] -= 1

        self.diameter = np.delete(self.diameter,index)
        #self.frustum = np.delete(self.frustum,index)
        #self.rall_ratio = np.delete(self.rall_ratio,index)
        self.distance_from_root = np.delete(self.distance_from_root,index)
        self.distance_from_parent = np.delete(self.distance_from_parent,index)
        #self.slope = np.delete(self.slope,index)
        self.branch_angle = np.delete(self.branch_angle,index, axis = 1)
        self.global_angle = np.delete(self.global_angle,index)
        self.local_angle = np.delete(self.local_angle,index)
        self.connection = np.delete(self.connection,index, axis = 0)
        self.connection = np.delete(self.connection,index, axis = 1)

        self.parent_index = np.delete(self.parent_index,index)
        I = np.where(self.parent_index > index)
        self.parent_index[I] -= 1
        self.child_index = np.delete(self.child_index,index,axis = 1)
        I , J = np.where(self.child_index  > index)
        self.child_index[I,J] -= 1
        if p.type is not 'soma':
            if len(p.children) == 1:
                self.branch_angle[0,new_parent_index] = np.nan
                self.branch_angle[1,new_parent_index] = np.nan
                self.branch_angle[2,new_parent_index] = np.nan
                gol, loc = self.calculate_node_angles(self.nodes_list[new_parent_index])
                self.child_index[:,new_parent_index] = np.array([self.get_index_for_no_soma_node(p.children[0]), np.nan])
                self.local_angle[new_parent_index] = loc
            if len(p.children) == 0:
                self.local_angle[new_parent_index] = np.nan
                self.child_index[:,new_parent_index] = np.array([np.nan, np.nan])

        #self.sholl_r = None # the position of the jumps for sholl analysis
        #self.sholl_n = None # the value at the jumping (the same size as self.sholl_x)
        self.set_ext_red_list()
        self.set_features()
        return p, l, r

    def extend_node(self,parent,location,ratio):
        """
        Extend the neuron by adding one end point and updates the attribute for the new neuron.
        Parameters:
        ___________
        Parent: the node that the extended node attached to

        location: the xyz of new node is the sum of location and xyz of parent

        ratio: the radius of new node is the radius of parent times ratio
        """
        self.n_node += 1
        if parent is 'soma':
            parent = self.root
        n = Node()
        in_par = self.get_index_for_no_soma_node(parent)
        n.type = 'apical'
        R = ratio * self.diameter[in_par]
        n.parent = parent
        parent.add_child(n)

        self.location = np.append(self.location, (self.location[:,in_par] + location).reshape([3,1]), axis = 1)
        self.diameter = np.append(self.diameter, R)
        self.nodes_list.append(n)
        #self.frustum = np.append(self.frustum,np.nan)
        self.branch_order = np.append(self.branch_order ,0)
        self.branch_order[self.get_index_for_no_soma_node(parent)] += 1
        #self.rall_ratio = np.append(self.rall_ratio ,np.nan)
        self.distance_from_root = np.append(self.distance_from_root,np.nan)
        self.distance_from_parent = np.append(self.distance_from_parent ,np.nan)
        #self.slope = np.append(self.slope ,np.nan)
        if(self.branch_angle.shape[1] == 0):
            self.branch_angle = np.nan*np.ones([3,1])
        else:
            self.branch_angle = np.append(self.branch_angle, np.nan*np.ones([3,1]), axis = 1)
        self.global_angle = np.append(self.global_angle ,np.nan)
        self.local_angle = np.append(self.local_angle ,np.nan)

        l = self.connection.shape[0]
        I = np.nan*np.zeros([1,l])
        (J,) = np.where(~np.isnan(self.connection[self.get_index_for_no_soma_node(parent),:]))
        I[0,J] = self.connection[self.get_index_for_no_soma_node(parent),J]
        self.connection = np.append(self.connection, I , axis = 0)
        self.connection = np.append(self.connection, np.nan*np.zeros([l+1,1]), axis = 1)
        self.connection[l,l] = LA.norm(location,2)

        self.parent_index = np.append(self.parent_index, self.get_index_for_no_soma_node(parent))
        self.child_index = np.append(self.child_index,np.array([np.nan, np.nan]).reshape(2,1), axis = 1)
        if parent.type is not 'soma':
            if(len(parent.children) == 1):
                self.child_index[:,self.get_index_for_no_soma_node(parent)] = np.array([self.get_index_for_no_soma_node(n), np.nan])
            if(len(parent.children) == 2):
                self.child_index[1,self.get_index_for_no_soma_node(parent)] = self.get_index_for_no_soma_node(n)
        update_list = self._list_for_local_update(n)
        self._update_attribute(update_list)

        self.set_ext_red_list()
        self.set_features()
        return self.get_index_for_no_soma_node(n)

    def add_extra_node(self, node):
         print (1)

    def slide(self, moving_node_index, no_branch_node_index):
        """

        """
        # adjust nodes
        moving_node = self.nodes_list[moving_node_index]
        no_branch_node = self.nodes_list[no_branch_node_index]
        parent_moving_node = moving_node.parent
        parent_moving_node_index = self.get_index_for_no_soma_node(parent_moving_node)
        parent_moving_node.remove_child(moving_node)
        moving_node.parent = no_branch_node
        no_branch_node.add_child(moving_node)

        # adjust parent_index and child_index
        self.parent_index[moving_node_index] = no_branch_node_index
        a = self.child_index[:,parent_moving_node_index]
        if(self.branch_order[parent_moving_node_index] == 2):
            if(a[0] == moving_node_index):
                self.child_index[:,parent_moving_node_index] = np.array([a[1],np.nan])
            if(a[1] == moving_node_index):
                self.child_index[:,parent_moving_node_index] = np.array([a[0],np.nan])
        if(self.branch_order[parent_moving_node_index] == 1):
            self.child_index[:,parent_moving_node_index] = np.array([np.nan,np.nan])
        self.branch_order[parent_moving_node_index] -= 1

        #self.set_parent()

        if(self.branch_order[no_branch_node_index] == 1):
            a = self.child_index[:,no_branch_node_index]
            self.child_index[:,no_branch_node_index] = np.array([a[0],moving_node_index])
        if(self.branch_order[no_branch_node_index] == 0):
            self.child_index[:,no_branch_node_index] = np.array([moving_node_index,np.nan])
        self.branch_order[no_branch_node_index] += 1
        # adjust location
        (segment,) = np.where(~np.isnan(self.connection[:,moving_node_index]))
        self.location[0,segment] +=  self.location[0,no_branch_node_index] - self.location[0,parent_moving_node_index]
        self.location[1,segment] +=  self.location[1,no_branch_node_index] - self.location[1,parent_moving_node_index]
        self.location[2,segment] +=  self.location[2,no_branch_node_index] - self.location[2,parent_moving_node_index]

        # adjust connection
        (up_ind,) = np.where(~np.isnan(self.connection[parent_moving_node_index,:]))
        self.connection[np.ix_(segment,up_ind)] = np.nan
        (down_ind,) = np.where(~np.isnan(self.connection[no_branch_node_index,:]))
        a = self.distance_from_parent[down_ind].reshape([1,len(down_ind)])
        A = np.repeat(a,len(segment),axis = 0)
        self.connection[np.ix_(segment,down_ind)] = A

        self.set_ext_red_list()
        self.set_distance_from_root()
        self.set_distance_from_parent()
        self.set_branch_angle()
        self.set_global_angle()
        self.set_local_angle()
        #self.set_frustum()
        self.set_features()

    def horizental_stretch(self, node_index, parent_node, scale):
        (up,) = np.where(~np.isnan(self.connection[node_index,:]))
        (down,) = np.where(~np.isnan(self.connection[:,parent_node]))
        I = np.intersect1d(up,down)
        A = self.location[:,I]
        loc = self.location[:,parent_node]
        A[0,:] = A[0,:] - loc[0]
        A[1,:] = A[1,:] - loc[1]
        A[2,:] = A[2,:] - loc[2]
        r = self.location[:,node_index] - loc
        r = r/LA.norm(r,2)
        A = scale*A +(1-scale)*(np.dot(np.expand_dims(r,axis = 1),np.expand_dims(np.dot(r,A),axis = 0)))
        A[0,:] = A[0,:] + loc[0]
        A[1,:] = A[1,:] + loc[1]
        A[2,:] = A[2,:] + loc[2]
        self.location[:,I] = A
        self.set_distance_from_root()
        self.set_distance_from_parent()
        for i in I:
            (J,) = np.where(~np.isnan(self.connection[:,i]))
            self.connection[J,i] = self.distance_from_parent[i]
        self.set_branch_angle()
        self.set_global_angle()
        self.set_local_angle()
        self.set_features()

    def vertical_stretch(self, node_index, parent_node, scale):
        (up,) = np.where(~np.isnan(self.connection[node_index,:]))
        (down,) = np.where(~np.isnan(self.connection[:,parent_node]))
        I = np.intersect1d(up,down)
        A = self.location[:,I]
        loc = self.location[:,parent_node]
        A[0,:] = A[0,:] - loc[0]
        A[1,:] = A[1,:] - loc[1]
        A[2,:] = A[2,:] - loc[2]
        r = self.location[:,node_index] - loc
        new_loc = -(1-scale)*(r)
        r = r/LA.norm(r,2)
        A = A -(1-scale)*(np.dot(np.expand_dims(r,axis = 1),np.expand_dims(np.dot(r,A),axis = 0)))
        A[0,:] = A[0,:] + loc[0]
        A[1,:] = A[1,:] + loc[1]
        A[2,:] = A[2,:] + loc[2]
        self.location[:,I] = A
        (T,) = np.where(~np.isnan(self.connection[:,node_index]))
        T = list(T)
        T.remove(node_index)
        A = self.location[:,T]
        A[0,:] += new_loc[0]
        A[1,:] += new_loc[1]
        A[2,:] += new_loc[2]
        self.location[:,T] = A
        self.set_distance_from_root()
        self.set_distance_from_parent()
        T = np.array(T)
        for i in np.append(T,I):
            (J,) = np.where(~np.isnan(self.connection[:,i]))
            self.connection[J,i] = self.distance_from_parent[i]
        self.set_branch_angle()
        self.set_global_angle()
        self.set_local_angle()
        self.set_features()

    """ -------------
    def sinusidal(self, node_index, parent_index, hight, n_vertical, n_horizental):
       
      #  NOT READY
       
        (up,) = np.where(~np.isnan(self.connection[node_index,:]))
        (down,) = np.where(~np.isnan(self.connection[:,parent_node]))
        I = np.intersect1d(up,down)
        A = self.location[:,I]
        loc = self.location[:,parent_node]

        A[0,:] = A[0,:] - loc[0]
        A[1,:] = A[1,:] - loc[1]
        A[2,:] = A[2,:] - loc[2]
        r = self.location[:,node_index] - loc
        new_loc = -(1-scale)*(r)
        r = r/LA.norm(r,2)
        A = A -(1-scale)*(np.dot(np.expand_dims(r,axis = 1),np.expand_dims(np.dot(r,A),axis = 0)))
        A[0,:] = A[0,:] + loc[0]
        A[1,:] = A[1,:] + loc[1]
        A[2,:] = A[2,:] + loc[2]
        self.location[:,I] = A
        (T,) = np.where(~np.isnan(self.connection[:,node_index]))
        T = list(T)
        T.remove(node_index)
        A = self.location[:,T]
        A[0,:] += new_loc[0]
        A[1,:] += new_loc[1]
        A[2,:] += new_loc[2]
        self.location[:,T] = A
        self.set_distance_from_root()
        self.set_distance_from_parent()
        T = np.array(T)
        for i in np.append(T,I):
            (J,) = np.where(~np.isnan(self.connection[:,i]))
            self.connection[J,i] = self.distance_from_parent[i]
        self.set_branch_angle()
        self.set_global_angle()
        self.set_local_angle()
        self.set_features()
    """
    def get_root(self):

        """
        Obtain the root Node

        Returns
        -------
        root : :class:`Node`
        """
        return self.__root

    def is_root(self, node):

        """
        Check whether a Node is the root Node

        Parameters
        -----------
        node : :class:`Node`
            Node to be check if root

        Returns
        --------
        is_root : boolean
            True is the queried Node is the root, False otherwise
        """
        if node.parent is None:
            return True
        else:
            return False

    def is_leaf(self, node):

        """
        Check whether a Node is a leaf Node, i.e., a Node without children

        Parameters
        -----------
        node : :class:`Node`
            Node to be check if leaf Node

        Returns
        --------
        is_leaf : boolean
            True is the queried Node is a leaf, False otherwise
        """
        if len(node.children) == 0:
            return True
        else:
            return False

    def is_branch(self, node):

        """
        Check whether a Node is a branch Node, i.e., a Node with two children

        Parameters
        -----------
        node : :class:`Node`
            Node to be check if branch Node

        Returns
        --------
        is_leaf : boolean
            True is the queried Node is a branch, False otherwise
        """

        if len(node.children) == 2:
            return True
        else:
            return False

    def find_root(self, node):
        if node.parent is not None:
            node = self.find_root(node.parent)
        return node

    def add_node_with_parent(self, node, parent):

        """
        Add a Node to the tree under a specific parent Node

        Parameters
        -----------
        node : :class:`Node`
            Node to be added
        parent : :class:`Node`
            parent Node of the newly added Node
        """
        node.parent = parent
        if parent is not None:
            parent.add_child(node)
        self.add_node(node)

    def add_node(self, node):
        self.nodes_list.append(node)

    def read_swc(self, input_file):

        """
        Read the swc file and fill the attributes accordingly.
        The assigned attributes are:
            n_soma
            n_node
            nodes_list
            location
            type
            diameter
            parent_index
            child_index
        """

        self.n_soma = 0
        self.nodes_list = []
        self.location = np.array([0, 0, 0] ).reshape(3,1)
        self.type = 1
        self.parent_index = np.array([0])
        child_index = lil_matrix((2,1000000))
        f = open(input_file, 'r',encoding='utf-8')
        print('start read a swc file')
        B = True
        try:
            for line in f:
                #print("a line")
                if line == '\n':
                    break
                if not line.startswith('#'):
                    split = line.split()
                    # print(split[0].rstrip(), split[6].rstrip())
                    index = int(split[0].rstrip())
                    swc_type = int(split[1].rstrip())
                    x = float(split[2].rstrip())
                    y = float(split[3].rstrip())
                    z = float(split[4].rstrip())
                    radius = float(split[5].rstrip())
                    parent_index = int(split[6].rstrip())

                    if(parent_index == -1):
                        self.n_soma += 1
                        x_root = x
                        y_root = y
                        z_root = z
                        self.diameter = radius
                    else:
                        if(swc_type == 1):
                            self.n_soma += 1
                        self.location = np.append(self.location, np.array([x - x_root, y - y_root, z - z_root]).reshape(3,1), axis = 1)
                        self.diameter = np.append(self.diameter, radius)
                        self.type = np.append(self.type, swc_type)
                        self.parent_index = np.append(self.parent_index, parent_index - 1)
                        if(parent_index != 1):
                            # print(index-1, parent_index)
                            if(child_index[0,parent_index-1]==0):
                                child_index[0,parent_index-1] = index-1
                            else:
                                child_index[1,parent_index-1] = index-1

                    node = Node()
                    node.xyz = np.array([x,y,z])
                    node.r = np.array([radius])
                    node.set_type(swc_type)
                    if(parent_index == -1):
                        self.add_node(node)
                        self.root = node
                    else:
                        # print(len(self.nodes_list))
                        self.add_node_with_parent(node,self.nodes_list[parent_index-1])
            self.n_node = len(self.nodes_list)
            a = child_index[:,0:self.n_node]
            #a = a -1
            a = a.toarray()
            a[a==0] = np.nan
            self.child_index = a
            f.close()
        except:
            print('deleted Neuron')
            print('unexpected error:', sys.exc_info())



    def read_swc_matrix(self, input_file):
        """
        Read the an swc matrix and fill the attributes accordingly.
        The assigned attributes are:
            n_soma
            n_node
            nodes_list
            location
            type
            diameter
            parent_index
            child_index
        """

        self.n_soma = 0
        self.nodes_list = []
        self.location = np.array([0, 0, 0]).reshape(3, 1)
        self.type = 1
        self.parent_index = np.array([0])
        child_index = lil_matrix((2, 1000000))  # Compressed Sparse Row matrix,construct an empty matrix with shape (2, 1000000)
        n_node = input_file.shape[0]  # input_files M: n_nodes * 7
        for line in range(n_node):
            index = input_file[line, 0]
            swc_type = input_file[line, 1]
            x = input_file[line, 2]
            y = input_file[line, 3]
            z = input_file[line, 4]
            radius = input_file[line, 5]
            parent_index = int(input_file[line, 6])
            if(parent_index == -1):  # root soma
                self.n_soma += 1
                x_root = x
                y_root = y
                z_root = z
                self.diameter = radius
            else:
                if(swc_type == 1):  # other soma
                    self.n_soma += 1
                # not soma nodes
                self.location = np.append(self.location, np.array([x - x_root, y - y_root, z - z_root]).reshape(3, 1), axis = 1)  # node to root manhaton distance
                self.diameter = np.append(self.diameter, radius)  # list diameter
                self.type = np.append(self.type, swc_type)  # list type
                self.parent_index = np.append(self.parent_index, parent_index - 1)
                if(parent_index != 1):  # count the index of child, expect root soma
                    if(child_index[0, parent_index-1] == 0):
                        child_index[0, parent_index-1] = index-1  # parent add child
                    else:
                        child_index[1, parent_index-1] = index-1

            node = Node()
            node.xyz = np.array([x, y, z])
            node.r = np.array([radius])
            node.set_type(swc_type)
            if(parent_index == -1):  # add soma
                self.add_node(node)
                self.root = node
            else:
                #print(parent_index)
                self.add_node_with_parent(node, self.nodes_list[parent_index-1])
        self.n_node = len(self.nodes_list)
        print('n_node:', self.n_node)
        a = child_index[:, 0:self.n_node]  # 2*n_node, first child, second child
        a = a.toarray()
        a[a  == 0] = np.nan
        print("size of soma: ", self.n_soma)
        self.child_index = a


    def get_swc(self):
        swc = np.zeros([self.n_node, 7])
        remain = [self.root]
        index = np.array([-1])
        for i in range(self.n_node):
            n = remain[0]
            swc[i,0] = i+1
            swc[i,1] = n.set_type_from_name()
            ind = self.get_index_for_no_soma_node(n)
            if(ind > self.n_soma):
                swc[i,2] = self.location[0,ind]
                swc[i,3] = self.location[1,ind]
                swc[i,4] = self.location[2,ind]
                swc[i,5] = self.diameter[ind]
                swc[i,6] = index[0]
            else:
                swc[i,2] = n.xyz[0]
                swc[i,3] = n.xyz[1]
                swc[i,4] = n.xyz[2]
                swc[i,5] = n.r
                swc[i,6] = 1
            for m in n.children:
                remain.append(m)
                index = np.append(index,i+1)
            remain = remain[1:]
            index = index[1:]
        swc[0,6] = -1
        return swc

    def write_swc(self, input_file):

        """
        Used to write an SWC file from a morphology stored in this
        :class:`Neuron`.

        """
        writer = open(input_file, 'w')
        swc = self.get_swc()
        for i in range(swc.shape[0]):
            string = (str(swc[i,0])+' '+str(swc[i,1]) + ' ' + str(swc[i,2]) +
                          ' ' + str(swc[i,3]) + ' ' + str(swc[i,4]) +
                          ' ' + str(swc[i,5]) + ' ' + str(swc[i,6]))
            writer.write(string + '\n')
            writer.flush()
        writer.close()

    def get_random_branching_or_end_node(self):
        (b,) = np.where(self.branch_order[self.n_soma:] == 2)
        (e,) = np.where(self.branch_order[self.n_soma:] == 0)
        I = np.append(b,e)
        if(len(I) == 0):
            n = Node()
            n.type = 'empty'
        else:
            I += self.n_soma
            i = np.floor(len(I)*np.random.rand())
            n = self.nodes_list[I[i]]
        return n

    def get_random_no_soma_node(self):
        l = self.n_node - self.n_soma
        return self.nodes_list[(np.floor(l*np.random.rand()) + self.n_soma).astype(int)]

    def get_random_branching_node(self):
        """
        Return one of the branching point in the neuron.
        dependency:
            self.branch_order
            self.nodes_list
            self.n_soma
        """
        (I,) = np.where(self.branch_order[self.n_soma:] == 2)
        if(len(I) == 0):
            n = Node()
            n.type = 'empty'
        else:
            I += self.n_soma
            i = np.floor(len(I)*np.random.rand())
            n = self.nodes_list[I[i]]
        return n

    def get_random_order_one_node_not_in_certain_index(self, index):
        """
        Return one of the order one point in the neuron.
        dependency:
            self.branch_order
            self.nodes_list
            self.n_soma
        """
        (I,) = np.where(self.branch_order == 1)
        I = I[I>=self.n_soma]
        I = np.setdiff1d(I,index)
        if(len(I) == 0):
            n = Node()
            n.type = 'empty'
        else:
            i = np.floor(len(I)*np.random.rand())
            n = self.nodes_list[I[i]]
        return n

    def get_random_non_branch_node_not_in_certain_index(self, index):
        """
        Return one of the order one point in the neuron.
        dependency:
            self.branch_order
            self.nodes_list
            self.n_soma
        """
        (I,) = np.where(self.branch_order != 2)
        I = I[I>=self.n_soma]
        I = np.setdiff1d(I,index)
        if(len(I) == 0):
            n = Node()
            n.type = 'empty'
        else:
            i = np.floor(len(I)*np.random.rand())
            n = self.nodes_list[I[i]]
        return n

    def is_soma(self):
        if(self.n_node == self.n_soma):
            return True
        else:
            return False

    def set_nodes_values(self):
        i = 0
        for n in self.nodes_list:
            n.xyz = self.location[:,i]
            n.r = self.diameter[i]
            i += 1

    def show_features(self,size_x = 15,size_y = 17 ,bin_size = 20):
        n = 6
        m = 2

        plt.figure(figsize=(size_x,size_y))

        plt.subplot(n,m,1)
        a = self.global_angle
        b = plt.hist(a[~np.isnan(a)],bins = bin_size,color = 'g')
        #plt.xlabel('angle (radian)')
        plt.ylabel('density')
        plt.title('Global angles')

        plt.subplot(n,m,2)
        a = self.local_angle
        b = plt.hist(a[~np.isnan(a)],bins = bin_size,color = 'g')
        #plt.xlabel('angle (radian)')
        plt.ylabel('density')
        plt.title('Local angles')

        plt.subplot(n,m,3)
        plt.title('Neuronal/Euclidian distance from root')
        a = self.features['ratio_euclidian_neuronal']
        plt.hist(a[~np.isnan(a)],bins = bin_size ,color = 'g')
        #plt.xlabel('ratio')
        plt.ylabel('density')

        plt.subplot(n,m,4)
        plt.hist(self.distance_from_parent,bins = bin_size,color = 'g')
        plt.title('Distance from parent')
        #plt.xlabel('distance (um)')
        plt.ylabel('density')

        plt.subplot(n,m,5)
        plt.hist(self.distance_from_root,bins = bin_size)
        #plt.xlabel('distance (um)')
        plt.ylabel('density')
        plt.title('Distance from soma')

        plt.subplot(n,m,6)
        a = self.features['branch_angle']
        plt.hist(a[~np.isnan(a)],bins = bin_size)
        #plt.xlabel('angle (radian)')
        plt.ylabel('density')
        plt.title('Angle at the branching points')

        plt.subplot(n,m,7)
        a = self.features['curvature']
        plt.hist(a,bins = bin_size)
        #plt.xlabel('angle (radian)')
        plt.ylabel('density')
        plt.title('curvature')

        plt.subplot(n,m,8)
        a = self.features['neural_important']
        plt.hist(a,bins = bin_size)
        #plt.xlabel('angle (radian)')
        plt.ylabel('density')
        plt.title('lenght of neural segments')

        plt.subplot(n,m,9)
        a = self.features['ratio_neural_euclidian_important']
        plt.hist(a,bins = bin_size)
        #plt.xlabel('angle (radian)')
        plt.ylabel('density')
        plt.title('ratio of neural to euclidian distance for segments')
        #fig, ax = plt.subplots(n,m,6)

        plt.subplot(n,m,10)
        ind = np.arange(4)
        width = 0.35
        plt.bar(ind,(self.n_node,self.features['Nbranch'],self.features['initial_segments'],self.features['discrepancy_space']),color='r');
        #plt.title('Numberical features')
        #plt.set_xticks(ind + width)
        plt.xticks(ind,('Nnodes', 'Nbranch', 'Ninitials', 'discrepancy'))

class Node(object):

    """
    Node class for each nodes in the Neuron class.
    each node has parent (another node), children (None, one or more nodes), radius,
    Euclidian cordinates and type

    The children are in a list
    """

    def __init__(self):

        self.parent = None
        self.children = []
        self.r = np.array([0.])
        self.xyz = np.array([0.,0.,0.])
        self.type = None # it can be soma, dendrite, axon, basal, apical

    def get_parent(self):

        """
        Return the parent Node of this one.

        Returns
        -------
        parent : :class:`Node`
           In case of the root, None is returned. Otherwise a :class:`Node` is
            returned
        """
        return self.parent

    def set_parent(self, parent):

        """
        Set the parent Node of a given other Node

        Parameters
        ----------
        Node : :class:`Node`
        """
        self.__parent = parent

    def get_children(self):

        """
        Return the children nodes of this one (if any)

        Returns
        -------
        children : list :class:`Node`
           In case of a leaf an empty list is returned
        """
        return self.__children

    def set_children(self, children):

        """
        Set the children nodes of this one

        Parameters
        ----------

        children: list :class:`Node`
        """
        self.__children = children

    def get_radius(self):

        """
        Returns
        -------
        radius : float
        """
        return self.r

    def set_radius(self, radius):
        self.r = radius

    def getxyz(self):

        """
        Returns
        -------
        radius : float
        """
        return self.xyz

    def setxyz(self, xyz):
        self.xyz = xyz

    def set_type(self,index):
        if(index == 0):
            self.type = 'undefined'
        elif(index == 1):
            self.type = 'soma'
        elif(index == 2):
            self.type = 'axon'
        elif(index == 3):
            self.type = 'basal'
        elif(index == 4):
            self.type = 'apical'

    def set_type_from_name(self):
        if(self.type == 'undefined'):
            return 0
        if(self.type == 'soma'):
            return 1
        if(self.type == 'axon'):
            return 2
        if(self.type == 'basal'):
            return 3
        if(self.type == 'apical'):
            return 4

    def add_child(self, child_node):

        """
        add a child to the children list of a given Node

        Parameters
        -----------
        Node :  :class:`Node`
        """
        self.children.append(child_node)

    def remove_child(self, child):
        """
        Remove a child Node from the list of children of a specific Node

        Parameters
        -----------
        Node :  :class:`Node`
            If the child doesn't exist, you get into problems.
        """
        self.children.remove(child)
