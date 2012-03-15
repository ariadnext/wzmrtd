/*
 * wzMRTD - An electronic passport reader library
 * Copyright (c) 2007, Johann Dantant - www.wzpass.net
 *
 * Please read LICENSE.txt for license and copyright info.
 */

#include "wzmrtd_i.h"

/* Default values */
const char *def_rdr_name    = "#0";
const char *def_raw_prefix  = "";
const char *def_out_xml     = "mrtd.xml";
const char *def_out_dir     = ".";

/* Program options */
const char *rdr_name   = NULL;
const char *rdr_option = NULL;
const char *mrz_string = NULL;
const char *out_xml    = NULL;
const char *out_dir    = NULL;
BOOL xml_sep_img = FALSE;
BOOL xml_add_unp = FALSE;
BOOL xml_add_raw = FALSE;
BOOL raw_sep_txt = FALSE;
BOOL raw_sep_img = FALSE;
const char *raw_prefix = NULL;
BOOL overwrite   = FALSE;
BOOL silent      = FALSE;
BOOL verbose     = FALSE;
BOOL debug       = FALSE;
BOOL version     = FALSE;
BOOL help        = FALSE;

void banner(int argc, char **argv);
void usage(int argc, char **argv);
int parse_args(int argc, char **argv);
static BOOL WZMRTD_LINK callback(const char *info, const char *trace, DWORD pos, DWORD max);

int main(int argc, char **argv)
{
  MRTD_CTX_ST *mrtd_ctx;
  LONG err;

  MrtdSetCallback(callback);

  /* Parse the command line args */
  if (parse_args(argc, argv))
  {
    /* Parser error */
    if (!silent)
    {   
      fprintf(stderr, "Invalid(s) option(s) specified\n");
      if (help)
      {
        usage(argc, argv);
      } else
      {
        fprintf(stderr, "Try wzmrtd -h for help\n");
      }
    }
    return EXIT_FAILURE;
  }

  if (help)
  {
    usage(argc, argv);
    return EXIT_SUCCESS;
  }

  if (version)
  {
    banner(argc, argv);
    return EXIT_SUCCESS;
  }

  /* Say what we are going to do */
  if (!silent)
  {
    banner(argc, argv);

    if (rdr_name != NULL)
      printf("Reader: %s\n", rdr_name);

    if (mrz_string != NULL)
      printf("MRZ   : %s\n", mrz_string);
  }

  /* Instanciate the wzMRTD library */
  mrtd_ctx = MrtdAllocCtx();  
  if (mrtd_ctx == NULL)   
  {
    if (!silent)
      fprintf(stderr, "Allocation failed\n");
    return EXIT_FAILURE;
  }

  /* Connect to the passport through the specified reader */
  if (!MrtdCardConnect(mrtd_ctx, rdr_name, rdr_option))
  {
    if (!silent)
    {
      fprintf(stderr, "\nConnect failed\n");
    }
    goto failed;
  }

  /* Read the passport */
  if (!MrtdReadPassport(mrtd_ctx, mrz_string))
  {
    if (!silent)
    {
      fprintf(stderr, "\nRead failed\n");
    }
    goto failed;
  }

  /* Disconnect from the card (and the reader BTW) */
  MrtdCardDisconnect(mrtd_ctx);

  /* Now export it */
  if (out_dir != NULL)
  {
    if (!MrtdSaveToFilesEx(mrtd_ctx, out_dir, raw_prefix, raw_sep_txt, raw_sep_img, overwrite))
    {
      if (!silent)
      {
        fprintf(stderr, "\nExport to Raw files failed\n");
      }
      goto failed;
    }
  }
  if (out_xml != NULL)
  {
    if (!MrtdSaveToXMLEx(mrtd_ctx, out_xml, xml_sep_img, xml_add_unp, xml_add_raw, overwrite))
    {
      if (!silent)
      {
        fprintf(stderr, "\nExport to XML failed\n");
      }
      goto failed;
    }
  }

  /* If not output specified, we send the minimal XML stream to stdout */
  if ((out_xml == NULL) && (out_dir == NULL) && !silent)
  {
    if (!MrtdSaveToXMLEx(mrtd_ctx, NULL, FALSE, FALSE, FALSE, FALSE))
      goto failed;
  }


  /* Done */
  MrtdFreeCtx(mrtd_ctx);  
  return EXIT_SUCCESS;

failed:
  err = MrtdGetLastError(mrtd_ctx);
  if (!silent && (err != MRTD_SUCCESS))
    fprintf(stderr, "%s\n", MrtdTranslateError(err));
  MrtdFreeCtx(mrtd_ctx);
  return EXIT_FAILURE;
}
  
