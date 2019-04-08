from cnn_model import CNN_Model
from data import DataLoader
import tensorflow as tf
import numpy as np
import os
from collections import Counter

os.environ['TF_CPP_MIN_LOG_LEVEL'] = '1'

TESTDIR = "C:/Users/Administrator/Desktop/neuron_factor_tree_test/test_1"
model_save_dir = TESTDIR + "/save"
eval_path = TESTDIR + "/data/morpho_eval"
class_map = {"L4_BC": 1, "L4_PC": 2, "L4_SC": 3, "L5_MC": 4, "L5_PC": 5,
                          "L6_PC": 6, "L23_BC": 7, "L23_MC": 8, "L23_PC": 9}

def neuron_eval(eval_path):

    print("\nNeuron Evaluating...\n")

    graph = tf.Graph()
    with graph.as_default():
        session_conf = tf.ConfigProto(
            allow_soft_placement=True,  # automatically choose a device usable.
            log_device_placement=True)  # print device info.
        sess = tf.Session(config=session_conf)
        with sess.as_default():
            # load saved meta graph.
            ckpt = tf.train.latest_checkpoint(model_save_dir)

            assert ckpt, "No check point found."
            if ckpt is not None:
                saver = tf.train.import_meta_graph("{}.meta".format(ckpt))
                saver.restore(sess, ckpt)

            # get the placeholders.
            input_x = graph.get_operation_by_name("input_x").outputs[0]
            dropout_keep_prob = graph.get_operation_by_name("dropout_prob").outputs[0]

            # the output to eval.
            predictions = graph.get_operation_by_name("output/predictions").outputs[0]

            correct_predictions=0
            count=0
            path_list = os.listdir(eval_path)
            for p in path_list:
                class_path = os.path.join(eval_path, p)
                if os.path.isdir(class_path):  # if is dir, dir's name is neuron class.
                    neuron_label = class_map[p]
                    f_list = os.listdir(class_path)
                    for f in f_list:
                        f = os.path.join(class_path, f)
                        if os.path.isfile(f) and f[-4:]=="nfss" :  # f is nfss file, predict it's label.
                            data_loader = DataLoader(f, batch_size=-1, sequence_size=10, feature_size=19, mode="eval")
                            if data_loader.sequences_num==0:
                                continue
                            predictions_ = []
                            for b in range(data_loader.num_batches):
                                sess.run(tf.assign(dropout_keep_prob, 0.0))
                                x_test_batch, _ = data_loader.next_batch()  # don't need y_test_batch.

                                feed = {input_x: x_test_batch}
                                prediction_bacth = sess.run(predictions, feed)
                                predictions_ = np.concatenate([predictions_, prediction_bacth])
                            label_counter = Counter(predictions_)
                            top_label_ = label_counter.most_common(1)  # use most appeared label as prediction.
                            if neuron_label in [key for key, _ in top_label_]:  # real y label in most appeared means predict right.
                                correct_predictions += 1
                            count += 1
                            print("{:g}:{:g}".format(float(neuron_label), float(top_label_[0][0])))
            print("Accuracy:{:g}".format(correct_predictions/float(count)))

if __name__ == '__main__':
    neuron_eval(eval_path)
