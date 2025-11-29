from py_doubly_linked_list import DoublyLinkedList

test_list = DoublyLinkedList([1,2,3,4])

print(test_list)

index = 0

for item in test_list:
    print(item)
    if(item == 2):
        del test_list[index]
        index -= 1
    index += 1