void banner(int argc, char **argv)
{
  printf("\n");
  printf("wzMRTD %s -- A passport reading software\n", WZMRTD_VERSION);
  printf("Copyright (c) 2007, Johann Dantant - www.wzpass.net\n");
  printf("\n");
}

void usage(int argc, char **argv)
{
  fprintf(stderr, "Usage: wzmrtd [INPUT] [-z MRZ] [-x FILE] [-d DIRECTORY] [OPTIONS]\n\n");
  fprintf(stderr, "\n");
  fprintf(stderr, "Input specifier:\n");
  fprintf(stderr, "  -r #i           use PC/SC reader number i to read the passport\n");
  fprintf(stderr, "  -r \"NAME\"       use PC/SC reader named NAME to read the passport\n");
  fprintf(stderr, "  -r SPX [DEVICE] use Pro-Active SpringProx reader to read the passport\n");
  fprintf(stderr, "                  if DEVICE is specified, it is specified to SpringProx\n");
  fprintf(stderr, "                  library as the communication device to be used\n");
  fprintf(stderr, "\n");
  fprintf(stderr, "MRZ:\n");
  fprintf(stderr, "  -z \"MRZ\"        second line of the machine readable zone of the\n");
  fprintf(stderr, "                  document. It is needed for basic authentication.\n");
  fprintf(stderr, "\n");
  fprintf(stderr, "Output specifier:\n");
  fprintf(stderr, "  -f FILE         passport content is saved in one single XML file\n");
  fprintf(stderr, "                  XML output options are enabled\n");
  fprintf(stderr, "  -t DIRECTORY    passport content is saved as binary files (one file\n");
  fprintf(stderr, "                  for each DG)\n");
  fprintf(stderr, "                  Raw output options are enabled\n");
  fprintf(stderr, "\n");
  fprintf(stderr, "XML output options:\n");
  fprintf(stderr, "  -xsi            Create separe files for images, instead of putting\n");
  fprintf(stderr, "                  them into the XML file itself\n");
// Not yet implemented by the library :
//  fprintf(stderr, "  -xu             Keep unprocessed content in the XML file instead of\n");
//  fprintf(stderr, "                  discarding it\n");
//  fprintf(stderr, "  -xr             Add a raw output of each DG in the XML file\n");
  fprintf(stderr, "\n");
  fprintf(stderr, "Raw output options:\n");
// Not yet implemented by the library :
//  fprintf(stderr, "  -rst            Add separate text files whenever possible\n");
//  fprintf(stderr, "  -rsi            Add separate images files whenever possible\n");
  fprintf(stderr, "  -pfx NAME       Use NAME as prefix for naming the output files\n");
  fprintf(stderr, "\n");
  fprintf(stderr, "Miscellaneous options:\n");
  fprintf(stderr, "  -y              Overwrite already existing files\n");
  fprintf(stderr, "  -s(ilent)       Suppress startup banner and any kind of output\n");
  fprintf(stderr, "  -v(erbose)      Show details on operation being performed\n");
  fprintf(stderr, "  -d(ebug)        Enable debug output (to stderr)\n");
  fprintf(stderr, "  -Version        Show version, and exits right now\n");
  fprintf(stderr, "\n");
}

