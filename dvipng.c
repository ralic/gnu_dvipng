/* dvipng.c */
/**********************************************************************
 ****************************  Intro  *********************************
 **********************************************************************
 * This program translates TeX's DVI-Code into Portable Network Graphics.
 *
 * Copyright (C) Jan-�ke Larsson 2002-2003
 *
 * Recipe:
 * Take a simple dvi2?? converter, dvilj was found suitable.
 * Read and relish the code of xdvi and dvips
 * Choose a png-drawing library with a simple interface and support
 *  on many platforms: gd
 * Stir, sprinkle with some insights of your own, and enjoy
 *
 **********************************************************************
 */

#define MAIN
#include "dvipng.h"

/**********************************************************************/
/*******************************  main  *******************************/
/**********************************************************************/
int main(int argc, char ** argv)
{
  bool parsestdin;
  
#ifdef TIMING
# ifdef HAVE_GETTIMEOFDAY
  gettimeofday(&Tp, NULL);
  timer = Tp.tv_sec + ((float)(Tp.tv_usec))/ 1000000.0;
# else
#  ifdef HAVE_FTIME
  ftime(&timebuffer);
  timer = timebuffer.time + (float)(timebuffer.millitm) / 1000.0;
#  endif
# endif
#endif
  //  setbuf(stderr, NULL);

  initcolor();
  parsestdin = DecodeArgs(argc, argv);

#ifdef HAVE_LIBKPATHSEA
  /* Use extra paths as used by dvips */
  kpse_set_program_name(argv[0],"dvips");
  /* If dvipng is not installed in the texmf tree, and SELFAUTO...
   * is used in texmf.cnf, kpathsea will not find a) Virtual fonts
   * b) ps2pk.map or psfonts.map c) PostScript fonts
   *
   * We adjust these things here
   */
# ifdef ENV_SELFAUTOLOC
  putenv(ENV_SELFAUTOLOC);
# endif
# ifdef ENV_SELFAUTODIR
  putenv(ENV_SELFAUTODIR);
# endif
# ifdef ENV_SELFAUTOPARENT
  putenv(ENV_SELFAUTOPARENT);
# endif
  kpse_set_program_enabled (kpse_pk_format, makeTexPK, kpse_src_compile);
  kpse_init_prog("DVIPNG", resolution, MFMODE, "cmr10");
#endif

#ifdef HAVE_FT2
  if (FT_Init_FreeType( &libfreetype ))
    Fatal("an error occured during freetype initialisation"); 
# ifdef DEBUG
  {
    FT_Int      amajor, aminor, apatch;
    
    FT_Library_Version( libfreetype, &amajor, &aminor, &apatch );
    DEBUG_PRINT((DEBUG_FT,"\n  FREETYPE VERSION: FreeType %d.%d.%d", 
		 amajor, aminor, apatch));
  }
# endif
  InitPSFontMap();
#endif

  DrawPages();

  if (parsestdin) {
    char    line[STRSIZE];

    printf("%s> ",dvi!=NULL?dvi->name:"");
    fgets(line,STRSIZE,stdin);
    while(!feof(stdin)) {
      DecodeString(line);
      if (dvi!=NULL) {
	DVIReOpen(dvi);
	DrawPages();
      }
      printf("%s> ",dvi!=NULL?dvi->name:"");
      fgets(line,STRSIZE,stdin);
    }
    printf("\n");
  }

#ifdef TIMING
# ifdef HAVE_GETTIMEOFDAY
  gettimeofday(&Tp, NULL);
  timer = (Tp.tv_sec + (float)(Tp.tv_usec)/1000000.0) - timer;
# else
#  ifdef HAVE_FTIME
  ftime(&timebuffer);
  timer = (timebuffer.time + (float)(timebuffer.millitm)/1000.0) - timer;
#  endif
# endif
  
  if (ndone > 0) 
    fprintf(stderr,
	    "Time of complete run: %.2f s, %d page(s), %.4f s/page.\n",
	    timer, ndone, timer / ndone);
  if (my_toc >= 0.0001) 
    fprintf(stderr,
	    "Thereof in TIC/TOC region %.5f s.\n",my_toc);
#endif

  ClearFonts();
#ifdef HAVE_FT2
  if (FT_Done_FreeType(libfreetype)) 
    Fatal("an error occured during freetype destruction"); 
#endif  

  exit(EXIT_SUCCESS);
}



