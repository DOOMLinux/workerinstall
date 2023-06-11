#include <argp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include <libxml/tree.h>
#include <libxml/parser.h>

const char *argp_program_version = "1.0";
const char *argp_program_bug_address = "<bug@example.com>";

/* Program documentation. */
static char doc[] =
    "workerinstall - installs/upgrades/purges packages from the package list.";

/* A description of the arguments we accept. */
static char args_doc[] = "PACKAGE";

/* The options we understand. */
static struct argp_option options[] = {
    {"install", 'i', 0, 0, "Install the package."},
    {"upgrade", 'u', 0, 0, "Upgrade the package."},
    {"purge", 'p', 0, 0, "Purge the package."},
    {"prefix", 'P', "DIR", 0, "Set the prefix path for the package installation."},
    //{"version", 'v', 0, 0, "Print version information."},
    {0}};

/* Used by main to communicate with parse_opt. */
struct arguments
{
  char *args[10]; /* arg1 */
  int pckg_c;
  int install, upgrade, purge, version;
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
  case 'i':
    arguments->install = 1;
    break;
  case 'u':
    arguments->upgrade = 1;
    break;
  case 'p':
    arguments->purge = 1;
    break;
  case 'v':
    arguments->version = 1;
    break;
  case 'P':
    arguments->prefix = arg;
    break;
  case ARGP_KEY_ARG:
    if (state->arg_num >= 10) // Check if arg_num is >= the size of the args array
      /* Too many arguments. */
      argp_usage(state);

    arguments->args[state->arg_num] = arg;
    arguments->pckg_c++;
    break;
  case ARGP_KEY_END:
    if (state->arg_num < 1)
      /* Not enough arguments. */
      argp_usage(state);
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
  arguments.args[0] = NULL;
  arguments.args[1] = NULL;
  arguments.args[2] = NULL;
  arguments.args[3] = NULL;
  arguments.args[4] = NULL;
  arguments.args[5] = NULL;
  arguments.args[6] = NULL;
  arguments.args[7] = NULL;
  arguments.args[8] = NULL;
  arguments.args[9] = NULL;

  arguments.pckg_c = 0;
  arguments.prefix = "/usr";

  /* Parse the arguments */
  argp_parse(&argp, argc, argv, 0, 0, &arguments);

  /* Print the arguments */
  printf("Package: %s\nInstall directory: %s\n", arguments.args[0], arguments.prefix);

  for (size_t i = 0; i < arguments.pckg_c; i++)
  {
    printf("%s\n", arguments.args[i]);
  }

  char *filename = "/share/workerinstall/packages.xml";
  char *path = malloc(strlen(arguments.prefix) + strlen(filename) + 1);
  strcpy(path, arguments.prefix);
  strcat(path, filename);
  xmlDocPtr doc = xmlReadFile(path, NULL, 0);

  if (doc == NULL)
  {
    printf("Error: could not parse XML file\n");
    return 1;
  }

  xmlNodePtr root = xmlDocGetRootElement(doc);

  if (root == NULL)
  {
    printf("Error: could not get root element\n");
    xmlFreeDoc(doc);
    return 1;
  }

//  xmlNodePtr packageNode = NULL;

  bool all_packages_and_dependancies_found = false;
  //bool dependancies_to_find = false;
  int current_package = 0;

  while (!all_packages_and_dependancies_found)
  {
    printf("%d\n", current_package);

    // Find the Package node with the name 'workerinstall'
    xmlNodePtr packageNode = NULL;
    for (xmlNodePtr node = root->children; node != NULL; node = node->next)
    {
      if (xmlStrcmp(node->name, (const xmlChar *)"Package") == 0)
      {
        xmlNodePtr nameNode = xmlFirstElementChild(node);
        if (nameNode != NULL && xmlStrcmp(nameNode->name, (const xmlChar *)"Name") == 0)
        {
          xmlChar *name = xmlNodeGetContent(nameNode);
          if (xmlStrcmp(name, (const xmlChar *)arguments.args[current_package]) == 0)
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
      printf("Error: could not find Package node with name '%s'\n", arguments.args[current_package]);
      xmlFreeDoc(doc);
      return 1;
    }

    // Get the Version element of the Package node
    xmlNodePtr versionNode = xmlFirstElementChild(packageNode);
    while (versionNode != NULL && xmlStrcmp(versionNode->name, (const xmlChar *)"Version") != 0)
    {
      versionNode = versionNode->next;
    }

    if (versionNode == NULL)
    {
      printf("Error: could not find Version element of Package node\n");
      xmlFreeDoc(doc);
      return 1;
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
      xmlFreeDoc(doc);
      return 1;
    }

    xmlChar *name = xmlNodeGetContent(nameNode);
    xmlChar *version = xmlNodeGetContent(versionNode);
    printf("Package name: %s\n Package version: %s\n", name, version);
    xmlFree(version);
    current_package++;

    if (current_package >= arguments.pckg_c)
    {
      printf("%d %d\n", current_package, arguments.pckg_c);
    //  all_packages_and_dependancies_found = true;
      break;
    }
  }

  xmlFreeDoc(doc);
  return 0;
}
