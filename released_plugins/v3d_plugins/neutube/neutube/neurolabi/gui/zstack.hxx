/**@file zstack.h
 * @brief Stack class
 * @author Ting Zhao
 */
#ifndef _ZSTACK_HXX_
#define _ZSTACK_HXX_

#include <vector>
#include <string>

#ifdef _NEUTUBE_
#include "zglmutils.h"
#include "znumericparameter.h"
#endif

#include "tz_image_lib_defs.h"
#include "c_stack.h"
#include "tz_stack_document.h"
#include "zsinglechannelstack.h"
#include "zstackfile.h"
#include "tz_image_io.h"
#include "zresolution.h"
#include "zpoint.h"


//! Stack class
/*!
 *It supports multi-channel stacks. But most operations are on the first channel
 *by default. Internally, the class hosts voxel data in a 1-D array, which is
 *aligned by the order of (width, height, depth (number of slices), channel).
 *Therefore, voxels from the same row are stored contigously in an array. The
 *value or intensity of a voxel can have one of the following types:
 *  GREY: 8-bit unsigned integer
 *  GREY16: 16-bit unsigned integer
 *  FLOAT32: 32-bit float
 *  FLOAT64: 64-bit float
 *  COLOR: 24-bit color
 *Note that the COLOR type does not specify multi-channel data. It means the
 *single voxel has 3 contiguous bytes, which can be interpreted as a RGB color.
*/

class ZStack {
public:
  //! Default constructor
  /*! Default constructor: create an empty stack. */
  ZStack();

  //! Constructor
  /*!
   Construct a stack with essential attributes
   \param kind Type of the stack. It can be one of the following values:
               GREY, GREY16, FLOAT32, FLOAT64, COLOR
   \param width Width of the stack.
   \param height Height of the stack.
   \param depth Number of slices of the stack.
   \param nchannel Channel number of the stack.
   \param isVirtual Create a virtual stack if it is true. A virtual stack does
                    not contain any pixel data.
  */
  ZStack(int kind, int width, int height, int depth, int nchannel,
         bool isVirtual = false);

  //! Constructor
  /*!
   Construct a stack with stack data.
   \param stack Stack data
   \param dealloc The function applied on \a stack while the object is
                  deconstructed.
   */
  ZStack(Mc_Stack *stack,
         C_Stack::Mc_Stack_Deallocator *dealloc = C_Stack::kill);

  //! Destructor
  virtual ~ZStack();

public: /* attributes */
  //! Types of specifying subset of a stack
  enum EStackUnit {
    SINGLE_VOXEL, /*!< A single voxel of a stack*/
    SINGLE_PLANE, /*!< A single plane of a stack */
    SINGLE_ROW, /*!< A single row of a stack */
    SINGLE_CHANNEL, /*!< A single channel of a stack */
    WHOLE_DATA /*!< All data of a stack */
  };

  //! Get the C-compatible data
  /*!
   The returned pointer is owned by ZStack.
   \sa setData()
   */
  inline Mc_Stack* mc_stack() { return m_stack; }
  inline const Mc_Stack* mc_stack() const { return m_stack; }
  inline Mc_Stack* data() { return mc_stack(); }
  inline const Mc_Stack* data() const { return mc_stack(); }

  //! Set stack data
  void setData(Mc_Stack *stack,
               C_Stack::Mc_Stack_Deallocator *delloc = C_Stack::kill);

  //! Get the C-compatible data
  /*!
   *The returned pointer is owned by ZStack.
   */
  const Stack* c_stack(int c = 0) const;
  Stack* c_stack(int c = 0);
  ZSingleChannelStack* singleChannelStack(int c = 0);
  const ZSingleChannelStack* singleChannelStack(int c = 0) const;

  //! Width of the stack.
  inline int width() const { return C_Stack::width(mc_stack()); }
  //! Height of the stack.
  inline int height() const { return C_Stack::height(mc_stack()); }
  //! Number of slices of the stack.
  inline int depth() const { return C_Stack::depth(mc_stack()); }
  //! Voxel type of the stack.
  inline int kind() const { return C_Stack::kind(mc_stack()); }
  //! Channel number of the stack.
  inline int channelNumber() const {
    return C_Stack::channelNumber(m_stack);
  }

  //! Component of the stack
  enum EComponent {
    MC_STACK, SINGLE_CHANNEL_VIEW
  };

  //! Get the total byte number of the stack or its subset
  size_t getByteNumber(EStackUnit unit = WHOLE_DATA) const;

