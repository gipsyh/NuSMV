/* ---------------------------------------------------------------------------


   This file is part of the ``bmc'' package of NuSMV version 2.
   Copyright (C) 2009 by FBK.

   NuSMV version 2 is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   NuSMV version 2 is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with this library; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307  USA.

   For more information on NuSMV see <http://nusmv.fbk.eu>
   or email to <nusmv-users@fbk.eu>.
   Please report bugs to <nusmv-users@fbk.eu>.

   To contact the NuSMV development board, email to <nusmv@fbk.eu>.

-----------------------------------------------------------------------------*/

/*!
  \author Alessandro Mariotti
  \brief bmc Opt module

  This module contains all the bmc options handling functions

*/

#include "nusmv/core/bmc/bmc.h"
#include "nusmv/core/bmc/bmcInt.h"
#include "nusmv/core/cinit/NuSMVEnv.h"
#include "nusmv/core/utils/StreamMgr.h"

/*---------------------------------------------------------------------------*/
/* Constant declarations                                                     */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Type declarations                                                         */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Structure declarations                                                    */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Variable declarations                                                     */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Macro declarations                                                        */
/*---------------------------------------------------------------------------*/

/**AutomaticStart*************************************************************/

/*---------------------------------------------------------------------------*/
/* Static function prototypes                                                */
/*---------------------------------------------------------------------------*/

/* Generic getter functions */
static void *opt_get_integer(OptsHandler_ptr opts, const char *val, void *arg);

static void *opt_get_string(OptsHandler_ptr opts, const char *val, void *arg);

static boolean opt_check_bmc_pb_length(OptsHandler_ptr opts, const char *val,
				       void *arg);
static boolean opt_check_bmc_pb_loop(OptsHandler_ptr opts, const char *val,
				     void *arg);
static boolean opt_check_bmc_invar_alg(OptsHandler_ptr opts, const char *val,
				       void *arg);
static void *opt_get_bmc_invar_alg(OptsHandler_ptr opts, const char *val,
				   void *arg);

#if NUSMV_HAVE_INCREMENTAL_SAT
static boolean opt_check_bmc_inc_invar_alg(OptsHandler_ptr opts,
					   const char *val, void *arg);
static void *opt_get_bmc_inc_invar_alg(OptsHandler_ptr opts, const char *val,
				       void *arg);
#endif

/**AutomaticEnd***************************************************************/

/*---------------------------------------------------------------------------*/
/* Definition of exported functions                                          */
/*---------------------------------------------------------------------------*/

void Bmc_init_opt(NuSMVEnv_ptr env)
{
	OptsHandler_ptr opts = NuSMVEnv_get_value(env, ENV_OPTS_HANDLER);
	boolean options_registered;
	boolean res;

	options_registered =
		OptsHandler_is_option_registered(opts, BMC_OPT_INITIALIZED) &&
		OptsHandler_get_bool_option_value(opts, BMC_OPT_INITIALIZED);

	/* If options are registered, do not register them again */
	if (options_registered)
		return;

	/* -------------------- BMC STUFF -------------------- */
	res = OptsHandler_register_bool_option(opts, BMC_MODE, false, false);
	nusmv_assert(res);

	res = OptsHandler_register_generic_option(
		opts, BMC_DIMACS_FILENAME, DEFAULT_DIMACS_FILENAME, true);
	nusmv_assert(res);

	res = OptsHandler_register_generic_option(opts,
						  BMC_INVAR_DIMACS_FILENAME,
						  DEFAULT_INVAR_DIMACS_FILENAME,
						  true);
	nusmv_assert(res);

	{
		char def[20];
		int chars = snprintf(def, 20, "%d", DEFAULT_BMC_PB_LENGTH);
		SNPRINTF_CHECK(chars, 20);
		res = OptsHandler_register_option(
			opts, BMC_PB_LENGTH, def,
			(Opts_CheckFnType)opt_check_bmc_pb_length,
			(Opts_ReturnFnType)opt_get_integer, true,
			INTEGER_OPTION, env);
		nusmv_assert(res);
	}
	res = OptsHandler_register_option(
		opts, BMC_PB_LOOP, DEFAULT_BMC_PB_LOOP,
		(Opts_CheckFnType)opt_check_bmc_pb_loop,
		(Opts_ReturnFnType)opt_get_string, true, GENERIC_OPTION, env);
	nusmv_assert(res);

	res = OptsHandler_register_option(
		opts, BMC_INVAR_ALG, DEFAULT_BMC_INVAR_ALG,
		(Opts_CheckFnType)opt_check_bmc_invar_alg,
		(Opts_ReturnFnType)opt_get_bmc_invar_alg, true, GENERIC_OPTION,
		env);
	nusmv_assert(res);

#if NUSMV_HAVE_INCREMENTAL_SAT
	res = OptsHandler_register_option(
		opts, BMC_INC_INVAR_ALG, DEFAULT_BMC_INC_INVAR_ALG,
		(Opts_CheckFnType)opt_check_bmc_inc_invar_alg,
		(Opts_ReturnFnType)opt_get_bmc_inc_invar_alg, true,
		GENERIC_OPTION, env);
	nusmv_assert(res);
#endif

	res = OptsHandler_register_bool_option(opts, BMC_OPTIMIZED_TABLEAU,
					       DEFAULT_BMC_OPTIMIZED_TABLEAU,
					       true);
	nusmv_assert(res);

	res = OptsHandler_register_bool_option(opts, BMC_FORCE_PLTL_TABLEAU,
					       DEFAULT_BMC_FORCE_PLTL_TABLEAU,
					       true);
	nusmv_assert(res);

	res = OptsHandler_register_bool_option(opts, BMC_SBMC_IL_OPT, true,
					       false);
	nusmv_assert(res);

	res = OptsHandler_register_bool_option(opts, BMC_SBMC_GF_FG_OPT, true,
					       true);
	nusmv_assert(res);

	res = OptsHandler_register_bool_option(opts, BMC_SBMC_CACHE_OPT, true,
					       false);
	nusmv_assert(res);

	res = OptsHandler_register_bool_option(opts, BMC_OPT_INITIALIZED, true,
					       false);

	/* Flag the BMC options as initialized */
	nusmv_assert(res);
}

