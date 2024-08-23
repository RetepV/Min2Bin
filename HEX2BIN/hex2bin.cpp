#include <stdio.h>
#include "stdlib.h"
#include "string.h"


#define FALSE    0
#define TRUE    !FALSE


typedef int    BOOL;


char  szStartupPath[1024];
char  szInputFile[1024];
char  szOutputFile[1024];
BOOL  bHex2Bin = TRUE;
BOOL  bFill = FALSE;
BOOL  bHelp = FALSE;
char  chFillChar = '\0';


void printhelp( void )
{
  printf( "\nHelp:\n\n" );
  printf( "  This program converts binary rom image files from\n" );
  printf( "  binary to hexdump format.\n" );
  printf( "Run as:\n\n" );
  printf( "  hex2bin [/<options>] <inputfile> <outputfile>\n\n" );
  printf( "Options:\n\n" );
  printf( "  /?    Show this help.\n" );
  printf( "  /[hH]    Hexdunp format to binary (default).\n" );
  printf( "  /[bB]    Binary to hexdump format.\n\n" );
  printf( "  /[fF]    Fill remaining space in binary with 00's\n      or some other given value (use /c XX).\n" );
  printf( "  /[cC] XX  Give a different value for filling.\n      The value is a HEX number.\n" );
}


int normalizestring( char *pszString )
{
  char  *pszWalk;
  char  *pszInsert;
  BOOL  bInARun = FALSE;

  if( pszString == NULL )
  {
    return( -1 );
  }
  
  pszWalk = pszString;
  pszInsert = pszString;
  
  while( *pszWalk != '\0' )
  {
    if( bInARun )
    {
      switch( *pszWalk )
      {
        case ' ':
        case '\t':
        case '\r':
        case '\n':
        {
          pszWalk++;
          break;
        }
        default:
        {
          bInARun = FALSE;
          *pszInsert++ = *pszWalk++;
        }
      }
    }
    else
    {
      switch( *pszWalk )
      {
        case ' ':
        case '\t':
        case '\r':
        case '\n':
        {
          bInARun = TRUE;
          *pszInsert++ = ' ';
          pszWalk++;
          break;
        }
        default:
        {
          *pszInsert++ = *pszWalk++;
          break;
        }
      }
    }
  }

  *pszInsert = 0;
  
  return( 0 );
}    


int  parsecmdline( int nCmdLineCnt, char *pArgs[] )
{
    int    nCnt;
    int    nFixedParam;
    char  chReadValue = '\0';
    
    if( nCmdLineCnt == 0 )
    {
      return( -1 );
    }
    if( pArgs == NULL )
    {
      return( -2 );
    }
    
    nFixedParam = 0;
    
    for( nCnt = 0; nCnt < nCmdLineCnt; nCnt++ )
    {
      char szDummy[1024];
      
      strcpy( szDummy, pArgs[nCnt] );
      
      if( chReadValue != '\0' )
      {
        switch( chReadValue )
        {
          case 'C':
          {
            sscanf( szDummy, "%02x", &chFillChar );
            chReadValue = '\0';
            break;
          }
        }
      }
      else
      {
        switch( szDummy[0] )
        {
          case '/':
          case '-':
          {
            char  chCode = szDummy[1];
            
            switch( chCode )
            {
              case '?':
              {
                bHelp = TRUE;
                break;
              }
              case 'f':
              case 'F':
              {
                bFill = TRUE;
                break;
              }
              case 'c':
              case 'C':
              {
                chReadValue = 'C';
                break;
              }
              case 'h':
              case 'H':
              {
                bHex2Bin = TRUE;
                break;
              }
              case 'b':
              case 'B':
              {
                bHex2Bin = FALSE;
                break;
              }
            }
            
            break;
          }
          default:
          {
            switch( nFixedParam )
            {
              case 0:
              {
                strcpy( szStartupPath, szDummy );
                break;
              }
              case 1:
              {
                strcpy( szInputFile, szDummy );
                break;
              }
              case 2:
              {
                strcpy( szOutputFile, szDummy );
                break;
               }
          }
          
          nFixedParam++;
          break;
          }
        }
      }
    }
    
    return( 0 );
}


