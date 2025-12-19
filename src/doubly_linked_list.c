// TODO rich comparison, repeat option, check errors
#define PY_SSIZE_T_CLEAN
#include <Python.h>
#include <structmember.h>
#include <string.h>
#include <stddef.h>
#include <math.h>
#if PY_MINOR_VERSION < 10
    #define Py_IsNone(x) Py_Is((x), Py_None) // Define these so that we can use them on older versions
    #define Py_Is(x,y) ((x) == (y))
    static inline PyObject* Py_NewRef(PyObject *obj)
    {
        Py_INCREF(obj);
        return obj;
    }
#endif

// Typedefs

typedef struct DLLNode
{
	PyObject* value;
    struct DLLNode* next;
    struct DLLNode* prev;
	PyObject* key;
} DLLNode;

// - - - - - DoublyLinkedListNode - - - - - //

// Initalization and Deallocation

static void
DLLNode_dealloc(DLLNode* op)
{
    Py_XDECREF(op->value);
    Py_XDECREF(op->key);
    free(op);
}

static void DLLNode_dealloc_chain(DLLNode* op)
{
    Py_XDECREF(op->value);
    if(op->next != NULL) {
        DLLNode_dealloc_chain(op->next);
    }
    free(op);
}

static DLLNode* DLLNode_new()
{
    DLLNode *self = malloc(sizeof(DLLNode));
    self->value = Py_NewRef(Py_None);
    if (!self->value) {
        DLLNode_dealloc(self);
        return NULL;
    }
    self->key = NULL;
    self->next = NULL;
    self->prev = NULL;
    return self;
}

// __Methods__

static PyObject* DLLNode_str(DLLNode* op)
{
    DLLNode* self = op;
    return PyUnicode_FromFormat("%S", self->value);
}

static PyObject* DLLNode_repr(DLLNode* op)
{
    DLLNode* self = op;
    return PyUnicode_FromFormat("%R", self->value);
}


// - - - - - DoublyLinkedList - - - - - //

typedef struct
{
	PyObject_HEAD
    DLLNode* head;
    DLLNode* tail;
    DLLNode* cursor;
    Py_ssize_t cursor_pos;
    Py_ssize_t length;
} DoublyLinkedList;

static PyTypeObject DoublyLinkedListType;

// Define internal helper methods

static int DoublyLinkedList_locate(PyObject*, Py_ssize_t);
static int DoublyLinkedList_cursor_insert(PyObject*, PyObject*, int);
static int DoublyLinkedList_append_iterator(PyObject*, PyObject*, int);
static int DoublyLinkedList_cursor_delete(PyObject*);

// Initialization and deallocation

static void
DoublyLinkedList_dealloc(PyObject *op)
{
    DoublyLinkedList* self = (DoublyLinkedList*)op;
    if(self->head) { DLLNode_dealloc_chain(self->head); }
    Py_TYPE(self)->tp_free(self);
}

static PyObject* DoublyLinkedList_new(PyTypeObject *type, PyObject *args, PyObject *kwds)
{
    DoublyLinkedList *self;
    self = (DoublyLinkedList*)type->tp_alloc(type, 0);
    if(self)
    {
        self->head = NULL;
        self->tail = NULL;
        self->cursor = NULL;
        self->cursor_pos = 0;
        self->length = 0;
    }
    return (PyObject*)self;
}

static int
DoublyLinkedList_init(PyObject* op, PyObject *args)
{
    DoublyLinkedList* self = (DoublyLinkedList*)op;
    PyObject* iterable = NULL;
    if(!PyArg_ParseTuple(args, "|O", &iterable)) { return -1; }
    if(iterable)
    {
        if(DoublyLinkedList_append_iterator((PyObject*)self, iterable, 1)) { return -1; }
    }
    return 0;
}

// Methods

static PyObject* DoublyLinkedList_insert(PyObject* op, PyObject* args, PyObject* kwds)
{
    DoublyLinkedList* self = (DoublyLinkedList*)op;
    static char* kwlist[] = {"object", "index", "forward", NULL};
    PyObject* object = NULL;
    Py_ssize_t index;
    int forward = 1;
    if(!PyArg_ParseTupleAndKeywords(args, kwds, "On|i", kwlist,
                                    &object, &index, &forward))
    {                              
        return NULL;
    }
    if(DoublyLinkedList_locate((PyObject*)self, index)) { return NULL; }
    if(DoublyLinkedList_cursor_insert((PyObject*)self, object, forward)) { return NULL; }
    return Py_NewRef(Py_None);
}

