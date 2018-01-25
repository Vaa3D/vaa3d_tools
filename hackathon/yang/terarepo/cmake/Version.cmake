set(TeraStitcher_VERSION_MAJOR 1)
set(TeraStitcher_VERSION_MINOR 10)
set(TeraStitcher_VERSION_PATCH 8)
set(TeraStitcher_VERSION
  "${TeraStitcher_VERSION_MAJOR}.${TeraStitcher_VERSION_MINOR}.${TeraStitcher_VERSION_PATCH}")
add_definitions( -DTERASTITCHER_MAJOR=${TeraStitcher_VERSION_MAJOR} )
add_definitions( -DTERASTITCHER_MINOR=${TeraStitcher_VERSION_MINOR} )
add_definitions( -DTERASTITCHER_PATCH=${TeraStitcher_VERSION_PATCH} )

set(TeraStitcher2_VERSION_MAJOR 2)
set(TeraStitcher2_VERSION_MINOR 0)
set(TeraStitcher2_VERSION_PATCH 5)
set(TeraStitcher2_VERSION
  "${TeraStitcher2_VERSION_MAJOR}.${TeraStitcher2_VERSION_MINOR}.${TeraStitcher2_VERSION_PATCH}")
add_definitions( -DTERASTITCHER2_MAJOR=${TeraStitcher2_VERSION_MAJOR} )
add_definitions( -DTERASTITCHER2_MINOR=${TeraStitcher2_VERSION_MINOR} )
add_definitions( -DTERASTITCHER2_PATCH=${TeraStitcher2_VERSION_PATCH} )

set(TeraConverter_VERSION_MAJOR 3)
set(TeraConverter_VERSION_MINOR 1)
set(TeraConverter_VERSION_PATCH 15)
set(TeraConverter_VERSION
  "${TeraConverter_VERSION_MAJOR}.${TeraConverter_VERSION_MINOR}.${TeraConverter_VERSION_PATCH}")
add_definitions( -DTERACONVERTER_MAJOR=${TeraConverter_VERSION_MAJOR} )
add_definitions( -DTERACONVERTER_MINOR=${TeraConverter_VERSION_MINOR} )
add_definitions( -DTERACONVERTER_PATCH=${TeraConverter_VERSION_PATCH} )