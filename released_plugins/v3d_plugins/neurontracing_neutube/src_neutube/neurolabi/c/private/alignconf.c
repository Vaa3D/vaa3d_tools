static void extract_file_id(const char *file_path, char *id)
{
  int start = strlen(file_path) - 1;
  int length = 0;
  
  while (file_path[start--] != '.');
 
  while (file_path[start--] != '_') {
    length++;
  }

  start += 2;

  strncpy(id, file_path + start, length);
  id[length] = '\0';
}

static void align_id(const char *file1, const char *file2, char *id) 
{
  char id2[100];
  extract_file_id(file1, id);
  extract_file_id(file2, id2);
  strcat(id, "_");
  strcat(id, id2);
}

static void write_align_result(const char *result_dir,
			       const char *file1, const char *file2, 
			       const int offset[])
{
  char result_file[150];
  char id[100];
  align_id(file1, file2, id);
  strcat(id, ".txt");

  fullpath(result_dir, id, result_file);

  FILE *fp = fopen(result_file, "w");
  if (offset != NULL) {
    fprintf(fp, "%s %s %d %d %d", file1, file2, offset[0], offset[1], offset[2]);
  } else {
    fprintf(fp, "failed");
  }
  fclose(fp);
}