// Watch out with the ucBytes array. Sscanf seems to read %02X as an integer. Therefore,
// instead of just filling 1 byte for each entry, it actually fills 4. And for each
// following variable it reads 4 bytes again, but overwrites 3 bytes of the previous
// array entry.
//
// So, the line '#0000 11 22 33 44 55 66 77 88 99 aa bb cc dd ee ff' gets parsed into
// the array by sscanf like so:
//
// 11 00 00 00
// 11 22 00 00 00
// 11 22 33 00 00 00
//
// Now, we only read 16 bytes per line. But if the array is 16 bytes long, for bytes
// 13, 14 and 15 will overwite memory. So we actually need to make the array 3 bytes
// longer than we need.
//
// This works, although it is probably one of the most ugly hacks I have ever made :D.
//
int convert_from_hex( void )
{
  FILE      *pInputFile;
  FILE      *pOutputFile;
  char      szAddress[6];
  char      szInputLine[1024];
  BOOL      bStartFound = FALSE;
  unsigned int  uiAddress;
  unsigned char  ucBytes[19];
  int        nCnt;
  int        nBytesRead;
  
  if(( pInputFile = fopen( szInputFile, "r" )) == NULL )
  {
    printf( "\n*** The inputfile '%s' couldn't be found.\n%c", szInputFile, 0x07 );
    return( -1 );
  }
  
  if(( pOutputFile = fopen( szOutputFile, "r" )) != NULL )
  {
    char  chDummy;
    
    fclose( pOutputFile );
     printf( "\n*** The outputfile '%s' already exists. Overwrite? (y/n)?%c ", szOutputFile, 0x07 );
     chDummy = getchar();
     if(( chDummy == 'n' ) || ( chDummy == 'N' ))
     {
       return( -2 );
     }
  }
    
  if(( pOutputFile = fopen( szOutputFile, "wb" )) == NULL )
  {
    printf( "\n*** The outputfile '%s' could not be opened for (over)writing.\n%c", pOutputFile, 0x07 );
    return( -1 );
  }
  
  printf( "Analyzing\n" );
  
  printf( "\nConverting\n" );
  
  while( ! feof( pInputFile ))
  {       
    // One line:
    // XXXX: XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX
    if( fgets( szInputLine, 1024, pInputFile ) != NULL )
    {
      printf( "READ: %s", szInputLine );
      normalizestring( szInputLine );
      printf( "      %s\n", szInputLine );

      // Each line must be at least 5 chars long. If not, skip.
      if( strlen( szInputLine ) < 5 )
      {
        printf( "  => SKIP (length)\n" );
        continue;
      }
      // Each line should begin with '#' If not, then the last line has been found.
      // Character 4 from the inputline needs to be a ':'. If not, skip.
      if( szInputLine[4] != ':' )
      {
        // Skip this line.
        printf( "  => SKIP (colon)\n" );
        continue;
      }         
      nCnt = sscanf( szInputLine, "%04X: %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X",
              &uiAddress,
              (unsigned char *)&ucBytes[0],
              (unsigned char *)&ucBytes[1],
              (unsigned char *)&ucBytes[2],
              (unsigned char *)&ucBytes[3],
              (unsigned char *)&ucBytes[4],
              (unsigned char *)&ucBytes[5],
              (unsigned char *)&ucBytes[6],
              (unsigned char *)&ucBytes[7],
              (unsigned char *)&ucBytes[8],
              (unsigned char *)&ucBytes[9],
              (unsigned char *)&ucBytes[10],
              (unsigned char *)&ucBytes[11],
              (unsigned char *)&ucBytes[12],
              (unsigned char *)&ucBytes[13],
              (unsigned char *)&ucBytes[14],
              (unsigned char *)&ucBytes[15] );
      printf( "\r  %04X", uiAddress );
    
      printf( "  FIELDS: %d\n", nCnt );

      fwrite( ucBytes, 1, nCnt - 1, pOutputFile );

      strnset( szInputLine, '\0', 1024 );
    }
  }
  
  fclose( pOutputFile );
  fclose( pInputFile );
  
  printf( "\nReady\n\n" );

  return( 0 );
}


