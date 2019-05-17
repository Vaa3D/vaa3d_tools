# -*- coding:UTF-8 -*-
import numpy as np
import Neuron as Neuron
from Neuron import Node as Node
from numpy import linalg as LA

def get_main_points(neuron):
    """
    Returning the index of branching points and end points.

    Parameters
    ----------
    neuron: Neuron
        input neuron

    Returns
    -------
    selected_index: array
        the list of main point; branching points and end points
    """
    (branch_index,) = np.where(neuron.branch_order[neuron.n_soma:] == 2)
    (endpoint_index,) = np.where(neuron.branch_order[neuron.n_soma:] == 0)
    selected_index = np.union1d(branch_index + neuron.n_soma,
                                endpoint_index + neuron.n_soma)
    selected_index = np.append(range(neuron.n_soma), selected_index)
    return selected_index


def straigh_subsample(neuron, num, distance):
    """
    Subsampling a neuron from original neuron. It has all the main points of the original neuron,
    i.e endpoints or branching nodes, are not changed and meanwhile the distance of two consecutive nodes
    of subsample neuron is around the 'distance'.
    for each segment between two consecuative main points, a few nodes from the segment will be added to the selected node;
    it starts from the far main point, and goes on the segment toward the near main point. Then the first node which is
    going to add has the property that it is the farest node from begining on the segment such that its distance from begining is
    less than 'distance'. The next nodes will be selected similarly. this procesure repeat for all the segments.

    Parameters
    ----------
    distance: float
        the mean distance between pairs of consecuative nodes.

    Returns
    -------
    Neuron: the subsampled neuron
    """

    # Selecting the main points: branching nodes and end nodes
    selected_index = get_main_points(neuron)

    #print(num)


    # for each segment between two consecuative main points, a few nodes from the segment will be added to the selected node.
    # These new nodes will be selected base on the fact that neural distance of two consecuative nodes is around 'distance'.
    # Specifically, it starts from the far main point, and goes on the segment toward the near main point. Then the first node which is
    # going to add has the property that it is the farest node from begining on the segment such that its distance from begining is
    # less than 'distance'. The next nodes will be selected similarly.

    for i in selected_index:
        upList = np.array([i], dtype = int)
        index = neuron.parent_index[i]
        dist = neuron.distance_from_parent[i]
        while(~np.any(selected_index == index)):  # find selected_index & it's parent index in selected_index
            upList = np.append(upList, index)
            index = neuron.parent_index[index]
            dist = np.append(dist, sum(neuron.distance_from_parent[upList]))
        dist = np.append(0, dist)
        (I,) = np.where(np.diff(np.floor(dist/distance)) > 0)  # select node be added
        I = upList[I]
        selected_index = np.append(selected_index, I)
    selected_index = np.unique(selected_index)
    print("main_node_index", selected_index.shape[0])
    if (selected_index.shape[0] < num):
        print('exit to straight')
        return neuron
    neuron = neuron_with_selected_nodes(neuron, selected_index)
    return neuron

def neuron_with_selected_nodes(neuron, selected_index):
    """
    Giving back a new neuron made up with the selected_index nodes of self.
    if node A is parent (or grand parent) of node B in the original neuron,
    it is the same for the new neuron.

    Parameters
    ----------
    selected_index: numpy array
        the index of nodes from original neuron for making new neuron.

    Returns
    -------
    Neuron: the subsampled neuron.
    """
    parent = parent_id(neuron, selected_index)
    # making the list of nodes
    n_list = []
    for i in range(selected_index.shape[0]):
        n = Node()
        n.xyz = neuron.nodes_list[selected_index[i]].xyz
        n.r = neuron.nodes_list[selected_index[i]].r
        n.type = neuron.nodes_list[selected_index[i]].type
        n_list.append(n)
    # adjusting the childern and parents for the nodes.
    for i in np.arange(1, selected_index.shape[0]):
        j = parent[i]
        n_list[i].parent = n_list[j]
        n_list[j].add_child(n_list[i])
    # print(n_list)
    # print("***********")
    print("after straight the number of nodes", len(n_list))
    return Neuron.Neuron(file_format='only list of nodes', input_file=n_list)


