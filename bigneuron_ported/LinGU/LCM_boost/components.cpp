/*
 * components.cpp
 *
 *  Created on: May 20, 2015
 *      Author: gulin
 */


# include <cstdlib>
# include <iostream>
# include <iomanip>
# include <fstream>
# include <ctime>
# include <cmath>
# include <cstring>

using namespace std;

# include "components.hpp"

//****************************************************************************80

int file_column_count ( string filename )

//****************************************************************************80
//
//  Purpose:
//
//    FILE_COLUMN_COUNT counts the columns in the first line of a file.
//
//  Discussion:
//
//    The file is assumed to be a simple text file.
//
//    Most lines of the file are presumed to consist of COLUMN_NUM words,
//    separated by spaces.  There may also be some blank lines, and some
//    comment lines, which have a "#" in column 1.
//
//    The routine tries to find the first non-comment non-blank line and
//    counts the number of words in that line.
//
//    If all lines are blanks or comments, it goes back and tries to analyze
//    a comment line.
//
//  Licensing:
//
//    This code is distributed under the GNU LGPL license.
//
//  Modified:
//
//    05 July 2009
//
//  Author:
//
//    John Burkardt
//
//  Parameters:
//
//    Input, string FILENAME, the name of the file.
//
//    Output, int FILE_COLUMN_COUNT, the number of columns assumed
//    to be in the file.
//
{
  int column_num;
  ifstream input;
  bool got_one;
  string text;
//
//  Open the file.
//
  input.open ( filename.c_str ( ) );

  if ( !input )
  {
    column_num = -1;
    cerr << "\n";
    cerr << "FILE_COLUMN_COUNT - Fatal error!\n";
    cerr << "  Could not open the file:\n";
    cerr << "  \"" << filename << "\"\n";
    exit ( 1 );
  }
//
//  Read one line, but skip blank lines and comment lines.
//
  got_one = false;

  for ( ; ; )
  {
    getline ( input, text );

    if ( input.eof ( ) )
    {
      break;
    }

    if ( s_len_trim ( text ) <= 0 )
    {
      continue;
    }

    if ( text[0] == '#' )
    {
      continue;
    }
    got_one = true;
    break;
  }

  if ( !got_one )
  {
    input.close ( );

    input.open ( filename.c_str ( ) );

    for ( ; ; )
    {
      input >> text;

      if ( input.eof ( ) )
      {
        break;
      }

      if ( s_len_trim ( text ) == 0 )
      {
        continue;
      }
      got_one = true;
      break;
    }
  }

  input.close ( );

  if ( !got_one )
  {
    cerr << "\n";
    cerr << "FILE_COLUMN_COUNT - Warning!\n";
    cerr << "  The file does not seem to contain any data.\n";
    return -1;
  }

  column_num = s_word_count ( text );

  return column_num;
}
//****************************************************************************80

int file_row_count ( string input_filename )

//****************************************************************************80
//
//  Purpose:
//
//    FILE_ROW_COUNT counts the number of row records in a file.
//
//  Discussion:
//
//    It does not count lines that are blank, or that begin with a
//    comment symbol '#'.
//
//  Licensing:
//
//    This code is distributed under the GNU LGPL license.
//
//  Modified:
//
//    23 February 2009
//
//  Author:
//
//    John Burkardt
//
//  Parameters:
//
//    Input, string INPUT_FILENAME, the name of the input file.
//
//    Output, int FILE_ROW_COUNT, the number of rows found.
//
{
  //int bad_num;
  int comment_num;
  ifstream input;
//  int i;
  string line;
  int record_num;
  int row_num;

  row_num = 0;
  comment_num = 0;
  record_num = 0;
  //bad_num = 0;

  input.open ( input_filename.c_str ( ) );

  if ( !input )
  {
    cerr << "\n";
    cerr << "FILE_ROW_COUNT - Fatal error!\n";
    cerr << "  Could not open the input file: \"" << input_filename << "\"\n";
    exit ( 1 );
  }

  for ( ; ; )
  {
    getline ( input, line );

    if ( input.eof ( ) )
    {
      break;
    }

    record_num = record_num + 1;

    if ( line[0] == '#' )
    {
      comment_num = comment_num + 1;
      continue;
    }

    if ( s_len_trim ( line ) == 0 )
    {
      comment_num = comment_num + 1;
      continue;
    }

    row_num = row_num + 1;

  }

  input.close ( );

  return row_num;
}
//****************************************************************************80

