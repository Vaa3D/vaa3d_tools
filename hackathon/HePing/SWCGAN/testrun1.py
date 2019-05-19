import os
os.environ['KERAS_BACKEND'] = 'theano'
import train

if __name__ == '__main__':
    input_dim = 100
    n_epochs = 50
    n_nodes = 256
    batch_size = 8
    d_iter = 5
    g_iter = 5
    geom_model, morph_model, disc_model, gan_model = \
        train.train_model(input_dim=input_dim,
                          n_nodes=n_nodes,
                          batch_size=batch_size,
                          n_epochs=n_epochs,
                          d_iters=d_iter,
                          g_iters=g_iter,
                          verbose=True)
