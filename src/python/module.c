#define PY_SSIZE_T_CLEAN
#include <Python.h>

#include "vm.h"

static PyObject *native_execute(PyObject *self, PyObject *args,
                                PyObject *kwargs)
{
    const char *bytecode = NULL;
    Py_ssize_t length = 0;
    Py_ssize_t max_stack = 256;
    unsigned long long max_instructions = 100000;
    static char *names[] = {"bytecode", "max_stack", "max_instructions", NULL};
    TinyVMConfig config;
    TinyVMResult result = {0};
    TinyVMError error = {0};
    TinyVMStatus status;
    PyObject *output;
    size_t i;

    (void)self;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "y#|nK", names,
                                     &bytecode, &length,
                                     &max_stack, &max_instructions)) {
        return NULL;
    }

    if (max_stack <= 0 || max_instructions == 0) {
        PyErr_SetString(PyExc_ValueError, "VM limits must be positive.");
        return NULL;
    }

    config.max_stack = (size_t)max_stack;
    config.max_instructions = (uint64_t)max_instructions;

    status = tinyvm_execute((const uint8_t *)bytecode, (size_t)length,
                            &config, &result, &error);

    if (status != TINYVM_OK) {
        PyObject *exception = PyExc_RuntimeError;

        if (status == TINYVM_ERROR_INVALID_ARGUMENT ||
            status == TINYVM_ERROR_INVALID_OPCODE ||
            status == TINYVM_ERROR_TRUNCATED_BYTECODE ||
            status == TINYVM_ERROR_INVALID_JUMP) {
            exception = PyExc_ValueError;
        } else if (status == TINYVM_ERROR_DIVISION_BY_ZERO) {
            exception = PyExc_ZeroDivisionError;
        } else if (status == TINYVM_ERROR_ARITHMETIC_OVERFLOW) {
            exception = PyExc_OverflowError;
        } else if (status == TINYVM_ERROR_OUT_OF_MEMORY) {
            exception = PyExc_MemoryError;
        }

        PyErr_Format(exception, "%s (byte offset %zu)",
                     error.message, error.byte_offset);
        tinyvm_result_free(&result);
        return NULL;
    }

    output = PyList_New((Py_ssize_t)result.count);
    if (output == NULL) {
        tinyvm_result_free(&result);
        return NULL;
    }

    for (i = 0; i < result.count; i++) {
        PyObject *number = PyLong_FromLongLong((long long)result.values[i]);
        if (number == NULL) {
            Py_DECREF(output);
            tinyvm_result_free(&result);
            return NULL;
        }
        PyList_SET_ITEM(output, (Py_ssize_t)i, number);
    }

    tinyvm_result_free(&result);
    return output;
}

static PyMethodDef methods[] = {
    {"execute", (PyCFunction)native_execute, METH_VARARGS | METH_KEYWORDS,
     "Execute encoded TinyVM bytecode."},
    {NULL, NULL, 0, NULL}
};

static struct PyModuleDef module = {
    PyModuleDef_HEAD_INIT,
    "_native",
    "TinyVM native module.",
    -1,
    methods
};

PyMODINIT_FUNC PyInit__native(void)
{
    return PyModule_Create(&module);
}