static PyObject* DoublyLinkedList_append(PyObject* op, PyObject* args, PyObject* kwds)
{
    DoublyLinkedList* self = (DoublyLinkedList*) op;
    static char* kwlist[] = {"object", "forward", NULL};
    PyObject* object = NULL;
    int forward = 1;
    if (!PyArg_ParseTupleAndKeywords(args, kwds, "O|i", kwlist, &object, &forward)) { return NULL; }
    if(forward) { self->cursor = self->tail; self->cursor_pos = self->length-1; if(self->cursor_pos < 0) {self->cursor_pos = 0;} }
    else { self->cursor = self->head; self->cursor_pos = 0; }
    if(DoublyLinkedList_cursor_insert((PyObject*)self, object, forward)) { return NULL; }
    return Py_NewRef(Py_None);
}

static PyObject* DoublyLinkedList_index(PyObject* op, PyObject* args, PyObject* kwds)
{
    DoublyLinkedList* self = (DoublyLinkedList*)op;
    static char* kwlist[] = {"value", "start", "stop", NULL};
    PyObject* value; Py_ssize_t start = 0; Py_ssize_t stop = self->length;
    if(!PyArg_ParseTupleAndKeywords(args, kwds, "O|nn", kwlist, &value, &start, &stop)) { return NULL; }
    if(DoublyLinkedList_locate((PyObject*)self, start)) { return NULL; }
    for(Py_ssize_t i=start; i<stop; i++)
    {
        int rslt = PyObject_RichCompareBool((self->cursor)->value, value, Py_EQ);
        if(rslt == -1) { return NULL; }
        if(rslt)
        {
            PyObject* rtn = PyLong_FromSsize_t(i); if(!rtn) { return NULL; }
            return rtn;
        }
        self->cursor = (self->cursor)->next;
        self->cursor_pos += 1;
    }
    PyObject* err_format = PyUnicode_FromFormat("%S not in list", value); if(!err_format) { return NULL; }
    const char* err_str = PyUnicode_AsUTF8(err_format); if(!err_str) { Py_DECREF(err_format); return NULL; }
    PyErr_SetString(PyExc_ValueError, err_str);
    Py_DECREF(err_format);
    return NULL;
}

static PyObject* DoublyLinkedList_pop(PyObject* op, PyObject* args, PyObject* kwds)
{
    DoublyLinkedList* self = (DoublyLinkedList*) op;
    static char* kwlist[] = {"index", NULL};
    Py_ssize_t index = self->length-1;
    if(!PyArg_ParseTupleAndKeywords(args, kwds, "|n", kwlist, &index)) { return NULL; }
    if(DoublyLinkedList_locate((PyObject*)self, index)) { return NULL; }
    DLLNode* cursor = self->cursor;
    PyObject* popped = Py_NewRef(cursor->value);
    if(DoublyLinkedList_cursor_delete((PyObject*)self)) { return NULL; }
    return popped;
}

static PyObject* DoublyLinkedList_remove(PyObject* op, PyObject* args, PyObject* kwds)
{
    DoublyLinkedList* self = (DoublyLinkedList*)op;
    if(!DoublyLinkedList_index((PyObject*)self, args, kwds)) { return NULL; }
    if(DoublyLinkedList_cursor_delete((PyObject*)self)) { return NULL; }
    return Py_NewRef(Py_None);
}

static PyObject* DoublyLinkedList_extend(PyObject* op, PyObject* args, PyObject* kwds)
{
    DoublyLinkedList* self = (DoublyLinkedList* )op;
    static char* kwlist[] = {"iterable", "forward", NULL};
    PyObject* iterable;
    int forward = 1;
    if(!PyArg_ParseTupleAndKeywords(args, kwds, "O|i", kwlist, &iterable, &forward)) { return NULL; }
    if(DoublyLinkedList_append_iterator((PyObject*)self, iterable, forward)) { return NULL; }
    return Py_NewRef(Py_None);
}

