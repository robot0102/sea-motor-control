///////////////////////////////////////////////////////////////////////////                                                                     
// data_file_macros_gen.h
//
// Author: Gabriel Aguirre-Ollinger 
// Documentation start: 06.03.2020
// 
// Description:		 
//					 
//					 
// Modifications record:
//		
//
///////////////////////////////////////////////////////////////////////////

#ifndef DATA_FILE_MACROS_GEN_H
#define DATA_FILE_MACROS_GEN_H

#include <stdio.h>  
#include <stdlib.h>  
#include <string.h>  
#include <errno.h>  

extern int errno;

/////////////////////////////////////////////////////////////////////////////
// Character definitions:
/////////////////////////////////////////////////////////////////////////////

#define CH_SPACE	' '
#define CH_COMMA	','
#define CH_NEWLN	'\n'
#define CH_TAB	    '\t'
#define CH_EOS	    '\0'
#define CH_BKSLASH  '\\'

/////////////////////////////////////////////////////////////////////////////
// Data directories:
/////////////////////////////////////////////////////////////////////////////

#define BASE_DIR	    "/home/pc104/Documents/_test_innfos_HAN/"
#define APP_DIR	        "/home/pc104/Documents/_test_innfos_HAN/apps/"

#define DATA_DIR		"/home/pc104/Documents/_data_files/"

/////////////////////////////////////////////////////////////////////////////
// File extensions:
/////////////////////////////////////////////////////////////////////////////

#define LEN_NAME_MAX 200

#define EXT_DAT		".dat"
#define EXT_PAR		".par"
#define EXT_KEY		".key"

#define EXT_TXT 	".txt"
#define EXT_HEADER	".h"

/////////////////////////////////////////////////////////////////////////////
// File names:
/////////////////////////////////////////////////////////////////////////////

// Experimental file header:
#define DATA_FNAME_HEAD		"_data_" 

// Variables info file:
#define VARS_EDITABLE_FNAME	"vars_sys_editme" 
#define KEY_FNAME   		"vars_sys_key" // variables key file (space-separated strings to make it Matlab-readable) 

// Variables' identifiers:
#define LEN_VAR_STR     10

#define DAT_STR		"DAT"
#define PAR_STR		"PAR"

// Macros - file management:
#define MACROS_FILE_MGMT_H			"_macros_files_sys" 

#define WRITE_DAT_SYS_STR			"WRITE_DAT_SYS"
#define WRITE_PAR_SYS_STR			"WRITE_PAR_SYS"

// Macros - parameter values storage:
#define MACROS_PARAM_VALUES_H			"_macros_param_values" 

/////////////////////////////////////////////////////////////////////////////
// Other variables:
/////////////////////////////////////////////////////////////////////////////

#define NUM_DIGITS_PREC 7 // number of precision digits for numerical values

#define N_FIELDS_VARS_FILE	4

#define COL_VAR_TYPE	0
#define COL_VAR_NAME	1
#define COL_VAR_SIZE    2
#define COL_VAR_ACTIVE  3

#define N_WORDS_MAX		500
#define LEN_WORD_MAX	25

/////////////////////////////////////////////////////////////////////////////
// File functions:
/////////////////////////////////////////////////////////////////////////////

void files_sys_macros_gen();  

void vars_file_read_help(char vars_file_words[][LEN_WORD_MAX], int* n_vars_sys,
					     int* start_idx_var, int* start_idx_par);

void write_data_string_logger(FILE* file_id, char* var_name_str, char* var_length_str, int num_digits, char* logger_id);
void write_data_string_fwrite(FILE* file_id, char* var_name_str, char* var_length_str, int num_digits, char* file_idstr);

// Ancillary functions:
void file_path_create(char* file_path, char* f_dir, char* file_name, char* file_ext); 
void copy_text_file(FILE* from_file_id, FILE* to_file_id);
int is_alphanum(char ch);

#endif