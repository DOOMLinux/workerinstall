#include <stdlib.h>
#include <argp.h>
#include <string.h>
#include <stdbool.h>

#include <libxml/tree.h>
#include <libxml/parser.h>

#include <math.h>

/*
    worker's global variables
*/
/**
 * verbose, quiet
 */
int v, q;
/**
 * prefix
 */
char *prefix; // i.e. root / or ./test-root/
/*
  worker's abstract syntax tree.
*/
/*
  How does worker's AST work for package resolvement?
  basically we can probably create multiple root nodes representing top level packages.
  as in packages defined by the user.
  This is what the AST looks like if you just specify worker.
        [worker]
       /
 [libxml2]
  This is as worker only has one dependancy, libxml2 and libxml2 does not have dependancies.
  If worker had more than 2 dependancies, these dependancies will be grouped under virtual nodes. (virtual as in it is not a package)
  virtual is represented by 'data' as the string "virtual".

  Packages are installed in postorder traversal.
*/
struct node
{
    char *data;    // package name
    char *version; // package version, sure this can be added to data as an atom such as portage does it =sys-apps/portage-3.0.45.3-r2
    struct node *parent;
    struct node *left;
    struct node *right;
};
/*
   Error handling
*/
typedef enum
{
    CRITICAL_HALT,
    PACKAGE_COULD_NOT_RESOLVE,
    DEPENDANCY_RECURSION,
    SYSTEMD_DEPENDANCY, // we absolutely hate systemd
    USER_ERROR,
} it_is_error_time;
void error(it_is_error_time e)
{
    // Sure a switch would work.
    // This is supposed to be funny.
    char *er = (e == CRITICAL_HALT) ? "CRITICAL HALT" : (e == PACKAGE_COULD_NOT_RESOLVE) ? "PACKAGES COULD NOT BE RESOLVED"
                                                    : (e == DEPENDANCY_RECURSION)        ? "DEPENDANCY RECURSION"
                                                    : (e == SYSTEMD_DEPENDANCY)          ? "SYSTEMD DEPENDANCY"
                                                    : (e == USER_ERROR)                  ? "USER ERROR"
                                                                                         : "OTHER";
    printf("\x1b[1;31mworker halt.\x1b[0;22m\nError code: 0x%x %s\n", e, er);
    exit(-1);
}
/*
    argp
*/
const char *argp_program_version =
    "0.1";
const char *argp_program_bug_address =
    "<worker@doomlinux.org>";

/* Program documentation. */
static char doc[] =
    "worker -- a package manager";

/* A description of the arguments we accept. */
static char args_doc[] = "package(s)";

/* The options we understand. */
static struct argp_option options[] = {
    {"upgrade", 'u', 0, 0, "Upgrade a package"},
    {"purge", 'p', 0, 0, "Purge the package."},
    {"prefix", 'P', "DIR", 0, "Set the prefix path for the package installation."},
    {"update", 'U', 0, 0, "Update the worker packagelist."},
    {"verbose", 'v', 0, 0, "Produce verbose output"},
    {"quiet", 'q', 0, 0, "Don't produce any output"},
    {"silent", 's', 0, OPTION_ALIAS},
    //{"output", 'o', "FILE", 0,
    // "Output to FILE instead of standard output"},
    {0}};

/* Used by main to communicate with parse_opt. */
struct arguments
{
    char *args[50]; /* arg1 & arg2 */
    int amount_of_args;
    int silent, verbose, upgrade, update_packages;
    char *output_file;
    char *prefix;
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
    case 'q':
    case 's':
        arguments->silent = 1;
        break;
    case 'v':
        arguments->verbose = 1;
        break;
    case 'o':
        arguments->output_file = arg;
        break;
    case 'U':
        arguments->update_packages = 1;
        break;
    case 'P':
        arguments->prefix = arg;
        break;
    case ARGP_KEY_ARG:
        // if (state->arg_num >= 2)
        /* Too many arguments. */
        // argp_usage(state);

        arguments->args[state->arg_num] = arg;
        arguments->amount_of_args++;

        break;

    case ARGP_KEY_END:
        // if (state->arg_num < 2)
        /* Not enough arguments. */
        // argp_usage(state);
        break;

