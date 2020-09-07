"""
This module contains utility functions.
"""


def wrap_index(idx, size):
    """
    Function which converts an index of a sequence into a positive one.
    Hereby, a positive index remains unaltered while a negative index is
    converted.

    .. note:: This function mimics the behavior of standard Python sequences.

    :param idx: The index.
    :param size: The length of the sequence.
    :return: The converted index.
    """
    if idx < -size or size <= idx:
        raise IndexError("Index out of range!")
    return (idx % size) % size
