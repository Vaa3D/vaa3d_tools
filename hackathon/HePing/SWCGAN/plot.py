import matplotlib.pyplot as plt


def plot_loss_trace(loss):
    """
    Plot trace of loss.

    Parameters
    ----------
    loss: list or array of a loss trace
    """
    plt.figure(figsize=(3, 2))
    plt.plot(loss)
    plt.show()


def plot_adjacency(X_parent_real, X_parent_gen):
    """
    Plot a pair of adjacency matrices side by side.

    stuff.
    """
    for sample in range(X_parent_real.shape[0]):
        plt.figure(figsize=(10, 5))
        plt.subplot(1, 2, 1)  # nrows,nclos, index
        plt.imshow(X_parent_real[sample, :, :],
                   interpolation='none',
                   cmap='Greys')  # 2D mesh
        plt.subplot(1, 2, 2)
        plt.imshow(X_parent_gen[sample, :, :],
                   interpolation='none',
                   cmap='Greys')
