#ifndef ZSINGLECHANNELSTACK_H
#define ZSINGLECHANNELSTACK_H

#include "tz_image_lib_defs.h"
#include "c_stack.h"

class ZStack_Projection;
class ZStack_Stat;

class ZSingleChannelStack
{
public:
  ZSingleChannelStack();
  //  ZSingleChannelStack(int kind, int width, int height, int depth,
  //                      bool isVirtual = false);
  //  ZSingleChannelStack(Stack *stack, C_Stack::Stack_Decllocator *delloc);
  //  ZSingleChannelStack(const ZSingleChannelStack &src);
  virtual ~ZSingleChannelStack();

public:
  inline int width() const { return m_stack->width; }
  inline int height() const { return m_stack->height; }
  inline int depth() const { return m_stack->depth; }
  inline int kind() const { return m_stack->kind; }

  //Enclosed Stack-type data.
  inline Stack* data() const { return m_stack; }

  enum EComponent {
    STACK, STACK_PROJ, STACK_STAT
  };

  void deprecateDependent(EComponent component);
  void deprecate(EComponent component);
  bool isDeprecated(EComponent component) const;

  //release ownership if it has, otherwise return false
  //bool releaseOwnership();
  // subtract most common value of the histogram from this stack, use Stack_Sub_Common
  void subMostCommonValue();

  /* A stack is virtual if its data array is null */
  bool isVirtual() const { return m_stack->array == NULL; }     //m_stack should never be NULL;

  size_t voxelNumber();
  size_t dataByteCount();

  //Minimal value of the stack.
  double min() const;
  //Maximum value of the stack.
  double max() const;
  //Voxel value at a given position
  double value(int x, int y, int z) const;
  double value(size_t index) const;

  inline uint8_t value8(size_t index) const {
    return *((uint8*)(m_stack->array) + index);
  }

  ZStack_Stat* getStat() const;
  ZStack_Projection* getProj();

  void setValue(int x, int y, int z, double v);
  void setValue(size_t index, double value);

  //Maximum voxel value along a z-parallel line passing (<x>, <y>).
  int maxIntensityDepth(int x, int y) const;

  inline uint8_t* array8() { return m_data.array8; }
  inline uint16_t* array16() { return m_data.array16; }
  inline float* array32() { return m_data.array32; }
  inline double* array64() { return m_data.array64; }
  inline color_t* arrayc() { return m_data.arrayc; }

  //inline void incrStamp() { m_stamp++; }

  // shift image based on offset
  void shiftLocation(int *offset, int width = -1, int height = -1, int depth = -1);

  bool hasSameValue(size_t index1, size_t index2);

public: /* data operation */
  //Clean all associated memory
  void clean();
  //Set stack data
  void setData(Stack *stack,
               C_Stack::Stack_Deallocator *delloc = C_Stack::kill);

public: /* operations */
  enum Proj_Mode {
    MAX_PROJ
  };

  enum Stack_Axis {
    X_AXIS,
    Y_AXIS,
    Z_AXIS
  };

  void *projection(Proj_Mode mode, Stack_Axis axis = Z_AXIS);

  void bcAdjustHint(double *scale, double *offset);
  bool isBinary();

  //ZSingleChannelStack* clone();

public: /* processing routines */
  bool binarize(int threshold = 0);
  bool bwsolid();
  bool enhanceLine(double sigmaX, double sigmaY, double sigmaZ);
  bool enhanceLine();
  bool watershed();

private:
  void init();
  void copyData(const Stack *stack);

private:
  Stack *m_stack;
  C_Stack::Stack_Deallocator *m_delloc;
  //bool m_isOwner;
  //int m_stamp;
  ZStack_Projection* m_proj;
  mutable ZStack_Stat *m_stat;
  Image_Array m_data;
};

class ZStack_Projection {
public:
  ZStack_Projection(Stack *parent = NULL) { m_parent = parent; m_proj = NULL;}
  ~ZStack_Projection() {if (m_proj != NULL) { Kill_Image(m_proj); }}

  void update(Stack *stack);
  //void update(Stack *stack, int stamp);
  inline void* data() { return (void*)m_proj->array; }

private:
  Stack *m_parent;
  //int m_stamp;
  Image *m_proj;
};

class ZStack_Stat {
public:
  ZStack_Stat(Stack *parent = NULL) { m_parent = parent; m_hist = NULL; }
  ~ZStack_Stat() { if (m_hist != NULL) { free(m_hist); } }

  void update(Stack *stack);
  //void update(Stack *stack, int stamp);
  inline int* hist() { return m_hist; }

  Stack *m_parent;
  //int m_stamp;
  int *m_hist; /* set to NULL if not available */
  double m_min;
  double m_max;
  double m_greyScale;
  double m_greyOffset;
};


#endif // ZSINGLECHANNELSTACK_H