int i4_min ( int i1, int i2 )

//****************************************************************************80
//
//  Purpose:
//
//    I4_MIN returns the minimum of two I4's.
//
//  Licensing:
//
//    This code is distributed under the GNU LGPL license.
//
//  Modified:
//
//    13 October 1998
//
//  Author:
//
//    John Burkardt
//
//  Parameters:
//
//    Input, int I1, I2, two integers to be compared.
//
//    Output, int I4_MIN, the smaller of I1 and I2.
//
{
  int value;

  if ( i1 < i2 )
  {
    value = i1;
  }
  else
  {
    value = i2;
  }
  return value;
}
//****************************************************************************80

int i4block_components ( int l, int m, int n, int *a, int *c )

//****************************************************************************80
//
//  Purpose:
//
//    I4BLOCK_COMPONENTS assigns contiguous nonzero pixels to a common component.
//
//  Discussion:
//
//    On input, the A array contains values of 0 or 1.
//
//    The 0 pixels are to be ignored.  The 1 pixels are to be grouped
//    into connected components.
//
//    The pixel A(I,J,K) is "connected" to the pixels:
//
//      A(I-1,J,  K  ),  A(I+1,J,  K  ),
//      A(I,  J-1,K  ),  A(I,  J+1,K  ),
//      A(I,  J,  K-1),  A(I,  J,  K+1),
//
//    so most pixels have 6 neighbors.
//
//    On output, COMPONENT_NUM reports the number of components of nonzero
//    data, and the array C contains the component assignment for
//    each nonzero pixel, and is 0 for zero pixels.
//
//  Licensing:
//
//    This code is distributed under the GNU LGPL license.
//
//  Modified:
//
//    01 February 2012
//
//  Author:
//
//    John Burkardt
//
//  Parameters:
//
//    Input, int L, M, N, the order of the array.
//
//    Input, int A[L*M*N], the pixel array.
//
//    Output, int C[L*M*N], the component array.
//
//    Output, int I4BLOCK_COMPONENTS, the number of components
//    of nonzero data.
//
{
  int b;
  int c1;
 // int c2;
  int component;
  int component_num;
  int i;
  int j;
  int k;
  int north;
  int *p;
  int *q;
  int up;
  int west;
//
//  Initialization.
//
  for ( k = 0; k < n; k++ )
  {
    for ( j = 0; j < m; j++ )
    {
      for ( i = 0; i < l; i++ )
      {
        c[i+j*l+k*l*m] = 0;
      }
    }
  }
  component_num = 0;
//
//  P is simply used to store the component labels.  The dimension used
//  here is, of course, usually an absurd overestimate.
//
  p = new int[l*m*n+1];
  for ( i = 0; i <= l * m * n; i++ )
  {
    p[i] = i;
  }
//
//  "Read" the array one pixel at a time.  If a (nonzero) pixel has a north or
//  west neighbor with a label, the current pixel inherits it.
//  In case the labels disagree, we need to adjust the P array so we can
//  later deal with the fact that the two labels need to be merged.
//
  for ( i = 0; i < l; i++ )
  {
    for ( j = 0; j < m; j++ )
    {
      for ( k = 0; k < n; k++ )
      {
        if ( i == 0 )
        {
          north = 0;
        }
        else
        {
          north = c[i-1+j*l+k*l*m];
        }

        if ( j == 0 )
        {
          west = 0;
        }
        else
        {
          west = c[i+(j-1)*l+k*l*m];
        }

        if ( k == 0 )
        {
          up = 0;
        }
        else
        {
          up = c[i+j*l+(k-1)*l*m];
        }

        if ( a[i+j*l+k*l*m] != 0 )
        {
//
//  New component?
//
          if ( north == 0 && west == 0 && up == 0 )
          {
            component_num = component_num + 1;
            c[i+j*l+k*l*m] = component_num;
          }
//
//  One predecessor is labeled.
//
          else if ( north != 0 && west == 0 && up == 0 )
          {
            c[i+j*l+k*l*m] = north;
          }
          else if ( north == 0 && west != 0 && up == 0 )
          {
            c[i+j*l+k*l*m] = west;
          }
          else if ( north == 0 && west == 0 && up != 0 )
          {
            c[i+j*l+k*l*m] = up;
          }
//
//  Two predecessors are labeled.
//
          else if ( north == 0 && west != 0 && up != 0 )
          {
            c[i+j*l+k*l*m] = i4_min ( west, up );
            c1 = i4_min ( p[west], p[up] );
            p[west] = c1;
            p[up] = c1;
          }
          else if ( north != 0 && west == 0 && up != 0 )
          {
            c[i+j*l+k*l*m] = i4_min ( north, up );
            c1 = i4_min ( p[north], p[up] );
            p[north] = c1;
            p[up] = c1;
          }
          else if ( north != 0 && west != 0 && up == 0 )
          {
            c[i+j*l+k*l*m] = i4_min ( north, west );
            c1 = i4_min ( p[north], p[west] );
            p[north] = c1;
            p[west] = c1;
          }
//
//  Three predecessors are labeled.
//
          else if ( north != 0 && west != 0 && up != 0 )
          {
            c[i+j*l+k*l*m] = i4_min ( north, i4_min ( west, up ) );
            c1 = i4_min ( p[north], i4_min ( p[west], p[up] ) );
            p[north] = c1;
            p[west] = c1;
            p[up] = c1;
          }
        }
      }
    }
  }
//
//  When a component has multiple labels, have the higher labels
//  point to the lowest one.
//
  for ( component = component_num; 1 <= component; component-- )
  {
    b = component;
    while ( p[b] != b )
    {
      b = p[b];
    }
    p[component] = b;
  }
//
//  Locate the minimum label for each component.
//  Assign these mininum labels new consecutive indices.
//
  q = new int[component_num+1];

  for ( j = 0; j <= component_num; j++ )
  {
    q[j] = 0;
  }

  i = 0;
  for ( component = 1; component <= component_num; component++ )
  {
    if ( p[component] == component )
    {
      i = i + 1;
      q[component] = i;
    }
  }

  component_num = i;
//
//  Replace the labels by consecutive labels.
//
  for ( i = 0; i < l; i++ )
  {
    for ( j = 0; j < m; j++ )
    {
      for ( k = 0; k < n; k++ )
      {
        c[i+j*l+k*l*m] = q [ p [ c[i+j*l+k*l*m] ] ];
      }
    }
  }

  delete [] p;
  delete [] q;

  return component_num;
}
//****************************************************************************80

