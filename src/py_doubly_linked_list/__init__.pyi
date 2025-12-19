from typing import Iterable, TypeVar, overload
from collections.abc import Callable, MutableSequence
from _typeshed import SupportsRichComparison, SupportsRichComparisonT, _T
import sys

_S = TypeVar("_S")

class DoublyLinkedList(MutableSequence[_T]):
    @overload
    def __init__(self) -> None: ...
    def __init__(self, iterable: Iterable[_T], /) -> None: ...
    def append(self, object: _T, forward: bool = True) -> None:
        """Append object to the end of the list. Set forward to false to append to the start."""
        ...
    def clear(self) -> None:
        """Remove all items from the list."""
        ...
    def copy(self) -> DoublyLinkedList[_T]:
        """Return a shallow copy of the list."""
        ...
    def count(self, value: _T) -> int:
        """Return number of occurrences of value in the list."""
        ...
    def extend(self, iterable: Iterable[_T], forward: bool = True) -> None:
        """Extend list by appending elements from the iterable. Set forward to false to extend from the start."""
        ...
    def index(self, value: _T, start: int = 0, stop: int = sys.maxsize) -> int:
        """Return first index of value.  
        Raises ValueError if the value is not present."""
        ...
    def insert(self, object: _T, index: int, forward: bool = True) -> None:
        """Insert object after index. Set forward to false to insert before index."""
        ...
    def pop(self, index: int = -1) -> _T:
        """Remove and return item at index (default last).  
        Raises IndexError if list is empty or index is out of range."""
        ...
    def remove(self, value: _T) -> None:
        """Remove first occurence of value.  
        Raises ValueError if the value is not present."""
        ...
    def reverse(self) -> None:
        """Reverse the order of the list."""
        ...
    @overload
    def sort(self: DoublyLinkedList[SupportsRichComparisonT], key: None = None, reverse: bool = False) -> None:
        """In-place sort in ascending order, equal objects are not swapped. Reverse will reverse the sort order."""
        ...
    @overload
    def sort(self, key: Callable[[_T], SupportsRichComparison], reverse: bool = False) -> None:
        """In-place sort in ascending order, equal objects are not swapped. Key can be applied to values and the list will be sorted based on the result of applying the key. Reverse will reverse the sort order."""
        ...
    @overload
    def __add__(self, value: Iterable[_T], /) -> DoublyLinkedList[_T]: ...
    @overload
    def __add__(self, value: Iterable[_S], /) -> DoublyLinkedList[_T | _S]: ...