static PyObject* DoublyLinkedList_copy(PyObject* op)
{
    DoublyLinkedList* copy = (DoublyLinkedList*)DoublyLinkedList_new(&DoublyLinkedListType, NULL, NULL); if(!copy) { return NULL; }
    if(DoublyLinkedList_append_iterator((PyObject*)copy, op, 1)) { return NULL; }
    return (PyObject*)copy;
}

static PyObject* DoublyLinkedList_reverse(PyObject* op)
{
    DoublyLinkedList* self = (DoublyLinkedList*)op;
    Py_ssize_t middle = self->length / 2;
    PyObject* temp;
    DLLNode* node1 = self->head;
    DLLNode* node2 = self->tail;
    for(Py_ssize_t i = 0; i < middle; i++){
        temp = (PyObject*)node1->value;
        node1->value = node2->value;
        node2->value = (PyObject*)temp;
        node1 = node1->next;
        node2 = node2->prev;
    }
    return Py_NewRef(Py_None);
}

static PyObject* DoublyLinkedList_count(PyObject* op, PyObject* args, PyObject* kwds)
{
    DoublyLinkedList* self = (DoublyLinkedList*)op;
    static char* kwlist[] = {"value", NULL};
    PyObject* value;
    if(!PyArg_ParseTupleAndKeywords(args, kwds, "O", kwlist, &value)) { return NULL; }
    DLLNode* temp = self->head;
    Py_ssize_t count = 0;
    for(Py_ssize_t i = 0; i<self->length; i++)
    {
        int rslt = PyObject_RichCompareBool(temp->value, value, Py_EQ);
        if(rslt == -1) { return NULL; }
        if(rslt) { count += 1; }
        temp = temp->next;
    }
    PyObject* rtn = PyLong_FromSsize_t(count); if(!rtn) { return NULL; }
    return rtn;
}

static PyObject* DoublyLinkedList_clear_method(PyObject* op)
{
    DoublyLinkedList* self = (DoublyLinkedList*)op;
    if(self->length == 0) { return Py_NewRef(Py_None); }
    DLLNode_dealloc_chain(self->head);
    self->head = NULL; self->tail = NULL; self->cursor = NULL;
    self->length = 0; self->cursor_pos = 0;
    return Py_NewRef(Py_None);
}

//Helper method for sort, swaps two nodes that are next to each
static void swap(DLLNode* node1, DLLNode* node2)
{
    PyObject* temp = node1->value;
    node1->value = node2->value;
    node2->value = temp;
}

static void swap_with_key(DLLNode* node1, DLLNode* node2)
{
    PyObject* temp = node1->value;
    node1->value = node2->value;
    node2->value = temp;
    temp = node1->key;
    node1->key = node2->key;
    node2->key = temp;
}

static PyObject* DoublyLinkedList_sort(PyObject* op, PyObject* args, PyObject* kwds)
{
    DoublyLinkedList* self = (DoublyLinkedList*)op;
    char* kwlist[] = {"key", "reverse", NULL};
    PyObject* key = NULL; int reverse = 0;
    if(!PyArg_ParseTupleAndKeywords(args, kwds, "|Oi", kwlist, &key, &reverse)) { return NULL; }
    int operator;
    if(reverse) { operator = Py_GT; } else { operator = Py_LT; }
    DLLNode* temp = self->head;
    DLLNode* next = temp->next;
    DLLNode* prev;
    int comparison;

    if(key) {
		if(!PyCallable_Check(key)) { PyErr_SetString(PyExc_TypeError, "Key must be a callable"); return NULL; }
		for(int i = 0; i < self->length; i++)
        {
			PyObject* value_key = PyObject_CallOneArg(key, temp->value);
			if(!value_key) { return NULL; }
			temp->key = value_key;
			temp = temp->next;
		}
		for(int i = 1; i < self->length; i++)
        {
            temp = next;
            next = temp->next;
            for(int j = i; j >= 1; j--)
            {
                prev = temp->prev;
                comparison = PyObject_RichCompareBool(temp->key, prev->key, operator);
                if(comparison == -1) { return NULL; }
                if(comparison) { swap_with_key(prev, temp); temp = prev; }
                else { break; }
            }
    	}
		temp = self->head;
		for(int i =0; i < self->length; i++)
        {
			Py_DECREF(temp->key);
            temp->key = NULL;
			temp = temp->next;
		}
		self->cursor_pos = 0;
        self->cursor = self->head;
    	return Py_NewRef(Py_None);
	}
	else
    {
    	for(int i = 1; i < self->length; i++)
        {
            temp = next;
            next = temp->next;
            for(int j = i; j >= 1; j--)
            {
                prev = temp->prev;
                comparison = PyObject_RichCompareBool(temp->value, prev->value, operator);
                if(comparison == -1) { return NULL; }
                if(comparison) { swap(prev, temp); temp = prev; }
                else { break; }
        	}
    	}
    	self->cursor_pos = 0;
        self->cursor = self->head;
    	return Py_NewRef(Py_None);
	}
}

