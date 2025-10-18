#define PY_SSIZE_T_CLEAN
#include <python3.13/Python.h>
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
    return 0;
}

static int
DLLNode_clear(PyObject *op)
{
    DLLNode* self = (DLLNode* )op;
    Py_CLEAR(self->value);
    return 0;
}

static void
DLLNode_dealloc(PyObject *op)
{
    DLLNode* self = (DLLNode* )op;
    PyObject_GC_UnTrack(op);
    (void)DLLNode_clear(op);
    Py_XDECREF(self->next);
    Py_XDECREF(self->prev);
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

static int
DLLNode_init(DLLNode *self, PyObject *args, PyObject *kwds)
{
    static char *kwlist[] = {"value", "next", "prev", NULL};
    PyObject* value = NULL,* next = NULL,* prev = NULL;
    if (!PyArg_ParseTupleAndKeywords(args, kwds, "|OOO", kwlist,
                                     &value, &next, &prev))
        return -1;
    if (value) {
        Py_SETREF(self->value, Py_NewRef(value));
    }
    if (next) {
        if(!PyObject_IsInstance(next,(PyObject* )&DLLNodeType) && !Py_IsNone(next)){
            PyErr_SetString(PyExc_TypeError, "next attribute must be py_doubly_linked_list.DLLNode or NoneType");
            return -1;
        }
        Py_SETREF(self->next, Py_NewRef(next));
    }
    if (prev) {
        if(!PyObject_IsInstance(prev,(PyObject* )&DLLNodeType) && !Py_IsNone(prev)){
            PyErr_SetString(PyExc_TypeError, "prev attribute must be py_doubly_linked_list.DLLNode or NoneType");
            return -1;
        }
        Py_SETREF(self->prev, Py_NewRef(prev));
    }
    return 0;
}

// Getters and Setters

static PyObject* DLLNode_value_get(PyObject* op, void* closure){
    DLLNode* self = (DLLNode* )op;
    return Py_NewRef(self->value);
}

static int DLLNode_value_set(PyObject* op, PyObject* value, void* closure){
    DLLNode* self = (DLLNode* )op;
    if(value==NULL){
        PyErr_SetString(PyExc_TypeError, "Cannot delete the next attribute");
        return -1;
    }
    Py_SETREF(self->value, Py_NewRef(value));
}

static PyObject* DLLNode_next_get(PyObject* op, void* closure){
    DLLNode* self = (DLLNode* )op;
    return Py_NewRef(self->next);
}

static int DLLNode_next_set(PyObject* op, PyObject* value, void* closure){
    DLLNode* self = (DLLNode* )op;
    if(value==NULL){
        PyErr_SetString(PyExc_TypeError, "Cannot delete the next attribute");
        return -1;
    }
    if(!PyObject_IsInstance(value,(PyObject* )&DLLNodeType) && !Py_IsNone(value)){
        PyErr_SetString(PyExc_TypeError, "next attribute must be py_doubly_linked_list.DLLNode");
        return -1;
    }
    Py_SETREF(self->next, Py_NewRef(value));
    return 0;
}

static PyObject* DLLNode_prev_get(PyObject* op, void* closure){
    DLLNode* self = (DLLNode* )op;
    return Py_NewRef(self->prev);
}

static int DLLNode_prev_set(PyObject* op, PyObject* value, void* closure){
    DLLNode* self = (DLLNode* )op;
    if(value==NULL){
        PyErr_SetString(PyExc_TypeError, "Cannot delete the prev attribute");
        return -1;
    }
    if(!PyObject_IsInstance(value,(PyObject* )&DLLNodeType) && !Py_IsNone(value)){
        PyErr_SetString(PyExc_TypeError, "prev attribute must be py_doubly_linked_list.DLLNode");
        return -1;
    }
    Py_SETREF(self->prev, Py_NewRef(value));
    return 0;
}

static PyGetSetDef DLLNode_getsetters[] = {
    {"value", DLLNode_value_get, DLLNode_value_set,
    "value held by node", NULL},
    {"next", DLLNode_next_get, DLLNode_next_set, 
    "next node", NULL},
    {"prev", DLLNode_prev_get, DLLNode_prev_set,
    "prev node", NULL},
    {NULL}
};

// __Methods__

static PyObject* DLLNode_str(PyObject* op, PyObject* Py_UNUSED(dummy)){
    DLLNode* self = (DLLNode* )op;
    return PyUnicode_FromFormat("%S", self->value);
}

// Methods

static PyObject* DLLNode_insert(PyObject* op, PyObject* args, PyObject* kwds){
    DLLNode* self = (DLLNode* )op;
    static char *kwlist[] = {"value", "forward", NULL};
    PyObject* value = NULL;
    int forward = 1;
    if(!PyArg_ParseTupleAndKeywords(args, kwds, "O|i", kwlist, &value, &forward)){
        return NULL;
    }
    DLLNode* node = DLLNode_new(&DLLNodeType, NULL, NULL);
    Py_SETREF(node->value, value);
    if(forward){
        if(Py_IsNone(self->next)){
            Py_SETREF(self->next, node);
            Py_SETREF(node->prev, Py_NewRef(self));
        }
        else{
            DLLNode* temp = self->next;
            Py_SETREF(node->prev, Py_NewRef(self));
            Py_SETREF(node->next, Py_NewRef(temp));
            Py_SETREF(self->next, Py_NewRef(node));
            Py_SETREF(temp->prev, node);
        }
    }
    else{
        if(Py_IsNone(self->prev)){
            Py_SETREF(self->prev, node);
            Py_SETREF(node->next, Py_NewRef(self));
        }
        else{
            DLLNode* temp = self->prev;
            Py_SETREF(node->next, Py_NewRef(self));
            Py_SETREF(node->prev, Py_NewRef(temp));
            Py_SETREF(self->prev, Py_NewRef(node));
            Py_SETREF(temp->next, node);
        }
    }
    return Py_NewRef(Py_None);
}

static PyMethodDef DLLNode_methods[] = {
    {"insert", DLLNode_insert, METH_VARARGS|METH_KEYWORDS,
     "Insert a node"
    },
    {NULL, NULL, 0, NULL}
};

//Type Definition

static PyTypeObject DLLNodeType = {
    .ob_base = PyVarObject_HEAD_INIT(NULL, 0)
    .tp_name = "py_doubly_linked_list.DLLNode",
    .tp_doc = PyDoc_STR("Node for doubly linked list"),
    .tp_basicsize = sizeof(DLLNode),
    .tp_itemsize = 0,
    .tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE | Py_TPFLAGS_HAVE_GC,
    .tp_new = DLLNode_new,
    .tp_init = DLLNode_init,
    .tp_dealloc = DLLNode_dealloc,
    .tp_traverse = DLLNode_traverse,
    .tp_getset = DLLNode_getsetters,
    .tp_str = DLLNode_str,
    .tp_methods = DLLNode_methods
};

static int dllnode_module_exec(PyObject *m)
{
    if (PyType_Ready(&DLLNodeType) < 0) {
        return -1;
    }

    if (PyModule_AddObjectRef(m, "DLLNode", (PyObject *) &DLLNodeType) < 0) {
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
    int cursor_pos;
    int length;
} DoublyLinkedList;

static PyTypeObject DoublyLinkedListType;

// Define methods we will need

static int DoublyLinkedList_locate(PyObject*, int);
static int DoublyLinkedList_cursor_insert(PyObject*, PyObject*, int);
static PyObject* DoublyLinkedList_append(PyObject*, PyObject*, PyObject*);
static int DoublyLinkedList_append_iterator(PyObject*, PyObject*);
static int DoublyLinkedList_cursor_delete(PyObject*);

// Initialization and deallocation

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
        if(DoublyLinkedList_append_iterator(self, iterable)){
            return -1;
        }
    }
    return 0;
}

