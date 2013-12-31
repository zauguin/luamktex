/*
getdestdir.c
from mktexpk:
  argv[0] = "Dummy", argv[1] = "pk", argv[2] = path, argv[3] = mode
from mktextfm:
  argv[0] = "Dummy", argv[1] = "tfm", argv[2] = path
*/

#include <kpathsea/kpathsea.h>

#include "dirutil.h"
#include "getdestdir.h"

#define NUMBUF   32
#define LENBUF   128
#define MPATH    256
#define FTOP     "fonts"

/* error message */
static void
fatal (const char *s)
{
  fprintf (stderr, "%s\n", s);
}

char *
getdestdir (int ac, char **av)
{
  static char buff[MPATH];
  char *pb[NUMBUF];
  char *p, *q;
  char spec[32];
  int i, Num = 0, ispk, k;
  char *topdir;

  ispk = 0;

  for (i = 0; i < NUMBUF; i++) {
    if (!(pb[i] = (char *) malloc (LENBUF))) {
      fatal ("Memory allocation error.");
      return (NULL);
    }
  }

  if (ac != 3 && ac != 4) {
    fatal ("Argument error.");
    for (k = 0; k < NUMBUF; k++)
      free (pb[k]);
    return (NULL);
  }

  strcpy (spec, av[1]);

  for (p = av[2]; *p; p++) {    /* path */
    if (*p == '\\')
      *p = '/';
    else if (IS_KANJI(p))
      p++;
  }

  p = av[2];
  q = buff;

/* UNC name support */

  if (p[0] == '/' && p[1] == '/') {
    *q++ = *p++;
    *q++ = *p++;
  }

  while (*p) {
    if (*p == '/') {
      *q = *p;
      while (*p == '/')
        p++;
      p--;
    } else {
      *q = *p;
    }
    p++;
    q++;
  }
  *q = '\0';                    /* now path name of ${name}.mf is in buff. */
/*
#ifdef TEST
  return xstrdup(buff);
#endif
*/
  if (ac == 4)
    ispk = 1;                   /* called from mktexpk */

  if (!(p = strrchr (buff, '/'))) {
    fatal ("Invalid path name.");
    for (k = 0; k < NUMBUF; k++)
      free (pb[k]);
    return (NULL);
  }

  *p = '\0';                    /* get directory name */

  for (i = 0; i < NUMBUF; i++) {
    if (!(p = strrchr (buff, '/'))) {
      fatal ("Invalid path name.");
      for (k = 0; k < NUMBUF; k++)
        free (pb[k]);
      return (NULL);
    }
    *p = '\0';
    p++;
    strcpy (pb[i], p);
    if (!stricmp (pb[i], FTOP)) {
      Num = i;
      break;
    }
  }

  Num -= 2;
  if (Num < 0) {
    fprintf (stderr, "Font resources should be under a directory ");
    fprintf (stderr, "with the name \"fonts\".\n");
    fprintf (stderr, "Furthermore, there must be at least two directories ");
    fprintf (stderr, "under the directory \"fonts\".\n");
    fatal ("Invalid path name.");
    for (k = 0; k < NUMBUF; k++)
      free (pb[k]);
    return (NULL);
  }

  topdir = kpse_var_value ("MAKETEXPK_TOP_DIR");
  if (topdir && *topdir && ispk) {
    for (i = 0; topdir[i]; i++) {
      if (topdir[i] == '\\')
        topdir[i] = '/';
      else if (IS_KANJI(topdir+i))
        i++;
    }
    i = strlen (topdir);
    while(topdir[i - 1] == '/')
      i--;
    topdir[i] = '\0';

    if(!is_dir(topdir)) {
      if(make_dir_p(topdir)) {
        fprintf(stderr, "Failed to access %s.\n", topdir);
        return NULL;
      }
    }
#ifdef TEST
    printf ("%s\n", topdir);
#endif
    if (strnicmp (&topdir[i - 3], "/pk", 3) != 0) {
      strcat (topdir, "/pk");
      if (!is_dir(topdir)) {
        if (make_dir(topdir)) {
          fprintf(stderr, "Faild to access %s.\n", topdir);
          for (k = 0; k < NUMBUF; k++)
            free (pb[k]);
          return (NULL);
        }
      }
    }
    strcpy (buff, topdir);
    free(topdir);
  } else {
    if((topdir = kpse_var_value("TEXMFVAR")) != NULL) {
      for (i = 0; topdir[i]; i++) {
	if (topdir[i] == '\\')
	  topdir[i] = '/';
        else if (IS_KANJI(topdir+i))
          i++;
      }
      i = strlen (topdir);
      while(topdir[i - 1] == '/')
        i--;
      topdir[i] = '\0';

      if(!is_dir(topdir)) {
        if(make_dir_p(topdir)) {
          fprintf(stderr, "Failed to access %s.\n", topdir);
          return NULL;
        }
      }

      strcpy(buff, topdir);
      free(topdir);
    }

    strcat (buff, "/fonts");
    if (!is_dir (buff)) {
      if (make_dir (buff)) {
        for (k = 0; k < NUMBUF; k++)
          free (pb[k]);
        return (NULL);
      }
    }
    strcat (buff, "/");
    strcat (buff, spec);
    if (!is_dir (buff)) {
      if (make_dir (buff)) {
        for (k = 0; k < NUMBUF; k++)
          free (pb[k]);
        return (NULL);
      }
    }
  }

  if (ispk) {
    strcat (buff, "/");
    strcat (buff, av[3]);
    if (!is_dir (buff)) {
      if (make_dir (buff)) {
        for (k = 0; k < NUMBUF; k++)
          free (pb[k]);
        return (NULL);
      }
    }
  }

  for (i = Num; i > -1; i--) {
    strcat (buff, "/");
    strcat (buff, pb[i]);
    if (is_dir (buff))
      continue;
    else {
      if (make_dir (buff)) {
        for (k = 0; k < NUMBUF; k++)
          free (pb[k]);
        return (NULL);
      }
    }
  }

  for (k = 0; k < NUMBUF; k++)
    free (pb[k]);

  p = xstrdup(buff);
  return p;
}

#ifdef TEST
int
main (int ac, char *av[])
{
  kpse_set_program_name (av[0], "getdestdir");
  printf ("%s", getdestdir (ac, av));
  return 0;
}
#endif