// Internal Methods

// Takes in DoublyLinkedList and index, locates node at that index and sets cursor to it
static int DoublyLinkedList_locate(PyObject* op, Py_ssize_t index)
{
    DoublyLinkedList* self = (DoublyLinkedList*)op;
    if(index < 0) { index = self->length + index; }
    if(index >= self->length || index < 0)
    {
        PyErr_SetString(PyExc_IndexError, "Index out of bounds");
        return -1;
    }
    DLLNode* search_node = self->cursor;
    Py_ssize_t search_distance = index-self->cursor_pos;
    const Py_ssize_t head_distance = index;
    const Py_ssize_t tail_distance = index-(self->length-1);
    if(labs(head_distance) < labs(search_distance))
    {
        search_node = self->head;
        search_distance = head_distance;
    }
    else if(labs(tail_distance) < labs(search_distance))
    {
        search_node = self->tail;
        search_distance = tail_distance;
    }
    if(search_distance>0)
    {
        for(Py_ssize_t i = 0; i<search_distance; i++)
        {
            search_node = search_node->next;
        }
    }
    else if(search_distance<0)
    {
        for(Py_ssize_t i=0; i>search_distance; i--)
        {
            search_node = search_node->prev;
        }
    }
    self->cursor = search_node;
    self->cursor_pos = index;
    return 0;
}

// Create a new node with value and inserts it forwards or backwards and sets cursor to it.
static int DoublyLinkedList_cursor_insert(PyObject* op, PyObject* object, int forward)
{
    DoublyLinkedList* self = (DoublyLinkedList*)op;
    self->length += 1;
    DLLNode* node = DLLNode_new(); if(!node) { return -1; }
    Py_SETREF(node->value, Py_NewRef(object));
    if(self->cursor == NULL)
    {
        self->head = node;
        self->tail = node;
        self->cursor = node;
    }
    else
    {
        DLLNode* cursor = self->cursor;
        if(forward)
        {
            self->cursor_pos += 1;
            if(cursor->next == NULL)
            {
                node->prev = cursor;
                self->tail = node;
                cursor->next = node;
            }
            else
            {
                DLLNode* temp = cursor->next;
                node->prev = cursor;
                temp->prev = node;
                node->next = temp;
                cursor->next = node;
            }
        }
        else
        {
            if(cursor->prev == NULL)
            {
                cursor->prev = node;
                self->head = node;
                node->next = cursor;
            }
            else
            {
                DLLNode* temp = cursor->prev;
                node->prev = temp;
                cursor->prev = node;
                temp->next = node;
                node->next = node;
            }
        }
    }
    self->cursor = node;
    return 0;
}

static int DoublyLinkedList_cursor_delete(PyObject* op)
{
    DoublyLinkedList* self = (DoublyLinkedList*)op;
    self->length -= 1;
    DLLNode* cursor = self->cursor;
    if(cursor->next == NULL)
    {
        if(cursor->prev == NULL)
        {
            self->head = NULL;
            self->tail = NULL;
            self->cursor = NULL;
        }
        else
        {
            self->tail = cursor->prev;
            self->cursor = cursor->prev;
            (cursor->prev)->next = cursor->next;
            self->cursor_pos-=1;
        }
    }
    else
    {
        (cursor->next)->prev = cursor->prev;
        self->cursor = cursor->next;
        if(cursor->prev == NULL) { self->head = cursor->next; }
        else { (cursor->prev)->next = cursor->next; }
    }
    DLLNode_dealloc(cursor);
    return 0;
}