  //! Get the total voxel number of the stack or its subset
  size_t getVoxelNumber(EStackUnit unit = WHOLE_DATA) const;

  //! Voxel value at a given position and channel.
  double value(int x, int y, int z, int c = 0) const;

  //! Get voxel value given an idex
  double value(size_t index, int c = 0) const;

  //! Set voxel value at a given position and channel.
  void setValue(int x, int y, int z, int c, double v);

  /** @name raw data access
   *  array8(), array16(), array32(), array64() or arrayc() can be used to otain
   *  the raw data array of the stack. The choice of the function depends on the
   *  voxel type.
   */
  ///@{
  /** Array for 8-bit unsigned integer. */
  inline uint8_t* array8(int c = 0) {
    return const_cast<uint8_t*>(
          const_cast<const ZStack*>(this)->array8(c)); }

  inline const uint8_t* array8(int c = 0) const {
    return (uint8_t*) rawChannelData(c);
  }

  /** Array for 16-bit unsigned integer. */
  inline uint16_t* array16(int c = 0) {
    return const_cast<uint16_t*>(
          const_cast<const ZStack*>(this)->array16(c)); }

  inline const uint16_t* array16(int c = 0) const{
    return (uint16_t*) rawChannelData(c);
  }

  /** Array for single precision float. */
  inline float* array32(int c = 0) {
    return const_cast<float*>(
          const_cast<const ZStack*>(this)->array32(c)); }

  inline const float* array32(int c = 0) const {
    return (float*) rawChannelData(c);
  }

  /** Array for double precision float. */
  inline double* array64(int c = 0) {
    return const_cast<double*>(
          const_cast<const ZStack*>(this)->array64(c)); }

  inline const double* array64(int c = 0) const {
    return (double*) rawChannelData(c);
  }

  /** Array for RGB color. */
  inline color_t* arrayc(int c = 0) {
    return const_cast<color_t*>(
          const_cast<const ZStack*>(this)->arrayc(c)); }

  inline const color_t* arrayc(int c = 0) const {
    return (color_t*) rawChannelData(c);
  }
  ///@}

  inline uint8_t value8(size_t index) const {
    return ((uint8_t*) rawChannelData())[index];
  }
  inline uint8_t value8(size_t index, int c) const {
    return ((uint8_t*) rawChannelData(c))[index];
  }

  void deprecateDependent(EComponent component);
  void deprecateSingleChannelView(int channel);
  void deprecate(EComponent component);
  bool isDeprecated(EComponent component) const;
  bool isSingleChannelViewDeprecated(int channel) const;

  //shift one channel
  void shiftLocation(int *offset, int c = 0, int width = -1, int height = -1, int depth = -1);

  // make mc_stack
  static Mc_Stack* makeMcStack(const Stack *stack1, const Stack *stack2, const Stack *stack3);
  // subtract most common value of the histogram from this stack, use Stack_Sub_Common
  void subMostCommonValue(int c);
  // get average of all channels
  Stack* averageOfAllChannels();

  //Source of the stack. Usually it is the file where the image is originally read
  //from.
  const char* sourcePath() const;
  //inline Stack_Document* source() const { return m_source; }

  //Preferred z scale is the preferred scale ratio between z-axis and xy-plane
  //for anisotropic operations
  inline double preferredZScale() const { return m_preferredZScale; }

  /* A stack is virtual if its data array is null */
  bool isVirtual() const;

  // return voxelnumber or dataByteCount of all channels. Obsolete
  size_t voxelNumber() const;
  size_t dataByteCount() const;

  // return voxelnumber of dataByteCount of 1 channel. Obsolete
  size_t voxelNumber(int ch) const;
  size_t dataByteCount(int ch) const;

  //Minimal value of the stack.
  double min();
  double min(int c) const;
  //Maximum value of the stack.
  double max();
  double max(int c) const;

  double saturatedIntensity() const;

  int autoThreshold(int ch = 0) const;

  std::vector<double> color(size_t index) const;
  std::vector<double> color(int x, int y, int z) const;

  bool equalColor(size_t index, const std::vector<double> &co) const;
  bool equalColor(size_t index, const std::vector<uint8_t> &co) const;
  bool equalColor(size_t index, const uint8_t *co, size_t length) const;
  bool equalColor(size_t index, size_t channelOffset,
                  const uint8_t *co, size_t length) const;


  void setValue(size_t index, int c, double value);

  //Maximum voxel value along a z-parallel line passing (<x>, <y>).
  int maxIntensityDepth(int x, int y, int c = 0) const;

