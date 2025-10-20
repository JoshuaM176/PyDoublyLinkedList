#define PY_SSIZE_T_CLEAN
#include <Python.h>
#include <structmember.h>
#include <string.h>
#include <stdlib.h>
#include <stddef.h>
#include <stdio.h>
#include <math.h>

// - - - - - DoublyLinkedListNode - - - - - //

typedef struct {
	PyObject_HEAD
	PyObject* value;
    PyObject* next;
    PyObject* prev;
} DLLNode;

static PyTypeObject DLLNodeType;

// Initalization and Deallocation

static int
DLLNode_traverse(PyObject *op, visitproc visit, void *arg)
{
    DLLNode* self = (DLLNode* ) op;
    Py_VISIT(self->value);
    Py_VISIT(self->next);
    Py_VISIT(self->prev);
    return 0;
}

static int
DLLNode_clear(PyObject *op)
{
    DLLNode* self = (DLLNode* )op;
    Py_CLEAR(self->value);
    Py_CLEAR(self->next);
    Py_CLEAR(self->prev);
    return 0;
}

static void
DLLNode_dealloc(PyObject *op)
{
    DLLNode* self = (DLLNode* )op;
    PyObject_GC_UnTrack(op);
    (void)DLLNode_clear(op);
    Py_TYPE(self)->tp_free(self);
}

static PyObject *
DLLNode_new(PyTypeObject *type, PyObject *args, PyObject *kwds)
{
    DLLNode *self;
    self = (DLLNode *) type->tp_alloc(type, 0);
    if (self != NULL) {
        self->value = Py_NewRef(Py_None);
        if (self->value == NULL) {
            Py_DECREF(self);
            return NULL;
        }
        self->next = Py_NewRef(Py_None);
        if (self->next == NULL) {
            Py_DECREF(self);
            return NULL;
        }
        self->prev = Py_NewRef(Py_None);
        if (self->prev == NULL) {
            Py_DECREF(self);
            return NULL;
        }
    }
    return (PyObject *) self;
}

// Getters and Setters

static int DLLNode_set(PyObject* op, PyObject* value, void* closure){
    PyErr_SetString(PyExc_TypeError, "Default DLLNode attributes cannot be manually altered");
    return -1;
}

static PyObject* DLLNode_value_get(PyObject* op, void* closure){
    DLLNode* self = (DLLNode* )op;
    return Py_NewRef(self->value);
}

static PyObject* DLLNode_next_get(PyObject* op, void* closure){
    DLLNode* self = (DLLNode* )op;
    return Py_NewRef(self->next);
}

static PyObject* DLLNode_prev_get(PyObject* op, void* closure){
    DLLNode* self = (DLLNode* )op;
    return Py_NewRef(self->prev);
}

static PyGetSetDef DLLNode_getsetters[] = {
    {"value", DLLNode_value_get, DLLNode_set,
    "value held by node", NULL},
    {"next", DLLNode_next_get, DLLNode_set, 
    "next node", NULL},
    {"prev", DLLNode_prev_get, DLLNode_set,
    "prev node", NULL},
    {NULL}
};

// __Methods__

static PyObject* DLLNode_str(PyObject* op, PyObject* Py_UNUSED(dummy)){
    DLLNode* self = (DLLNode* )op;
    return PyUnicode_FromFormat("%S", self->value);
}

//Type Definition

static PyTypeObject DLLNodeType = {
    .ob_base = PyVarObject_HEAD_INIT(NULL, 0)
    .tp_name = "py_doubly_linked_list.DLLNode",
    .tp_doc = PyDoc_STR("Node for doubly linked list"),
    .tp_basicsize = sizeof(DLLNode),
    .tp_itemsize = 0,
    .tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_HAVE_GC,
    .tp_new = DLLNode_new,
    .tp_dealloc = DLLNode_dealloc,
    .tp_traverse = DLLNode_traverse,
    .tp_getset = DLLNode_getsetters,
    .tp_str = DLLNode_str
};

static int dllnode_module_exec(PyObject *m)
{
    if (PyType_Ready(&DLLNodeType) < 0) {
        return -1;
    }

    return 0;
}

// - - - - - DoublyLinkedList - - - - - //