static int DoublyLinkedList_append_iterator(PyObject* op, PyObject* iterable, int forward)
{
    DoublyLinkedList* self = (DoublyLinkedList*)op;
    PyObject* iterator = PyObject_GetIter(iterable); if(!iterator) { return -1; }
    if(forward)
    { 
        self->cursor = self->tail; self->cursor_pos = self->length-1;
        if(self->cursor_pos<0) { self->cursor_pos=0; }
    }
    else { self->cursor = self->head; self->cursor_pos = 0; }
    PyObject* item;
    while((item = PyIter_Next(iterator)) != NULL)
    {
        if(DoublyLinkedList_cursor_insert((PyObject*)self, item, forward)) { return -1; }
        Py_DECREF(item);
    }
    if(PyErr_Occurred())
    {
        Py_XDECREF(iterator);
        return -1;
    }
    Py_XDECREF(iterator);
    return 0;
}

static PyObject* DoublyLinkedList_rich_compare(PyObject* self, PyObject* other, int op)
{
    if(op == Py_NE)
    {
        PyObject* result = DoublyLinkedList_rich_compare(self, other, Py_EQ);
        if(result == Py_False)
        {
            Py_DECREF(Py_False);
            Py_RETURN_TRUE;
        }
        else if(result == Py_True)
        {
            Py_DECREF(Py_True);
            Py_RETURN_FALSE;
        }
        return NULL;
    }
    if(op != Py_EQ)
    {
        return Py_NotImplemented;
    }
    PyObject* iterator = PyObject_GetIter(other);
    if(!iterator) { return Py_False; }
    DLLNode* temp_node = ((DoublyLinkedList*)self)->head;
    PyObject* temp_iter;
    while(temp_node)
    {
        temp_iter = PyIter_Next(iterator);
        if(!temp_iter)
        {
            if(PyErr_Occurred())
            {
                Py_XDECREF(iterator);
                return NULL;
            }
            Py_RETURN_FALSE;
        }
        int rslt = PyObject_RichCompareBool(temp_node->value, temp_iter, Py_EQ); if(rslt == -1) { return NULL; }
        if(!rslt) { Py_RETURN_FALSE; }
        temp_node = temp_node->next;
    }
    temp_iter = PyIter_Next(iterator);
    if(temp_iter)
    {
        Py_RETURN_FALSE;
    }
    else if(PyErr_Occurred())
    {
        Py_XDECREF(iterator);
        return NULL;
    }
    Py_RETURN_TRUE;
}

// Mapping Methods

static PyObject* DoublyLinkedList_subscript(PyObject* op, PyObject* slice){
    DoublyLinkedList* self = (DoublyLinkedList*)op;
    if(PySlice_Check(slice))
    {
        Py_ssize_t start, stop, step;
        DoublyLinkedList* list_slice = (DoublyLinkedList*)DoublyLinkedList_new(&DoublyLinkedListType, NULL, NULL); if(!list_slice) { return NULL; }
        DLLNode* temp;
        if(PySlice_Unpack(slice, &start, &stop, &step) == -1) { return NULL; }
        if(start < 0) { start = self->length + start; }
        if(stop < 0) { stop = self->length + stop; } if(stop > self->length) { stop = self->length; }
        if(step > 0)
        {
            for(Py_ssize_t i = start; i < stop; i+=step)
            {
                if(DoublyLinkedList_locate((PyObject*)self, i)) { return NULL; }
                temp = self->cursor;
                if(DoublyLinkedList_cursor_insert((PyObject*)list_slice, temp->value, 1)) { return NULL; }
            }
        }
        if(step < 0)
        {
            for(Py_ssize_t i = start; i > stop; i+=step)
            {
                if(DoublyLinkedList_locate((PyObject*)self, i)) { return NULL; }
                temp = self->cursor;
                if(DoublyLinkedList_cursor_insert((PyObject*)list_slice, temp->value, 1)) { return NULL; }
            }
        }
        return (PyObject*)list_slice;
    }
    else if(PyLong_Check(slice))
    {
        Py_ssize_t index = PyLong_AsSsize_t(slice); if(index == -1 && PyErr_Occurred()) { return NULL; }
        if(DoublyLinkedList_locate((PyObject*)self, index)) { return NULL; }
        DLLNode* cursor = self->cursor;
        return Py_NewRef(cursor->value);
    }
    else { PyErr_SetString(PyExc_TypeError, "Index must be an integer or slice"); return NULL; }
}

