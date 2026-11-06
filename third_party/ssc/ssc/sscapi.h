/**
BSD-3-Clause
Copyright 2019 Alliance for Sustainable Energy, LLC
Redistribution and use in source and binary forms, with or without modification, are permitted provided
that the following conditions are met :
1.	Redistributions of source code must retain the above copyright notice, this list of conditions
and the following disclaimer.
2.	Redistributions in binary form must reproduce the above copyright notice, this list of conditions
and the following disclaimer in the documentation and/or other materials provided with the distribution.
3.	Neither the name of the copyright holder nor the names of its contributors may be used to endorse
or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,
INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
ARE DISCLAIMED.IN NO EVENT SHALL THE COPYRIGHT HOLDER, CONTRIBUTORS, UNITED STATES GOVERNMENT OR UNITED STATES
DEPARTMENT OF ENERGY, NOR ANY OF THEIR EMPLOYEES, BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY,
OR CONSEQUENTIAL DAMAGES(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

/**
   \file sscapi.h

   \brief SSC: SAM Simulation Core

   A general purpose simulation input/output framework.
   Cross-platform (Windows/MacOSX/Unix) and is 32 and 64-bit compatible.

   Be sure to use the correct library for your operating platform: ssc32
   or ssc64. Opaque pointer types will be 4-byte pointer on 32-bit architectures,
   and 8-byte pointer on 64-bit architectures.

   Shared libraries have the .dll file extension on Windows,
   .dylib on MacOSX, and .so on Linux/Unix.

  \copyright 2012 National Renewable Energy Laboratory
  \authors Aron Dobos, Steven Janzou
  */

#ifndef __ssc_api_h
#define __ssc_api_h

#if defined(__WINDOWS__)&&defined(__DLL__)
#define SSCEXPORT __declspec(dllexport)
#else
#define SSCEXPORT
#endif

#ifndef __SSCLINKAGECPP__

