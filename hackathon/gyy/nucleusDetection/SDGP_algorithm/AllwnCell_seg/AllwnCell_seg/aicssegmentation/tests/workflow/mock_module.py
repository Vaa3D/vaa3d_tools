from typing import List
import numpy as np


def function_single_input(img: np.ndarray):
    if img is None:
        raise ValueError("img")
    return 1


def function_single_input_with_parameters(img: np.ndarray, x: int, y: int):
    if img is None:
        raise ValueError("img")
    return x + y


def function_list_input(img: List[np.ndarray]):
    if img is None:
        raise ValueError("img")
    return 1


def function_list_input_with_parameters(img: List[np.ndarray], x: int, y: int):
    if img is None:
        raise ValueError("img")
    return x + y