int i4mat_components ( int m, int n, int a[], int c[] )

//****************************************************************************80
//
//  Purpose:
//
//    I4MAT_COMPONENTS assigns contiguous nonzero pixels to a common component.
//
//  Discussion:
//
//    On input, the A array contains values of 0 or 1.
//
//    The 0 pixels are to be ignored.  The 1 pixels are to be grouped
//    into connected components.
//
//    The pixel A(I,J) is "connected" to the pixels A(I-1,J), A(I+1,J),
//    A(I,J-1) and A(I,J+1), so most pixels have 4 neighbors.
//
//    (Another choice would be to assume that a pixel was connected
//    to the other 8 pixels in the 3x3 block containing it.)
//
//    On output, COMPONENT_NUM reports the number of components of nonzero
//    data, and the array C contains the component assignment for
//    each nonzero pixel, and is 0 for zero pixels.
//
//  Picture:
//
//    Input A:
//
//      0  2  0  0 17  0  3
//      0  0  3  0  1  0  4
//      1  0  4  8  8  0  7
//      3  0  6 45  0  0  0
//      3 17  0  5  9  2  5
//
//    Output:
//
//      COMPONENT_NUM = 4
//
//      C:
//
//      0  1  0  0  2  0  3
//      0  0  2  0  2  0  3
//      4  0  2  2  2  0  3
//      4  0  2  2  0  0  0
//      4  4  0  2  2  2  2
//
//  Licensing:
//
//    This code is distributed under the GNU LGPL license.
//
//  Modified:
//
//    01 March 2011
//
//  Author:
//
//    John Burkardt
//
//  Parameters:
//
//    Input, int M, N, the order of the array.
//
//    Input, int A[M*N], the pixel array.
//
//    Output, int C[M*N], the component array.
//
//    Output, int I4MAT_COMPONENTS, the number of components
//    of nonzero data.
//
{
  int b;
  int component;
  int component_num;
  int i;
  int j;
  int north;
  int *p;
  int *q;
  int west;
//
//  Initialization.
//
  for ( j = 0; j < n; j++ )
  {
    for ( i = 0; i < m; i++ )
    {
      c[i+j*m] = 0;
    }
  }
  component_num = 0;
//
//  P is simply used to store the component labels.  The dimension used
//  here is, of course, usually an absurd overestimate.
//
  p = new int[m*n+1];

  for ( i = 0; i <= m * n; i++ )
  {
    p[i] = i;
  }
//
//  "Read" the array one pixel at a time.  If a (nonzero) pixel has a north or
//  west neighbor with a label, the current pixel inherits it.
//  In case the labels disagree, we need to adjust the P array so we can
//  later deal with the fact that the two labels need to be merged.
//
  for ( i = 0; i < m; i++ )
  {
    for ( j = 0; j < n; j++ )
    {
      if ( i == 0 )
      {
        north = 0;
      }
      else
      {
        north = c[i-1+j*m];
      }

      if ( j == 0 )
      {
        west = 0;
      }
      else
      {
        west = c[i+(j-1)*m];
      }
      if ( a[i+j*m] != 0 )
      {
        if ( north == 0 )
        {
          if ( west == 0 )
          {
            component_num = component_num + 1;
            c[i+j*m] = component_num;
          }
          else
          {
            c[i+j*m] = west;
          }
        }
        else if ( north != 0 )
        {
          if ( west == 0 || west == north )
          {
            c[i+j*m] = north;
          }
          else
          {
            c[i+j*m] = i4_min ( north, west );
            if ( north < west )
            {
              p[west] = north;
            }
            else
            {
              p[north] = west;
            }
          }
        }
      }
    }
  }
//
//  When a component has multiple labels, have the higher labels
//  point to the lowest one.
//
  for ( component = component_num; 1 <= component; component-- )
  {
    b = component;
    while ( p[b] != b )
    {
      b = p[b];
    }
    p[component] = b;
  }
//
//  Locate the minimum label for each component.
//  Assign these mininum labels new consecutive indices.
//
  q = new int[component_num+1];

  for ( j = 0; j <= component_num; j++ )
  {
    q[j] = 0;
  }

  i = 0;
  for ( component = 1; component <= component_num; component++ )
  {
    if ( p[component] == component )
    {
      i = i + 1;
      q[component] = i;
    }
  }

  component_num = i;
//
//  Replace the labels by consecutive labels.
//
  for ( j = 0; j < n; j++ )
  {
    for ( i = 0; i < m; i++ )
    {
      c[i+j*m] = q [ p [ c[i+j*m] ] ];
    }
  }

  delete [] p;
  delete [] q;

  return component_num;
}
//****************************************************************************80

