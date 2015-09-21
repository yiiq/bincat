#include "Python.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <caml/alloc.h>
#include <caml/mlvalues.h>
#include <caml/memory.h>
#include <caml/callback.h>
#include <caml/printexc.h>


static PyObject *OcamlException;

static int load_caml(value **func, char *funcname)
{
	if (*func == NULL) {
		*func = caml_named_value (funcname) ;
		if (*func == NULL) {
			PyErr_SetString(PyExc_NameError, funcname);
			return 0;
		}
	}
	return 1;
}


static PyObject *process(PyObject *self, PyObject *args)
{
  static value *process_func;

  char *configfile;
  char *resultfile;

  if (!PyArg_ParseTuple(args, "ss", &configfile, &resultfile))
    return NULL;
  if (!load_caml(&process_func, "process"))
    return NULL;

  value res;
  res = caml_callback2_exn(*process_func, caml_copy_string(configfile), caml_copy_string(resultfile));
  if (Is_exception_result(res)) {
    value exn = Extract_exception(res);
	PyErr_SetString(OcamlException, (*caml_format_exception)(exn));
	return NULL;
  }
  return Py_None;
}


static PyMethodDef MlMethods[] = {
  { "process", process, METH_VARARGS, "Launch the analyzer" },
  {NULL, NULL, 0, NULL}
};

PyMODINIT_FUNC
initmlbincat(void)
{
	char *argv[2];
	argv[0]="";
	argv[1]=NULL;
	caml_startup(argv);
	PyObject *module;
	module = Py_InitModule("mlbincat", MlMethods);
	if (module == NULL)
	  return;

	OcamlException = PyErr_NewException("mlbincat.OcamlException", NULL, NULL);
	Py_INCREF(OcamlException);
	PyModule_AddObject(module, "error", OcamlException);
}

