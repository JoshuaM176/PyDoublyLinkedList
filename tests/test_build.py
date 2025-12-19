from py_doubly_linked_list import DoublyLinkedList

import sys
import weakref

class DummyClass():
    def __init__(self, id: int):
        self.id = id

    def __str__(self):
        return str(self.id)

def create_test_list():
    return DoublyLinkedList([DummyClass(0),DummyClass(1),DummyClass(2),DummyClass(3)])

def test_indexing():
    test_list = DoublyLinkedList([0,1,2,3,4,5,6,7,8,9,10])
    assert test_list[5] == 5
    assert test_list[10] == 10
    assert test_list[9] == 9
    assert test_list[1] == 1
    assert list(test_list[3:6]) == [3,4,5]

def test_length():
    test_list = create_test_list()
    assert len(test_list) == 4
    test_list.pop()
    assert len(test_list) == 3
    test_list.clear()
    assert len(test_list) == 0

def test_dereferencing():
    test_list = create_test_list()
    reference = weakref.ref(test_list[0])
    test_list.pop(0)
    assert reference() is None
    reference = weakref.ref(test_list[0])
    test_list.clear()
    assert reference() is None

def test_sort():
    test_list = DoublyLinkedList([1,5,3,8,6,7,4,2])
    test_list.sort()
    assert list(test_list) == [1,2,3,4,5,6,7,8]
    test_list.sort(lambda x : x * -1)
    assert list(test_list) == [8,7,6,5,4,3,2,1]

def test_rich_compare():
    test_list = DoublyLinkedList([1,2,3,4]) 
    test_list2 = DoublyLinkedList([1,2,3,4])
    test_reg_list = [1,2,3,4]
    test_list3 = DoublyLinkedList([2,1,3,4])
    test_list4 = DoublyLinkedList([1,2,3,4,5])
    assert test_list == test_list2
    assert test_list == test_reg_list
    assert test_list != test_list3
    assert test_list != test_list4
    assert test_list4 != test_list

if __name__ == "__main__":
    test_indexing()
    test_length()
    test_dereferencing()
    test_sort()
    test_rich_compare()