int *i4mat_data_read ( string input_filename, int m, int n )

//****************************************************************************80
//
//  Purpose:
//
//    I4MAT_DATA_READ reads data from an I4MAT file.
//
//  Discussion:
//
//    An I4MAT is an array of I4's.
//
//    The file is assumed to contain one record per line.
//
//    Records beginning with '#' are comments, and are ignored.
//    Blank lines are also ignored.
//
//    Each line that is not ignored is assumed to contain exactly (or at least)
//    M real numbers, representing the coordinates of a point.
//
//    There are assumed to be exactly (or at least) N such records.
//
//  Licensing:
//
//    This code is distributed under the GNU LGPL license.
//
//  Modified:
//
//    23 February 2009
//
//  Author:
//
//    John Burkardt
//
//  Parameters:
//
//    Input, string INPUT_FILENAME, the name of the input file.
//
//    Input, int M, the number of spatial dimensions.
//
//    Input, int N, the number of points.  The program
//    will stop reading data once N values have been read.
//
//    Output, int I4MAT_DATA_READ[M*N], the data.
//
{
  bool error;
  ifstream input;
  int i;
  int j;
  string line;
  int *table;
  int *x;

  input.open ( input_filename.c_str ( ) );

  if ( !input )
  {
    cerr << "\n";
    cerr << "I4MAT_DATA_READ - Fatal error!\n";
    cerr << "  Could not open the input file: \"" << input_filename << "\"\n";
    exit ( 1 );
  }

  table = new int[m*n];

  x = new int[m];

  j = 0;

  while ( j < n )
  {
    getline ( input, line );

    if ( input.eof ( ) )
    {
      break;
    }

    if ( line[0] == '#' || s_len_trim ( line ) == 0 )
    {
      continue;
    }

    error = s_to_i4vec ( line, m, x );

    if ( error )
    {
      continue;
    }

    for ( i = 0; i < m; i++ )
    {
      table[i+j*m] = x[i];
    }
    j = j + 1;

  }

  input.close ( );

  delete [] x;

  return table;
}
//****************************************************************************80

