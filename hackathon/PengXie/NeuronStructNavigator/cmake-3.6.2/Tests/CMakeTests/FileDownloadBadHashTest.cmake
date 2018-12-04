set(url "file:///local1/vaa3d_test/vaa3d_tools/hackathon/MK/NeuronStructNavigator/cmake-3.6.2/Tests/CMakeTests/FileDownloadInput.png")
set(dir "/local1/vaa3d_test/vaa3d_tools/hackathon/MK/NeuronStructNavigator/cmake-3.6.2/Tests/CMakeTests/downloads")

file(DOWNLOAD
  ${url}
  ${dir}/file3.png
  TIMEOUT 2
  STATUS status
  EXPECTED_HASH SHA1=5555555555555555555555555555555555555555
  )
