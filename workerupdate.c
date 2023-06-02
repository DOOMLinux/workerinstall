#include <argp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

const char *argp_program_version = "1.0";
const char *argp_program_bug_address = "<bug@example.com>";

/* Program documentation. */
static char doc[] =
    "workerupdate - updates the repository information from online";

/* A description of the arguments we accept. */
static char args_doc[] = "";

/* The options we understand. */
static struct argp_option options[] = {
    //{"version", 'v', 0, 0, "Print version information."},
    {0}};

/* Used by main to communicate with parse_opt. */
struct arguments
{
    char *args[1];
};

/* Parse a single option. */
static error_t
parse_opt(int key, char *arg, struct argp_state *state)
{
    /* Get the input argument from argp_parse, which we
       know is a pointer to our arguments structure. */
    struct arguments *arguments = state->input;

    switch (key)
    {
    case ARGP_KEY_ARG:
        if (state->arg_num > 0)
            /* Too many arguments. */
            argp_usage(state);

        arguments->args[state->arg_num] = arg;

        break;

    default:
        return ARGP_ERR_UNKNOWN;
    }
    return 0;
}

/* Our argp parser. */
static struct argp argp = {options, parse_opt, 0, 0};

int main(int argc, char **argv)
{
    struct arguments arguments;

    /* Set default values */
    arguments.args[0] = "default1";

    /* Parse the arguments */
    argp_parse(&argp, argc, argv, 0, 0, &arguments);

    printf("Beginning to fetch.\n");

    




    return 0;
}