typedef struct {
	PyObject_HEAD
    PyObject* head;
    PyObject* tail;
    PyObject* cursor;
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

static int
DoublyLinkedList_traverse(PyObject *op, visitproc visit, void *arg)
{
    DoublyLinkedList* self = (DoublyLinkedList* ) op;
    Py_VISIT(self->head);
    Py_VISIT(self->tail);
    Py_VISIT(self->cursor);
    return 0;
}

static int
DoublyLinkedList_clear(PyObject *op)
{
    DoublyLinkedList* self = (DoublyLinkedList* )op;
    Py_CLEAR(self->head);
    Py_CLEAR(self->tail);
    Py_CLEAR(self->cursor);
    return 0;
}

static void
DoublyLinkedList_dealloc(PyObject *op)
{
    DoublyLinkedList* self = (DoublyLinkedList* )op;
    Py_XDECREF(self->head);
    Py_XDECREF(self->tail);
    Py_XDECREF(self->cursor);
    Py_TYPE(self)->tp_free(self);
}

static PyObject *
DoublyLinkedList_new(PyTypeObject *type, PyObject *args, PyObject *kwds)
{
    DoublyLinkedList *self;
    self = (DoublyLinkedList *) type->tp_alloc(type, 0);
    if (self != NULL) {
        self->head = Py_NewRef(Py_None);
        if (self->head == NULL) {
            Py_DECREF(self);
            return NULL;
        }
        self->tail = Py_NewRef(Py_None);
        if (self->tail == NULL) {
            Py_DECREF(self);
            return NULL;
        }
        self->cursor = Py_NewRef(Py_None);
        if (self->cursor == NULL) {
            Py_DECREF(self);
            return NULL;
        }
        self->cursor_pos = 0;
        self->length = 0;
    }
    return (PyObject *) self;
}

static int
DoublyLinkedList_init(PyObject* op, PyObject *args)
{
    DoublyLinkedList* self = (DoublyLinkedList* )op;
    PyObject* iterable = NULL;
    if (!PyArg_ParseTuple(args, "|O", &iterable))
        return NULL;
    if(iterable){
        if(DoublyLinkedList_append_iterator(self, iterable, 1)) {return -1;}
    }
    return 0;
}

// Methods

static PyObject* DoublyLinkedList_insert(PyObject* op, PyObject* args, PyObject* kwds){
    DoublyLinkedList* self = (DoublyLinkedList* ) op;
    static char* kwlist[] = {"object", "index", "forward", NULL};
    PyObject* object = NULL;
    int index;
    int forward = 1;
    if (!PyArg_ParseTupleAndKeywords(args, kwds, "Oi|i", kwlist,
                                     &object, &index, &forward))
        return NULL;
    if(DoublyLinkedList_locate(self, index)) {return NULL;}
    if(DoublyLinkedList_cursor_insert(self, object, forward)) {return NULL;}
    return Py_NewRef(Py_None);
}

static PyObject* DoublyLinkedList_append(PyObject* op, PyObject* args, PyObject* kwds){
    DoublyLinkedList* self = (DoublyLinkedList* ) op;
    static char* kwlist[] = {"object", "forward", NULL};
    PyObject* object = NULL;
    int forward = 1;
    if (!PyArg_ParseTupleAndKeywords(args, kwds, "O|i", kwlist,
                                     &object, &forward))
        return NULL;
    if(forward) {Py_SETREF(self->cursor, Py_NewRef(self->tail));}
    else {Py_SETREF(self->cursor, Py_NewRef(self->head));}
    if(DoublyLinkedList_cursor_insert(self, object, forward)) {return NULL;}
    return Py_NewRef(Py_None);
}

static PyObject* DoublyLinkedList_index(PyObject* op, PyObject* args, PyObject* kwds){
    DoublyLinkedList* self = (DoublyLinkedList* )op;
    static char* kwlist[] = {"value", "start", "stop", NULL};
    PyObject* value; Py_ssize_t start = 0; Py_ssize_t stop = self->length;
    DLLNode* cursor;
    if(!PyArg_ParseTupleAndKeywords(args, kwds, "O|ii", kwlist, &value, &start, &stop)) {return NULL;}
    for(Py_ssize_t i=start; i<stop; i++){
        if(DoublyLinkedList_locate(self, i)) {return NULL;}
        cursor = self->cursor;
        if(cursor->value == value) {
            PyObject* rtn = PyLong_FromSsize_t(i); if(!rtn) {return NULL;}
            return rtn;}
    }
    PyObject* err_value = PyObject_Str(value); if(!err_value) {return NULL;}
    PyObject* err_format = PyUnicode_FromFormat("%U not in list", err_value); if(!err_format) {return NULL;}
    PyErr_SetString(PyExc_ValueError, PyUnicode_AsUTF8(err_format));
    Py_DECREF(err_value); Py_DECREF(err_format);
    return NULL;
}

static PyObject* DoublyLinkedList_pop(PyObject* op, PyObject* args, PyObject* kwds){
    DoublyLinkedList* self = (DoublyLinkedList* ) op;
    static char* kwlist[] = {"index", NULL};
    int index = self->length-1;
    if(!PyArg_ParseTupleAndKeywords(args, kwds, "|i", kwlist, &index)) {return NULL;}
    if(DoublyLinkedList_locate(self, index)) {return NULL;}
    DLLNode* cursor = (DLLNode* )self->cursor;
    PyObject* popped = Py_NewRef(cursor->value);
    if(DoublyLinkedList_cursor_delete(self)) {return NULL;}
    return popped;
}

static PyObject* DoublyLinkedList_remove(PyObject* op, PyObject* args, PyObject* kwds){
    DoublyLinkedList* self = (DoublyLinkedList* )op;
    static char* kwlist[] = {"value", NULL};
    if(!DoublyLinkedList_index(self, args, kwds)) {return NULL;}
    if(DoublyLinkedList_cursor_delete(self)) {return NULL;}
    return Py_NewRef(Py_None);
}

static PyObject* DoublyLinkedList_extend(PyObject* op, PyObject* args, PyObject* kwds){
    DoublyLinkedList* self = (DoublyLinkedList* )op;
    static char* kwlist[] = {"iterable", "forward", NULL};
    PyObject* iterable;
    int forward = 1;
    if(!PyArg_ParseTupleAndKeywords(args, kwds, "O|i", kwlist, &iterable, &forward)) {return NULL;}
    if(DoublyLinkedList_append_iterator(self, iterable, forward)) {return NULL;}
    return Py_NewRef(Py_None);
}

static PyObject* DoublyLinkedList_copy(PyObject* op){
    DoublyLinkedList* copy = DoublyLinkedList_new(&DoublyLinkedListType, NULL, NULL); if(!copy) {return NULL;}
    if(DoublyLinkedList_append_iterator(copy, op, 1)) {return NULL;}
    return copy;
}

static PyObject* DoublyLinkedList_reverse(PyObject* op){
    DoublyLinkedList* self = (DoublyLinkedList* )op;
    Py_ssize_t middle = self->length / 2;
    PyObject* temp;
    DLLNode* node1 = self->head;
    DLLNode* node2 = self->tail;
    for(int i = 0; i < middle; i++){
        temp = node1->value;
        node1->value = node2->value;
        node2->value = temp;
        node1 = node1->next;
        node2 = node2->prev;
    }
    return Py_NewRef(Py_None);
}

static PyObject* DoublyLinkedList_count(PyObject* op, PyObject* args, PyObject* kwds) {
    DoublyLinkedList* self = (DoublyLinkedList* )op;
    static char* kwlist[] = {"value", NULL};
    PyObject* value;
    if(!PyArg_ParseTupleAndKeywords(args, kwds, "O", kwlist, &value)) {return NULL;}
    DLLNode* temp = self->head;
    Py_ssize_t count = 0;
    for(int i = 0; i<self->length; i++){
        if(temp->value==value) {count += 1;}
        temp = temp->next;
    }
    PyObject* rtn = PyLong_FromSsize_t(count); if(!rtn) {return NULL;}
    return rtn;
}

static PyObject* DoublyLinkedList_clear_method(PyObject* op){
    DoublyLinkedList* self = (DoublyLinkedList* )op;
    if(self->length == 0){
        return Py_NewRef(Py_None);
    }
    Py_DECREF(self->cursor);
    DLLNode* temp = self->head;
    DLLNode* next;
    for(int i = 0; i < self->length; i++){
        next = temp->next;
        Py_DECREF(temp); Py_DECREF(temp);
        temp = next;
    }
    self->head = Py_NewRef(Py_None); self->tail = Py_NewRef(Py_None); self->cursor = Py_NewRef(Py_None);
    self->length = 0; self->cursor_pos = 0;
    return Py_NewRef(Py_None);
}

// Internal Methods

// Takes in DoublyLinkedList and index, locates node at that index and sets cursor to it
static int DoublyLinkedList_locate(PyObject* op, Py_ssize_t index){
    DoublyLinkedList* self = (DoublyLinkedList* )op;
    if(index < 0) {index = self->length - index;}
    if(index >= self->length || index < 0){
        PyErr_SetString(PyExc_IndexError, "Index out of bounds");
        return -1;
    }
    DLLNode* search_node = self->cursor;
    int search_distance = index-self->cursor_pos;
    const int head_distance = index;
    const int tail_distance = index-(self->length-1);
    if(abs(head_distance) < abs(search_distance)){
        search_node = self->head;
        search_distance = head_distance;
    }
    else if(abs(tail_distance) < abs(search_distance)){
        search_node = self->tail;
        search_distance = tail_distance;
    }
    if(search_distance>0){
        for(int i = 0; i<search_distance; i++){
            search_node = search_node->next;
        }
    }
    else if(search_distance<0){
        for(int i=0; i>search_distance; i--){
            search_node = search_node->prev;
        }
    }
    Py_SETREF(self->cursor, Py_NewRef(search_node));
    self->cursor_pos = index;
    return 0;
}

// Create a new node with value and inserts it forwards or backwards and sets cursor to it.
static int DoublyLinkedList_cursor_insert(PyObject* op, PyObject* object, int forward) {
    DoublyLinkedList* self = (DoublyLinkedList* )op;
    self->length += 1;
    DLLNode* node = DLLNode_new(&DLLNodeType, NULL, NULL); if(!node) {return NULL;}
    Py_SETREF(node->value, Py_NewRef(object));
    if(Py_IsNone(self->cursor)){
        Py_SETREF(self->head, node);
        Py_SETREF(self->tail, Py_NewRef(node));
        Py_SETREF(self->cursor, Py_NewRef(node));
    }
    else{
        DLLNode* cursor = self->cursor;
        if(forward){
            self->cursor_pos += 1;
            if(Py_IsNone(cursor->next)){
                Py_SETREF(cursor->next, node);
                Py_SETREF(node->prev, Py_NewRef(cursor));
                Py_SETREF(self->tail, Py_NewRef(node));
            }
            else{
                DLLNode* temp = cursor->next;
                Py_SETREF(node->prev, Py_NewRef(cursor));
                Py_SETREF(node->next, Py_NewRef(temp));
                Py_SETREF(cursor->next, Py_NewRef(node));
                Py_SETREF(temp->prev, node);
            }
        }
        else{
            if(Py_IsNone(cursor->prev)){
                Py_SETREF(cursor->prev, node);
                Py_SETREF(node->next, Py_NewRef(cursor));
                Py_SETREF(self->head, Py_NewRef(node));
            }
            else{
                DLLNode* temp = cursor->next;
                Py_SETREF(node->next, Py_NewRef(cursor));
                Py_SETREF(node->prev, Py_NewRef(temp));
                Py_SETREF(cursor->prev, Py_NewRef(node));
                Py_SETREF(temp->next, node);
            }
        }
    }
    Py_SETREF(self->cursor, Py_NewRef(node));
    return 0;
}

static int DoublyLinkedList_cursor_delete(PyObject* op){
    DoublyLinkedList* self = (DoublyLinkedList* )op;
    self->length -= 1;
    DLLNode* cursor = self->cursor;
    if(Py_IsNone(cursor->next)){
        if(Py_IsNone(cursor->prev)){
            Py_SETREF(self->head, Py_NewRef(Py_None));
            Py_SETREF(self->tail, Py_NewRef(Py_None));
            Py_SETREF(self->cursor, Py_NewRef(Py_None));
        }
        else{
            Py_SETREF(((DLLNode*)(cursor->prev))->next, Py_NewRef(cursor->next));
            Py_SETREF(self->tail, Py_NewRef(cursor->prev));
            Py_SETREF(self->cursor, Py_NewRef(cursor->prev));
            self->cursor_pos-=1;
        }
    }
    else{
        if(Py_IsNone(cursor->prev)){
            Py_SETREF(self->head, Py_NewRef(cursor->next));
        }
        else{
            Py_SETREF(((DLLNode*)(cursor->prev))->next, Py_NewRef(cursor->next));
        }
        Py_SETREF(((DLLNode*)(cursor->next))->prev, Py_NewRef(cursor->prev));
        Py_SETREF(self->cursor, Py_NewRef(cursor->next));
    }
    return 0;
}

static int DoublyLinkedList_append_iterator(PyObject* op, PyObject* iterable, int forward){
    DoublyLinkedList* self = (DoublyLinkedList* )op;
    PyObject* iterator = PyObject_GetIter(iterable);
    if(!iterator){
        return -1;
    }
    if(forward) {Py_SETREF(self->cursor, Py_NewRef(self->tail));}
    else {Py_SETREF(self->cursor, Py_NewRef(self->head));}
    PyObject* item;
    while((item = PyIter_Next(iterator)) != NULL){
        if(DoublyLinkedList_cursor_insert(self, item, forward)) {return -1;}
        Py_DECREF(item);
    }
    if(PyErr_Occurred()){
        Py_XDECREF(iterator);
        return -1;

    }
    Py_XDECREF(iterator);
    return 0;
}

// Mapping Methods


// Sequence Methods 

static int DoublyLinkedList_len(PyObject* op, PyObject* args, PyObject* kwds){
    DoublyLinkedList* self = (DoublyLinkedList* )op;
    return self->length;
}

static PyObject* DoublyLinkedList_item(PyObject* op, Py_ssize_t index){
    DoublyLinkedList* self = (DoublyLinkedList* )op;
    if(DoublyLinkedList_locate(self, index)) {return NULL;}
    DLLNode* cursor = (DLLNode* )self->cursor;
    return Py_NewRef(cursor->value);
}

static int DoublyLinkedList_ass_item(PyObject* op, int index, PyObject* value) {
    DoublyLinkedList* self = (DoublyLinkedList* )op;
    if(DoublyLinkedList_locate(self, index)) {return -1;}
    if(!value){
        if(DoublyLinkedList_cursor_delete(self)) {return -1;}
        return 0;
    }
    DLLNode* cursor = (DLLNode* )self->cursor;
    Py_SETREF(cursor->value, Py_NewRef(value));
    return 0;
}

static PyObject* DoublyLinkedList_concat(PyObject* op, PyObject* concat){
    PyObject* new_list = DoublyLinkedList_new(&DoublyLinkedListType, NULL, NULL); if(new_list == NULL) {return NULL;}
    if(DoublyLinkedList_append_iterator(new_list, op, 1)) {return NULL;}
    if(DoublyLinkedList_append_iterator(new_list, concat, 1)) {return NULL;}
    return new_list;
}

static PyObject* DoublyLinkedList_inplace_concat(PyObject* op, PyObject* concat){
    if(DoublyLinkedList_append_iterator(op, concat, 1)) {return NULL;}
    return Py_NewRef(op);
}

static int DoublyLinkedList_contains(PyObject* op, PyObject* value){
    DoublyLinkedList* self = (DoublyLinkedList* )op;
    DLLNode* temp = self->head;
    for(int i = 0; i<self->length; i++){
        if(temp->value==value) {return 1;}
        temp = temp->next;
    }
    return 0;
}

// __Methods__

static PyObject* DoublyLinkedList_str(PyObject* op, PyObject* Py_UNUSED(dummy)){
    DoublyLinkedList* self = (DoublyLinkedList* )op;
    if(self->length == 0) {return PyUnicode_FromString("[]");}
    PyObject* string = PyUnicode_FromString("["); if(!string) {return NULL;}
    PyObject* new_string;
    DLLNode* temp = self->head;
    for(int i = 1; i<self->length; i++){
        PyObject* node_str = DLLNode_str(temp, NULL); if(!node_str) {return NULL;}
        PyObject* format_node_str = PyUnicode_FromFormat("%U, ", node_str); if(!format_node_str) {return NULL;}
        new_string = PyUnicode_Concat(string, format_node_str); if(!new_string) {return NULL;}
        Py_DECREF(node_str); Py_DECREF(format_node_str); Py_DECREF(string);
        string = new_string;
        temp = temp->next;
    }
    new_string = PyUnicode_Concat(string, PyUnicode_FromFormat("%U]", DLLNode_str(temp, NULL)));
    Py_DECREF(string);
    string = new_string;
    return string;
}

static PyMethodDef DoublyLinkedList_methods[] = {
    {"append", DoublyLinkedList_append, METH_VARARGS|METH_KEYWORDS,
    "Append object to the end of the list. Set forward to false to append to the start."},
    {"clear", DoublyLinkedList_clear_method, METH_NOARGS,
    "Remove all items from the list."},
    {"copy", DoublyLinkedList_copy, METH_NOARGS,
    "Return a shallow copy of the list."},
    {"count", DoublyLinkedList_count, METH_VARARGS|METH_KEYWORDS,
    "Return number of occurrences of value in the list."},
    {"extend", DoublyLinkedList_extend, METH_VARARGS|METH_KEYWORDS,
    "Extend list by appending elements from the iterable. Set forward to false to extend from the start."},
    {"index", DoublyLinkedList_index, METH_VARARGS|METH_KEYWORDS,
    "Return first index of value.\nRaises ValueError if the value is not present."},
    {"insert", DoublyLinkedList_insert, METH_VARARGS|METH_KEYWORDS,
     "Insert object after index. Set forward to false to insert before index."},
    {"pop", DoublyLinkedList_pop, METH_VARARGS|METH_KEYWORDS,
    "Remove and return item at index (default last).\nRaises IndexError if list is empty or index is out of range."},
    {"remove", DoublyLinkedList_remove, METH_VARARGS|METH_KEYWORDS,
    "Reverse the order of the list."},
    {"reverse", DoublyLinkedList_reverse, METH_NOARGS,
    "Remove first occurence of value.\nRaises ValueError if the value is not present."},
    {NULL, NULL, 0, NULL}
};

static PySequenceMethods DoublyLinkedList_sequence = {
    .sq_length = DoublyLinkedList_len,
    .sq_item = DoublyLinkedList_item,
    .sq_ass_item = DoublyLinkedList_ass_item,
    .sq_concat = DoublyLinkedList_concat,
    .sq_inplace_concat = DoublyLinkedList_inplace_concat,
    .sq_contains = DoublyLinkedList_contains
};

//Member Definition

static PyMemberDef DoublyLinkedList_members[] = {
    {"head", T_OBJECT_EX, offsetof(DoublyLinkedList, head), 0,
     "Head node."},
    {"tail", T_OBJECT_EX, offsetof(DoublyLinkedList, tail), 0,
     "Tail node."},
    {"cursor", T_OBJECT_EX, offsetof(DoublyLinkedList, cursor), 0,
     "Cursor tracking most recently accessed node."},
    {"cursor_pos", T_INT, offsetof(DoublyLinkedList, cursor_pos), 0,
     "Index of cursor"},
    {"length", T_INT, offsetof(DoublyLinkedList, length), 0,
     "Length of list"},
    {NULL}
};

// Type Definition

static PyTypeObject DoublyLinkedListType = {
    .ob_base = PyVarObject_HEAD_INIT(NULL, 0)
    .tp_name = "py_doubly_linked_list.DoublyLinkedList",
    .tp_doc = PyDoc_STR("Node for doubly linked list"),
    .tp_basicsize = sizeof(DoublyLinkedList),
    .tp_itemsize = 0,
    .tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_HAVE_GC,
    .tp_new = DoublyLinkedList_new,
    .tp_init = DoublyLinkedList_init,
    .tp_dealloc = DoublyLinkedList_dealloc,
    .tp_traverse = DoublyLinkedList_traverse,
    .tp_str = DoublyLinkedList_str,
    .tp_methods = DoublyLinkedList_methods,
    .tp_members = DoublyLinkedList_members,
    .tp_as_sequence = &DoublyLinkedList_sequence,
};

static int doubly_linked_list_module_exec(PyObject *m)
{
    if (PyType_Ready(&DoublyLinkedListType) < 0) {return -1;}
    if (PyModule_AddObjectRef(m, "DoublyLinkedList", (PyObject *) &DoublyLinkedListType) < 0) {return -1;}
    return 0;
}

#if PY_MINOR_VERSION >= 12

static PyModuleDef_Slot py_doubly_linked_list_module_slots[] = {
    {Py_mod_exec, dllnode_module_exec},
    {Py_mod_exec, doubly_linked_list_module_exec},
    {Py_mod_multiple_interpreters, Py_MOD_MULTIPLE_INTERPRETERS_NOT_SUPPORTED},
    {0, NULL}
};

#else

static PyModuleDef_Slot py_doubly_linked_list_module_slots[] = {
    {Py_mod_exec, dllnode_module_exec},
    {Py_mod_exec, doubly_linked_list_module_exec},
    {0, NULL}
};

#endif

static struct PyModuleDef py_doubly_linked_list_module = {
	PyModuleDef_HEAD_INIT,
	"py_doubly_linked_list",
	"A library implementing a doubly linked list for python",
	.m_slots = py_doubly_linked_list_module_slots
};

PyMODINIT_FUNC PyInit_py_doubly_linked_list(void) {
	return PyModuleDef_Init(&py_doubly_linked_list_module);
}