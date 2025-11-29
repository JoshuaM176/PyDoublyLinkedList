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

if __name__ == "__main__":
    test_length()
    test_dereferencing()
    test_list = DoublyLinkedList([1,2,3])
    test_list.merge_sort()