// Methods

static PyObject* DoublyLinkedList_insert(PyObject* op, PyObject* args, PyObject* kwds){
    DoublyLinkedList* self = (DoublyLinkedList* ) op;
    static char *kwlist[] = {"value", "index", "forward", NULL};
    PyObject* value = NULL;
    int index;
    int forward = 1;
    if (!PyArg_ParseTupleAndKeywords(args, kwds, "Oi|i", kwlist,
                                     &value, &index, &forward))
        return NULL;
    if(DoublyLinkedList_locate(self, index)){
        return NULL;
    }
    if(DoublyLinkedList_cursor_insert(self, value, forward)){
        return NULL;
    }
    return Py_NewRef(Py_None);
}

static PyObject* DoublyLinkedList_append(PyObject* op, PyObject* args, PyObject* kwds){
    DoublyLinkedList* self = (DoublyLinkedList* ) op;
    static char *kwlist[] = {"value", "forward", NULL};
    PyObject* value = NULL;
    int forward = 1;
    if (!PyArg_ParseTupleAndKeywords(args, kwds, "O|i", kwlist,
                                     &value, &forward))
        return NULL;
    if(forward){
        Py_SETREF(self->cursor, Py_NewRef(self->tail));
    }
    else{
        Py_SETREF(self->cursor, Py_NewRef(self->head));
    }
    if(DoublyLinkedList_cursor_insert(self, value, forward)){
        return NULL;
    }
    return Py_NewRef(Py_None);
}