void set_bmc_mode(OptsHandler_ptr opt)
{
	boolean res = OptsHandler_set_bool_option_value(opt, BMC_MODE, true);
	nusmv_assert(res);
}
void unset_bmc_mode(OptsHandler_ptr opt)
{
	boolean res = OptsHandler_set_bool_option_value(opt, BMC_MODE, false);
	nusmv_assert(res);
}
boolean opt_bmc_mode(OptsHandler_ptr opt)
{
	return OptsHandler_get_bool_option_value(opt, BMC_MODE);
}

/* BMC Stuff */
char *get_bmc_dimacs_filename(OptsHandler_ptr opt)
{
	return OptsHandler_get_string_option_value(opt, BMC_DIMACS_FILENAME);
}
void set_bmc_dimacs_filename(OptsHandler_ptr opt, char *str)
{
	boolean res =
		OptsHandler_set_option_value(opt, BMC_DIMACS_FILENAME, str);
	nusmv_assert(res);
}
char *get_bmc_invar_dimacs_filename(OptsHandler_ptr opt)
{
	return OptsHandler_get_string_option_value(opt,
						   BMC_INVAR_DIMACS_FILENAME);
}
void set_bmc_invar_dimacs_filename(OptsHandler_ptr opt, char *str)
{
	boolean res = OptsHandler_set_option_value(
		opt, BMC_INVAR_DIMACS_FILENAME, str);
	nusmv_assert(res);
}

int get_bmc_pb_length(OptsHandler_ptr opt)
{
	return OptsHandler_get_int_option_value(opt, BMC_PB_LENGTH);
}

void set_bmc_pb_length(OptsHandler_ptr opt, const int k)
{
	boolean res = OptsHandler_set_int_option_value(opt, BMC_PB_LENGTH, k);
	nusmv_assert(res);
}

const char *get_bmc_pb_loop(OptsHandler_ptr opt)
{
	return OptsHandler_get_string_option_value(opt, BMC_PB_LOOP);
}

void set_bmc_pb_loop(OptsHandler_ptr opt, const char *loop)
{
	boolean res = OptsHandler_set_option_value(opt, BMC_PB_LOOP, loop);
	nusmv_assert(res);
}

const char *get_bmc_invar_alg(OptsHandler_ptr opt)
{
	return OptsHandler_get_string_option_value(opt, BMC_INVAR_ALG);
}

void set_bmc_invar_alg(OptsHandler_ptr opt, const char *alg)
{
	boolean res = OptsHandler_set_option_value(opt, BMC_INVAR_ALG, alg);
	nusmv_assert(res);
}

#if NUSMV_HAVE_INCREMENTAL_SAT
const char *get_bmc_inc_invar_alg(OptsHandler_ptr opt)
{
	return OptsHandler_get_string_option_value(opt, BMC_INC_INVAR_ALG);
}

void set_bmc_inc_invar_alg(OptsHandler_ptr opt, const char *alg)
{
	boolean res = OptsHandler_set_option_value(opt, BMC_INC_INVAR_ALG, alg);
	nusmv_assert(res);
}
#endif

void set_bmc_optimized_tableau(OptsHandler_ptr opt)
{
	boolean res = OptsHandler_set_bool_option_value(
		opt, BMC_OPTIMIZED_TABLEAU, true);
	nusmv_assert(res);
}

