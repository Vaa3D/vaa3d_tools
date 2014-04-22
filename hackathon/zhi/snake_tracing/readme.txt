1. unzip VXL.zip

2. Run CMake to install VXL
Turn the following options OFF:
BUILD_BRL
BUILD_CONVERSIONS
BUILD_EXAMPLES
BUILD_GEL
BUILD_OUL
BUILD_OXL
BUILD_PRIP
BUILD_TBL
BUILD_TESTING

Turn the following options ON:
BUILD_RPL_RGTL
VNL_CONFIG_LEGACY_METHODS
VXL_FORCE_V3P_PNG (Mac ONLY)

run configure and then turn the following option ON:
BUILD_RPL_RTVL

3. unzip ITK4.3.2.zip

4. run CMake to install ITK
Turn OFF the following:
BUILD_TESTING
BUILD_EXAMPLES

Configure again.
Turn ON the following:
ITKGroup_* or ITK_BUILD_ALL_MODULES
ITK_USE_64BITS_IDS
ITK_USE_REVIEW
ITK_USE_SYSTEM_VXL

On the next Configure you will have to specify VXL_DIR.