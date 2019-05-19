import modelDCGAN as models
from keras.optimizers import RMSprop, Adagrad, Adam,SGD
import numpy as np
from neuron_dataset import *
import numpy.linalg as LA
import plot
import pickle
import h5py
from datetime import datetime
def save_swc(X_locations, X_parent,path,epoch,batch):
    """
    save generate swc
    :param X_locations:
    :param X_parent:
    :return:
    """
    locations = np.squeeze(X_locations)  # remove one dimension
    # print(locations)
    parent = np.squeeze(X_parent).argmax(axis=1) + 1  # argmax:Returns the indices of the maximum values along an axis.
    #print("parent : ",parent)
    full = np.zeros([parent.shape[0]+1, parent.shape[0]+1])
    full[range(1, parent.shape[0]+1), parent - 1] = 1  # full neighbor matrix ,parent = 1
    #print(full)
    full = LA.pinv(np.eye(parent.shape[0]+1) - full)  # pseudoinverse of a matrix,all parent
    # print(full)
    locations = np.dot(full, np.append(np.zeros([1, 3]), locations, axis=0)) # ?? dot:Dot product of two arrays, full * loactions(loactions add soma location)
    # print("locations: ",locations)
    M = np.zeros([parent.shape[0]+1, 7])
    M[:, 0] = np.arange(1, parent.shape[0]+2)
    M[0, 1] = 1
    M[1:, 1] = 3  # type
    M[:, 2:5] = locations*100
    M[:, 5] = 1.0
    M[1:, 6] = parent
    M[0, 6] = -1  # soma
    path = ('%s/epoch%s_batch%s.swc' % (path, epoch, batch))
    print(path, "\n")
    # write swc
    f = open(path, 'w')
    for i in range(M.shape[0]):
        f.write("{0} {1} {2} {3} {4} {5} {6}\n".format(int(M[i][0]), int(M[i][1]), np.float16(M[i][2]), np.float16(M[i][3]), np.float16(M[i][4]), M[i][5], int(M[i][6])))
    f.close()


def save_model_weights(g_model, m_model, d_model,
                       level, epoch, batch, list_d_loss, model_path_root):
    """
    Save model weights.

    Parameters
    ----------
    g_model: keras model object
        geometry generator model
    m_model: keras model object
        morphology generator model
    d_model: keras model object
        discriminator model
    level: int
        level in the hierarchy
    epoch: int
        epoch #
    batch: int
        mini-batch #
    list_d_loss: list
        list of discriminator loss trace
    model_path_root: str
        path where model files should be saved
    """
    model_path = ('%s/level%s' % (model_path_root, level))
    if not os.path.exists(model_path):
        os.mkdir(model_path)  # create directory
    print(model_path)
    print("\n")
    g_file = os.path.join(model_path, '%s_epoch_%s_batch_%s.h5' % (g_model.name, epoch, batch))
    print(g_file)

    g_model.save_weights(g_file, overwrite=True)

    m_file = os.path.join(model_path, '%s_epoch_%s_batch_%s.h5' %
                          (m_model.name, epoch, batch))
    print(m_file)
    m_model.save_weights(m_file, overwrite=True)

    d_file = os.path.join(model_path, '%s_epoch_%s_batch_%s.h5' %
                          (d_model.name, epoch, batch))
    print(d_file)
    d_model.save_weights(d_file, overwrite=True)

    d_loss_file = os.path.join(model_path, '%s_epoch_%s_batch_%s.h5' %
                               ('DiscLoss', epoch, batch))
    print(d_loss_file)
    pickle.dump(list_d_loss, open(d_loss_file, "wb"))


