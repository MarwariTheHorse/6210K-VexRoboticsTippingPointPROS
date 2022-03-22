#include <stdio.h>
#include <Python.h>
#include <pyhelper.hpp>

static PyObject* arnav_foo(PyObject* self, PyObject* args)
{
	printf_s("... in C++...: foo() method\n");
	return PyLong_FromLong(51);
}

static PyObject* arnav_show(PyObject* self, PyObject* args)
{
	PyObject *a;
	if(PyArg_UnpackTuple(args, "", 1, 1, &a))
	{
		printf_s("C++: show(%ld)\n", PyLong_AsLong(a));
	}

	return PyLong_FromLong(0);
}

static struct PyMethodDef methods[] = {
	{ "foo", arnav_foo, METH_VARARGS, "Returns the number"},
	{ "show", arnav_show, METH_VARARGS, "Show a number" },
	{ NULL, NULL, 0, NULL }
};

static struct PyModuleDef modDef = {
	PyModuleDef_HEAD_INIT, "arnav", NULL, -1, methods, 
	NULL, NULL, NULL, NULL
};

static PyObject* PyInit_arnav(void)
{
	return PyModule_Create(&modDef);
}

int main()
{
	PyImport_AppendInittab("arnav", &PyInit_arnav);

	CPyInstance hInstance;

	const char pFile[] = "pyemb6.py";
	FILE* fp = _Py_fopen(pFile, "r");
	PyRun_AnyFile(fp, pFile);

	return 0;
}