void i4mat_header_read ( string input_filename, int *m, int *n )

//****************************************************************************80
//
//  Purpose:
//
//    I4MAT_HEADER_READ reads the header from an I4MAT file.
//
//  Discussion:
//
//    An I4MAT is an array of I4's.
//
//  Licensing:
//
//    This code is distributed under the GNU LGPL license.
//
//  Modified:
//
//    23 February 2009
//
//  Author:
//
//    John Burkardt
//
//  Parameters:
//
//    Input, string INPUT_FILENAME, the name of the input file.
//
//    Output, int *M, the number of spatial dimensions.
//
//    Output, int *N, the number of points
//
{
  *m = file_column_count ( input_filename );

  if ( *m <= 0 )
  {
    cerr << "\n";
    cerr << "I4MAT_HEADER_READ - Fatal error!\n";
    cerr << "  FILE_COLUMN_COUNT failed.\n";
    exit ( 1 );
  }

  *n = file_row_count ( input_filename );

  if ( *n <= 0 )
  {
    cerr << "\n";
    cerr << "I4MAT_HEADER_READ - Fatal error!\n";
    cerr << "  FILE_ROW_COUNT failed.\n";
    exit ( 1 );
  }

  return;
}
//****************************************************************************80

int i4vec_components ( int n, int a[], int c[] )

