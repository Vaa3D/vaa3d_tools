import os
import sys
sys.path.append("..")
import tables
from keras import backend as K
K.set_image_data_format("channels_last")
from model import unet_model_3d
from generator_Unet import get_training_and_validation_generators
from training import load_old_model, train_model
import argparse
parser = argparse.ArgumentParser()
parser.add_argument("--opt", type=str, default='Adam')
parser.add_argument("--lr", type=float, default=0.001)
args = parser.parse_args()
import  warnings
warnings.filterwarnings('ignore')

os.environ["CUDA_VISIBLE_DEVICES"] = "0"

config = dict()
config["pool_size"] = (2, 2, 2)  # pool size for the max pooling operations
config["image_shape"] = (80, 144, 112)  # This determines what shape the images will be cropped/resampled to.
# orishape: input_shape = (200, 324, 268)
config["n_labels"] = 10
config["input_shape"] = tuple(list(config["image_shape"])+[1] )
config["deconvolution"] = True  # if False, will use upsampling instead of deconvolution
config["batch_size"] = 1
config["validation_batch_size"] = config["batch_size"]
config["n_epochs"] = 500  # cutoff the training after this many epochs
config["patience"] = 10  # learning rate will be reduced after this many epochs if the validation loss is not improving
config["early_stop"] = 15  # training will be stopped after this many epochs without the validation loss improving
config["learning_rate_drop"] = 0.5  # factor by which the learning rate will be reduced
config["train_file"] = os.path.abspath("./DataPreprocess/train.h5")
config["val_file"] = os.path.abspath("./DataPreprocess//val.h5")
config["model_file"] = os.path.abspath("./logs/U_model.h5")
config["overwrite"] = True  # If True, do not load model.


def open_data_file(filename, readwrite="r"):
    return tables.open_file(filename, readwrite)


def main(overwrite=False):
    # <class 'tables.file.File'>
    train_file_opened = open_data_file(config["train_file"])
    val_file_opened = open_data_file(config["val_file"])

    # the shape of train_file_opened.root.data???
    n_train_steps = int(len(train_file_opened.root.data)/config["batch_size"]) - 1
    n_validation_steps = int(len(val_file_opened.root.data) / config["validation_batch_size"]) - 1

    print('train steps : ', n_train_steps)
    print('valdation steps : ', n_validation_steps)

    model = unet_model_3d(input_shape=config["input_shape"],
                          pool_size=config["pool_size"],
                          n_labels=config["n_labels"],
                          TrainOP = args.opt,
                          initial_learning_rate=args.lr,
                          deconvolution=config["deconvolution"])
    if not overwrite and os.path.exists(config["model_file"]):
        model.load_weights(config["model_file"],by_name = True)
        print(' ------------  load model !')

    train_generator, validation_generator = get_training_and_validation_generators(
        train_file_opened,
        val_file_opened,
        batch_size=config["batch_size"],
        validation_batch_size=config["validation_batch_size"],
        InputShape=config["image_shape"])

    # run training
    train_model(model=model,
                model_file=config["model_file"],
                training_generator=train_generator,
                validation_generator=validation_generator,
                steps_per_epoch=n_train_steps,
                validation_steps=n_validation_steps,
                initial_learning_rate=args.lr,
                learning_rate_drop=config["learning_rate_drop"],
                learning_rate_patience=config["patience"],
                early_stopping_patience=config["early_stop"],
                n_epochs=config["n_epochs"],
                logging_file = './logs/U_log.log')
    #logs_loss = LossHistory()

    train_file_opened.close()
    val_file_opened.close()


if __name__ == "__main__":
    main(overwrite=config["overwrite"])
