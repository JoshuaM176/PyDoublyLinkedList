from py_doubly_linked_list import DoublyLinkedList
import sys
import weakref

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

def test_node_refs(): # Most methods end up calling the same internal methods for adding and deleting, so no need to thoroughly test every one
    test_list = create_test_list()
    head_ref = weakref.ref(test_list.head); tail_ref = weakref.ref(test_list.tail); middle_ref = weakref.ref(test_list.head.next)
    assert sys.getrefcount(head_ref()) == 3; assert sys.getrefcount(tail_ref()) == 4; assert sys.getrefcount(middle_ref()) == 3
    test_list[1]; assert sys.getrefcount(tail_ref()) == 3; assert sys.getrefcount(middle_ref()) == 4
    test_list.pop(1); assert 'dead' in repr(middle_ref)
    test_list.pop(0); assert 'dead' in repr(head_ref)
    test_list.pop(0); assert sys.getrefcount(tail_ref()) == 4
    test_list.insert(TestClass(4), 0); assert sys.getrefcount(tail_ref()) == 3
    test_list.clear(); assert 'dead' in repr(tail_ref)

def test_cursor_pos():
    pass #TODO

if __name__ == "__main__":
    test_length()
    test_node_refs()