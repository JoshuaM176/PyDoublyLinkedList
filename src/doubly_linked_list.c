#define PY_SSIZE_T_CLEAN
#include <Python.h>
#include <structmember.h>
#include <string.h>
#include <stddef.h>
#include <math.h>
#include <mpi.h>
#include <stdio.h>
#if PY_MINOR_VERSION < 10
    #define Py_IsNone(x) Py_Is((x), Py_None) // Define these so that we can use them on older versions
    #define Py_Is(x,y) ((x) == (y))
    static inline PyObject* Py_NewRef(PyObject *obj) {
        Py_INCREF(obj);
        return obj;
    }
#endif
// - - - - - DoublyLinkedListNode - - - - - //

typedef struct {
	PyObject_HEAD
	PyObject* value;
    PyObject* next;
    PyObject* prev;
	PyObject* key;
} DLLNode;

static PyTypeObject DLLNodeType;

// Initalization and Deallocation

static void
DLLNode_dealloc(PyObject *op)
{
    DLLNode* self = (DLLNode* )op;
    Py_XDECREF(self->next);
    Py_XDECREF(self->value);
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
        self->prev = Py_None;
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
    PyObject* rtn = PyUnicode_FromFormat("%S", self->value); if(!rtn) {return NULL;}
    return rtn;
}

//Type Definition

static PyTypeObject DLLNodeType = {
    .ob_base = PyVarObject_HEAD_INIT(NULL, 0)
    .tp_name = "py_doubly_linked_list.doubly_linked_list.DLLNode",
    .tp_doc = PyDoc_STR("Node for doubly linked list"),
    .tp_basicsize = sizeof(DLLNode),
    .tp_itemsize = 0,
    .tp_flags = Py_TPFLAGS_DEFAULT,
    .tp_new = DLLNode_new,
    .tp_dealloc = DLLNode_dealloc,
    .tp_getset = DLLNode_getsetters,
    .tp_str = (reprfunc)DLLNode_str
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
        return -1;
    if(iterable){
        if(DoublyLinkedList_append_iterator((PyObject*)self, iterable, 1)) {return -1;}
    }
    return 0;
}

// Methods

static PyObject* DoublyLinkedList_insert(PyObject* op, PyObject* args, PyObject* kwds){
    DoublyLinkedList* self = (DoublyLinkedList*) op;
    static char* kwlist[] = {"object", "index", "forward", NULL};
    PyObject* object = NULL;
    Py_ssize_t index;
    int forward = 1;
    if (!PyArg_ParseTupleAndKeywords(args, kwds, "On|i", kwlist,
                                     &object, &index, &forward))
        return NULL;
    if(DoublyLinkedList_locate((PyObject*)self, index)) {return NULL;}
    if(DoublyLinkedList_cursor_insert((PyObject*)self, object, forward)) {return NULL;}
    return Py_NewRef(Py_None);
}

static PyObject* DoublyLinkedList_append(PyObject* op, PyObject* args, PyObject* kwds){
    DoublyLinkedList* self = (DoublyLinkedList*) op;
    static char* kwlist[] = {"object", "forward", NULL};
    PyObject* object = NULL;
    int forward = 1;
    if (!PyArg_ParseTupleAndKeywords(args, kwds, "O|i", kwlist,
                                     &object, &forward))
        return NULL;
    if(forward) {Py_SETREF(self->cursor, Py_NewRef(self->tail));}
    else {Py_SETREF(self->cursor, Py_NewRef(self->head));}
    if(DoublyLinkedList_cursor_insert((PyObject*)self, object, forward)) {return NULL;}
    return Py_NewRef(Py_None);
}

