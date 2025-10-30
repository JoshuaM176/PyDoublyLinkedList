from py_doubly_linked_list import DoublyLinkedList
import random

test_list = DoublyLinkedList([random.randint(0, 100) for i in range(1000)])
print(test_list)
test = test_list.sort(reverse=True)

test_list.pop(5)

test_list.insert(5, 10)
print("test")
print(test_list[0])