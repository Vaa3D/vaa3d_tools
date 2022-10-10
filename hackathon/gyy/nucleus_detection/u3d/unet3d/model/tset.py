import  tensorflow as tf

gpus=tf.config.experimental.list_physical_devices(device_type='GPU')

print(gpus)