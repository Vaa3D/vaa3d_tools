

import os
import tensorflow as tf
import modeling
import optimization
from utils import time_now_string
from hyperparameters import Hyperparamters as hp
from classifier_utils import ClassifyProcessor
import numpy as np

num_labels = hp.num_labels
processor = ClassifyProcessor() 
bert_config_file = os.path.join(hp.bert_path,'albert_config.json')
bert_config = modeling.AlbertConfig.from_json_file(bert_config_file)

def safe_div(numerator, denominator):
    """安全除，分母为0时返回0"""
    numerator, denominator = tf.cast(numerator,dtype=tf.float64), tf.cast(denominator,dtype=tf.float64)
    zeros = tf.zeros_like(numerator, dtype=numerator.dtype) # 创建全0Tensor
    denominator_is_zero = tf.equal(denominator, zeros) # 判断denominator是否为零
    return tf.where(denominator_is_zero, zeros, numerator / denominator) # 如果分母为0，则返回零


def pr_re_f1(cm, pos_indices):
    num_classes = cm.shape[0]
    with tf.Session() as sess:
        print(pos_indices)
        pos_indices = pos_indices.eval()
    neg_indices = [i for i in range(num_classes) if i not in pos_indices]
    cm_mask = np.ones([num_classes, num_classes])
    cm_mask[neg_indices, neg_indices] = 0  # 将负样本预测正确的位置清零零
    diag_sum = tf.reduce_sum(tf.diag_part(cm * cm_mask))  # 正样本预测正确的数量

    cm_mask = np.ones([num_classes, num_classes])
    cm_mask[:, neg_indices] = 0  # 将负样本对应的列清零
    tot_pred = tf.reduce_sum(cm * cm_mask)  # 所有被预测为正的样本数量

    cm_mask = np.ones([num_classes, num_classes])
    cm_mask[neg_indices, :] = 0  # 将负样本对应的行清零
    tot_gold = tf.reduce_sum(cm * cm_mask)  # 所有正样本的数量

    pr = safe_div(diag_sum, tot_pred)
    re = safe_div(diag_sum, tot_gold)
    f1 = safe_div(2. * pr * re, pr + re)

    return pr, re, f1


class NetworkAlbert(object):
    def __init__(self,is_training):
        # Training or not
        self.is_training = is_training    
        
        # Placeholder       
        self.input_ids = tf.placeholder(tf.int32, shape=[None, hp.sequence_length], name='input_ids')
        self.input_masks = tf.placeholder(tf.int32, shape=[None,  hp.sequence_length], name='input_masks')
        self.segment_ids = tf.placeholder(tf.int32, shape=[None,  hp.sequence_length], name='segment_ids')
        self.label_ids = tf.placeholder(tf.float32, shape=[None,hp.num_labels], name='label_ids')
               
        # Load BERT model
        self.model = modeling.AlbertModel(
                                    config=bert_config,
                                    is_training=self.is_training,
                                    input_ids=self.input_ids,
                                    input_mask=self.input_masks,
                                    token_type_ids=self.segment_ids,
                                    use_one_hot_embeddings=False)

        # Get the feature vector by BERT
        output_layer = self.model.get_pooled_output()      
         
        # Hidden size 
        hidden_size = output_layer.shape[-1].value                            

        with tf.name_scope("Full-connection"):  
            output_weights = tf.get_variable(
                  "output_weights", [num_labels, hidden_size],
                  initializer=tf.truncated_normal_initializer(stddev=0.02))            
            output_bias = tf.get_variable(
                  "output_bias", [num_labels], initializer=tf.zeros_initializer())   
            logits = tf.nn.bias_add(tf.matmul(output_layer, output_weights, transpose_b=True), output_bias)            
            # Prediction sigmoid(Multi-label)
            self.probabilities = tf.nn.sigmoid(logits)


        with tf.variable_scope("Prediction"):             
            # Prediction               
            zero = tf.zeros_like(self.probabilities)
            one = tf.ones_like(self.probabilities)
            self.predictions = tf.where(self.probabilities < 0.1, x=zero, y=one)
            # self.Precision, self.Recall_Score, self.F1_Score = pr_re_f1(self.predictions, self.label_ids)
            # self.Recall_Score = tf.metrics.recall(labels=self.label_ids, predictions=self.predictions)
            # self.F1_Score = tf.metrics.F1Score(labels=self.label_ids, predictions=self.predictions)

        with tf.variable_scope("loss"):
            # Summary for tensorboard
            if self.is_training:
                self.accuracy = tf.reduce_mean(tf.to_float(tf.equal(self.predictions, self.label_ids)))
                print('self.label_ids:', self.label_ids)
                # self.Precision, _ = tf.metrics.precision(labels=self.label_ids, predictions=self.predictions)
                # tf.summary.scalar('Accuracy', self.accuracy)
                # tf.summary.scalar('predictions', self.Precision)

                # tf.summary.scalar('predictions', self.predictions)
                # tf.summary.scalar('Recall_Score', self.Recall_Score)
                # tf.summary.scalar('F1_Score', self.F1_Score)

                # print('Accuracy:', self.accuracy)
                # sess = tf.Session()
                # self.predictions = sess.run(self.predictions)
                # print(self.predictions)
                # print(self.label_ids)
                # recall.update_state()

            # Initial embedding by BERT
            ckpt = tf.train.get_checkpoint_state(hp.saved_model_path)
            checkpoint_suffix = ".index"
            if ckpt and tf.gfile.Exists(ckpt.model_checkpoint_path + checkpoint_suffix):
                print('='*10,'Restoring model from checkpoint!','='*10)
                print("%s - Restoring model from checkpoint ~%s" % (time_now_string(),
                                                                    ckpt.model_checkpoint_path))
            else:                   
                print('='*10,'First time load chatV3D-x!','='*10)
                tvars = tf.trainable_variables()
                if hp.init_checkpoint:
                   (assignment_map, initialized_variable_names) = \
                     modeling.get_assignment_map_from_checkpoint(tvars,
                                                                 hp.init_checkpoint)
                   tf.train.init_from_checkpoint(hp.init_checkpoint, assignment_map)
                                
            # Loss and Optimizer
            if self.is_training:
                # Global_step
                self.global_step = tf.Variable(0, name='global_step', trainable=False)             
                per_example_loss = tf.nn.sigmoid_cross_entropy_with_logits(labels=self.label_ids,logits=logits)
                self.loss = tf.reduce_mean(per_example_loss)

                # Optimizer BERT
                train_examples = processor.get_train_examples(hp.data_dir)
                num_train_steps = int(
                    len(train_examples) / hp.batch_size * hp.num_train_epochs)
                #num_train_steps = 10000
                num_warmup_steps = int(num_train_steps * hp.warmup_proportion)
                print('num_train_steps',num_train_steps)
                self.optimizer = optimization.create_optimizer(self.loss,
                                                                hp.learning_rate, 
                                                                num_train_steps, 
                                                                num_warmup_steps,
                                                                hp.use_tpu,
                                                                Global_step=self.global_step)    

                # Summary for tensorboard                 
                tf.summary.scalar('loss', self.loss)
                self.merged = tf.summary.merge_all()
                
                
                
if __name__ == '__main__':
    # Load model
    albert = NetworkAlbert(is_training=True)