//****************************************************************************80
//
//  Purpose:
//
//    I4VEC_COMPONENTS assigns contiguous nonzero pixels to a common component.
//
//  Discussion:
//
//    This calculation is trivial compared to the 2D problem, and is included
//    primarily for comparison.
//
//    On input, the A array contains values of 0 or 1.
//
//    The 0 pixels are to be ignored.  The 1 pixels are to be grouped
//    into connected components.
//
//    The pixel A(I) is "connected" to the pixels A(I-1) and A(I+1).
//
//    On output, COMPONENT_NUM reports the number of components of nonzero
//    data, and the array C contains the component assignment for
//    each nonzero pixel, and is 0 for zero pixels.
//
//  Picture:
//
//    Input A:
//
//      0 0 1 2 4 0 0 4 0 0 0 8 9 9 1 2 3 0 0 5 0 1 6 0 0 0 4 0
//
//    Output:
//
//      COMPONENT_NUM = 6
//
//      C:
//
//      0 0 1 1 1 0 0 2 0 0 0 3 3 3 3 3 3 0 0 4 0 5 5 0 0 0 6 0
//
//  Licensing:
//
//    This code is distributed under the GNU LGPL license.
//
//  Modified:
//
//    01 March 2011
//
//  Author:
//
//    John Burkardt
//
//  Parameters:
//
//    Input, int N, the order of the vector.
//
//    Input, int A(N), the pixel array.
//
//    Output, int C[N], the component array.
//
//    Output, int I4VEC_COMPONENTS, the number of components
//    of nonzero data.
//
{
  int component_num;
  int j;
  int west;
//
//  Initialization.
//
  for ( j = 0; j < n; j++ )
  {
    c[j] = 0;
  }
  component_num = 0;
//
//  "Read" the array one pixel at a time.  If a (nonzero) pixel has a west
//  neighbor with a label, the current pixel inherits it.  Otherwise, we have
//  begun a new component.
//
  west = 0;

  for ( j = 0; j < n; j++ )
  {
    if ( a[j] != 0 )
    {
      if ( west == 0 )
      {
        component_num = component_num + 1;
      }
      c[j] = component_num;
    }
    west = c[j];
  }

  return component_num;
}
//****************************************************************************80

int s_len_trim ( string s )

//****************************************************************************80
//
//  Purpose:
//
//    S_LEN_TRIM returns the length of a string to the last nonblank.
//
//  Licensing:
//
//    This code is distributed under the GNU LGPL license.
//
//  Modified:
//
//    05 July 2009
//
//  Author:
//
//    John Burkardt
//
//  Parameters:
//
//    Input, string S, a string.
//
//    Output, int S_LEN_TRIM, the length of the string to the last nonblank.
//    If S_LEN_TRIM is 0, then the string is entirely blank.
//
{
  int n;

  n = s.length ( );

  while ( 0 < n )
  {
    if ( s[n-1] != ' ' )
    {
      return n;
    }
    n = n - 1;
  }

  return n;
}
//****************************************************************************80

int s_to_i4 ( string s, int *last, bool *error )

//****************************************************************************80
//
//  Purpose:
//
//    S_TO_I4 reads an I4 from a string.
//
//  Licensing:
//
//    This code is distributed under the GNU LGPL license.
//
//  Modified:
//
//    05 July 2009
//
//  Author:
//
//    John Burkardt
//
//  Parameters:
//
//    Input, string S, a string to be examined.
//
//    Output, int *LAST, the last character of S used to make IVAL.
//
//    Output, bool *ERROR is TRUE if an error occurred.
//
//    Output, int *S_TO_I4, the integer value read from the string.
//    If the string is blank, then IVAL will be returned 0.
//
{
  char c;
  int i;
  int isgn;
  int istate;
  int ival;

  *error = false;
  istate = 0;
  isgn = 1;
  i = 0;
  ival = 0;

  for ( ; ; )
  {
    c = s[i];
    i = i + 1;
//
//  Haven't read anything.
//
    if ( istate == 0 )
    {
      if ( c == ' ' )
      {
      }
      else if ( c == '-' )
      {
        istate = 1;
        isgn = -1;
      }
      else if ( c == '+' )
      {
        istate = 1;
        isgn = + 1;
      }
      else if ( '0' <= c && c <= '9' )
      {
        istate = 2;
        ival = c - '0';
      }
      else
      {
        *error = true;
        return ival;
      }
    }
//
//  Have read the sign, expecting digits.
//
    else if ( istate == 1 )
    {
      if ( c == ' ' )
      {
      }
      else if ( '0' <= c && c <= '9' )
      {
        istate = 2;
        ival = c - '0';
      }
      else
      {
        *error = true;
        return ival;
      }
    }
//
//  Have read at least one digit, expecting more.
//
    else if ( istate == 2 )
    {
      if ( '0' <= c && c <= '9' )
      {
        ival = 10 * (ival) + c - '0';
      }
      else
      {
        ival = isgn * ival;
        *last = i - 1;
        return ival;
      }

    }
  }
//
//  If we read all the characters in the string, see if we're OK.
//
  if ( istate == 2 )
  {
    ival = isgn * ival;
    *last = s_len_trim ( s );
  }
  else
  {
    *error = true;
    *last = 0;
  }

  return ival;
}
//****************************************************************************80