#ifdef __cplusplus
extern "C" {
#endif

#endif // __SSCLINKAGECPP__

/** Returns the library version number as an integer.  Version numbers start at 1. */
SSCEXPORT int ssc_version();

/** Returns information about the build configuration of this particular SSC library binary as a text string that lists the compiler, platform, build date/time and other information. */
SSCEXPORT const char *ssc_build_info();

/** @name Data types:
  * Possible data types for ssc_var_t in an ssc_data_t:
*/
/**@{*/
#define SSC_INVALID 0
#define SSC_STRING 1
#define SSC_NUMBER 2
#define SSC_ARRAY 3     // only numeric entries
#define SSC_MATRIX 4    // only numeric entries
#define SSC_TABLE 5
#define SSC_DATARR 6    // entries may be any SSC type
#define SSC_DATMAT 7    // entries may be any SSC type
/**@}*/

/**
 * An opaque reference to a structure that holds a hash table of variables. This structure can contain any number of
 * variables (ssc_var_t) referenced by name, which can hold strings, numbers, arrays, matrices, and tables.
 * Matrices are stored in row-major order, where the array size is nrows*ncols, and the array index is calculated by
 * r*ncols+c.
 *
 * An ssc_data_t object holds all input and output variables for a simulation. It does not distinguish
 * between input, output, and input variables - that is handled at the model context level.
 *
 * Example of assigning an SSC_STRING:
 *      ssc_data_t p_data = ssc_data_create();
 *      ssc_data_set_string(p_data, "string");
 *      ...
 *      ssc_data_free(p_data);
 */
typedef void* ssc_data_t;

/**
 * An opaque reference to a structure that holds a variable of the possible types above. This structure provides a way
 * to collect variables of different types into a single container, the ssc_data_t hash table. Each entry in a ssc_data_t
 * can be worked with as ssc_var_t or indirectly from the ssc_data_* functions below.
 *
 * Example of assigning an SSC_DATARR:
 *      \verbatim
 *      ssc_data_t p_data = ssc_data_create();
 *      ssc_var_t datarr[2];
 *      for (size_t i = 0; i < 2; i++){
 *          vd[i] = ssc_var_create();
 *          ssc_var_set_string(vd[i], "string");
 *      }
 *      ssc_data_set_data_array(data, "array", &vd[0], 2);
 *      ...
 *      # free p_data, datarr
 *      \endverbatim
 */
typedef void* ssc_var_t;

/** The numeric type used in the SSC API. All numeric values are stored in this format. SSC uses 64-bit double numbers
 * at the library interface and calculations inside compute modules generally are performed with double-precision
 * 64-bit floating point internally. */
typedef double ssc_number_t;

/** The boolean type used internally in SSC. Zero values represent false; non-zero represents true. */
typedef int ssc_bool_t;

/** Returns an empty ssc_var_t which will need to be freed after use. */
SSCEXPORT ssc_var_t ssc_var_create();

SSCEXPORT void ssc_var_free(ssc_var_t p_var);

/** Clears all of the values in a var object. Type is reset to SSC_INVALID */
SSCEXPORT void ssc_var_clear( ssc_var_t p_var );

/** Get type of variable as defined above, which will determine which setter and getter function must be used. */
SSCEXPORT int ssc_var_query(ssc_var_t p_var);

/** Invalid types are 0x0. Strings and numbers are 1x1. Arrays and tables are nx1 and matrices are nxm. */
SSCEXPORT void ssc_var_size(ssc_var_t p_var, int* nrows, int* ncols);

/** Assigns a copy of a string value to the p_var variable. Type becomes SSC_STRING. */
SSCEXPORT void ssc_var_set_string( ssc_var_t p_var, const char *value );

/** Assigns a copy of a numeric value to the p_var variable. Type becomes SSC_NUMBER. */
SSCEXPORT void ssc_var_set_number( ssc_var_t p_var, ssc_number_t value );

/** Assigns a copy of an array of doubles of given length to the p_var variable. Type becomes SSC_ARRAY. */
SSCEXPORT void ssc_var_set_array( ssc_var_t p_var, ssc_number_t *pvalues, int length );

/** Assigns a copy of a matrix of doubles of given dimension to the p_var variable. Type becomes SSC_MATRIX. */
SSCEXPORT void ssc_var_set_matrix( ssc_var_t p_var, ssc_number_t *pvalues, int nrows, int ncols );

/** Assigns a copy of a table to the p_var variable. Type becomes SSC_TABLE. */
SSCEXPORT void ssc_var_set_table( ssc_var_t p_var, ssc_data_t table );

/** Assigns a copy of a variable to the r-th entry in an array of ssc_var_t. Type becomes SSC_DATARR. */
SSCEXPORT void ssc_var_set_data_array(ssc_var_t p_var, ssc_var_t p_var_entry, int r );

/** Assigns a copy of a variable to the r,c-th entry in a matrix of ssc_var_t. Type becomes SSC_DATMAT. */
SSCEXPORT void ssc_var_set_data_matrix(ssc_var_t p_var, ssc_var_t p_var_entry, int r, int c );

/** Returns a copy of the string value. */
SSCEXPORT const char *ssc_var_get_string( ssc_var_t p_var);

/** Returns a copy of the numeric value. */
SSCEXPORT ssc_number_t ssc_var_get_number( ssc_var_t p_var );

/** Returns a reference to the numeric array and gets the length. */
SSCEXPORT ssc_number_t *ssc_var_get_array(ssc_var_t p_var,  int *length );

/** Returns a reference to the numeric matrix and gets the dimensions. */
SSCEXPORT ssc_number_t *ssc_var_get_matrix( ssc_var_t p_var, int *nrows, int *ncols );

/** Returns a reference to the ssc_data_t table stored in p_var. */
SSCEXPORT ssc_data_t ssc_var_get_table( ssc_var_t p_var);

/** Returns a reference to the r-th variable entry in the variant array. */
SSCEXPORT ssc_var_t ssc_var_get_var_array(ssc_var_t p_var, int r);

/** Returns a reference to the r,c-th variable entry in the variant matrix. */
SSCEXPORT ssc_var_t ssc_var_get_var_matrix(ssc_var_t p_var, int r, int c);

/** Creates a new data object in memory.  A data object stores a table of named values, where each value can be of any SSC datatype. */
SSCEXPORT ssc_data_t ssc_data_create();

/** Frees the memory associated with a data object, where p_data is the data container to free. */
SSCEXPORT void ssc_data_free( ssc_data_t p_data );

/** Clears all of the variables in a data object. Type becomes SSC_INVALID. */
SSCEXPORT void ssc_data_clear( ssc_data_t p_data );

/** Unassigns the variable with the specified name. */
SSCEXPORT void ssc_data_unassign( ssc_data_t p_data, const char *name );

/** Rename a variable in the data table. returns 1 if succeeded*/
SSCEXPORT int ssc_data_rename( ssc_data_t p_data, const char *oldname, const char *newname );

/** Querys the data object for the data type of the variable with the specified name. Returns the data object's data type, or SSC_INVALID if that variable was not found. */
SSCEXPORT int ssc_data_query( ssc_data_t p_data, const char *name );

/** Returns the name of the first variable in the table, or 0 (NULL) if the data object is empty. */
SSCEXPORT const char *ssc_data_first( ssc_data_t p_data );

/** Returns the name of the next variable in the table, or 0 (NULL) if there are no more variables in the table.  ssc_data_first must be called first. Example that iterates over all variables in a data object:

   \verbatim
	const char *key = ssc_data_first( my_data );
	while (key != 0)
	{
		int type = ssc_data_query( my_data, key );
		key = ssc_data_next( my_data );
	}
    \endverbatim

 */
SSCEXPORT const char *ssc_data_next( ssc_data_t p_data );

SSCEXPORT ssc_var_t ssc_data_lookup(ssc_data_t p_data, const char *name);

/** Returns a reference to a stored variable by case-matching the name. */
SSCEXPORT ssc_var_t ssc_data_lookup_case(ssc_data_t p_data, const char *name);

/** @name Assigning variable values.
The following functions do not take ownership of the data pointers for arrays, matrices, and tables. A deep copy is made into the internal SSC engine. You must remember to free the table that you create to pass into
ssc_data_set_table( ) for example.
*/
/**@{*/

SSCEXPORT void ssc_data_set_var(ssc_data_t p_data, const char *name, ssc_var_t p_var);

/** Assigns value of type @a SSC_STRING */
SSCEXPORT void ssc_data_set_string( ssc_data_t p_data, const char *name, const char *value );

/** Assigns value of type @a SSC_NUMBER */
SSCEXPORT void ssc_data_set_number( ssc_data_t p_data, const char *name, ssc_number_t value );

/** Assigns value of type @a SSC_ARRAY */
SSCEXPORT void ssc_data_set_array( ssc_data_t p_data, const char *name, ssc_number_t *pvalues, int length );

/** Assigns value of type @a SSC_MATRIX . Matrices are specified as a continuous array, in row-major order.  Example: the matrix [[5,2,3],[9,1,4]] is stored as [5,2,3,9,1,4]. */
SSCEXPORT void ssc_data_set_matrix( ssc_data_t p_data, const char *name, ssc_number_t *pvalues, int nrows, int ncols );

/** Assigns value of type @a SSC_TABLE. */
SSCEXPORT void ssc_data_set_table( ssc_data_t p_data, const char *name, ssc_data_t table );

/** Assigns value of type @a SSC_DATAARR. */
SSCEXPORT void ssc_data_set_data_array(ssc_data_t p_data, const char *name, ssc_var_t *data_array, int nrows );

/** Assigns value of type @a SSC_DATAMAT. */
SSCEXPORT void ssc_data_set_data_matrix(ssc_data_t p_data, const char *name, ssc_var_t *data_matrix, int nrows, int ncols );
/**@}*/

/** @name Retrieving variable values.
The following functions return internal references to memory, and the returned string, array, matrix, and tables should not be freed by the user.
*/
/**@{*/
/** Returns the value of a @a SSC_STRING variable with the given name. */
SSCEXPORT const char *ssc_data_get_string( ssc_data_t p_data, const char *name );

/** Returns the value of a @a SSC_NUMBER variable with the given name. */
SSCEXPORT ssc_bool_t ssc_data_get_number( ssc_data_t p_data, const char *name, ssc_number_t *value );

/** Returns the reference of a @a SSC_ARRAY variable with the given name. */
SSCEXPORT ssc_number_t *ssc_data_get_array( ssc_data_t p_data, const char *name, int *length );

/** Returns the reference of a @a SSC_MATRIX variable with the given name. Matrices are specified as a continuous array, in row-major order.  Example: the matrix [[5,2,3],[9,1,4]] is stored as [5,2,3,9,1,4]. */
SSCEXPORT ssc_number_t *ssc_data_get_matrix( ssc_data_t p_data, const char *name, int *nrows, int *ncols );

/** Returns the reference of a @a SSC_TABLE variable with the given name. */
SSCEXPORT ssc_data_t ssc_data_get_table( ssc_data_t p_data, const char *name );

/** Returns the reference of a @a SSC_DATAARR variable with the given name. */
SSCEXPORT ssc_var_t ssc_data_get_data_array(ssc_data_t p_data, const char *name, int *nrows);

/** Returns the reference of a @a SSC_DATAMAT variable with the given name. */
SSCEXPORT ssc_var_t ssc_data_get_data_matrix(ssc_data_t p_data, const char *name, int* nrows, int* ncols );

/**@}*/

// NOTE: EnergyPlus: json_to_ssc_data / ssc_data_to_json removed (see sscapi.cpp)

/** The opaque data structure that stores information about a compute module. */
typedef void* ssc_entry_t;

/** Returns compute module information for the i-th module in the SSC library. Returns 0 (NULL) for an invalid index. Example:

	\verbatim
	int i=0;
	ssc_entry_t p_entry;
	while( p_entry = ssc_module_entry(i++) )
	{
		printf("Compute Module '%s': \n",
		     	ssc_entry_name(p_entry),
			    ssc_entry_description(p_entry) );
	}
	\endverbatim
*/
SSCEXPORT ssc_entry_t ssc_module_entry( int index );

/** Returns the name of a compute module.  This is the name that is used to create a new compute module. */
SSCEXPORT const char *ssc_entry_name( ssc_entry_t p_entry );

/** Returns a short text description of a compute module. */
SSCEXPORT const char *ssc_entry_description( ssc_entry_t p_entry );

/** Returns version information about a compute module. */
SSCEXPORT int ssc_entry_version( ssc_entry_t p_entry );

/** An opaque reference to a computation module. A computation module performs a transformation on a ssc_data_t. It usually is used to calculate output variables given a set of input variables, but it can also be used to change the values of variables defined as INOUT. Modules types have unique names, and store information about what input variables are required, what outputs can be expected, along with specific data type, unit, label, and meta information about each variable. */
typedef void* ssc_module_t;

/** An opaque reference to variable information.  A compute module defines its input/output variables. */
typedef void* ssc_info_t;

/** Creates an instance of a compute module with the given name. Returns 0 (NULL) if invalid name given and the module could not be created */
SSCEXPORT ssc_module_t ssc_module_create( const char *name );

/** Releases an instance of a compute module created with ssc_module_create */
SSCEXPORT void ssc_module_free( ssc_module_t p_mod );

/** @name Variable types:*/
/**@{*/
#define SSC_INPUT 1
#define SSC_OUTPUT 2
#define SSC_INOUT 3
/**@}*/

/** Returns references to variable info objects.  Returns NULL for invalid index. Note that the ssc_info_* functions that return strings may return NULL if the computation module has not specified a value, i.e. no units or no grouping name. Example for a previously created 'p_mod' object:

	\verbatim
	int i=0;
	const ssc_info_t p_inf = NULL;
	while ( p_inf = ssc_module_var_info( p_mod, i++ ) )
	{
		int var_type = ssc_info_var_type( p_inf );   // SSC_INPUT, SSC_OUTPUT, SSC_INOUT
		int data_type = ssc_info_data_type( p_inf ); // SSC_STRING, SSC_NUMBER, SSC_ARRAY, SSC_MATRIX

		const char *name = ssc_info_name( p_inf );
		const char *label = ssc_info_label( p_inf );
		const char *units = ssc_info_units( p_inf );
		const char *meta = ssc_info_meta( p_inf );
		const char *group = ssc_info_group( p_inf );
	}
	\endverbatim
*/
SSCEXPORT ssc_info_t ssc_module_var_info( ssc_module_t p_mod, int index );

/** Returns variable type information: SSC_INPUT, SSC_OUTPUT, or SSC_INOUT */
SSCEXPORT int ssc_info_var_type( ssc_info_t p_inf );

/** Returns the data type of a variable: SSC_STRING, SSC_NUMBER, SSC_ARRAY, SSC_MATRIX, SSC_TABLE */
SSCEXPORT int ssc_info_data_type( ssc_info_t p_inf );

/** Returns the name of a variable */
SSCEXPORT const char *ssc_info_name( ssc_info_t p_inf );

/** Returns the short label description of the variable */
SSCEXPORT const char *ssc_info_label( ssc_info_t p_inf );

/** Returns the units of the values for the variable */
SSCEXPORT const char *ssc_info_units( ssc_info_t p_inf );

/** Returns any extra information about a variable */
SSCEXPORT const char *ssc_info_meta( ssc_info_t p_inf );

/** Returns any grouping information.  Variables can be assigned to groups for presentation to the user, for example */
SSCEXPORT const char *ssc_info_group( ssc_info_t p_inf );

/** Returns information about whether a variable is required to be assigned for
a compute module to run.  It may alternatively be given a default value, specified as '?=<value>'. */
SSCEXPORT const char *ssc_info_required( ssc_info_t p_inf );

/** Returns constraints on the values accepted.  For example, MIN, MAX, BOOLEAN, INTEGER, POSITIVE are possible constraints. */
SSCEXPORT const char *ssc_info_constraints( ssc_info_t p_inf );

/** Returns additional information for use in a target application about how to show the variable to the user. */
SSCEXPORT const char *ssc_info_uihint( ssc_info_t p_inf );

/** Specify whether the built-in execution handler prints messages and progress updates to the command line console. */
SSCEXPORT void ssc_module_exec_set_print( int print );

/** The simplest way to run a computation module over a data set. Simply specify the name of the module, and a data set.  If the whole process succeeded, the function returns 1, otherwise 0.  No error messages are available. This function can be thread-safe, depending on the computation module used. If the computation module requires the execution of external binary executables, it is not thread-safe. However, simpler implementations that do all calculations internally are probably thread-safe.  Unfortunately there is no standard way to report the thread-safety of a particular computation module. */
SSCEXPORT ssc_bool_t ssc_module_exec_simple( const char *name, ssc_data_t p_data );

/** Another very simple way to run a computation module over a data set. The function returns NULL on success.  If something went wrong, the first error message is returned. Because the returned string references a common internal data container, this function is never thread-safe.  */
SSCEXPORT const char *ssc_module_exec_simple_nothread( const char *name, ssc_data_t p_data );

/** @name Action/notification types that can be sent to a handler function:
  *	SSC_LOG: Log a message in the handler. f0: (int)message type, f1: time, s0: message text, s1: unused.
  *	SSC_UPDATE: Notify simulation progress update. f0: percent done, f1: time, s0: current action text, s1: unused.
*/
/**@{*/
#define SSC_LOG 0
#define SSC_UPDATE 1
/**@}*/

/** Runs an instantiated computation module over the specified data set. Returns Boolean: 1 or 0. Detailed notices, warnings, and errors can be retrieved using the ssc_module_log function. */
SSCEXPORT ssc_bool_t ssc_module_exec( ssc_module_t p_mod, ssc_data_t p_data ); /* uses default internal built-in handler */

/** An opaque pointer for transferring external executable output back to SSC */
typedef void* ssc_handler_t;

/** A full-featured way to run a compute module with a callback function to handle custom logging, progress updates, and cancelation requests. Returns Boolean: 1 or 0 indicating success or failure. */
SSCEXPORT ssc_bool_t ssc_module_exec_with_handler(
	ssc_module_t p_mod,
	ssc_data_t p_data,
	ssc_bool_t (*pf_handler)( ssc_module_t, ssc_handler_t, int action, float f0, float f1, const char *s0, const char *s1, void *user_data ),
	void *pf_user_data );

/** @name Message types:*/
/**@{*/
#define SSC_NOTICE 1
#define SSC_WARNING 2
#define SSC_ERROR 3
/**@}*/

/** Retrive notices, warnings, and error messages from the simulation. Returns a NULL-terminated ASCII C string with the message text, or NULL if the index passed in was invalid. */
SSCEXPORT const char *ssc_module_log( ssc_module_t p_mod, int index, int *item_type, float *time );

/** DO NOT CALL THIS FUNCTION: immediately causes a segmentation fault within the library. This is only useful for testing crash handling from an external application that is dynamically linked to the SSC library */
SSCEXPORT void __ssc_segfault();

/**
 * Functions for calling python as an external process with python_handler
 */

SSCEXPORT void set_python_path(const char* abs_path);

SSCEXPORT const char *get_python_path();

/**
 * Functions for calling stateful compute modules
 */

SSCEXPORT ssc_module_t ssc_stateful_module_create(const char *name, ssc_data_t p_data);

#ifndef __SSCLINKAGECPP__

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif // __SSCLINKAGECPP__

#endif