def parent_id(neuron, selected_index):
    """
    Return the parent id of all the selected_index of the neurons.

    Parameters
    ----------
    selected_index: numpy array
        the index of nodes

    Returns
    -------
    parent_id: the index of parent of each element in selected_index in
    this array.
    """
    length = len(neuron.nodes_list)
    selected_length = len(selected_index)
    adjacency = np.zeros([length,length])
    adjacency[neuron.parent_index[1:], range(1,length)] = 1
    full_adjacency = np.linalg.inv(np.eye(length) - adjacency)
    selected_full_adjacency = full_adjacency[np.ix_(selected_index,selected_index)]
    selected_adjacency = np.eye(selected_length) - np.linalg.inv(selected_full_adjacency)
    selected_parent_id = np.argmax(selected_adjacency, axis=0)
    return selected_parent_id


def straight_prune_subsample(neuron, number_of_nodes):
    """
    Subsampling a neuron with straightening and pruning. At the first step, it
    strighten the neuron with 200 nodes (if the number of nodes for the
    neuron is less than 200, it doesn't change it). Then the neuron is pruned
    with a twice the distance used for straightening. If the number of nodes
    is less than 'number_of_nodes' the algorithm stops otherwise it increases
    the previous distance by one number and does the same on the neuron.

    Parameters
    ----------
    neuron: Neuron
        input neuron
    number_of_nodes: int
        the number of nodes for the output neuron

    Returns
    -------
    sp_neuron: Neuron
        the subsample neuron after straightening and pruning.
    """
    l = sum(neuron.distance_from_parent)
    branch_number = len(np.where(neuron.branch_order[neuron.n_soma:] == 2))
    distance = l / (number_of_nodes - branch_number)
    print("distance:", distance)
    if(neuron.n_node<number_of_nodes):
        return None
    if(neuron.n_node > number_of_nodes+20):
        print("the number of nodes:", neuron.n_node)
        neuron = straigh_subsample(neuron, number_of_nodes,distance)
    sp_neuron = prune(neuron=neuron, number_of_nodes=number_of_nodes, threshold=2*distance)
    print("after prune the number of nodes:", len(sp_neuron.nodes_list))
    while(len(sp_neuron.nodes_list)>number_of_nodes):
        distance += 1
        sp_neuron = straigh_subsample(sp_neuron, number_of_nodes, distance)
        sp_neuron = prune(neuron=sp_neuron,
                                 number_of_nodes=number_of_nodes,
                                 threshold=2*distance)
        print("after prune2 the number of nodes:", len(sp_neuron.nodes_list))
    return sp_neuron


def prune(neuron,
          number_of_nodes,
          threshold):
    """
    Pruning the neuron. It removes all the segments that thier length is less
    than threshold unless the number of nodes becomes lower than lowest_number.
    In the former case, it removes the segments until the number of nodes is
    exactly the lowest_number.

    Parameters
    ----------
    neuron: Neuron
        input neuron.
    number_of_nodes: int
        the number of nodes for output neuron.

    Returns
    -------
    pruned_neuron: Neuron
        The pruned neuron.
    """

    n = len(neuron.nodes_list)
    # print("before prune the size of neuron:", n)
    for i in range(n - number_of_nodes):
        length, index = shortest_tips(neuron)
        if(length < threshold):
            neuron = remove_node(neuron, index)
        else:
            break
    neuron.set_distance_from_parent()
    return neuron


def remove_node(neuron, index):

    neuron.n_node -= 1
    node = neuron.nodes_list[index]
    parent_index = neuron.get_index_for_no_soma_node(node.parent)
    p = node.parent
    node.parent.remove_child(node)
    neuron.location = np.delete(neuron.location,index, axis = 1)
    neuron.nodes_list.remove(node)
    neuron.branch_order = np.delete(neuron.branch_order,index)
    new_parent_index = neuron.get_index_for_no_soma_node(p)
    neuron.branch_order[new_parent_index] -= 1
    neuron.parent_index = np.delete(neuron.parent_index,index)
    I = np.where(neuron.parent_index > index)
    neuron.parent_index[I] -= 1
    neuron.set_distance_from_parent()
    return neuron


def shortest_tips(neuron):
    """
    Returing the initial node of segment with the given end point.
    The idea is to go up from the tip.
    """
    (endpoint_index,) = np.where(neuron.branch_order[neuron.n_soma:] == 0)  # leaf node
    (branch_index,) = np.where(neuron.branch_order[neuron.n_soma:] == 2)
    selected_index = np.union1d(neuron.n_soma + endpoint_index,
                                neuron.n_soma + branch_index)
    selected_index = np.append(0, selected_index)
    par = parent_id(neuron, range(1,len(endpoint_index) + 1))
    dist = neuron.location[:, endpoint_index] - neuron.location[:, par]
    lenght = sum(dist**2, 2)
    index = np.argmin(lenght)

    return np.sqrt(min(lenght)), endpoint_index[index] + neuron.n_soma