    default:
        return ARGP_ERR_UNKNOWN;
    }
    return 0;
}
/* Our argp parser. */
static struct argp argp = {options, parse_opt, args_doc, doc};
/*
  functions
*/
void start_create_tree(struct arguments arguments);
void print_created_tree();
void install_created_tree();
void update_packages();
char *get_package_atom(char *package_name);
bool is_valid_atom(char *atom);
void safe_is_valid(char *atom);
/*
  begin main
*/
int main(int argc, char **argv)
{
    struct arguments arguments;

    /* Default values. */
    arguments.silent = 0;
    arguments.verbose = 0;
    arguments.output_file = "-";
    arguments.prefix = "/";
    prefix = "/";
    arguments.amount_of_args = 0;
    arguments.upgrade = 0;
    arguments.update_packages = 0;

    /* Parse our arguments; every option seen by parse_opt will
       be reflected in arguments. */
    argp_parse(&argp, argc, argv,
               0,
               0, &arguments);

    // copy argument prefix to prefix.
    // strcpy(prefix, arguments.prefix);
    prefix = arguments.prefix;
    v = arguments.verbose; // copy verbose to v
    q = arguments.silent;  // copy silent to q

    // check if there are no arguments AND no argument for update packages.xml, sure yeah this could be done in parse_opt, but not really.
    if (arguments.amount_of_args == 0 && arguments.update_packages == 0)
    {
        // const struct argp_state *state;
        // argp_usage(state);
        printf("Usage: worker [-pqsuUv?V] [-P DIR] [--purge] [--prefix=DIR] [--quiet]\n            [--silent] [--upgrade] [--update] [--verbose] [--help] [--usage]\n            [--version] package(s)\n");
        return 0;
    }

    if (arguments.update_packages)
    {
        update_packages();
        return 0;
    }

    // we have to start creating the tree.

    if (arguments.silent != 1)
    {
        printf("worker ");
    }

    start_create_tree(arguments);

    // print created tree for end user.
    // we also ask, we should probably add --ask as an argument.

    print_created_tree();

    // install created tree

    install_created_tree();

    // we exit lmao
    exit(0);
}

void start_create_tree(struct arguments arguments)
{
    printf("start_create_tree\n");
    // this part is tricky, I don't know how to do it.
    // basically in this function we have to go through all arguments in arguments.args,
    // and then create a tree, depending on the amount of packages it is sort of... difficult to generate it using a binary tree.
    //
    // if it's a single package we can just
    //              (package)
    //            /
    //        (dependancy)
    // no virtual packages required.
    // if there are multiple packages in the top level we have
    //              (virtual)
    //          /            \
    //      (package)       (package)
    // if we have 3 or more basically.
    //              (virtual)
    //            /          \
    //     (package)       (virtual)
    //                   /          \
    //               (package)       (package)
    // how tf do I create a tree like this????????????

    // We can separate this tree creation process in two.
    // The initial, arguments -> tree
    // And then the dependancy check where we add the dependencies to the tree.

    // Define root_node

    struct node root;

    // initial.

    bool is_done = false;
    int on_what_package = 0;
    int on_what_previous_package = 0;
    int iteration_without_package_count_rise = 0;
    bool is_shift = false; // are we shifting everything over?
    while (!is_done)
    {
        if (on_what_package >= arguments.amount_of_args)
            break;

        if (on_what_package == on_what_previous_package)
        {
            iteration_without_package_count_rise += 1;
        }
        if (iteration_without_package_count_rise == 5)
        {
            printf("start_create_tree loop #1 had 5 iterations with no package_count change.\n");
            error(PACKAGE_COULD_NOT_RESOLVE);
            break;
        }

        if (root.data == NULL)
        {
            root.data = (is_valid_atom(arguments.args[on_what_package]) == true) ? arguments.args[on_what_package] : "get_package_atom(arguments.args[on_what_package]";
            on_what_package++;
            printf("Set root.data %s;\n", root.data);
        }

        printf("%d %d %d;\n", on_what_package, on_what_previous_package, iteration_without_package_count_rise);
        on_what_previous_package = on_what_package;
    }

    // previous code was absolutely useless, add back from git if it proves useful.
}

