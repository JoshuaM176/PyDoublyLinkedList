# py-doubly-linked-list
## Description
This library provides an implementation of a doubly linked list for python written with the c api. It is meant to be as close as possible to a regular list in its interface, meaning you can interact with it in almost all of the same ways you interact with a regular python list.
## Methods
- append  
Append object to the end of the list. Set forward to false to append to the start.  
```Python
doubly_linked_list.append(object: Any, forward: bool = True)
```
- clear  
Remove all items from the list.  
```Python
doubly_linked_list.clear()
```
- copy  
Return a shallow copy of the list.  
```Python
doubly_linked_list.copy()
```
- count  
Return number of occurrences of value in the list.  
```Python
doubly_linked_list.count(value: Any)
```
- extend  
Extend list by appending elements from the iterable. Set forward to false to extend from the start.  
```Python
doubly_linked_list.extend(iterable: Iterable, forward: bool = True)
```
- index  
Return first index of value. Raises ValueError if the value is not present.  
```Python
doubly_linked_list.index(value: Any, start: int, stop: int)
```
- insert  
Insert object after index. Set forward to false to insert before index.  
```Python
doubly_linked_list.insert(object: Any, index: int, forward: bool = True)
```
- pop  
Remove and return item at index (default last). Raises IndexError if list is empty or index is out of range.  
```Python
doubly_linked_list.pop(index: int = -1)
```
- remove  
Remove first occurence of value. Raises ValueError if the value is not present.  
```Python
doubly_linked_list.remove(value: Any)
```
- reverse  
Reverse the order of the list.  
```Python
doubly_linked_list.reverse()
```
- sort  
In-place sort in ascending order, equal objects are not swapped. Key can be applied to values and the list will be sorted based on the result of applying the key. Reverse will reverse the sort order.
```Python
doubly_linked_list.sort(key: Callable = None, reverse: bool = False)
```
The DoublyLinkedList also supports:
- concatenation with other iterables
- in-place concatenation with other iterables
- indexing and assignment by index
- slicing
- iterating  

Things that are not currently supported but might be in the future:
- assigning slices
- repeating, i.e list = [1] * 5 create a list with 1 repeated 5 times

# License
Copyright (c) 2025 Joshua A. Morningstar

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
