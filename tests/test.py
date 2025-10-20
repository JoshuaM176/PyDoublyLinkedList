from py_doubly_linked_list import DoublyLinkedList
import sys

class TestClass():
    def __init__(self, id: int):
        self.id = id

    def __str__(self):
        return str(self.id)

def create_test_list():
    return DoublyLinkedList([TestClass(0),TestClass(1),TestClass(2),TestClass(3)])

def test_length():
    test_list = create_test_list()
    assert len(test_list) == 4
    test_list.pop(0); assert len(test_list) == 3
    new_val = TestClass(4)
    test_list.insert(new_val, 2); assert len(test_list) == 4
    test_list.append(new_val); assert len(test_list) ==5
    test_list.remove(new_val); assert len(test_list) == 4
    test_list.extend([new_val, new_val]); assert len(test_list) == 6
    test_list.clear(); assert len(test_list) == 0

def test_node_refs(): # Most changes to ref counts happen in the same two internal functions, so no need to thoroughly test every method
    pass #TODO

def test_cursor_pos():
    pass #TODO

if __name__ == "__main__":
    test_length()
    test_node_refs()