void safe_is_valid(char *atom)
{
    if (!is_valid_atom(atom))
    {
        error(USER_ERROR);
    }
}
bool is_valid_atom(char *atom)
{
    bool exists = false;
    bool found_it_or_checked_it = false; // Have we found the package, or have we checked every source?
    // Sources we can check are packages.xml and community.xml
    // community.xml may or may not exist.
    // this is a while loop instead of a for loop
    // since adding custom repositories will become a thing.
    char *repositories[2] = {"packages.xml", "community.xml"};
    // for now let's hardcode the possible package sources.
    int checking_source = 0;
    char aa[strlen(atom) + 1];
    strcpy(aa, atom);

    

    while (!found_it_or_checked_it)
    {
        // printf("checking: %s\n", repositories[checking_source]);

        // it is time we check packages.xml

        char *path = "";
        asprintf(&path, "%susr/share/workerinstall/%s", prefix, repositories[checking_source]);
        /*char *filename = "usr/share/workerinstall/packages.xml";
        char *path = malloc(strlen(prefix) + strlen(filename) + 1);
        strcpy(path, prefix);
        strcat(path, filename);*/
        xmlDocPtr doc = xmlReadFile(path, NULL, 0);

        if (doc == NULL)
        {
            printf("Can't parse yo package mate\n");
            // return 1;
            found_it_or_checked_it = true; // invalid repository, returning.
            break;
        }

        xmlNodePtr root = xmlDocGetRootElement(doc);

        if (root == NULL)
        {
            printf("Error: could not get root element\n");
            xmlFreeDoc(doc);
            found_it_or_checked_it = true; // invalid repository, returning.
            break;
        }

        xmlNodePtr packageNode = NULL;
        for (xmlNodePtr node = root->children; node != NULL; node = node->next)
        {
            if (xmlStrcmp(node->name, (const xmlChar *)"Package") == 0)
            {
                xmlNodePtr nameNode = xmlFirstElementChild(node);
                if (nameNode != NULL && xmlStrcmp(nameNode->name, (const xmlChar *)"Name") == 0)
                {
                    char *t = strtok(atom + 1, "-");
                    xmlChar *name = xmlNodeGetContent(nameNode);
                    if (xmlStrcmp(name, (const xmlChar *)t) == 0)
                    {
                        packageNode = node;

                        xmlNodePtr versionNode = xmlFirstElementChild(packageNode);
                        while (versionNode != NULL && xmlStrcmp(versionNode->name, (const xmlChar *)"Version") != 0)
                        {
                            versionNode = versionNode->next;
                        }
                        //printf("%s\n", strtok(atom + 1, "-"));
                        // if(version == strtok(aa,"-"))

                        xmlFree(name);
                        break;
                    }
                    xmlFree(name);
                }
            }
        }

        if (packageNode == NULL)
        {
            // error(PACKAGE_COULD_NOT_RESOLVE);
            break;
            // not this repo.
        }
        else
        {
            // Get the Version element of the Package node
            xmlNodePtr versionNode = xmlFirstElementChild(packageNode);
            while (versionNode != NULL && xmlStrcmp(versionNode->name, (const xmlChar *)"Version") != 0)
            {
                versionNode = versionNode->next;
            }

            if (versionNode == NULL)
            {
                printf("Error: could not find Version element of Package node\n");
                // xmlFreeDoc(doc);
                // return 1;
            }

            // Get the Name element of the Package node
            xmlNodePtr nameNode = xmlFirstElementChild(packageNode);
            while (nameNode != NULL && xmlStrcmp(nameNode->name, (const xmlChar *)"Name") != 0)
            {
                nameNode = nameNode->next;
            }

            if (nameNode == NULL)
            {
                printf("Error: could not find Name element of Package node\n");
                // xmlFreeDoc(doc);
                // return 1;
            }

            xmlChar *name = xmlNodeGetContent(nameNode);
            xmlChar *version = xmlNodeGetContent(versionNode);
            // printf("Package name: %s\nPackage version: %s\n", name, version);

            // printf("=%s-%s", name, version);
            exists = true;
            // asprintf(&atom, "=%s-%s", name, version);
            // printf("I FOUND IT FOR YA %s\n", atom);
        }

        // increment checking_source and go to next.

        checking_source++;
        if (checking_source == 2)
        {
            found_it_or_checked_it = true;
        }

        free(path);
    }

    return exists;
}