// Sequence Methods 

static Py_ssize_t DoublyLinkedList_len(PyObject* op, PyObject* args, PyObject* kwds)
{
    DoublyLinkedList* self = (DoublyLinkedList*)op;
    return self->length;
}

static PyObject* DoublyLinkedList_item(PyObject* op, Py_ssize_t index)
{
    DoublyLinkedList* self = (DoublyLinkedList*)op;
    if(DoublyLinkedList_locate((PyObject*)self, index)) { return NULL; }
    DLLNode* cursor = self->cursor;
    return Py_NewRef(cursor->value);
}

static int DoublyLinkedList_ass_item(PyObject* op, Py_ssize_t index, PyObject* value)
{
    DoublyLinkedList* self = (DoublyLinkedList*)op;
    if(DoublyLinkedList_locate((PyObject*)self, index)) { return -1; }
    if(!value)
    {
        if(DoublyLinkedList_cursor_delete((PyObject*)self)) { return -1; }
        return 0;
    }
    DLLNode* cursor = self->cursor;
    Py_SETREF(cursor->value, Py_NewRef(value));
    return 0;
}

static PyObject* DoublyLinkedList_concat(PyObject* op, PyObject* concat)
{
    PyObject* new_list = DoublyLinkedList_new(&DoublyLinkedListType, NULL, NULL); if(new_list == NULL) { return NULL; }
    if(DoublyLinkedList_append_iterator(new_list, op, 1)) { return NULL; }
    if(DoublyLinkedList_append_iterator(new_list, concat, 1)) { return NULL; }
    return new_list;
}

static PyObject* DoublyLinkedList_inplace_concat(PyObject* op, PyObject* concat)
{
    if(DoublyLinkedList_append_iterator(op, concat, 1)) { return NULL; }
    return Py_NewRef(op);
}

static int DoublyLinkedList_contains(PyObject* op, PyObject* value)
{
    DoublyLinkedList* self = (DoublyLinkedList*)op;
    DLLNode* temp = self->head;
    for(Py_ssize_t i = 0; i<self->length; i++)
    {
        if(temp->value==value) { return 1; }
        temp = temp->next;
    }
    return 0;
}

// __Methods__

static PyObject* DoublyLinkedList_str(PyObject* op, PyObject* Py_UNUSED(dummy))
{
    DoublyLinkedList* self = (DoublyLinkedList*)op;
    if(self->length == 0) { return PyUnicode_FromString("[]"); }
    PyObject* string = PyUnicode_FromString("["); if(!string) { return NULL; }
    PyObject* new_string;
    DLLNode* temp = self->head;
    for(Py_ssize_t i = 1; i < self->length; i++)
    {
        PyObject* node_str = DLLNode_repr(temp); if(!node_str) { return NULL; }
        PyObject* format_node_str = PyUnicode_FromFormat("%U, ", node_str); if(!format_node_str) { return NULL; }
        new_string = PyUnicode_Concat(string, format_node_str); if(!new_string) { return NULL; }
        Py_DECREF(node_str); Py_DECREF(format_node_str); Py_DECREF(string);
        string = new_string;
        temp = temp->next;
    }
    new_string = PyUnicode_Concat(string, PyUnicode_FromFormat("%U]", DLLNode_repr(temp)));
    Py_DECREF(string);
    string = new_string;
    return string;
}

