function cleanUpOverall(in,row,col)
global state
state.imageProc.cell.currentImage{1}(:,:,in)=...
	cleanupImage(state.imageProc.cell.currentImage{1}(:,:,in),row,col);