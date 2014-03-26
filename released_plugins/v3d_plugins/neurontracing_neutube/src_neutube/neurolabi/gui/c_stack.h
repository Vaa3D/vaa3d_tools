#ifndef C_STACK_H
#define C_STACK_H

#include <string>
#include <vector>

#include "tz_image_lib_defs.h"
#include "tz_mc_stack.h"
#include "tz_cuboid_i.h"

//! Functions for C-compatible stack
/*!
 * Two types of stack structure are used:
 *   Stack: single-channel stack
 *   Mc_Stack: Multiple-channel stack
 */

namespace C_Stack {
//General
//! Coordinate to index
/*!
 * Get the array index of the coordinates (\a x, \a y, \a z) in the stack with
 * the size (\a width, \a height, \a depth).
 *
 * \return Array index if the coordinates are in the stack; -1 if the
 *         coordinates are out of range.
*/
ssize_t offset(int x, int y, int z, int width, int height, int depth);

//Functions for Stack

//Stack deallocator, mainly used for constructing a ZStack object
typedef void Stack_Deallocator(Stack*);

/*!
 * \brief Create a stack
 * \param kind Voxel type of the stack.
 * \param width Width of the stack.
 * \param height Height of the stack.
 * \param depth Number of slices of the stack.
 * \return A pointer to the stack. The user is reponsible for freeing it by
 *         calling the kill().
 * \sa kill()
 */
Stack *make(int kind, int width, int height, int depth);

//Kill from memory pool
/*!
 * \brief Kill a stack (free up all associated memory).
 * \param stack The stack object to kill. It must be created by make().
 * \sa make()
 */
void kill(Stack *stack);

//Free all associated memory
void systemKill(Stack *stack);

//Delete the stack by 'delete' in C++
inline void cppDelete(Stack *stack) { delete stack; }

/** @name Make copies
 */
///@{
//Clone a stack
/*!
 * \brief Clone a stack.
 * \param stack The object to be cloned.
 * \return The cloned object. The user is responsible for freeing it.
 */
Stack* clone(const Stack *stack);

//Copy is essentially the same as <clone>. But <clone> is preferred.
//Stack* copy(const Stack *stack);
void copyValue(const Stack *src, Stack *dst);
void copyPlaneValue(Stack *stack, void *array, int slice);
///@}

//Attributes of a stack
inline int width(const Stack *stack) { return stack->width; }
inline int height(const Stack *stack) { return stack->height; }
inline int depth(const Stack *stack) { return stack->depth; }
inline int kind(const Stack *stack) { return stack->kind; }

inline void setWidth(Stack *stack, int width) { stack->width = width; }
inline void setHeight(Stack *stack, int height) { stack->height = height; }
inline void setDepth(Stack *stack, int depth) { stack->depth = depth; }
inline void setKind(Stack *stack, int kind) { stack->kind = kind; }
void setAttribute(Stack *stack, int kind, int width, int height, int depth);
inline size_t planeByteNumber(const Stack *stack) {
  return static_cast<size_t>(stack->kind) * stack->width * stack->height;
}
inline size_t area(const Stack *stack) {
  return (size_t) width(stack) * height(stack);
}

size_t voxelNumber(const Stack *stack);

//Voxel access
inline uint8_t* array8(const Stack *stack) { return (uint8_t*) stack->array; }
double value(const Stack *stack, size_t index);
double value(const Stack *stack, int x, int y, int z, int c);
void setPixel(Stack *stack, int x, int y, int z, int c, double v);
void setZero(Stack *stack);
Stack sliceView(const Stack *stack, int slice);
Stack* channelExtraction(const Stack *stack, int channel);

//Stack manipulation
//Crop a stack
Stack* crop(const Stack *stack,int left,int top,int front,
            int width,int height,int depth, Stack *desstack);

Stack* crop(const Stack *stack, const Cuboid_I &box, Stack *desstack);

//Crop a stack using its bound box
Stack* boundCrop(const Stack *stack, int margin = 0);

Stack* resize(const Stack* stack,int width,int height,int depth);
Stack* translate(Stack *stack, int kind, int in_place);


void print(const Stack *stack);

//Stack statistics
double min(const Stack *stack);
double max(const Stack *stack);
double sum(const Stack *stack);
int* hist(const Stack *stack);

//Miscellanea
size_t closestForegroundPixel(const Stack *stack, double x, double y, double z);

//Functions for Mc_Stack
typedef void Mc_Stack_Deallocator(Mc_Stack*);
Mc_Stack *make(int kind, int width, int height, int depth, int channelNumber);

Mc_Stack* clone(const Mc_Stack *stack);

inline int width(const Mc_Stack *stack) { return stack->width; }
inline int height(const Mc_Stack *stack) { return stack->height; }
inline int depth(const Mc_Stack *stack) { return stack->depth; }
inline int kind(const Mc_Stack *stack) { return stack->kind; }
inline int channelNumber(const Mc_Stack *stack) { return stack->nchannel; }

inline uint8_t* array8(const Mc_Stack *stack) {
  return (uint8_t*) stack->array; }

inline size_t voxelNumber(const Mc_Stack *stack) {
  return (size_t) width(stack) * height(stack) * depth(stack);
}

inline size_t planeByteNumber(const Mc_Stack *stack) {
  return static_cast<size_t>(stack->kind) * stack->width * stack->height;
}

inline size_t volumeByteNumber(const Mc_Stack *stack) {
  return planeByteNumber(stack) * depth(stack);
}

inline size_t elementNumber(const Mc_Stack *stack) {
  return ((size_t) width(stack)) * height(stack) * depth(stack) *
      channelNumber(stack);
}

void setAttribute(Mc_Stack *stack, int kind, int width, int height, int depth,
                  int channel);

void setChannelNumber(Mc_Stack *stack, int nchannel);
inline void cppDelete(Mc_Stack *stack) { delete stack; }
void systemKill(Mc_Stack *stack);

void kill(Mc_Stack *stack);

void copyPlaneValue(Mc_Stack *stack, void *array, int channel, int slice);
void copyChannelValue(Mc_Stack *mc_stack, int chan, const Stack *stack);

bool hasSameValue(Mc_Stack *stack, size_t index1, size_t index2);
bool hasSameValue(Mc_Stack *stack, size_t index1, size_t index2,
                  size_t channelOffset);

bool hasValue(Mc_Stack *stack, size_t index, uint8_t *buffer);
bool hasValue(Mc_Stack *stack, size_t index,
              uint8_t *buffer, size_t channelOffset);

void view(const Stack *src, Mc_Stack *dst);
void view(const Mc_Stack *src, Stack *dst, int channel);

Mc_Stack* translate(const Mc_Stack *stack, int targetKind);

int neighborTest(int conn, int width, int height, int depth, size_t index,
                 int *isInBound);
int neighborTest(int conn, int width, int height, int depth,
                 int x, int y, int z, int *isInBound);

Stack* extractChannel(const Stack *stack, int c);

void setStackValue(Stack *stack, const std::vector<size_t> &indexArray,
                   double value);

std::vector<size_t> getNeighborIndices(
    const Stack *stack, const std::vector<size_t> &indexArray,
    int conn, double value);


double min(const Mc_Stack *stack);
double max(const Mc_Stack *stack);

ssize_t indexFromCoord(int x, int y, int z, int width, int height, int depth);
void indexToCoord(size_t index, int width, int height, int *x, int *y, int *z);

void write(const std::string &filePath, const Stack *stack);
void write(const std::string &filePath, const Mc_Stack *stack);
Mc_Stack* read(const std::string &filePath, int channel = -1);
Stack* readSc(const std::string &filePath);

Mc_Stack* resize(const Mc_Stack *stack, int width, int height, int depth);

//Paint routines
void drawPatch(Stack *canvas, const Stack *patch,
               int dx, int dy, int dz, int transparentValue);

int digitWidth(int n);
int integerWidth(int n, int interval);
int drawDigit(Stack *canvas, int n, int dx, int dy, int dz);
void drawInteger(Stack *canvas, int n, int dx, int dy, int dz, int interval = 10);
}

#endif // C_STACK_H