  //If the stack can be thresholded.
  bool isThresholdable();

  //If the stack is tracable
  bool isTracable();

  //If the source of the stack is an swc
  bool isSwc();



  void *getDataPointer(int c, int slice) const;

  ZStack* clone() const;

  ZStackFile* source() { return &m_source; }

public: /* data operation */
  //Clean all associated memory except the source
  void clean();
  void cleanChannel(int c = 0);   //remove content of this channel
  void removeChannel(int c = 0); //remove channel
  //Load stack from Stack, split channel if necessary
  bool load(Stack *stack, bool isOwner = true);

  //Load stack from a file
  bool load(const std::string &filepath);
  bool loadImageSequence(const char *filePath);

  bool importJsonFile(const std::string &filePath);

  //Load stack from several single channel stack, stack can be null
  bool load(const Stack *ch1, const Stack *ch2, const Stack *ch3);

  // return output file name, some image format might not support some data, so the real file name might be changed
  std::string save(const std::string &filepath) const;
  void setSource(const std::string &filepath, int channel = -1);
  void setSource(const ZStackFile &file);
  void setSource(Stack_Document *stackDoc);
  void setResolution(double x, double y, double z, char unit);

  // get number of channel for lsm,tiff,raw
  static int getChannelNumber(const std::string &filepath);

  inline const void *rawChannelData() const { return m_stack->array; }
  inline void* rawChannelData() {
    return const_cast<void*>(static_cast<const ZStack&>(*this).rawChannelData());
  }

  const void *rawChannelData(int c) const;
  void* rawChannelData(int c);
  //Stack* channelData(int c);

public: /* operations */

  void* projection(ZSingleChannelStack::Proj_Mode mode,
                   ZSingleChannelStack::Stack_Axis axis = ZSingleChannelStack::Z_AXIS,
                   int c = 0);

  void bcAdjustHint(double *scale, double *offset, int c = 0);
  bool isBinary();
  bool updateFromSource();

  bool hasSameValue(size_t index1, size_t index2);
  bool hasSameValue(size_t index1, size_t index2, size_t channelOffset);

  ZStack* createSubstack(const std::vector<std::vector<double> > &selected);

  double averageIntensity(ZStack *mask);

  void copyValue(const void *buffer, size_t length, int ch = 0);
  void copyValue(const void *buffer, size_t length, void *loc);

  void setOffset(double dx, double dy, double dz);
  inline ZPoint getOffset() { return m_offset; }

public: /* processing routines */
  bool binarize(int threshold = 0);
  bool bwsolid();
  bool enhanceLine();
  void extractChannel(int c);
  Stack* copyChannel(int c);
  bool watershed(int c = 0);
  inline const ZResolution& resolution() const { return m_resolution; }

private:
  ZStack(const ZStack &src); //uncopyable

  void init();
  bool canMerge(const Stack *s1, const Stack *s2);
  void setChannelNumber(int nchannel);

public:
  void initChannelColors();

  // read lsm file, fill Cz_Lsminfo, Lsm_Channel_Colors and channel names and colors
#ifdef _NEUTUBE_
  std::vector<ZVec3Parameter*>& channelColors() { initChannelColors(); return m_channelColors; }
  glm::vec3 getChannelColor(size_t ch) { initChannelColors(); return m_channelColors[ch]->get(); }
  bool getLSMInfo(const QString &filepath);
  void logLSMInfo();
  void setChannelColor(int ch, double r, double g, double b);
#endif

private:
  Mc_Stack *m_stack; //Master data
  C_Stack::Mc_Stack_Deallocator *m_delloc; //Dellocator of the master data
  //ZStack_Projection *m_proj;
  //ZStack_Stat *m_stat;
  ZStackFile m_source;
  double m_preferredZScale;
  ZResolution m_resolution;
  ZPoint m_offset;
  mutable std::vector<Stack> m_stackView;
  mutable std::vector<ZSingleChannelStack*> m_singleChannelStack;
  mutable char m_buffer[1]; //Buffer of text field of temporary stack

  //float color for each channel

  bool m_isLSMFile;

#ifdef _NEUTUBE_
  std::vector<ZVec3Parameter*> m_channelColors;
  Cz_Lsminfo m_lsmInfo;
  Lsm_Channel_Colors m_lsmChannelInfo;
  Lsm_Time_Stamp_Info m_lsmTimeStampInfo;
  std::vector<QString> m_lsmChannelNames;
  std::vector<double> m_lsmTimeStamps;
  std::vector<int> m_lsmChannelDataTypes;
#endif
};


#endif
