from functools import partial
import numpy as np

from keras import backend as K
import tensorflow as tf



###   -----------------   VAE loss   --------------------  ###
#  1.  dice loss

#  2.  L2
def L2_Loss(y_true, y_pred):
    return 1-K.mean(K.pow(y_true - y_pred,2))/10000000.0


#  3.  mean-std-loss
def MSL(y_true, y_pred):
    mean = y_pred[:,0:128]
    std = y_pred[:, 128:]
    return K.mean(mean*mean) + K.mean(std*std) - K.mean(K.log(std*std)) - 1


###   -----------------   focal loss   --------------------  ###

def focal_loss(y_true, y_pred):
    gamma = 2.
    alpha = 0.5

    pt_1 = tf.where(tf.equal(y_true, 1), y_pred, tf.ones_like(y_pred))
    pt_0 = tf.where(tf.equal(y_true, 0), y_pred, tf.zeros_like(y_pred))
    pt_1 = K.clip(pt_1, 1e-3, 0.999)
    pt_0 = K.clip(pt_0, 1e-3, 0.999)
    return -K.sum(alpha * K.pow(1. - pt_1, gamma) * K.log(pt_1)) - K.sum((1 - alpha)* K.pow(pt_0, gamma) * K.log(1. - pt_0))





# ###   -----------------   dice loss   --------------------  ###

def dice_single_coefficient(y_true, y_pred, smooth=1e-5):
    y_true_f = K.flatten(y_true)
    y_pred_f = K.flatten(y_pred)
    intersection = K.sum(y_true_f * y_pred_f)
    return (2. * intersection + smooth) / (K.sum(y_true_f) + K.sum(y_pred_f) + smooth)


def dice_coefficient(y_true, y_pred, smooth=1e-5):
    #print('  --------------  shape : ',y_true.shape,y_pred.shape)
    dice = 0
    count=0
    n = int(y_pred.shape[4])
    if n>1:
        # print(' multi --------------------------------- ')
        for i in range(0,n,1):          # in 3.27 from range(1,n,1) modify to range(0,n,1), in order to count 0 into loss
            dice += dice_single_coefficient(y_true[:,:,:,:,i],y_pred[:,:,:,:,i])    # weighted loss???
            count+=1
        return dice / (count + smooth)  # count+smooth ~ count
    else:
        return dice_single_coefficient(y_true,y_pred)

# def dice_single_coefficient(y_true, y_pred, smooth=1e-5):
#     y_true_f = K.flatten(y_true)
#     y_pred_f = K.flatten(y_pred)
#     intersection = K.sum(y_true_f * y_pred_f)
#     return (2. * intersection + smooth) / (K.sum(y_true_f) + K.sum(y_pred_f) + smooth)


def dice_coefficient1(y_true, y_pred, smooth=1e-5):
    #print('  --------------  shape : ',y_true.shape,y_pred.shape)

    return dice_single_coefficient(y_true[:,:,:,:,8],y_pred[:,:,:,:,1])


#
# def dice_coefficient1(y_true,y_pred, smooth=1e-5):
#
#     y_true1 = y_pred[0]
#     y_true = y_true1[:,:,:,0:2]
#     y_pred = y_true1[:,:,:,2:4]
#     # print(' loss ', y_true.shape,  y_pred.shape)
#     return dice_single_coefficient(y_true, y_pred)


def dice_coefficient_loss1(y_true, y_pred):
    return 1-dice_coefficient1(y_true, y_pred)



def dice_coefficient_loss(y_true, y_pred):
    return 1-dice_coefficient(y_true, y_pred)




def weighted_dice_coefficient(y_true, y_pred, axis=(-3, -2, -1), smooth=0.00001):
    return K.mean(2. * (K.sum(y_true * y_pred,
                              axis=axis) + smooth/2)/(K.sum(y_true,
                                                            axis=axis) + K.sum(y_pred,
                                                                               axis=axis) + smooth))


def weighted_dice_coefficient_loss(y_true, y_pred):
    return -weighted_dice_coefficient(y_true, y_pred)


def label_wise_dice_coefficient(y_true, y_pred, label_index):
    return dice_coefficient(y_true[:, label_index], y_pred[:, label_index])


def get_label_dice_coefficient_function(label_index):
    f = partial(label_wise_dice_coefficient, label_index=label_index)
    f.__setattr__('__name__', 'label_{0}_dice_coef'.format(label_index))
    return f


dice_coef = dice_coefficient
dice_coef_loss = dice_coefficient_loss






###   -----------------   weight dice loss   --------------------  ###
'''

def Mean_weighted_dice_coef(y_true, y_pred, mean=0.03):
    w_1 = 1 / mean ** 2
    w_0 = 1 / (1 - mean) ** 2
    y_true_f_1 = K.flatten(y_true)
    y_pred_f_1 = K.flatten(y_pred)
    y_true_f_0 = K.flatten(1 - y_true)
    y_pred_f_0 = K.flatten(1 - y_pred)

    intersection_0 = K.sum(y_true_f_0 * y_pred_f_0)
    intersection_1 = K.sum(y_true_f_1 * y_pred_f_1)

    return 2 * (w_0 * intersection_0 + w_1 * intersection_1) / (
                (w_0 * (K.sum(y_true_f_0) + K.sum(y_pred_f_0))) + (w_1 * (K.sum(y_true_f_1) + K.sum(y_pred_f_1))))


def Mean_weighted_dice_loss(y_true, y_pred, mean=0.03):
    return 1- Mean_weighted_dice_coef(y_true, y_pred, mean=mean)

'''
def Mean_weighted_dice_coef(y_true, y_pred, epl=1e-5):
    y_B = y_true[:,1,:,:,:]
    y_T = y_true[:,0,:,:,:]
    y_true = y_T * y_B
    y_pred = y_pred * y_B
    y_true_f = K.flatten(y_true)
    y_pred_f = K.flatten(y_pred)
    intersection = K.sum(y_true_f * y_pred_f)
    return (2. * intersection + epl) / (K.sum(y_true_f) + K.sum(y_pred_f) + epl)


def Mean_weighted_dice_loss(y_true, y_pred):
    return - Mean_weighted_dice_coef(y_true, y_pred)






###   -----------------   weight CrossEntropy loss   --------------------  ###
def WeightCrossEntropyLoss(y_true, y_pred):
    return -WeightCrossEntropy(y_true, y_pred)


def WeightCrossEntropy(y_true, y_pred, epl=1e-5):
    return K.mean(y_true[:,1,:,:,:]*(y_true[:,0,:,:,:] * K.log(K.clip(y_pred, epl, 1.0-epl)) + (1-y_true[:,0,:,:,:])*K.log(1-K.clip(y_pred, epl, 1.0-epl))))













###   -----------------   mix loss   --------------------  ###
def mixedLoss(y_ture,y_pred,alpha = 1e-6):
    return alpha * focal_loss(y_ture,y_pred) + dice_coefficient_loss(y_ture[:,0,:,:,:],y_pred[:,0,:,:,:])














