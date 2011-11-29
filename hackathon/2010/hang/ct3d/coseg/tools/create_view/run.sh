echo "==================== make ======================"
make
rm *.tree
echo "==================Create Tree===================="
./create_tree test.tif 0 100 1
echo "===================View Tree====================="
./view_tree *.tree