void unset_bmc_optimized_tableau(OptsHandler_ptr opt)
{
	boolean res = OptsHandler_set_bool_option_value(
		opt, BMC_OPTIMIZED_TABLEAU, false);
	nusmv_assert(res);
}

boolean opt_bmc_optimized_tableau(OptsHandler_ptr opt)
{
	return OptsHandler_get_bool_option_value(opt, BMC_OPTIMIZED_TABLEAU);
}

void set_bmc_force_pltl_tableau(OptsHandler_ptr opt)
{
	boolean res = OptsHandler_set_bool_option_value(
		opt, BMC_FORCE_PLTL_TABLEAU, true);
	nusmv_assert(res);
}
void unset_bmc_force_pltl_tableau(OptsHandler_ptr opt)
{
	boolean res = OptsHandler_set_bool_option_value(
		opt, BMC_FORCE_PLTL_TABLEAU, false);
	nusmv_assert(res);
}
boolean opt_bmc_force_pltl_tableau(OptsHandler_ptr opt)
{
	return OptsHandler_get_bool_option_value(opt, BMC_FORCE_PLTL_TABLEAU);
}

void set_bmc_sbmc_gf_fg_opt(OptsHandler_ptr opt)
{
	boolean res = OptsHandler_set_bool_option_value(opt, BMC_SBMC_GF_FG_OPT,
							true);
	nusmv_assert(res);
}

void unset_bmc_sbmc_gf_fg_opt(OptsHandler_ptr opt)
{
	boolean res = OptsHandler_set_bool_option_value(opt, BMC_SBMC_GF_FG_OPT,
							false);
	nusmv_assert(res);
}

boolean opt_bmc_sbmc_gf_fg_opt(OptsHandler_ptr opt)
{
	return OptsHandler_get_bool_option_value(opt, BMC_SBMC_GF_FG_OPT);
}

void set_bmc_sbmc_il_opt(OptsHandler_ptr opt)
{
	boolean res =
		OptsHandler_set_bool_option_value(opt, BMC_SBMC_IL_OPT, true);
	nusmv_assert(res);
}

void unset_bmc_sbmc_il_opt(OptsHandler_ptr opt)
{
	boolean res =
		OptsHandler_set_bool_option_value(opt, BMC_SBMC_IL_OPT, false);
	nusmv_assert(res);
}

boolean opt_bmc_sbmc_il_opt(OptsHandler_ptr opt)
{
	return OptsHandler_get_bool_option_value(opt, BMC_SBMC_IL_OPT);
}

void set_bmc_sbmc_cache(OptsHandler_ptr opt)
{
	boolean res = OptsHandler_set_bool_option_value(opt, BMC_SBMC_CACHE_OPT,
							true);
	nusmv_assert(res);
}

void unset_bmc_sbmc_cache(OptsHandler_ptr opt)
{
	boolean res = OptsHandler_set_bool_option_value(opt, BMC_SBMC_CACHE_OPT,
							false);
	nusmv_assert(res);
}

boolean opt_bmc_sbmc_cache(OptsHandler_ptr opt)
{
	return OptsHandler_get_bool_option_value(opt, BMC_SBMC_CACHE_OPT);
}

/*---------------------------------------------------------------------------*/
/* Definition of internal functions                                          */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Definition of static functions                                            */
/*---------------------------------------------------------------------------*/

/*!
  \brief Check function for the bmc_pb_lenght option

  Check function for the bmc_pb_lenght option
*/
static boolean opt_check_bmc_pb_length(OptsHandler_ptr opts, const char *val,
				       void *arg)
{
	const NuSMVEnv_ptr env = NUSMV_ENV(arg);
	StreamMgr_ptr streams =
		STREAM_MGR(NuSMVEnv_get_value(env, ENV_STREAM_MANAGER));

	Outcome res;
	int length = PTR_TO_INT(opt_get_integer(opts, val, arg));

	char *str_loop = OptsHandler_get_string_option_value(opts, BMC_PB_LOOP);

	int tmp = Bmc_Utils_ConvertLoopFromString(str_loop, &res);

	int loop = Bmc_Utils_RelLoop2AbsLoop(tmp, length);

	if ((res == OUTCOME_SUCCESS) &&
	    Bmc_Utils_Check_k_l(length, loop) == OUTCOME_SUCCESS) {
		return true;
	} else {
		StreamMgr_print_error(
			streams, "Given value is not compatible with current ");
		StreamMgr_print_error(streams, "loopback value set to %s.\n",
				      str_loop);
	}

	return false;
}