static PyObject* DoublyLinkedList_index(PyObject* op, PyObject* args, PyObject* kwds){
    DoublyLinkedList* self = (DoublyLinkedList* )op;
    static char* kwlist[] = {"value", "start", "stop", NULL};
    PyObject* value; Py_ssize_t start = 0; Py_ssize_t stop = self->length;
    DLLNode* cursor;
    if(!PyArg_ParseTupleAndKeywords(args, kwds, "O|nn", kwlist, &value, &start, &stop)) {return NULL;}
    for(Py_ssize_t i=start; i<stop; i++){
        if(DoublyLinkedList_locate((PyObject*)self, i)) {return NULL;}
        cursor = (DLLNode*)self->cursor;
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
    Py_ssize_t index = self->length-1;
    if(!PyArg_ParseTupleAndKeywords(args, kwds, "|n", kwlist, &index)) {return NULL;}
    if(DoublyLinkedList_locate((PyObject*)self, index)) {return NULL;}
    DLLNode* cursor = (DLLNode* )self->cursor;
    PyObject* popped = Py_NewRef(cursor->value);
    if(DoublyLinkedList_cursor_delete((PyObject*)self)) {return NULL;}
    return popped;
}

static PyObject* DoublyLinkedList_remove(PyObject* op, PyObject* args, PyObject* kwds){
    DoublyLinkedList* self = (DoublyLinkedList* )op;
    if(!DoublyLinkedList_index((PyObject*)self, args, kwds)) {return NULL;}
    if(DoublyLinkedList_cursor_delete((PyObject*)self)) {return NULL;}
    return Py_NewRef(Py_None);
}

static PyObject* DoublyLinkedList_extend(PyObject* op, PyObject* args, PyObject* kwds){
    DoublyLinkedList* self = (DoublyLinkedList* )op;
    static char* kwlist[] = {"iterable", "forward", NULL};
    PyObject* iterable;
    int forward = 1;
    if(!PyArg_ParseTupleAndKeywords(args, kwds, "O|i", kwlist, &iterable, &forward)) {return NULL;}
    if(DoublyLinkedList_append_iterator((PyObject*)self, iterable, forward)) {return NULL;}
    return Py_NewRef(Py_None);
}

static PyObject* DoublyLinkedList_copy(PyObject* op){
    DoublyLinkedList* copy = (DoublyLinkedList*)DoublyLinkedList_new(&DoublyLinkedListType, NULL, NULL); if(!copy) {return NULL;}
    if(DoublyLinkedList_append_iterator((PyObject*)copy, op, 1)) {return NULL;}
    return (PyObject*)copy;
}

static PyObject* DoublyLinkedList_reverse(PyObject* op){
    DoublyLinkedList* self = (DoublyLinkedList* )op;
    Py_ssize_t middle = self->length / 2;
    PyObject* temp;
    DLLNode* node1 = (DLLNode*)self->head;
    DLLNode* node2 = (DLLNode*)self->tail;
    for(Py_ssize_t i = 0; i < middle; i++){
        temp = (PyObject*)node1->value;
        node1->value = node2->value;
        node2->value = (PyObject*)temp;
        node1 = (DLLNode*)node1->next;
        node2 = (DLLNode*)node2->prev;
    }
    return Py_NewRef(Py_None);
}

static PyObject* DoublyLinkedList_count(PyObject* op, PyObject* args, PyObject* kwds) {
    DoublyLinkedList* self = (DoublyLinkedList* )op;
    static char* kwlist[] = {"value", NULL};
    PyObject* value;
    if(!PyArg_ParseTupleAndKeywords(args, kwds, "O", kwlist, &value)) {return NULL;}
    DLLNode* temp = (DLLNode*)self->head;
    Py_ssize_t count = 0;
    for(Py_ssize_t i = 0; i<self->length; i++){
        if(temp->value==value) {count += 1;}
        temp = (DLLNode*)temp->next;
    }
    PyObject* rtn = PyLong_FromSsize_t(count); if(!rtn) {return NULL;}
    return rtn;
}

static PyObject* DoublyLinkedList_clear_method(PyObject* op){
    DoublyLinkedList* self = (DoublyLinkedList* )op;
    if(self->length == 0){
        return Py_NewRef(Py_None);
    }
    Py_DECREF(self->cursor); Py_DECREF(self->head); Py_DECREF(self->tail);
    self->head = Py_NewRef(Py_None); self->tail = Py_NewRef(Py_None); self->cursor = Py_NewRef(Py_None);
    self->length = 0; self->cursor_pos = 0;
    return Py_NewRef(Py_None);
}

//Helper method for sort, swaps two nodes that are next to each
static void swap(PyObject* op, PyObject* node1, PyObject* node2) {
    DoublyLinkedList* self = (DoublyLinkedList*)op;
    DLLNode* temp1 = (DLLNode*)node1;
    DLLNode* temp2 = (DLLNode*)node2;
    if(Py_IsNone(temp1->prev)) {
        self->head = (PyObject*)temp2;
    }
    else{
        ((DLLNode*)(temp1->prev))->next = (PyObject*)temp2;
    }
    if(Py_IsNone(temp2->next)){
        Py_SETREF(self->tail, (PyObject*)temp1);
    }
    else{
        ((DLLNode*)(temp2->next))->prev = (PyObject*)temp1;
    }
    temp1->next = temp2->next;
    temp2->prev = temp1->prev;
    temp2->next = (PyObject*)temp1;
    temp1->prev = (PyObject*)temp2;
}

static PyObject* DoublyLinkedList_sort(PyObject* op, PyObject* args, PyObject* kwds) {
    DoublyLinkedList* self = (DoublyLinkedList* )op;
    char* kwlist[] = {"key", "reverse", NULL};
    PyObject* key = NULL; int reverse = 0;
    if(!PyArg_ParseTupleAndKeywords(args, kwds, "|Oi", kwlist, &key, &reverse)) { return NULL; }
    int operator;
    if(reverse) { operator = Py_GT; } else { operator = Py_LT; }
    DLLNode* temp = (DLLNode*)self->head;
    DLLNode* next = (DLLNode*)temp->next;
    DLLNode* prev;
    int comparison;

    if(key) {
		if(!PyCallable_Check(key)) {
			PyErr_SetString(PyExc_TypeError, "Key must be a callable"); return NULL; }
		for(int i = 0; i < self->length; i++) {
			PyObject* value_key = PyObject_CallOneArg(key, temp->value);
			if(!value_key) { return NULL; }
			temp->key = value_key;
			temp = (DLLNode*)temp->next;
		}

		temp = (DLLNode*)self->head;
		for(int i = 1; i < self->length; i++) {
        	temp = next;
        	next = (DLLNode*)temp->next;
            for(int j = i; j >= 1; j--) {
                prev = (DLLNode*)temp->prev;
                comparison = PyObject_RichCompareBool(temp->key, prev->key, operator);
                if(comparison == -1) { return NULL; }
                if(comparison){swap((PyObject*)self, (PyObject*)prev, (PyObject*)temp);}
                else { break; }
            }
    	}
		temp = (DLLNode*)self->head;
		for(int i =0; i < self->length; i++) {
			Py_DECREF(temp->key);
			temp = (DLLNode*)temp->next;
		}
		self->cursor_pos = 0;
    	Py_SETREF(self->cursor, Py_NewRef(self->head));
    	return Py_NewRef(Py_None);
	}
	else{
    	for(int i = 1; i < self->length; i++) {
        	temp = next;
        	next = (DLLNode*)temp->next;
            for(int j = i; j >= 1; j--) {
                prev = (DLLNode*)temp->prev;
                comparison = PyObject_RichCompareBool(temp->value, prev->value, operator);
                if(comparison == -1) { return NULL; }
                if(comparison){swap((PyObject*)self, (PyObject*)prev, (PyObject*)temp);}
                else { break; }
        	}
    	}
    	self->cursor_pos = 0;
    	Py_SETREF(self->cursor, Py_NewRef(self->head));
    	return Py_NewRef(Py_None);
	}
}

static void merge_sort(double* list, long long start, long long end) {
    if(end-start == 0) { return; }
    if(end-start == 1) {
        if(list[start] > list[end]) {
            int temp = list[start];
            list[start] = list[end];
            list[end] = temp;
        }
        return;
    }
    long long start1 = start;
    long long start2 = end - (end-start)/2;
    long long end1 = start2-1;
    long long end2 = end;
    merge_sort(list, start1, end1);
    merge_sort(list, start2, end2);
    double* temp_list = (double*)malloc((end-start+1) * sizeof(double));
    long long cursor1 = start1;
    long long cursor2 = start2;
    long long cursor = 0;
    while(cursor1 <= end1 && cursor2 <= end2) {
        if(list[cursor1] < list[cursor2]) {
            temp_list[cursor] = list[cursor1];
            cursor1++;
        }
        else{
            temp_list[cursor] = list[cursor2];
            cursor2++;
        }
        cursor++;
    }
    if(cursor1 > end1) {
        for(long long i = cursor2; i <= end2; i++) {
            temp_list[cursor] = list[i];
            cursor++;
        }
    }
    else{
        for(long long i = cursor1; i <= end1; i++) {
            temp_list[cursor] = list[i];
            cursor++;
        }     
    }
    cursor = start;
    for(long long i = 0; i < end-start+1; i++) {
        list[cursor] = temp_list[i];
        cursor++;
    }
    free(temp_list);
    return;
}

static void merge_proc(double* list, long long start, long long end, int rank, int sending_rank, long long sending_size) {
    int tag = sending_rank<<4; tag+=rank;
    double* temp_list = (double*)malloc((end-start+1+sending_size) * sizeof(double));
    double* recieved_values = (double*)malloc(sending_size * sizeof(double));
    MPI_Recv(recieved_values, sending_size, MPI_DOUBLE, sending_rank, tag, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    long long cursor1 = start;
    long long cursor2 = 0;
    long long cursor = 0;
    while(cursor1 <= end && cursor2 < sending_size) {
        if(list[cursor1] < recieved_values[cursor2]) {
            temp_list[cursor] = list[cursor1];
            cursor1++;
        }
        else{
            temp_list[cursor] = recieved_values[cursor2];
            cursor2++;
        }
        cursor++;
           if(rank == 0 && sending_rank == 2) {
        }
    }
    if(cursor1 > end) {
        for(long long i = cursor2; i <= sending_size; i++) {
            temp_list[cursor] = recieved_values[i];
            cursor++;
        }
    }
    else{
        for(long long i = cursor1; i <= end; i++) {
            temp_list[cursor] = list[i];
            cursor++;
        }     
    }
    cursor = start;
    for(long long i = 0; i < end-start+1+sending_size; i++) {
        list[cursor] = temp_list[i];
        cursor++;
    }
    free(temp_list);
    free(recieved_values);
    return;
}

static PyObject* DoublyLinkedList_merge_sort(PyObject* op, PyObject* args, PyObject* kwds) {
    int rank;
    int comm_size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &comm_size);
    DoublyLinkedList* self = (DoublyLinkedList* )op;
    double* values = malloc(sizeof(double)*self->length);
    DLLNode* temp = (DLLNode*)self->head;
    for(int i = 0; i < self->length; i++) {
        values[i] = PyFloat_AsDouble(temp->value); if(!values[i]) { return NULL; }
        temp = (DLLNode*)temp->next;
    }
    long long length = self->length;
    double nodes_per_proc = (double)length/comm_size;
    long long start = ceil(nodes_per_proc * rank);
    long long end = ceil(nodes_per_proc * (1 + rank))-1;
    long long size = end - start + 1;
    merge_sort(values, start, end);
    int spacing = 1;
    while(spacing < comm_size) {
        if(rank % spacing != 0) {
            break;
        }
        if(rank / spacing % 2 == 0) { // recieving if true
            if(rank + spacing < comm_size) {
                int tag = (rank + spacing)<<4; // sending is first 4 bits, recieving is second 4 bits
                tag += rank;
                long long sending_size; MPI_Recv(&sending_size, 1, MPI_LONG_LONG, rank+spacing, tag, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                merge_proc(values, start, end, rank, rank+spacing, sending_size);
                size += sending_size;
                end += sending_size;
            }
        }
        else {
            int tag = rank<<4; // sending is first 4 bits, recieving is second 4 bits
            tag += (rank-spacing);
            printf("Send %i Recieve %i Size %i\n", rank, rank-spacing, size);
            MPI_Send(&size, 1, MPI_LONG_LONG, rank-spacing, tag, MPI_COMM_WORLD);
            MPI_Send(&values[start], size, MPI_DOUBLE, rank-spacing, tag, MPI_COMM_WORLD);
        }
        spacing *= 2;
    }
    if(1) {
        temp = (DLLNode*)self->head;
        for(int i = 0; i < self->length; i++) {
            temp->value = PyLong_FromLong(values[i]);
            temp = (DLLNode*)temp->next;
        }
    }
    free(values);
    MPI_Finalize();
	return Py_NewRef(Py_None); 
}

// Internal Methods

// Takes in DoublyLinkedList and index, locates node at that index and sets cursor to it
static int DoublyLinkedList_locate(PyObject* op, Py_ssize_t index){
    DoublyLinkedList* self = (DoublyLinkedList* )op;
    if(index < 0) {index = self->length + index;}
    if(index >= self->length || index < 0){
        PyErr_SetString(PyExc_IndexError, "Index out of bounds");
        return -1;
    }
    DLLNode* search_node = (DLLNode*)self->cursor;
    Py_ssize_t search_distance = index-self->cursor_pos;
    const Py_ssize_t head_distance = index;
    const Py_ssize_t tail_distance = index-(self->length-1);
    if(abs(head_distance) < abs(search_distance)){
        search_node = (DLLNode*)self->head;
        search_distance = head_distance;
    }
    else if(abs(tail_distance) < abs(search_distance)){
        search_node = (DLLNode*)self->tail;
        search_distance = tail_distance;
    }
    if(search_distance>0){
        for(Py_ssize_t i = 0; i<search_distance; i++){
            search_node = (DLLNode*)search_node->next;
        }
    }
    else if(search_distance<0){
        for(Py_ssize_t i=0; i>search_distance; i--){
            search_node = (DLLNode*)search_node->prev;
        }
    }
    Py_SETREF(self->cursor, Py_NewRef((PyObject*)search_node));
    self->cursor_pos = index;
    return 0;
}

// Create a new node with value and inserts it forwards or backwards and sets cursor to it.
// We don't create a reference to the prev node since we can guarentee that as long as it is in the list it will have a reference from its prev anyway.
// This avoids cycles
static int DoublyLinkedList_cursor_insert(PyObject* op, PyObject* object, int forward) {
    DoublyLinkedList* self = (DoublyLinkedList*)op;
    self->length += 1;
    DLLNode* node = (DLLNode*)DLLNode_new(&DLLNodeType, NULL, NULL); if(!node) {return -1;}
    Py_SETREF(node->value, Py_NewRef(object));
    if(Py_IsNone(self->cursor)){
        Py_SETREF(self->head, (PyObject*)node);
        Py_SETREF(self->tail, Py_NewRef((PyObject*)node));
        Py_SETREF(self->cursor, Py_NewRef((PyObject*)node));
    }
    else{
        DLLNode* cursor = (DLLNode*)self->cursor;
        if(forward){
            self->cursor_pos += 1;
            if(Py_IsNone(cursor->next)){
                node->prev = (PyObject*)cursor;
                Py_SETREF(self->tail, Py_NewRef((PyObject*)node));
		        Py_SETREF(cursor->next, (PyObject*)node);

            }
            else{
                DLLNode* temp = (DLLNode*)cursor->next;
                node->prev = (PyObject*)cursor;
                temp->prev = (PyObject*)node;
                Py_SETREF(node->next, Py_NewRef((PyObject*)temp));
                Py_SETREF(cursor->next, Py_NewRef((PyObject*)node));
            }
        }
        else{
            if(Py_IsNone(cursor->prev)){
                cursor->prev = (PyObject*)node;
                Py_SETREF(self->head, Py_NewRef((PyObject*)node));
                Py_SETREF(node->next, Py_NewRef((PyObject*)cursor));
            }
            else{
                DLLNode* temp = (DLLNode*)cursor->prev;
                node->prev = (PyObject*)temp;
                cursor->prev = (PyObject*)node;
                Py_SETREF(temp->next, (PyObject*)node);
                Py_SETREF(node->next, Py_NewRef((PyObject*)cursor));
            }
        }
    }
    Py_SETREF(self->cursor, Py_NewRef((PyObject*)node));
    return 0;
}

static int DoublyLinkedList_cursor_delete(PyObject* op){
    DoublyLinkedList* self = (DoublyLinkedList* )op;
    self->length -= 1;
    DLLNode* cursor = (DLLNode*)self->cursor;
    if(Py_IsNone(cursor->next)){
        if(Py_IsNone(cursor->prev)){
            Py_SETREF(self->head, Py_NewRef(Py_None));
            Py_SETREF(self->tail, Py_NewRef(Py_None));
            Py_SETREF(self->cursor, Py_NewRef(Py_None));
        }
        else{
            Py_SETREF(self->tail, Py_NewRef(cursor->prev));
            Py_SETREF(self->cursor, Py_NewRef(cursor->prev));
            Py_SETREF(((DLLNode*)(cursor->prev))->next, Py_NewRef(cursor->next));
            self->cursor_pos-=1;
        }
    }
    else{
        ((DLLNode*)(cursor->next))->prev = cursor->prev;
        Py_SETREF(self->cursor, Py_NewRef(cursor->next));
        if(Py_IsNone(cursor->prev)){
            Py_SETREF(self->head, Py_NewRef(cursor->next));
        }
        else{
            Py_SETREF(((DLLNode*)(cursor->prev))->next, Py_NewRef(cursor->next));
        }
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
        if(DoublyLinkedList_cursor_insert((PyObject*)self, item, forward)) {return -1;}
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

static PyObject* DoublyLinkedList_subscript(PyObject* op, PyObject* slice){
    DoublyLinkedList* self = (DoublyLinkedList* )op;
    if(PySlice_Check(slice)) {
        Py_ssize_t start, stop, step;
        DoublyLinkedList* list_slice = (DoublyLinkedList*)DoublyLinkedList_new(&DoublyLinkedListType, NULL, NULL); if(!list_slice) {return NULL;}
        DLLNode* temp;
        if (PySlice_Unpack(slice, &start, &stop, &step) == -1) {return NULL;}
        if(start < 0) {start = self->length + start;}
        if(stop < 0) {stop = self->length + stop;} if(stop > self->length) {stop = self->length;}
        if(step > 0) {
            for(Py_ssize_t i = start; i < stop; i+=step){
                if(DoublyLinkedList_locate((PyObject*)self, i)) {return NULL;}
                temp = (DLLNode*)self->cursor;
                if(DoublyLinkedList_cursor_insert((PyObject*)list_slice, temp->value, 1)) {return NULL;}
            }
        }
        if(step < 0) {
            for(Py_ssize_t i = start; i > stop; i+=step){
                if(DoublyLinkedList_locate((PyObject*)self, i)) {return NULL;}
                temp = (DLLNode*)self->cursor;
                if(DoublyLinkedList_cursor_insert((PyObject*)list_slice, temp->value, 1)) {return NULL;}
            }
        }
        return (PyObject*)list_slice;
    }
    else if(PyLong_Check(slice)) {
        Py_ssize_t index = PyLong_AsSsize_t(slice); if(index == -1 && PyErr_Occurred()) {return NULL;}
        if(DoublyLinkedList_locate((PyObject*)self, index)) {return NULL;}
        DLLNode* cursor = (DLLNode*)self->cursor;
        return Py_NewRef(cursor->value);
    }
    else {PyErr_SetString(PyExc_TypeError, "Index must be an integer or slice"); return NULL;}

}

// Sequence Methods 

static Py_ssize_t DoublyLinkedList_len(PyObject* op, PyObject* args, PyObject* kwds){
    DoublyLinkedList* self = (DoublyLinkedList*)op;
    return self->length;
}

static PyObject* DoublyLinkedList_item(PyObject* op, Py_ssize_t index){
    DoublyLinkedList* self = (DoublyLinkedList* )op;
    if(DoublyLinkedList_locate((PyObject*)self, index)) {return NULL;}
    DLLNode* cursor = (DLLNode*)self->cursor;
    return Py_NewRef(cursor->value);
}

static int DoublyLinkedList_ass_item(PyObject* op, Py_ssize_t index, PyObject* value) {
    DoublyLinkedList* self = (DoublyLinkedList*)op;
    if(DoublyLinkedList_locate((PyObject*)self, index)) {return -1;}
    if(!value){
        if(DoublyLinkedList_cursor_delete((PyObject*)self)) {return -1;}
        return 0;
    }
    DLLNode* cursor = (DLLNode*)self->cursor;
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
    DoublyLinkedList* self = (DoublyLinkedList*)op;
    DLLNode* temp = (DLLNode*)self->head;
    for(Py_ssize_t i = 0; i<self->length; i++){
        if(temp->value==value) {return 1;}
        temp = (DLLNode*)temp->next;
    }
    return 0;
}

// __Methods__

static PyObject* DoublyLinkedList_str(PyObject* op, PyObject* Py_UNUSED(dummy)){
    DoublyLinkedList* self = (DoublyLinkedList*)op;
    if(self->length == 0) {return PyUnicode_FromString("[]");}
    PyObject* string = PyUnicode_FromString("["); if(!string) {return NULL;}
    PyObject* new_string;
    DLLNode* temp = (DLLNode*)self->head;
    for(Py_ssize_t i = 1; i<self->length; i++){
        PyObject* node_str = DLLNode_str((PyObject*)temp, NULL); if(!node_str) {return NULL;}
        PyObject* format_node_str = PyUnicode_FromFormat("%U, ", node_str); if(!format_node_str) {return NULL;}
        new_string = PyUnicode_Concat(string, format_node_str); if(!new_string) {return NULL;}
        Py_DECREF(node_str); Py_DECREF(format_node_str); Py_DECREF(string);
        string = new_string;
        temp = (DLLNode*)temp->next;
    }
    new_string = PyUnicode_Concat(string, PyUnicode_FromFormat("%U]", DLLNode_str((PyObject*)temp, NULL)));
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
    {"merge_sort", (PyCFunction)DoublyLinkedList_merge_sort, METH_VARARGS|METH_KEYWORDS,
    "In-place sort in ascending order, equal objects are not swapped. Key can be applied to values and the list will be sorted based on the result of applying the key. Reverse will reverse the sort order."},
	{NULL, NULL, 0, NULL}
};

static PyMappingMethods DoublyLinkedList_map = {
    .mp_subscript = DoublyLinkedList_subscript
};

static PySequenceMethods DoublyLinkedList_sequence = {
    .sq_length = (lenfunc)DoublyLinkedList_len,
    .sq_item = DoublyLinkedList_item,
    .sq_ass_item = DoublyLinkedList_ass_item,
    .sq_concat = DoublyLinkedList_concat,
    .sq_inplace_concat = DoublyLinkedList_inplace_concat,
    .sq_contains = DoublyLinkedList_contains
};

//Member Definition

#ifdef DEV

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

#else

static PyMemberDef DoublyLinkedList_members[] = {{NULL}};

#endif

// Type Definition

static PyTypeObject DoublyLinkedListType = {
    .ob_base = PyVarObject_HEAD_INIT(NULL, 0)
    .tp_name = "py_doubly_linked_list.doubly_linked_list.DoublyLinkedList",
    .tp_doc = PyDoc_STR("Node for doubly linked list"),
    .tp_basicsize = sizeof(DoublyLinkedList),
    .tp_itemsize = 0,
    .tp_flags = Py_TPFLAGS_DEFAULT,
    .tp_new = (newfunc)DoublyLinkedList_new,
    .tp_init = (initproc)DoublyLinkedList_init,
    .tp_dealloc = (destructor)DoublyLinkedList_dealloc,
    .tp_str = (reprfunc)DoublyLinkedList_str,
    .tp_methods = DoublyLinkedList_methods,
    .tp_members = DoublyLinkedList_members,
    .tp_as_sequence = &DoublyLinkedList_sequence,
    .tp_as_mapping = &DoublyLinkedList_map
};

static int doubly_linked_list_module_exec(PyObject *m)
{
    if (PyType_Ready(&DoublyLinkedListType) < 0) {return -1;}
    Py_INCREF(&DoublyLinkedListType);
    if (PyModule_AddObject(m, "DoublyLinkedList", (PyObject *) &DoublyLinkedListType) < 0) {
        Py_DECREF(&DoublyLinkedListType);
        Py_DECREF(m);
        return -1;
    }
    MPI_Init(NULL,NULL);
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
	"py_doubly_linked_list.doubly_linked_list",
	"A library implementing a doubly linked list for python",
	.m_slots = py_doubly_linked_list_module_slots
};

PyMODINIT_FUNC PyInit_doubly_linked_list(void) {
	return PyModuleDef_Init(&py_doubly_linked_list_module);
}