static PyObject* DoublyLinkedList_pop(PyObject* op, PyObject* args, PyObject* kwds){
    DoublyLinkedList* self = (DoublyLinkedList* ) op;
    static char *kwlist[] = {"index", NULL};
    int index;
    if (!PyArg_ParseTupleAndKeywords(args, kwds, "i", kwlist,
                                     &index))
        return NULL;
    if(DoublyLinkedList_locate(self, index)){
        return NULL;
    }
    DLLNode* cursor = (DLLNode* )self->cursor;
    PyObject* popped = Py_NewRef(cursor->value);
    if(DoublyLinkedList_cursor_delete(self)){
        return NULL;
    }
    return popped;
}

// Internal Methods

// Takes in DoublyLinkedList and index, locates node at that index and sets cursor to it
static int DoublyLinkedList_locate(PyObject* op, int index){
    DoublyLinkedList* self = (DoublyLinkedList* )op;
    if(index < 0) {
        index = self->length - index;
    }
    if(index >= self->length || index < 0) {
        PyErr_SetString(PyExc_IndexError, "Index out of bounds");
        return -1;
    }
    DLLNode* search_node = self->cursor;
    int search_distance = index-self->cursor_pos;
    const int head_distance = index;
    const int tail_distance = index-(self->length-1);
    if(abs(head_distance) < abs(search_distance)) {
        search_node = self->head;
        search_distance = head_distance;
    }
    else if(abs(tail_distance) < abs(search_distance)) {
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

static int DoublyLinkedList_cursor_insert(PyObject* op, PyObject* value, int forward) {
    DoublyLinkedList* self = (DoublyLinkedList* )op;
    self->length += 1;
    DLLNode* node = DLLNode_new(&DLLNodeType, NULL, NULL);
    Py_SETREF(node->value, Py_NewRef(value));
    if(Py_IsNone(self->cursor)){
        Py_SETREF(self->head, node);
        Py_SETREF(self->tail, Py_NewRef(node));
        Py_SETREF(self->cursor, Py_NewRef(node));
    }
    else{
        DLLNode* cursor = self->cursor;
        if(forward){
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
            self->cursor_pos += 1;
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
        if(!Py_IsNone(cursor->prev)){
            Py_SETREF(((DLLNode*)(cursor->prev))->next, Py_NewRef(cursor->next));
        }
        else{
            Py_SETREF(self->head, Py_NewRef(cursor->next));
        }
        Py_SETREF(((DLLNode*)(cursor->next))->prev, Py_NewRef(cursor->prev));
        Py_SETREF(self->cursor, Py_NewRef(cursor->next));
    }
    return 0;
}

static int DoublyLinkedList_append_iterator(PyObject* op, PyObject* iterable){
    DoublyLinkedList* self = (DoublyLinkedList* )op;
    PyObject* iterator = PyObject_GetIter(iterable);
    if(!iterator){
        return -1;
    }
    Py_SETREF(self->cursor, self->tail);
    PyObject* item;
    while((item = PyIter_Next(iterator)) != NULL){
        DoublyLinkedList_cursor_insert(self, item, 1);
        Py_DecRef(item);
    }
    Py_DecRef(iterator);
    return 0;
}

// Sequence Methods 

static int DoublyLinkedList_len(PyObject* op, PyObject* args, PyObject* kwds){
    DoublyLinkedList* self = (DoublyLinkedList* )op;
    return self->length;
}

static PyObject* DoublyLinkedList_item(PyObject* op, int index){
    DoublyLinkedList* self = (DoublyLinkedList* )op;
    if(DoublyLinkedList_locate(self, index)) {
        return NULL;
    }
    DLLNode* cursor = (DLLNode* )self->cursor;
    return Py_NewRef(cursor->value);
}

static int DoublyLinkedList_ass_item(PyObject* op, int index, PyObject* value) {
    DoublyLinkedList* self = (DoublyLinkedList* )op;
    if(DoublyLinkedList_locate(self, index)) {
        return -1;
    }
    DLLNode* cursor = (DLLNode* )self->cursor;
    Py_SETREF(cursor->value, Py_NewRef(value));
    return 0;
}

static PyObject* DoublyLinkedList_concat(PyObject* op, PyObject* concat){
    PyObject* new_list = DoublyLinkedList_new(&DoublyLinkedListType, NULL, NULL);
    if(DoublyLinkedList_append_iterator(new_list, op)){
        return NULL;
    }
    if(DoublyLinkedList_append_iterator(new_list, concat)){
        return NULL;
    }
    return new_list;
}

static PyObject* DoublyLinkedList_inplace_concat(PyObject* op, PyObject* concat){
    if(DoublyLinkedList_append_iterator(op, concat)){
        return NULL;
    }
    return Py_NewRef(op);
}

static PyMethodDef DoublyLinkedList_methods[] = {
    {"insert", DoublyLinkedList_insert, METH_VARARGS|METH_KEYWORDS,
     "Insert a node"},
    {"append", DoublyLinkedList_append, METH_VARARGS|METH_KEYWORDS,
    "Append a node"},
    {"pop", DoublyLinkedList_pop, METH_VARARGS|METH_KEYWORDS,
        "Pop a node"},
    {NULL, NULL, 0, NULL}
};

static PySequenceMethods DoublyLinkedList_sequence = {
    .sq_length = DoublyLinkedList_len,
    .sq_item = DoublyLinkedList_item,
    .sq_ass_item = DoublyLinkedList_ass_item,
    .sq_concat = DoublyLinkedList_concat,
    .sq_inplace_concat = DoublyLinkedList_inplace_concat
};

// Temp Member Definition

static PyMemberDef DoublyLinkedList_members[] = {
    {"head", Py_T_OBJECT_EX, offsetof(DoublyLinkedList, head), 0,
     "head"},
    {"tail", Py_T_OBJECT_EX, offsetof(DoublyLinkedList, tail), 0,
     "tail"},
    {"cursor", Py_T_OBJECT_EX, offsetof(DoublyLinkedList, cursor), 0,
     "cursor"},
    {"cursor_pos", Py_T_INT, offsetof(DoublyLinkedList, cursor_pos), 0,
     "cursor_pos"},
    {"length", Py_T_INT, offsetof(DoublyLinkedList, length), 0,
     "length"},
    {NULL}
};

// Type Definition

static PyTypeObject DoublyLinkedListType = {
    .ob_base = PyVarObject_HEAD_INIT(NULL, 0)
    .tp_name = "py_doubly_linked_list.DoublyLinkedList",
    .tp_doc = PyDoc_STR("Node for doubly linked list"),
    .tp_basicsize = sizeof(DoublyLinkedList),
    .tp_itemsize = 0,
    .tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,
    .tp_new = DoublyLinkedList_new,
    .tp_init = DoublyLinkedList_init,
    .tp_dealloc = DoublyLinkedList_dealloc,
    .tp_methods = DoublyLinkedList_methods,
    .tp_members = DoublyLinkedList_members,
    .tp_as_sequence = &DoublyLinkedList_sequence
};

static int doubly_linked_list_module_exec(PyObject *m)
{
    if (PyType_Ready(&DoublyLinkedListType) < 0) {
        return -1;
    }

    if (PyModule_AddObjectRef(m, "DoublyLinkedList", (PyObject *) &DoublyLinkedListType) < 0) {
        return -1;
    }

    return 0;
}

static PyModuleDef_Slot py_doubly_linked_list_module_slots[] = {
    {Py_mod_exec, dllnode_module_exec},
    {Py_mod_exec, doubly_linked_list_module_exec},
    // Just use this while using static types
    {Py_mod_multiple_interpreters, Py_MOD_MULTIPLE_INTERPRETERS_NOT_SUPPORTED},
    {0, NULL}
};

static struct PyModuleDef py_doubly_linked_list_module = {
	PyModuleDef_HEAD_INIT,
	"py_doubly_linked_list",
	"A library implementing a doubly linked list for python",
	.m_slots = py_doubly_linked_list_module_slots
};


PyMODINIT_FUNC PyInit_py_doubly_linked_list(void) {
	return PyModuleDef_Init(&py_doubly_linked_list_module);
}