int convert_to_hex( void )
{  
  FILE      *pInputFile;
  FILE      *pOutputFile;
  char      szHex[10];
  char      szOutputLine[1024];
  BOOL      bStartFound = FALSE;
  BOOL      bHashFound = FALSE;
  unsigned char  ucBytes[16];
  int       nCnt;
  int       nBytesRead;
  int       nOffset;
  int       nFileLen;
  char      szHeader[] = "\x0A\x12\x5B";
  char      szFooter[] = "\x5D\x0A\x14";
  
  if(( pInputFile = fopen( szInputFile, "rb" )) == NULL )
  {
    printf( "\n*** The inputfile '%s' could not be found.\n%c", szInputFile, 0x07 );
    return( -1 );
  }
  
  if(( pOutputFile = fopen( szOutputFile, "r" )) != NULL )
  {
    char  chDummy;
    
    fclose( pOutputFile );
     printf( "\n*** The outputfile '%s' already exists. Overwrite (y/n)? %c ", szOutputFile, 0x07 );
     chDummy = getchar();
     if(( chDummy == 'n' ) || ( chDummy == 'N' ))
     {
       return( -2 );
     }
  }
    
  if(( pOutputFile = fopen( szOutputFile, "w" )) == NULL )
  {
    printf( "\n*** The outputfile '%s' could not be found.\n%c", pOutputFile, 0x07 );
    return( -1 );
  }

  fseek( pInputFile, 0, SEEK_END );
  nFileLen = ftell( pInputFile);
  fseek( pInputFile, 0, SEEK_SET );

  nOffset = 0;

  printf( "Header\n" );
  
  fprintf( pOutputFile, "HEXDUMP: %04X, %04X\n", nOffset, nFileLen );
  fprintf( pOutputFile, "      0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F" );
  
  printf( "Converting\n" );
  
  while( ! feof( pInputFile ))
  {
    nBytesRead = fread( ucBytes, 1, 16, pInputFile );
    
    if(( nBytesRead == 0 ) && ( feof( pInputFile )))
    {
      break;
    }
    else
    {
      printf( "\r  %04X", nOffset );   
      printf( "  %d", nBytesRead );
      
      strcpy( szOutputLine, "" );
      
      if( nOffset != 0 )
      {
        strcat( szOutputLine, "\n" );
      }

      sprintf( szHex, "%04X:", nOffset );
      strcat( szOutputLine, szHex );

      for( nCnt = 0; nCnt < nBytesRead; nCnt++ )
      {
        sprintf( szHex, " %02X", ucBytes[nCnt] );
        strcat( szOutputLine, szHex );
      }
      strcat( szOutputLine, "\n" );

      fwrite( szOutputLine, strlen( szOutputLine ), 1, pOutputFile );
      
      nOffset += nBytesRead;
    }
  }
  
  fclose( pOutputFile );
  fclose( pInputFile );
      
  printf( "\nReady\n\n" );

  return( 0 );
}


int main( int argc, char *argv[] )
{
  printf( "\nhex2bin v1.0 Copyright (c) 2013 P. de Vroomen\n" );

  parsecmdline( argc, argv );
  
  if( bHelp == TRUE )
  {
      printhelp();
  }
  else
  {
    if( strcmp( szInputFile, "" ) == 0 )
    {
      printf( "\n*** No inputfile given.\n%c", 0x07 );
      printhelp();
    }
    else if( strcmp( szOutputFile, "" ) == 0 )
    {
      printf( "\n*** No ouputfile given.\n%c", 0x07 );
      printhelp();
    }
    else if( bHex2Bin )
    {
      convert_from_hex();
    }
    else if( ! bHex2Bin )
    {
      convert_to_hex();
    }
  }

  return( 0 );
}