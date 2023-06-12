
import os
#os.environ["CUDA_VISIBLE_DEVICES"] = '-1'
pwd = os.path.dirname(os.path.abspath(__file__))
import sys
sys.path.append(os.path.dirname(os.path.dirname(__file__)))
import numpy as np
import tensorflow as tf
from networks import NetworkAlbert
from classifier_utils import get_feature_test,id2label
from hyperparameters import Hyperparamters as hp
          

class Model(object,):
    def __init__(self):
        self.albert, self.sess = self.load_model()
    @staticmethod
    def load_model():
        with tf.Graph().as_default():
            sess = tf.Session()
            with sess.as_default():
                albert =  NetworkAlbert(is_training=False)
                saver = tf.train.Saver()  
                sess.run(tf.global_variables_initializer())
                checkpoint_dir = os.path.abspath(os.path.join(pwd, hp.file_save_model))
                # print (checkpoint_dir)
                ckpt = tf.train.get_checkpoint_state(checkpoint_dir)
                saver.restore(sess, ckpt.model_checkpoint_path)
        return albert,sess

MODEL = Model()
# print('Load model finished!')


def get_label(sentence):
    """
    Prediction of the sentence's label.
    """
    feature = get_feature_test(sentence)
    fd = {MODEL.albert.input_ids: [feature[0]],
          MODEL.albert.input_masks: [feature[1]],
          MODEL.albert.segment_ids:[feature[2]],
          }
    prediction = MODEL.sess.run(MODEL.albert.predictions, feed_dict=fd)[0]
    #print(prediction)
    return [id2label(l) for l in np.where(prediction==1)[0] if l!=0]    
     

if __name__ == '__main__':
    #
    sentences = ["对图像进行重建，并对所生成的swc进行分析"]
    for sentence in sentences:
         print(sentence, get_label(sentence))

    #以下代码配合Vaa3D-x的参数传递进行使用
    # args = sys.argv[1:]
    # sentences = []
    # s = str(args)
    # sentences.append(s)
    # for sentence in sentences:
    #     print("处理后的结果：", get_label(sentence))
    # print("over!")

    
    
    
