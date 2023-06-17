#include <argp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include <libxml/tree.h>
#include <libxml/parser.h>

#include <curl/curl.h>

const char *argp_program_version = "0.1";
const char *argp_program_bug_address = "<workerinstall@doomlinux.org>";

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
    {"update", 'U', 0, 0, "Update the worker packagelist."},
    //{"version", 'v', 0, 0, "Print version information."},
    {0}};

/* Used by main to communicate with parse_opt. */
struct arguments
{
  char *args[10]; /* arg1 */
  int pckg_c, no_pkgs;
  int install, upgrade, purge, version, update;
  char *prefix;
};

bool iterate_package_parser(struct arguments arguments);
bool update_pkglist(struct arguments arguments);
void parse_conf(struct arguments arguments);

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
  case 'U':
    arguments->update = 1;
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
      arguments->no_pkgs = 1;
      //argp_usage(state);
    break;

  default:
    return ARGP_ERR_UNKNOWN;
  }
  return 0;
}

/* workerinstall configuration */
char MAKEOPTS[255] = {0};
char UPDATE_URL[255] = {0};

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

  arguments.no_pkgs = 0;
  arguments.pckg_c = 0;
  arguments.update = 0;
  arguments.upgrade = 0;
  arguments.version = 0;
  arguments.prefix = "/";

  /* Parse the arguments */
  argp_parse(&argp, argc, argv, 0, 0, &arguments);

  if (arguments.update == 1)
  {
    printf("Beginning fetch for information: %s/etc/workerinstall/worker.conf\n", arguments.prefix);
    update_pkglist(arguments);
    return;
  }

  if(arguments.no_pkgs) { printf("hey bro you wanna run workerinstall --usage\n"); return 0; }

  /* Print the arguments */
  printf("Installation prefix: %s\n", arguments.prefix);

  /*for (size_t i = 0; i < arguments.pckg_c; i++)
  {
    printf("%s\n", arguments.args[i]);
  }*/

  parse_conf(arguments);

  iterate_package_parser(arguments);

  return 0;
}

bool iterate_package_parser(struct arguments arguments) {
  char *filename = "usr/share/workerinstall/packages.xml";
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
  // bool dependancies_to_find = false;
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
}
 
static size_t write_data(void *ptr, size_t size, size_t nmemb, void *stream)
{
  size_t written = fwrite(ptr, size, nmemb, (FILE *)stream);
  return written;
}

bool update_pkglist(struct arguments arguments) {
  parse_conf(arguments);

  //printf("Fetching UPDATE_URL %s\n", UPDATE_URL);

  printf("Beginning CURL %s\n", UPDATE_URL);

  CURL *curl_handle;
  //static const char *pagefilename = "packages.xml";
  FILE *pagefile;

  curl_global_init(CURL_GLOBAL_ALL);
 
  /* init the curl session */
  curl_handle = curl_easy_init();
 
  /* set URL to get here */
  curl_easy_setopt(curl_handle, CURLOPT_URL, UPDATE_URL);
 
  /* Switch on full protocol/debug output while testing */
  curl_easy_setopt(curl_handle, CURLOPT_VERBOSE, 1L);
 
  /* disable progress meter, set to 0L to enable it */
  curl_easy_setopt(curl_handle, CURLOPT_NOPROGRESS, 1L);
 
  /* send all data to this function  */
  curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, write_data);
 
  /* open the file */
  char *pagefilename = "usr/share/workerinstall/packages_replace.xml";
  char *conf_path = malloc(strlen(arguments.prefix) + strlen(pagefilename) + 1);
  strcpy(conf_path, arguments.prefix);
  strcat(conf_path, pagefilename);
  pagefile = fopen(conf_path, "wb+");
  if(pagefile) {
 
    /* write the page body to this file handle */
    curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, pagefile);
 
    /* get it! */
    curl_easy_perform(curl_handle);
 
    /* close the header file */
    fclose(pagefile);
  }
 
  /* cleanup curl stuff */
  curl_easy_cleanup(curl_handle);
 
  curl_global_cleanup();
 
  free(conf_path);

  return false;

}

void parse_conf(struct arguments arguments)
{
  char *conf_fn = "etc/workerinstall/worker.conf";
  char *conf_path = malloc(strlen(arguments.prefix) + strlen(conf_fn) + 1);
  strcpy(conf_path, arguments.prefix);
  strcat(conf_path, conf_fn);

  FILE *fp = fopen(conf_path, "r");
  if (fp == NULL)
  {
    return;
  }

  char line[1024] = {0};
  while (!feof(fp))
  {
    memset(line, 0, 1024);
    fgets(line, 1024, fp);
    if (line[0] == '#')
    {
      continue;
    }

    int len = strlen(line);
    char *pos = strchr(line, '=');
    if (pos == NULL)
    {
      continue;
    }
    char key[255] = {0};
    char val[255] = {0};

    int offset = 1;
    if (line[len - 1] == '\n')
    {
      offset = 2;
    }

    strncpy(key, line, pos - line);
    strncpy(val, pos + 1, line + len - offset - pos);

    // What the actual fucking hell man, I set UPDATE_URL when key matches MAKEOPTS and vice versa
    // Because this shit somehow fucks it all up when I have it assign MAKEOPTS to val when key is MAKEOPTS.
    // HOW THE FUCK DO WE CURL -j4 ???
    char MO_C[255] = {0};
    strncpy(MO_C, "MAKEOPTS", 9);
    char UU_C[255] = {0};
    strncpy(UU_C, "UPDATE_URL", 11);
    if(strcmp(MO_C, key)) {
      printf("Setting UPDATE_URL to %s\n", val);
      strcpy(&UPDATE_URL, val);
    }
    if(strcmp(UU_C, key)) {
      printf("Setting MAKEOPTS to %s\n", val);
      strcpy(&MAKEOPTS, val);
    }

    printf("\"%s\" -> \"%s\"\n", key, val);
  }

  free(conf_path);
}