static PyMethodDef DoublyLinkedList_methods[] = {
    {"append", (PyCFunction)DoublyLinkedList_append, METH_VARARGS|METH_KEYWORDS,
    "Append object to the end of the list. Set forward to false to append to the start."},
    {"clear", (PyCFunction)DoublyLinkedList_clear_method, METH_NOARGS,
    "Remove all items from the list."},
    {"copy", (PyCFunction)DoublyLinkedList_copy, METH_NOARGS,
    "Return a shallow copy of the list."},
    {"count", (PyCFunction)DoublyLinkedList_count, METH_VARARGS|METH_KEYWORDS,
    "Return number of occurrences of value in the list."},
    {"extend", (PyCFunction)DoublyLinkedList_extend, METH_VARARGS|METH_KEYWORDS,
    "Extend list by appending elements from the iterable. Set forward to false to extend from the start."},
    {"index", (PyCFunction)DoublyLinkedList_index, METH_VARARGS|METH_KEYWORDS,
    "Return first index of value.\nRaises ValueError if the value is not present."},
    {"insert", (PyCFunction)DoublyLinkedList_insert, METH_VARARGS|METH_KEYWORDS,
     "Insert object after index. Set forward to false to insert before index."},
    {"pop", (PyCFunction)DoublyLinkedList_pop, METH_VARARGS|METH_KEYWORDS,
    "Remove and return item at index (default last).\nRaises IndexError if list is empty or index is out of range."},
    {"remove", (PyCFunction)DoublyLinkedList_remove, METH_VARARGS|METH_KEYWORDS,
    "Remove first occurence of value.\nRaises ValueError if the value is not present."},
    {"reverse", (PyCFunction)DoublyLinkedList_reverse, METH_NOARGS,
    "Reverse the order of the list."},
    {"sort", (PyCFunction)DoublyLinkedList_sort, METH_VARARGS|METH_KEYWORDS,
    "In-place sort in ascending order, equal objects are not swapped. Key can be applied to values and the list will be sorted based on the result of applying the key. Reverse will reverse the sort order."},
    {NULL, NULL, 0, NULL}
};

static PyMappingMethods DoublyLinkedList_map =
{
    .mp_subscript = DoublyLinkedList_subscript
};

static PySequenceMethods DoublyLinkedList_sequence =
{
    .sq_length = (lenfunc)DoublyLinkedList_len,
    .sq_item = DoublyLinkedList_item,
    .sq_ass_item = DoublyLinkedList_ass_item,
    .sq_concat = DoublyLinkedList_concat,
    .sq_inplace_concat = DoublyLinkedList_inplace_concat,
    .sq_contains = DoublyLinkedList_contains
};

// Type Definition

static PyTypeObject DoublyLinkedListType =
{
    .ob_base = PyVarObject_HEAD_INIT(NULL, 0)
    .tp_name = "py_doubly_linked_list.doubly_linked_list.DoublyLinkedList",
    .tp_doc = PyDoc_STR("DoublyLinkedList class"),
    .tp_basicsize = sizeof(DoublyLinkedList),
    .tp_itemsize = 0,
    .tp_flags = Py_TPFLAGS_DEFAULT,
    .tp_new = (newfunc)DoublyLinkedList_new,
    .tp_init = (initproc)DoublyLinkedList_init,
    .tp_dealloc = (destructor)DoublyLinkedList_dealloc,
    .tp_str = (reprfunc)DoublyLinkedList_str,
    .tp_richcompare = (richcmpfunc)DoublyLinkedList_rich_compare,
    .tp_methods = DoublyLinkedList_methods,
    .tp_as_sequence = &DoublyLinkedList_sequence,
    .tp_as_mapping = &DoublyLinkedList_map
};

static int doubly_linked_list_module_exec(PyObject *m)
{
    if (PyType_Ready(&DoublyLinkedListType) < 0) { return -1; }
    Py_INCREF(&DoublyLinkedListType);
    if (PyModule_AddObject(m, "DoublyLinkedList", (PyObject*)&DoublyLinkedListType) < 0)
    {
        Py_DECREF(&DoublyLinkedListType);
        Py_DECREF(m);
        return -1;
    }
    return 0;
}

#if PY_MINOR_VERSION >= 12

static PyModuleDef_Slot py_doubly_linked_list_module_slots[] =
{
    {Py_mod_exec, doubly_linked_list_module_exec},
    {Py_mod_multiple_interpreters, Py_MOD_MULTIPLE_INTERPRETERS_NOT_SUPPORTED},
    {0, NULL}
};

#else

static PyModuleDef_Slot py_doubly_linked_list_module_slots[] =
{
    {Py_mod_exec, doubly_linked_list_module_exec},
    {0, NULL}
};

#endif

static struct PyModuleDef py_doubly_linked_list_module =
{
	PyModuleDef_HEAD_INIT,
	"py_doubly_linked_list.doubly_linked_list",
	"A library implementing a doubly linked list for python",
	.m_slots = py_doubly_linked_list_module_slots
};

PyMODINIT_FUNC PyInit_doubly_linked_list(void)
{
	return PyModuleDef_Init(&py_doubly_linked_list_module);
}
