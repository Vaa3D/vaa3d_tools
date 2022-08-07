import numpy as np
import pandas as pd
from scipy.ndimage.measurements import label


def simple_builder(bw, se=None, return_dataframe=False):
    """build object table based on connected component"""

    if se is None:
        obj_label, obj_num = label(bw > 0)
    else:
        obj_label, obj_num = label(bw > 0, structure=se)

    if return_dataframe:
        import pandas as pd

        obj_df = pd.DataFrame(np.arange(1, obj_num + 1, 1), columns=["object_index"])
        return obj_label, obj_df
    else:
        return obj_label


def masked_builder(bw, mask_label):
    """build object table based on mask image"""

    assert mask_label.max() > 0
    a = mask_label > 0
    b= a==bw
    c=mask_label[mask_label > 0]
    if mask_label.max() == 1:
        mask_label, num_label = label(mask_label > 0)
        label_list = np.arange(1, num_label + 1, 1)
    else:
        label_list = np.unique(mask_label[mask_label > 0])

    counter_offset = 0
    obj_label = np.zeros_like(mask_label)
    multi_index_list = []
    for ii, val in enumerate(label_list):
        single_mask = mask_label == val
        valid_bw = bw.copy()
        valid_bw[single_mask == 0] = 0
        valid_label, valid_label_num = label(valid_bw > 0)
        for valid_index in range(valid_label_num):
            index_plus = valid_index + 1
            this_obj_index = index_plus + counter_offset
            this_obj = valid_label == index_plus
            obj_label[this_obj] = this_obj_index
            multi_index_list.append([val, this_obj_index, this_obj.sum()])
        counter_offset = counter_offset + valid_label_num

    obj_df = pd.DataFrame(multi_index_list)
    obj_df.columns = ["mask_id", "obj_id", "vol"]

    return obj_label, obj_df.set_index(["mask_id", "obj_id"])


def hierachical_builder(img_list):
    print("under construction")
    pass