char *get_package_atom(char *package_name)
{
    // no this function doesn't support actual atom packages.
    // no we don't check if the package_name has a version number.
    // yes we should alias worker to workerinstall in packages.xml

    char *atom = "virtual"; // we just send a virtual lmaooo

    bool found_it_or_checked_it = false; // Have we found the package, or have we checked every source?
    // Sources we can check are packages.xml and community.xml
    // community.xml may or may not exist.
    // this is a while loop instead of a for loop
    // since adding custom repositories will become a thing.
    char *repositories[2] = {"packages.xml", "community.xml"};
    // for now let's hardcode the possible package sources.
    int checking_source = 0;
    while (!found_it_or_checked_it)
    {
        // printf("checking: %s\n", repositories[checking_source]);

        // it is time we check packages.xml

        char *path = "";
        asprintf(&path, "%susr/share/workerinstall/%s", prefix, repositories[checking_source]);
        /*char *filename = "usr/share/workerinstall/packages.xml";
        char *path = malloc(strlen(prefix) + strlen(filename) + 1);
        strcpy(path, prefix);
        strcat(path, filename);*/
        xmlDocPtr doc = xmlReadFile(path, NULL, 0);

        if (doc == NULL)
        {
            printf("Can't parse yo package mate\n");
            // return 1;
            found_it_or_checked_it = true; // invalid repository, returning.
            break;
        }

        xmlNodePtr root = xmlDocGetRootElement(doc);

        if (root == NULL)
        {
            printf("Error: could not get root element\n");
            xmlFreeDoc(doc);
            found_it_or_checked_it = true; // invalid repository, returning.
            break;
        }

        xmlNodePtr packageNode = NULL;
        for (xmlNodePtr node = root->children; node != NULL; node = node->next)
        {
            if (xmlStrcmp(node->name, (const xmlChar *)"Package") == 0)
            {
                xmlNodePtr nameNode = xmlFirstElementChild(node);
                if (nameNode != NULL && xmlStrcmp(nameNode->name, (const xmlChar *)"Name") == 0)
                {
                    xmlChar *name = xmlNodeGetContent(nameNode);
                    if (xmlStrcmp(name, (const xmlChar *)package_name) == 0)
                    {
                        packageNode = node;
                        xmlFree(name);
                        break;
                    }
                    xmlFree(name);
                }
            }
        }

        if (packageNode == NULL)
        {
            // printf("%s")
            // break;
            // not this repo.
        }
        else
        {
            // Get the Version element of the Package node
            xmlNodePtr versionNode = xmlFirstElementChild(packageNode);
            while (versionNode != NULL && xmlStrcmp(versionNode->name, (const xmlChar *)"Version") != 0)
            {
                versionNode = versionNode->next;
            }

            if (versionNode == NULL)
            {
                printf("Error: could not find Version element of Package node\n");
                // xmlFreeDoc(doc);
                // return 1;
            }

            // Get the Name element of the Package node
            xmlNodePtr nameNode = xmlFirstElementChild(packageNode);
            while (nameNode != NULL && xmlStrcmp(nameNode->name, (const xmlChar *)"Name") != 0)
            {
                nameNode = nameNode->next;
            }

            if (nameNode == NULL)
            {
                printf("Error: could not find Name element of Package node\n");
                // xmlFreeDoc(doc);
                // return 1;
            }

            xmlChar *name = xmlNodeGetContent(nameNode);
            xmlChar *version = xmlNodeGetContent(versionNode);
            // printf("Package name: %s\nPackage version: %s\n", name, version);

            asprintf(&atom, "=%s-%s", name, version);
            // printf("I FOUND IT FOR YA %s\n", atom);
        }

        // increment checking_source and go to next.

        checking_source++;
        if (checking_source == 2)
        {
            found_it_or_checked_it = true;
        }

        free(path);
    }

    return atom;
}

void print_created_tree()
{
    // Implement postorder traversal.
}

void install_created_tree()
{
    // Implement postorder traversal again.
}

void update_packages()
{
    // This is going to work by system()'ing a source of /etc/workerinstall/worker.conf
    // Followed by a curl of the address.

    char *format_string = "#!/bin/bash\nsource %setc/workerinstall/worker.conf\ncurl $UPDATE_URL -s -o %susr/share/workerinstall/packages_replace.xml\n";

    char *cmd;

    asprintf(&cmd, format_string, prefix, prefix);

    system(cmd);
}