int parse_args(int argc, char **argv)
{
  int i;
  int failed = 0;

  for (i=1; i<argc; i++)
  {
    if (!strcmp(argv[i], "-r"))
    {
      /* Name of the reader */
      if ((i<argc-1) && (argv[i+1][0] != '-'))
        rdr_name = argv[++i];
      else
        failed++;

      /* If there an option ? */
      if ((i<argc-1) && (argv[i+1][0] != '-'))
        rdr_option = argv[++i];
      continue;
    }
   
    if (!strcmp(argv[i], "-z") || !strcmp(argv[i], "-mrz"))
    {
      /* MRZ string */
      if ((i<argc-1) && (argv[i+1][0] != '-'))       
        mrz_string = argv[++i];
      else
        failed++;
      continue;
    }
    
    if (!strcmp(argv[i], "-f") || !strcmp(argv[i], "-xml"))
    {
      /* XML file name */
      if ((i<argc-1) && (argv[i+1][0] != '-'))
        out_xml = argv[++i];
      else
        out_xml = def_out_xml;
      continue;
    }

    if (!strcmp(argv[i], "-t") || !strcmp(argv[i], "-dir"))
    {
      /* Output directory */
      if ((i<argc-1) && (argv[i+1][0] != '-'))       
        out_dir = argv[++i];
      else
        out_dir = def_out_dir;
      continue;
    }

    if (!strcmp(argv[i], "-xsi"))
    {  
      xml_sep_img = TRUE;
      continue;
    }

    if (!strcmp(argv[i], "-xu"))
    {
      xml_add_unp = TRUE;
      continue;
    }
    
    if (!strcmp(argv[i], "-xr"))
    { 
      xml_add_raw = TRUE;
      continue;
    }

    if (!strcmp(argv[i], "-rst"))
    {
      raw_sep_txt = TRUE;
      continue;
    }
    
    if (!strcmp(argv[i], "-rsi"))
    {
      raw_sep_img = TRUE;
      continue;
    }
    
    if (!strcmp(argv[i], "-pfx") || !strcmp(argv[i], "-prefix"))
    {
      /* Prefix string */
      if ((i<argc-1) && (argv[i+1][0] != '-'))       
        raw_prefix = argv[++i];
      else
        failed++;
      continue;
    }
    
    if (!strncmp(argv[i], "-y", 2))
    {
      overwrite = TRUE;
      continue;
    }
    
    if (!strncmp(argv[i], "-s", 2))
    {
      silent = TRUE;
      continue;
    }
    
    if (!strncmp(argv[i], "-v", 2))
    {
      verbose = TRUE;
      continue;
    }
    
    if (!strncmp(argv[i], "-d", 2))
    {
      debug = TRUE;
      continue;
    }
    
    if (!strncmp(argv[i], "-V", 2))
    {
      version = TRUE;
      continue;
    }
    
    if (!strncmp(argv[i], "-h", 2) || !strcmp(argv[i], "-?"))
    {
      help = TRUE;
      continue;
    }
    
    failed++;
  }

  if (rdr_name == NULL)
  {
    /* Use PC/SC reader #0 as default */
    rdr_name = def_rdr_name;
  }

  /* Add default where needed */
  if (out_dir != NULL)
  {
    if (raw_prefix == NULL)
      raw_prefix = def_raw_prefix;
  }

  return failed;
}

static BOOL WZMRTD_LINK callback(const char *info, const char *trace, DWORD pos, DWORD max)
{
  if ((info != NULL) && (!silent))
  {
    fprintf(stdout, "\n%s", info);
  }

  if ((trace != NULL) && (debug))
  {
    fprintf(stderr, "\n%s", trace);
  }

  if ((pos != 0xFFFFFFFF) && (max != 0xFFFFFFFF) && (!silent))
  { 
    fprintf(stdout, "%05d/%05d\b\b\b\b\b\b\b\b\b\b\b", pos, max);
  }

  return TRUE;
}