/*!
  \brief Check function for the bmc_pb_loop option

  Check function for the bmc_pb_loop option
*/
static boolean opt_check_bmc_pb_loop(OptsHandler_ptr opts, const char *val,
				     void *arg)
{
	const NuSMVEnv_ptr env = NUSMV_ENV(arg);
	StreamMgr_ptr streams =
		STREAM_MGR(NuSMVEnv_get_value(env, ENV_STREAM_MANAGER));
	Outcome res;
	int length = OptsHandler_get_int_option_value(opts, BMC_PB_LENGTH);
	int tmp = Bmc_Utils_ConvertLoopFromString(val, &res);
	int loop = Bmc_Utils_RelLoop2AbsLoop(tmp, length);

	if ((res == OUTCOME_SUCCESS) &&
	    Bmc_Utils_Check_k_l(length, loop) == OUTCOME_SUCCESS) {
		return true;
	} else {
		StreamMgr_print_error(
			streams,
			"Invalid value for the bmc_loopback variable.\n");
	}
	return false;
}

/*!
  \brief Check function for the bmc_invar_alg option

  Check function for the bmc_invar_alg option
*/
static boolean opt_check_bmc_invar_alg(OptsHandler_ptr opts, const char *val,
				       void *arg)
{
	const NuSMVEnv_ptr env = NUSMV_ENV(arg);
	const StreamMgr_ptr streams =
		STREAM_MGR(NuSMVEnv_get_value(env, ENV_STREAM_MANAGER));

	void *res = opt_get_bmc_invar_alg(opts, val, arg);

	if (OPTS_VALUE_ERROR == res) {
		StreamMgr_print_error(
			streams,
			"The available INVAR solving (non-incremental) algorithms are:\n");
		StreamMgr_print_error(streams, "%s %s\n", BMC_INVAR_ALG_CLASSIC,
				      BMC_INVAR_ALG_EEN_SORENSSON);
		return false;
	}

	return true;
}

/*!
  \brief Get function for the bmc_invar_alg function

  Get function for the bmc_invar_alg function
*/
static void *opt_get_bmc_invar_alg(OptsHandler_ptr opts, const char *val,
				   void *arg)
{
	if (strcasecmp(BMC_INVAR_ALG_CLASSIC, val) == 0) {
		return BMC_INVAR_ALG_CLASSIC;
	} else if (strcasecmp(BMC_INVAR_ALG_EEN_SORENSSON, val) == 0) {
		return BMC_INVAR_ALG_EEN_SORENSSON;
	}
	return OPTS_VALUE_ERROR;
}

#if NUSMV_HAVE_INCREMENTAL_SAT

/*!
  \brief Check function for the bmc_inc_invar_alg function

  Check function for the bmc_inc_invar_alg function
*/
static boolean opt_check_bmc_inc_invar_alg(OptsHandler_ptr opts,
					   const char *val, void *arg)
{
	const NuSMVEnv_ptr env = NUSMV_ENV(arg);
	const StreamMgr_ptr streams =
		STREAM_MGR(NuSMVEnv_get_value(env, ENV_STREAM_MANAGER));

	void *res = opt_get_bmc_inc_invar_alg(opts, val, arg);

	if (OPTS_VALUE_ERROR == res) {
		StreamMgr_print_error(
			streams,
			"The available INVAR solving (incremental) algorithms are:\n");
		StreamMgr_print_error(streams, "%s %s\n",
				      BMC_INC_INVAR_ALG_DUAL,
				      BMC_INC_INVAR_ALG_ZIGZAG);
		return false;
	}

	return true;
}

/*!
  \brief Get function for the bmc_inc_invar_alg function

  Get function for the bmc_inc_invar_alg function
*/
static void *opt_get_bmc_inc_invar_alg(OptsHandler_ptr opts, const char *val,
				       void *arg)
{
	if (strcasecmp(BMC_INC_INVAR_ALG_ZIGZAG, val) == 0) {
		return BMC_INC_INVAR_ALG_ZIGZAG;
	} else if (strcasecmp(BMC_INC_INVAR_ALG_FALSIFICATION, val) == 0) {
		return BMC_INC_INVAR_ALG_FALSIFICATION;
	} else if (strcasecmp(BMC_INC_INVAR_ALG_DUAL, val) == 0) {
		return BMC_INC_INVAR_ALG_DUAL;
	}
	return OPTS_VALUE_ERROR;
}
#endif

/*!
  \brief Get the integer representation of the given string

  Get the integer representation of the given string
*/
static void *opt_get_integer(OptsHandler_ptr opts, const char *value, void *arg)
{
	int result;
	char *e[1];

	e[0] = "";
	result = (int)strtol(value, e, 10);
	if (strcmp(e[0], "") != 0) {
		return OPTS_VALUE_ERROR;
	}
	return PTR_FROM_INT(void *, result);
}

/*!
  \brief Get function for simple strings

  Get function for simple strings
*/
static void *opt_get_string(OptsHandler_ptr opts, const char *val, void *arg)
{
	return (void *)val;
}