bool s_to_i4vec ( string s, int n, int ivec[] )

//****************************************************************************80
//
//  Purpose:
//
//    S_TO_I4VEC reads an I4VEC from a string.
//
//  Licensing:
//
//    This code is distributed under the GNU LGPL license.
//
//  Modified:
//
//    05 July 2009
//
//  Author:
//
//    John Burkardt
//
//  Parameters:
//
//    Input, string S, the string to be read.
//
//    Input, int N, the number of values expected.
//
//    Output, int IVEC[N], the values read from the string.
//
//    Output, bool S_TO_I4VEC, is TRUE if an error occurred.
//
{
  int begin;
  bool error;
  int i;
  int lchar;
  int length;

  begin = 0;
  length = s.length ( );
  error = 0;

  for ( i = 0; i < n; i++ )
  {
    ivec[i] = s_to_i4 ( s.substr(begin,length), &lchar, &error );

    if ( error )
    {
      return error;
    }
    begin = begin + lchar;
    length = length - lchar;
  }

  return error;
}
//****************************************************************************80

int s_word_count ( string s )

//****************************************************************************80
//
//  Purpose:
//
//    S_WORD_COUNT counts the number of "words" in a string.
//
//  Licensing:
//
//    This code is distributed under the GNU LGPL license.
//
//  Modified:
//
//    05 July 2009
//
//  Author:
//
//    John Burkardt
//
//  Parameters:
//
//    Input, string S, the string to be examined.
//
//    Output, int S_WORD_COUNT, the number of "words" in the string.
//    Words are presumed to be separated by one or more blanks.
//
{
  bool blank;
  int char_count;
  int i;
  int word_count;

  word_count = 0;
  blank = true;

  char_count = s.length ( );

  for ( i = 0; i < char_count; i++ )
  {
    if ( isspace ( s[i] ) )
    {
      blank = true;
    }
    else if ( blank )
    {
      word_count = word_count + 1;
      blank = false;
    }
  }

  return word_count;
}
//****************************************************************************80

void timestamp ( )

//****************************************************************************80
//
//  Purpose:
//
//    TIMESTAMP prints the current YMDHMS date as a time stamp.
//
//  Example:
//
//    31 May 2001 09:45:54 AM
//
//  Licensing:
//
//    This code is distributed under the GNU LGPL license.
//
//  Modified:
//
//    08 July 2009
//
//  Author:
//
//    John Burkardt
//
//  Parameters:
//
//    None
//
{
# define TIME_SIZE 40

  static char time_buffer[TIME_SIZE];
  const struct std::tm *tm_ptr;
  //size_t len;
  std::time_t now;

  now = std::time ( NULL );
  tm_ptr = std::localtime ( &now );

  //len = std::strftime ( time_buffer, TIME_SIZE, "%d %B %Y %I:%M:%S %p", tm_ptr );

  std::cout << time_buffer << "\n";

  return;
# undef TIME_SIZE
}


