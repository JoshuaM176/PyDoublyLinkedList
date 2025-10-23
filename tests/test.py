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
    test_list = create_test_list(); assert sys.getrefcount(test_list.tail) == 4
    assert sys.getrefcount(test_list.head) == 2
    head = test_list.head; del test_list[0]; assert sys.getrefcount(head) == 2
    assert sys.getrefcount(test_list.tail) == 3
    node1 = test_list.head.next; assert sys.getrefcount(node1) == 3
    test_list.pop(1); assert sys.getrefcount(node1) == 2
    test_list = create_test_list(); node2 = test_list.head; assert(sys.getrefcount(node2)) == 3
    test_list.clear(); assert sys.getrefcount(node2) == 2

def test_cursor_pos():
    pass #TODO

if __name__ == "__main__":
    test_length()
    test_node_refs()