import time
from py_doubly_linked_list import DoublyLinkedList
from random_list import test_list

test_list = DoublyLinkedList(test_list)

start_time = time.time()
test_list.merge_sort()
end_time = time.time()

print(f"My parellel merge sort time is {end_time-start_time} seconds.")