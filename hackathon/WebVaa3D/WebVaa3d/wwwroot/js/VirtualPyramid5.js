const axios = window.axios;
//var canvas = document.createElement('canvas');
//var context = canvas.getContext('webgl2');
renderer = new THREE.WebGLRenderer({ alpha: false });
//camera = new THREE.PerspectiveCamera();
//scene = new THREE.Scene();
mouse = new THREE.Vector2();
clock = new THREE.Clock();
bbox = new THREE.BoxGeometry(537, 853, 153);
level = 0;

scene = [];
tiffwidth = 0;
tiffheight = 0;

rgba = new Uint8Array();
for (var i = 0; i <= 5; i++) {
  scene[i] = new THREE.Scene;
}

//console.log(scene);
camera = [];
for (var i = 0; i <= 5; i++) {
  camera[i] = new THREE.PerspectiveCamera();
}

controls = new THREE.OrbitControls(camera[level]);//创建控件对象



var spheres = [];
var spheresIndex = 0;



function init() {
    width = document.getElementById('view').clientWidth;
    height = document.getElementById('view').clientHeight;
    renderer.setSize(width, height);

    for (var i = 0; i <= 5; i++) {

        camera[i] = new THREE.PerspectiveCamera(45, width / height, 1, 10000);
        
    }

    document.getElementById("view").appendChild(renderer.domElement);

    controls = new THREE.OrbitControls(camera[0], document.getElementById('view'));//创建控件对象
    controls.addEventListener('change', render);//监听鼠标、键盘事件
    controls.target.set(269, 427, 77);

    camera[0].position.x = 431;
    camera[0].position.y = 494;
    camera[0].position.z = 1245;

    //renderer.setClearColor(0x000000, 1.0);
    renderer.setClearColor(0x000000, 1.0);
    //renderer.setPixelRatio(window.devicePixelRatio);
    document.getElementById("view").appendChild(renderer.domElement);

    var axesHelper = new THREE.AxesHelper(1000);
    scene[0].add(axesHelper);
    render();

}

function render() {
    //requestAnimationFrame(render);
    renderer.render(scene[level], camera[level]);

}
//console.log(scene.children);
//scene.children.material.blending = THREE["CustomBlending"];
//scene.children.material.blendEquation = THREE.MaxEquation;
render();


async function getPictures() {
    init();
    var id = 0;
    const res = await axios.get(`/api/tiff/${+id}`);
    var files = res.data;
    for (index in files) {
        const file = files[index];
        const response = await axios.get(`/api/tiff/${+id}/${file}`, { responseType: "arraybuffer" });

        var fileName = $("#file").val();
        console.log(file);
        var regex = /\d+/g;
        var filestr = file.match(regex);  
        var ifds = UTIF.decode(response.data);
        UTIF.decodeImages(response.data, ifds);   
     
        //var rgba = new Uint8Array(tiff.readRGBAImage());
        //rgba = tiff.readRGBAImage();
        //console.log(rgba);

        if (index == 0) {
            rgba = new Uint8Array(ifds[0].width * ifds[0].height * ifds.length * 4 * 12);

          
        }


        var j;
        for (j = 0; j < ifds.length; j++) {

            buffer = UTIF.toRGBA8(ifds[j]);
 
            var k; var contrast = 12; var thresh = 0;
            for (k = 0; k < buffer.byteLength; k = k + 4) {
                if (buffer[k] < thresh) { buffer[k + 3] = 0; }
                buffer[k] *= contrast;
                buffer[k + 1] *= contrast;
                buffer[k + 2] *= contrast;
               
            }
            //console.log(buffer);
            //console.log(buffer.slice(0,  tiff.width() * 4));
            for (x = 0; x < ifds[j].height; x++) {
                rgba.set(buffer.slice(x * ifds[j].width * 4, (x + 1) * ifds[j].width * 4), j * ifds[j].width * ifds[j].height * 4 * 12 + (Math.round(filestr[0] / 136960) * ifds[j].height + x) * 3 * ifds[j].width * 4 + Math.round(filestr[1] / 114560) * ifds[j].width * 4);


            }
            //console.log(rgba);
            //console.log(rgba, buffer);
            // rgba = mergeTypedArraysUnsafe(rgba, buffer);
            buffer = null;
        }
    
        tiffwidth = ifds[0].width;
        tiffheight = ifds[0].height;
       

    }

    console.log(tiffwidth, tiffheight, ifds.length);
 


    console.log(rgba);

    rgba2 = rearrangedata(rgba, ifds.length, tiffheight * 4, tiffwidth * 3, 2);
    //console.log(rgba2);
    rgba3 = rearrangedata(rgba, ifds.length, tiffwidth * 3, tiffheight * 4, 3);
    renderimage(rgba, tiffwidth * 3, tiffheight * 4, ifds.length, scene[0], 1);
    renderimage(rgba2, ifds.length, tiffheight * 4, tiffwidth * 3, scene[0], 2);
    renderimage(rgba3, ifds.length, tiffwidth * 3, tiffheight * 4, scene[0], 3);

    console.log(rgba,rgba2,rgba3);
    ////var texture2 = new THREE.TextureLoader().load( rgba );


    render();







    var sphereGeometry = new THREE.SphereBufferGeometry(0.1, 32, 32);
    var sphereMaterial = new THREE.MeshBasicMaterial({ color: 0xff0000 });
    for (var i = 0; i < 40; i++) {
        var sphere = new THREE.Mesh(sphereGeometry, sphereMaterial);
        scene[0].add(sphere);
        spheres.push(sphere);
    }

    var edges = new THREE.EdgesGeometry(bbox);
    var line = new THREE.LineSegments(edges, new THREE.LineBasicMaterial({ color: 0xffffff }));
    line.position.set(269, 427, 77);
    scene[0].add(line);




    var raycaster = new THREE.Raycaster();
    var mouse = new THREE.Vector2();



    tiff_width = [179, 215, 239, 253, 253];
    tiff_height = [213, 244, 244, 253, 253];
    tiff_depth = [153, 154, 205, 246, 247];
    unit_width = [114560, 68800, 38240, 20240, 10120];
    unit_height = [136960, 78080, 39040, 20240, 10120];
    unit_depth = [98560, 49280, 32800, 19680, 9840];

   


}

