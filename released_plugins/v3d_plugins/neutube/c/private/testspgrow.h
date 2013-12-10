/**@file testspgrow.h
 * @author Ting Zhao
 * @date 30-Dec-2009
 */

#ifndef _TESTSPGROW_H_
#define _TESTSPGROW_H_

static int golden_test()
{
  const char *stack_file = "../data/test/spgrow/diadem_e1.tif";
  if (fexist(stack_file)) {
    Stack *stack = Read_Stack((char*) stack_file);

    Stack *mask = Make_Stack(GREY, Stack_Width(stack), Stack_Height(stack),
	Stack_Depth(stack));
    Zero_Stack(mask);

    Locseg_Chain *chain1 = 
      Read_Locseg_Chain("../data/test/spgrow/diadem_e1/chain24.tb");
    Locseg_Chain *chain2 = 
      Read_Locseg_Chain("../data/test/spgrow/diadem_e1/chain35.tb");
    Locseg_Chain *chain3 = 
      Read_Locseg_Chain("../data/test/spgrow/diadem_e1/chain0.tb");
    Locseg_Chain *chain4 = 
      Read_Locseg_Chain("../data/test/spgrow/diadem_e1/chain14.tb");

    Locseg_Chain_Label_G(chain1, mask, 1.0, 0, -1, 1.0, 0.0, 0, 2);
    Locseg_Chain_Label_G(chain2, mask, 1.0, 0, -1, 1.0, 0.0, 0, 1);
    Locseg_Chain_Label_G(chain3, mask, 1.0, 0, -1, 1.0, 0.0, 0, 4);
    Locseg_Chain_Label_G(chain4, mask, 1.0, 0, -1, 1.0, 0.0, 0, 4);

    Sp_Grow_Workspace *sgw = New_Sp_Grow_Workspace();
    sgw->size = Stack_Voxel_Number(stack);
    sgw->resolution[0] = 0.3296;
    sgw->resolution[1] = 0.3296;
    sgw->resolution[2] = 1.0;
    Sp_Grow_Workspace_Set_Mask(sgw, mask->array);
    sgw->wf = Stack_Voxel_Weight_S;

    double inner = Locseg_Chain_Average_Score(chain1, stack, 1.0, 
	STACK_FIT_MEAN_SIGNAL);
    double outer = Locseg_Chain_Average_Score(chain1, stack, 1.0, 
	STACK_FIT_OUTER_SIGNAL);
    sgw->argv[3] = inner * 0.1 + outer * 0.9;
    sgw->argv[4] = (inner - outer) / 4.6 * 1.8;

    Int_Arraylist *path = Stack_Sp_Grow(stack, NULL, 0, NULL, 0, sgw);

    Stack *canvas = Translate_Stack(stack, COLOR, 1);

    Stack_Sp_Grow_Draw_Result(canvas, path, sgw);
    Locseg_Chain_Label(chain3, canvas, 1.0);
    Locseg_Chain_Label(chain4, canvas, 1.0);

    Write_Stack("../data/test/spgrow/tmp/diadem_e1_result.tif", canvas);   

    Kill_Locseg_Chain(chain1);
    Kill_Locseg_Chain(chain2);
    Kill_Locseg_Chain(chain3);
    Kill_Locseg_Chain(chain4);
    Kill_Stack(mask);
    Kill_Sp_Grow_Workspace(sgw);
    Kill_Int_Arraylist(path);
    Kill_Stack(canvas);

    if (fcmp("../data/test/spgrow/tmp/diadem_e1_result.tif",
	  "../data/test/spgrow/golden/diadem_e1_result.tif") != 0) {
      PRINT_EXCEPTION("Bug?", "Conflict with golden standard.");
      return 1;   
    }
  } else {
    printf("%s cannot be found\n", stack_file);
  }

  return 0;
}

#endif
