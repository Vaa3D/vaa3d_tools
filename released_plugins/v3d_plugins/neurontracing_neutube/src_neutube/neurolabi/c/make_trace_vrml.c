#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <utilities.h>
#include "tz_error.h"
#include "tz_constant.h"
#include "tz_neurochain.h"
#include "tz_vrml_io.h"
#include "tz_darray.h"
#include "tz_locne_chain.h"
#include "tz_stack_draw.h"
#include "tz_vrml_material.h"

INIT_EXCEPTION_MAIN(e)

void normalize_neurochain_pos(Locseg_Chain *chain, double z_scale)
{
  Local_Neuroseg *locseg = NULL;
  Locseg_Chain_Iterator_Start(chain, DL_HEAD);
  while ((locseg = Locseg_Chain_Next_Seg(chain)) != NULL) {
    Local_Neuroseg_Scale_Z(locseg, z_scale);
    locseg->seg.r1 *= locseg->seg.scale;
    locseg->seg.scale = 1.0;
  }
}

int main(int argc, char* argv[]) {
  static char *Spec[] = {
    "<neuron:string> [-z <double>] [-unit <string> -minlen <double>]", 
    "[-c] [-b <int> <int> <int>]",
    NULL};
  
  Process_Arguments(argc, argv, Spec, 1);
  
  char *neuron_name = Get_String_Arg("neuron");

  char file_path[100];
  sprintf(file_path, "../data/%s", neuron_name);

  int chain_number;
  /* load chains from the directory */
  Locseg_Chain *chain_array = Dir_Locseg_Chain_N(file_path, "^chain.*\\.tb",
						 &chain_number, NULL);
  printf("%d chaines loaded.\n", chain_number);

  sprintf(file_path, "../data/%s/all.wrl", neuron_name);
  FILE *fp = fopen(file_path, "w");

  Vrml_Head_Fprint(fp, "V2.0", "utf8");

  double z_scale = 1.0;
  sprintf(file_path, "../data/%s.res", neuron_name);

  if (Is_Arg_Matched("-z")) {
    z_scale = Get_Double_Arg("-z");
  } else {
    if (fexist(file_path)) {
      double res[3];
      int length;

      //darray_read2(file_path, res, &length);
      FILE *fp = fopen(file_path, "r");
      darray_fscanf(fp, res, 3);
      z_scale = res[0] / res[2] * 2.0;
      fclose(fp);
    }
  }

  printf("%g\n", z_scale);
  
  Vrml_Material *material = New_Vrml_Material();
  Vrml_SFVec3f_Set(material->diffuse_color, 0.0, 0.8, 0.0);

  int i;
  double min_length = 25.0;
  if (Is_Arg_Matched("-minlen")) {
    min_length = Get_Double_Arg("-minlen");
  }

  int total_length = 0;

  Rgb_Color color;
  //double h_step = 6.0 / chain_number;
  double hue = 0.0;

  int chain_length;

  sprintf(file_path, "../data/%s/length.csv", neuron_name);
  FILE *length_fp = fopen(file_path, "w");
  char delim = '\0';

  Bitmask_Set_Bit(DIFFUSE_COLOR, FALSE, &(material->default_mask));

  int valid_chain_number = 0;
  int max_length = 0;
  double total_geolen = 0.0;

  for (i = 0; i < chain_number; i++) {
    if (Is_Arg_Matched("-c")) {
      hue = ((double) random() / 0x7FFFFFFF) * 6.0;
      Set_Color_Hsv(&color, hue, 1.0, 1.0);
      //hue += h_step;
      Vrml_SFVec3f_Set(material->diffuse_color, (double)color.r / 255.0,
		       (double)color.g / 255.0, (double)color.b / 255.0);
    }
     
    chain_length = Locseg_Chain_Length(chain_array + i);
    if (delim != '\0') {
      fprintf(length_fp, "%c", delim);
    }
    fprintf(length_fp, "%d", chain_length);
    delim = ',';
    if (chain_length > max_length) {
      max_length = chain_length;
    }

    double length = Locseg_Chain_Geolen(chain_array + i);
    if (Is_Arg_Matched("-unit")) {
      if (strcmp(Get_String_Arg("-unit"), "n") == 0) {
	length = Locseg_Chain_Length(chain_array + i);
      }
    }
    
    if (length >= min_length) {
      total_length += chain_length;
      total_geolen += Locseg_Chain_Geolen(chain_array + i);
      valid_chain_number++;
      if (z_scale != 1.0) {
	normalize_neurochain_pos(chain_array + i, z_scale);
      }
      Locseg_Chain_Vrml_Fprint(fp, chain_array + i, material, 0);
      //Neurochain_Fprint_Vrml(chain_array[i], fp);
    }
    
    Clean_Locseg_Chain(chain_array + i);
  }
  
  printf("Number of valid chains: %d\n", valid_chain_number);
  printf("Number of segments: %d (max: %d)\n", total_length, max_length);
  printf("Total length: %g\n", total_geolen);
  fclose(length_fp);
  fclose(fp);

  sprintf(file_path, "../data/%s/allsoma.wrl", neuron_name);
  fp = fopen(file_path, "w");

  Vrml_Head_Fprint(fp, "V2.0", "utf8");

  int indent = 0;
  if (z_scale != 1.0) {
    Vrml_Node_Begin_Fprint(fp, "Transform", 0);
    SFVec3f scale;
    Vrml_SFVec3f_Set(scale, 1.0, 1.0, 1 / z_scale);
    Vrml_SFVec3f_Field_Fprintln(fp, "scale", scale, 2);
    indent += 2;
  }

  fprintf(fp, "children [\n");

  i = 0;
  sprintf(file_path, "../data/%s/soma%d.bn", neuron_name, i);

  

  while (fexist(file_path)) {
    Locne_Chain *soma = Read_Locne_Chain(file_path);
    Locne_Chain_Vrml_Fprint(fp, soma, NULL, indent);
    Kill_Locne_Chain(soma);
    i++;
    sprintf(file_path, "../data/%s/soma%d.bn", neuron_name, i);
  }

  Delete_Vrml_Material(material);

  fprintf(fp, "]\n");
  Vrml_Node_End_Fprint(fp, "Transform", 0);

  fclose(fp);

  free(chain_array);
  return 0;
}
