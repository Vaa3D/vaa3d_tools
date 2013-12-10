static int skip_argument(char *argv[]);

static int rearrange_arguments(int argc, char *argv[])
{
  int start = -1;
  int end = -1;
  int status = 0;
  
  int nfile = 0;

  int i = 1;
  while (i < argc) {
    switch (status) {
    case 0:
      if (argv[i][0] == '-') {
	i += skip_argument(argv + i);
      } else {
	start = i;
	status = 1;
        i++;
        nfile++;
      }
      break;
    case 1:
      if (argv[i][0] != '-') {
	i++;
        nfile++;
      } else {
	end = i - 1;
	status = 2;
      }
      break;
    default:
      break;
    }
    
    if (status == 2) {
      break;
    }
  }

  if (start < 0) {
    TZ_ERROR(ERROR_DATA_VALUE);
  }

  //int nfile = end - start + 1;
  char *tmp_argv[nfile];
  
  if (end < 0) {
    end = start + nfile - 1;
  }

  if (end != argc - 1) {
    memcpy(tmp_argv, argv + start, sizeof(char*) * nfile);
    memmove(argv + start, argv + end + 1, sizeof(char*) * (argc - end - 1));
    memcpy(argv + argc - nfile, tmp_argv, sizeof(char*) * nfile);
  }

  return argc - nfile + 1;
}
