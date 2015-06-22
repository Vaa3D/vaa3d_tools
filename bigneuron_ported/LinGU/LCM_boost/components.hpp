/*
 * components.hpp
 *
 *  Created on: May 20, 2015
 *      Author: gulin
 */

#ifndef COMPONENTS_HPP_
#define COMPONENTS_HPP_


int file_column_count ( std::string filename );
int file_row_count ( std::string input_filename );
int i4block_components ( int l, int m, int n, int a[], int c[] );
int i4_min ( int i1, int i2 );
int i4mat_components ( int m, int n, int a[], int c[] );
int *i4mat_data_read ( std::string input_filename, int m, int n );
void i4mat_header_read ( std::string input_filename, int *m, int *n );
int i4vec_components ( int n, int a[], int c[] );
int s_len_trim ( std::string s );
int s_to_i4 ( std::string s, int *last, bool *error );
bool s_to_i4vec ( std::string s, int n, int ivec[] );
int s_word_count ( std::string s );
void timestamp ( );


#endif /* COMPONENTS_HPP_ */
