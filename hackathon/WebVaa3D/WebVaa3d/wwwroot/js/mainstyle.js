function full_screen() {//图像全屏展示
  
    
    var docElm = document.documentElement;
    //W3C 
    if (docElm.requestFullscreen) {
        docElm.requestFullscreen();
    }
    //FireFox 
    else if (docElm.mozRequestFullScreen) {
        docElm.mozRequestFullScreen();
    }
    //Chrome等 
    else if (docElm.webkitRequestFullScreen) {
        docElm.webkitRequestFullScreen();
    }
    //IE11
    else if (elem.msRequestFullscreen) {
        elem.msRequestFullscreen();
    }
    //退出全屏
    if (document.exitFullscreen) {
        document.exitFullscreen();
    }
    else if (document.mozCancelFullScreen) {
        document.mozCancelFullScreen();
    }
    else if (document.webkitCancelFullScreen) {
        document.webkitCancelFullScreen();
    }
    else if (document.msExitFullscreen) {
        document.msExitFullscreen();
    }

    document.addEventListener("fullscreenchange", function () {
        fullscreenState.innerHTML = (document.fullscreen) ? "" : "not ";
    }, false);

    document.addEventListener("mozfullscreenchange", function () {
        fullscreenState.innerHTML = (document.mozFullScreen) ? "" : "not ";
    }, false);

    document.addEventListener("webkitfullscreenchange", function () {
        fullscreenState.innerHTML = (document.webkitIsFullScreen) ? "" : "not ";
    }, false);
    document.addEventListener("msfullscreenchange", function () {
        fullscreenState.innerHTML = (document.msFullscreenElement) ? "" : "not ";
    }, false);
 
  

}