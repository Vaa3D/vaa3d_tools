from keras import backend as K


def masked_softmax(input_layer,n_nodes, batch_size):
    """
    A Lambda layer to mask a matrix of outputs to be lower-triangular.
    Each row must sum up to one. We apply a lower triangular mask of ones
    and then add an upper triangular mask of a large negative number.
    :param input_layer:
    :param n_nodes:
    :param batch_size:
    :return:
    """
    mask_lower = K.theano.tensor.tril(K.ones((n_nodes, n_nodes)))
    mask_upper = \
        K.theano.tensor.triu(-100. * K.ones((n_nodes, n_nodes)), 1)
    mask_layer = mask_lower * input_layer + mask_upper
    mask_layer = mask_layer + 0 * K.eye(n_nodes)[0:n_nodes, 0:n_nodes]
    mask_layer = \
        K.reshape(mask_layer, (batch_size * n_nodes, n_nodes))
    softmax_layer = K.softmax(mask_layer)
    output_layer = K.reshape(softmax_layer, (batch_size, n_nodes, n_nodes))
    return output_layer


def full_matrix(adjaceny, n_nodes):
    return K.theano.tensor.nlinalg.matrix_inverse(K.eye(n_nodes)-adjaceny)


def batch_full_matrix(adjacency, n_nodes, batch_size):
    result, updates = \
        K.theano.scan(fn=lambda n: full_matrix(adjacency[n, :, :], n_nodes),
                      sequences=K.arange(batch_size))
    return result


def feature_extractor(inputs,
                      n_nodes,
                      batch_size):
    """
    Compute various features and concatenate them.
    Parameters
    ----------
    morphology_input: keras layer object
        (batch_size x n_nodes - 1 x n_nodes)
        the adjacency matrix of each sample.
    geometry_input: keras layer object
        (batch_size x n_nodes - 1 x 3)
        the locations of each nodes.
    n_nodes: int
        number of nodes
    batch_size: int
        batch size
    Returns
    -------
    features: keras layer object
        (batch_size x n_nodes x n_features)
        The features currently supports:
            - The adjacency
            - The full adjacency
            - locations
            - distance from imediate parents
    """
    geometry_input = inputs[:, :, :3]
    morphology_input = inputs[:, :, 3:]

    # adjacency = \
    #     K.concatenate([K.zeros(shape=(batch_size, 1, n_nodes)),
    #                    morphology_input], axis=1)  # add soam
    adjacency = morphology_input
    full_adjacency = \
        batch_full_matrix(adjacency, n_nodes, batch_size)
    # geometry_input = K.concatenate([K.zeros(shape=(batch_size, 1, 3)),
    #                                 geometry_input], axis=1)

    # distance = distance_from_parent(adjacency,
    #                                 geometry_input,
    #                                 n_nodes,
    #                                 batch_size)

    # distance = locations_by_distance_from_parent(full_adjacency=full_adjacency,
    #                                              distance_from_parent=geometry_input,
    #                                              batch_size=batch_size)
    #
    filled_full_adjacency_x = \
        full_adjacency*K.repeat_elements(K.expand_dims(geometry_input[:, :, 0], 2), n_nodes, axis=2)
    filled_full_adjacency_y = \
        full_adjacency*K.repeat_elements(K.expand_dims(geometry_input[:, :, 1], 2), n_nodes, axis=2)
    filled_full_adjacency_z = \
        full_adjacency*K.repeat_elements(K.expand_dims(geometry_input[:, :, 2], 2), n_nodes, axis=2)

    features = K.concatenate([adjacency,
                              full_adjacency,
                              geometry_input,
                              filled_full_adjacency_x,
                              filled_full_adjacency_y,
                              filled_full_adjacency_z], axis=2)
    return features