function mergeTypedArraysUnsafe(a, b) {
    var c = new a.constructor(a.length + b.length);
    console.log(c);
    c.set(a);
    c.set(b, a.length);

    return c;
}

function VF(event) {
    width = document.getElementById('view').clientWidth;
    height = document.getElementById('view').clientHeight;
    event.preventDefault();
    mouse.x = (event.clientX / width) * 2 - 1;
    mouse.y = - (event.clientY / height) * 2 + 1;

    controls.removeEventListener('change', render);
    points = new Array();
    var raycaster = new THREE.Raycaster();//光线投射器

    raycaster.setFromCamera(mouse, camera);//鼠标的二维坐标xy -1,1之间，射线起点设置为相机处
    var intersects = raycaster.intersectObjects(scene.children);//检查是否和射线相交的一组对象
    var intensity = new Array();
    for (var i = 0; i < intersects.length; i++) {

        intensity[i] = intersects[i].object.material.map.image.data[(parseInt(intersects[i].point.y % tiff_width[level]) - 1) * tiff_height[level] + parseInt(intersects[i].point.x % tiff_height[level])];

    }
    //console.log(intensity);
    var indexOfMaxintensity = intensity.reduce((iMax, x, i, arr) => x > arr[iMax] ? i : iMax, 0);
    //console.log(intersects[indexOfMaxintensity].point);
    //console.log(intersects[0].point);


    if (intersects !== null) {
        spheres[spheresIndex].position.copy(intersects[indexOfMaxintensity].point);
        spheres[spheresIndex].scale.set(1, 1, 1);
        spheresIndex = (spheresIndex + 1) % spheres.length;

    }
    //for (var i = 0; i < spheres.length; i++) {
    //    var sphere = spheres[i];
    //    //sphere.scale.multiplyScalar(0.98);
    //    //sphere.scale.clampScalar(0.01, 1);
    //}

    render();
    //console.log(spheres);





}
function onkeydown(event) {
    switch (event.keyCode) {
        case 68: draw(); break;
        case 27: document.removeEventListener('mousemove', VF, false); controls.addEventListener('change', render); break;
        case 66: goback(); break;



    }
}

window.addEventListener('keydown', onkeydown, false);
function draw() {
    //window.addEventListener('dblclick', window.addEventListener('mousemove', VF, false), false);
    document.addEventListener('mousemove', VF, false);


    //window.addEventListener('mouseup', quitVF, false);
    //function VF(event) {

    //controls.removeEventListener('change', render);
    //points = new Array();
    //var raycaster = new THREE.Raycaster();

    //raycaster.setFromCamera(mouse, camera);
    //var intersects = raycaster.intersectObjects(scene.children);
    //console.log(intersects[0].point);


    //}

    //function quitVF(event) {
    //    window.removeEventListener('mousedown', window.addEventListener('mousemove', VF, false), false);
    //    window.removeEventListener('mousemove', VF, false);
    //    controls.addEventListener('change', render);




    //}

}

function goback() {
    while (scene[level].children.length > 0) { scene[level].remove(scene[level].children[0]); }
    level = level - 1;
    render();


}