def train_model(input_dim=100, n_nodes=256, batch_size=16, n_epochs=100, d_iters=5, g_iters=5, verbose=True):
    geom_model = list()
    morph_model = list()
    disc_model = list()
    gan_model = list()
    # Discriminator
    d_model = models.discriminator(n_nodes=n_nodes,
                                   batch_size=batch_size)
    # Generators and GANs
    g_model, m_model = \
        models.generator(n_nodes=n_nodes,
                         noise_dim=input_dim,
                         batch_size=batch_size,
                         )
    stacked_model = \
        models.discriminator_on_generator(g_model,
                                           m_model,
                                           d_model,
                                           input_dim=input_dim)

    # Collect all models into a list
    disc_model.append(d_model)
    geom_model.append(g_model)
    morph_model.append(m_model)
    gan_model.append(stacked_model)

    # ###############
    # Optimizers
    # ###############
    optim_d = Adam()  # RMSprop(lr=lr_discriminator)
    optim_g = Adam() # RMSprop(lr=lr_generator)

    # ##############
    # Train
    # ##############
    # ---------------
    # Compile models
    # ---------------

    g_model.compile(loss='mse', optimizer=optim_g)
    m_model.compile(loss='mse', optimizer=optim_g)
    d_model.trainable = False
    stacked_model.compile(loss='binary_crossentropy',
                          optimizer=optim_g)
    d_model.trainable = True
    d_model.compile(loss='binary_crossentropy',
                    optimizer=optim_d)

    if verbose:
        print(20*'--')
    print("start loop epoch\n")
    BASE_DIR = os.path.dirname(os.path.abspath(__file__))  # get this file full path and remove filename
    ROOT_DIR = BASE_DIR
    DATA_PATH = os.path.join(ROOT_DIR, 'data/neuron_data1')
    f = open('./data/log', 'w')
    for e in range(n_epochs):
        g_iters = 0
        if verbose:
            print("")
            print("Epoch #{0}".format(e))
            print("")

        TRAIN_DATASET = neuron_dataset(root=DATA_PATH, batch_size=batch_size, npoints=n_nodes, num_channels=3)
        batch_data = {}
        cur_batch_data_geo = np.zeros([batch_size, n_nodes, 3])
        cur_batch_data_morp = np.zeros([batch_size, n_nodes, n_nodes])
        batch_idx = 0
        while TRAIN_DATASET.has_next_batch():
            print(str(datetime.now()))
            print("epoch:", e, "  batch:", batch_idx, "\n")
            f.writelines(str(datetime.now())+"\n")
            batch_data['geometry'], batch_data['morphology'] = TRAIN_DATASET.next_batch(augment=False)  # ----------------- not augment now----------------
            # bsize = batch_data['geometry'].shape[0]

            cur_batch_data_geo[0:batch_size, :, :] = batch_data['geometry']
            cur_batch_data_morp[0:batch_size, :, :] = batch_data['morphology']
            print(cur_batch_data_geo.shape, cur_batch_data_morp.shape)
            list_d_loss = list()
            list_g_loss = list()
            # ----------------------------
            # Step 1: Train discriminator
            # ----------------------------
            print("Step 1: Train discriminator\n")

            for d_iter in range(d_iters):
                    print("discriminator pretraining {0}".format(d_iter))
                    X_parent_real = cur_batch_data_morp
                    X_locations_real = cur_batch_data_geo

                    noise_code = np.random.rand(batch_size, 1, input_dim)  # Random values in a given shape
                    print("start predict")
                    X_locations_gen = g_model.predict(noise_code) # (batch_size,n_nodes,3)  then delete soma location,soma(0,0,0)
                    #X_locations_gen[:, 0:1, :] = np.zeros(shape=(batch_size, 1, 3))
                    # print(X_locations_gen.shape)
                    # print("===================")
                    X_parent_gen = m_model.predict(noise_code)  # (batch_size,n_nodes,n_nodes)
                    # parent = np.squeeze(X_parent_gen).argmax(axis=2)  # batch_size, n_nodes
                    # print(parent)
                    # X_parent_gen_proc = np.zeros((batch_size, n_nodes, n_nodes))
                    # X_parent_gen_proc[:, :, parent[:, :]] = 1
                    # print("generate parent")
                    # print(X_parent_gen_proc)

                    #X_parent_gen[:, 0:1, :] = np.zeros(shape=(batch_size, 1, n_nodes))
                    # print(X_parent_gen.shape)

                    y_real = np.ones((X_locations_real.shape[0], 1, 1))
                    y_gen = np.zeros((X_locations_gen.shape[0], 1, 1))

                    cutting = int(batch_size / 2)
                    X_locations_real_first_half = np.append(X_locations_real[:cutting, :, :],
                                                               X_locations_gen[:cutting, :, :],
                                                               axis=0)
                    X_parent_real_first_half = np.append(X_parent_real[:cutting, :, :],
                                                             X_parent_gen[:cutting, :, :],
                                                             axis=0)
                    y_real_first_half = np.append(y_real[:cutting, :, :],
                                                      y_gen[:cutting, :, :],
                                                      axis=0)

                    X_locations_real_second_half = np.append(X_locations_real[cutting:, :, :],
                                                                 X_locations_gen[cutting:, :, :],
                                                                 axis=0)
                    X_parent_real_second_half = np.append(X_parent_real[cutting:, :, :],
                                                              X_parent_real[cutting:, :, :],
                                                              axis=0)
                    y_real_second_half = np.append(y_real[cutting:, :, :],
                                                       y_gen[cutting:, :, :],
                                                       axis=0)


                    disc_loss = d_model.train_on_batch([X_locations_real_first_half,X_parent_real_first_half], y_real_first_half)
                    #disc_loss = d_model.train_on_batch([X_locations_real, X_parent_real], y_real)
                    list_d_loss.append(disc_loss)
                    print(disc_loss)
                    disc_loss = d_model.train_on_batch([X_locations_real_second_half, X_parent_real_second_half], y_real_second_half)
                    #disc_loss = d_model.train_on_batch([X_locations_gen, X_parent_gen], y_gen)
                    list_d_loss.append(disc_loss)
                    print(disc_loss)

            if verbose:
                print("After{0} iterations".format(d_iters))
                print("Discriminator loss = {0}".format(disc_loss))
                f.writelines('epoch:{0}, batch:{1}, After {2} iterations\n'.format(e,batch_idx,d_iters))
                f.writelines('discriminator loss: {0}\n'.format(disc_loss))

            # -------------------------------
            # step 2: train generators alternately
            # ---------------------------------
            # Freeze the discriminator
            print("step2 : train generations alternately\n")

            d_model.trainable = False

            noise_input = np.random.rand(batch_size, 1, input_dim)
            # print(noise_code)
            gen_loss = \
                        stacked_model.train_on_batch([noise_input],
                                                         y_real)
            list_g_loss.append(gen_loss)
            print("gen_loss", gen_loss)
            if verbose:
                print("")
                print("    Generator_Loss: {0}".format(gen_loss))
                f.writelines('epoch:{0}, batch:{1}, generator loss: {2}\n'.format(e,batch_idx,gen_loss))
                f.flush()
            # Unfreeze the discriminator
            d_model.trainable = True
            g_iters += 1
            batch_idx += 1

            # Save model weights (few times per epoch)
            if batch_idx % 1 == 0:
                if verbose:
                    print("level #{0} Epoch #{1} Batch #{2}".format(1, e, batch_idx))
                    save_swc(X_locations_gen[0, 1:, :], X_parent_gen[0, 1:, :], path='D:/gen_vir_experiment_code/swcgan_weight/generate_data', epoch=e, batch=batch_idx)
                    print("plot discriminator loss")
                    # plot.plot_loss_trace(list_d_loss, "discriminator loss")
                    # print("plot generator loss")
                    # plot.plot_loss_trace(list_g_loss, "generate loss")
                    #plot.plot_adjacency(X_parent_real[0:1, :, :], X_parent_gen[0:1, :, :], "parent real gen")
                    print("display loss trace\n")
                    save_model_weights(g_model, m_model, d_model, 0, e, batch_idx, list_d_loss, model_path_root= 'D:/gen_vir_experiment_code/swcgan_weight')

            # save model
            geom_model = g_model
            morph_model = m_model
            disc_model = d_model
            gan_model = stacked_model

    f.close()
    return geom_model, \
        morph_model, \
        disc_model, \
        gan_model










