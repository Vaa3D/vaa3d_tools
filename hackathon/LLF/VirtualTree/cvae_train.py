from __future__ import absolute_import, division, print_function, unicode_literals
import tensorflow as tf
from tensorflow import keras
from tensorflow.python.keras.optimizers import Adam
import numpy as np
from cvae import CVAE
import time

from branch_code_data import BranchCodeLoader

test_data = "C:/Users/Administrator/Desktop/TestVirtualPCNeuron/data"
save_dir = "C:/Users/Administrator/Desktop/TestVirtualPCNeuron/vae_save"
data_size = 20*3
batch_size = 1000

epochs = 100
latent_dim = 50
num_examples_to_generate = 16

# keeping the random vector constant for generation (prediction) so
# it will be easier to see the improvement.
random_vector_for_generation = tf.random.normal(
    shape=[num_examples_to_generate, latent_dim])
model = CVAE(latent_dim)

optimizer = tf.train.AdamOptimizer(1e-4)

def log_normal_pdf(sample, mean, logvar, raxis=1):
  log2pi = tf.math.log(2. * np.pi)
  return tf.reduce_sum(
      -.5 * ((sample - mean) ** 2. * tf.exp(-logvar) + logvar + log2pi),
      axis=raxis)

def compute_loss(model, x):
  mean, logvar = model.encode(x)
  z = model.reparameterize(mean, logvar)
  x_logit = model.decode(z)

  cross_ent = tf.nn.sigmoid_cross_entropy_with_logits(logits=x_logit, labels=x)
  logpx_z = -tf.reduce_sum(cross_ent, axis=[1, 2, 3])
  logpz = log_normal_pdf(z, 0., 0.)
  logqz_x = log_normal_pdf(z, mean, logvar)
  return -tf.reduce_mean(logpx_z + logpz - logqz_x)

def compute_apply_gradients(model, x, optimizer):
  with tf.GradientTape() as tape:
      loss = compute_loss(model, x)
  gradients = tape.gradient(loss, model.trainable_variables)
  optimizer.apply_gradients(zip(gradients, model.trainable_variables))


data_loader = BranchCodeLoader(test_data, batch_size)

for epoch in range(1, epochs + 1):
    start_time = time.time()
    for train_x in data_loader.next_batch_4():
        compute_apply_gradients(model, train_x, optimizer)
    end_time = time.time()

    if epoch % 1 == 0:

      loss = []
      for test_x in data_loader.get_test():
          loss.append(compute_loss(model, test_x))
      elbo = -tf.metrics.mean(loss)

      print('Epoch: {}, Test set ELBO: {}, '
            'time elapse for current epoch {}'.format(epoch,
                                                      elbo,
                                                      end_time - start_time))
